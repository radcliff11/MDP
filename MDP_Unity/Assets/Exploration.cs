using System;
using System.Threading;
using System.Timers;
using UnityEngine;
using static Arena;
using static Algo;
using System.Collections.Generic;

public class Exploration {
    private static System.Timers.Timer endTimer;
    static bool _testing;
    static bool _usingRightHandWall = true;
    static bool _underSixMinutes = true;
    public static bool _hasNewSensorData = false;

    static GridStatus[,] _testBoard;
    static Mutex _bufferMutex = new Mutex();
    public static string _sensorBuffer;
    public static Instruction calibrateInstru = Instruction.Stop;
    public static int STEPS_EVERY_CALIBRATE = 3;
    static int rightWallCounter = STEPS_EVERY_CALIBRATE;
    public static int overallCalibrateCounter = STEPS_EVERY_CALIBRATE * 2;

    static int _lastExploredCount = 0;
    static int _currExploredCount;
    static int countRHWSteps = 0;
    public static int totalSteps = 0;
    static Pos[] lastPoses = new Pos[8];
    static Direction[] lastDirs = new Direction[8];
    static List<Instruction> outgoingInstructions = new List<Instruction>();



    #region helper methods 	





    static int CountExploredGrids() {
        int explored = 0;
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 20; j++) {
                if (grids[i, j].gs != GridStatus.UNEXPLORED) {
                    explored++;
                }
            }
        }
        return explored;
    }

    static void CheckSwitchToFastestPath() {
        // check 8 grid loop
        if (lastPoses[totalSteps % 8].x == currentPos.x &&
            lastPoses[totalSteps % 8].y == currentPos.y) {
            if (lastDirs[totalSteps % 8] == currentDir) {
                _usingRightHandWall = false;
                return;
            }
        }
        lastPoses[totalSteps % 8] = currentPos;
        lastDirs[totalSteps % 8] = currentDir;

        // check larger loop
        _currExploredCount = CountExploredGrids();

        if (_currExploredCount - _lastExploredCount < 3) {
            countRHWSteps++;
            if (countRHWSteps >= 24) {
                _usingRightHandWall = false;
                countRHWSteps = 0;
            }
        } else {
            _lastExploredCount = _currExploredCount;
            countRHWSteps = 0;
        }
        Debug.Log("Count RHW Steps = " + countRHWSteps +
            ", last explored = " + _lastExploredCount +
            ", curr explored = " + _currExploredCount);
    }

    static bool FringeSafeToExplore(int i, int j) {
        try {
            if (grids[i, j].gs != GridStatus.EMPTY) {
                return false;
            }
        } catch { return false; }
        for (int a = -1; a < 2; a++) {
            for (int b = -1; b < 2; b++) {
                try {
                    if (grids[i + a, j + b].gs != GridStatus.EMPTY &&
                        grids[i + a, j + b].gs != GridStatus.VIRTUAL_WALL) {
                        return false;
                    }
                } catch { return false; }
            }
        }
        return true;
    }

    static Pos NextFringeToVisit() {

        for (int j = 19; j >= 0; j--) {
            for (int i = 0; i < 15; i++) {
                if (grids[i, j].gs == GridStatus.UNEXPLORED) {
                    if (FringeSafeToExplore(i + 1, j - 2)) {
                        return new Pos(i + 1, j - 2);
                    } else if (FringeSafeToExplore(i + 2, j - 1)) {
                        return new Pos(i + 2, j - 1);
                    }
                }
            }
        }
        Debug.Log("Can't find empty grid! Exploration finished");
        return new Pos(-1, -1); // as null value
    }

    static string SimulateSensorData(int checkX, int checkY, Direction dir) {
        int data = 1;

        for (int i = 0; i < 3; i++) {
            try {
                if (_testBoard[checkX, checkY] == GridStatus.EMPTY) {
                    data += 1;
                } else {
                    break;
                }
            } catch { break; }
            checkX += DirectionToVector(dir).x;
            checkY += DirectionToVector(dir).y;
        }

        if (data > 3) { data = -1; } // -1 is empty, other possible values are 1,2,3

        return (data.ToString());
    }

    static string UpdateSensorData() {
        string data = "";
        int checkX;
        int checkY;

        int curX = currentPos.x;
        int curY = currentPos.y;

        checkX = curX + 2 * DirectionToVector(currentDir).x + DirectionToVector(LeftDirection()).x;
        checkY = curY + 2 * DirectionToVector(currentDir).y + DirectionToVector(LeftDirection()).y;
        data += SimulateSensorData(checkX, checkY, currentDir);
        data += ",";

        checkX = curX + 2 * DirectionToVector(currentDir).x;
        checkY = curY + 2 * DirectionToVector(currentDir).y;
        data += SimulateSensorData(checkX, checkY, currentDir);
        data += ",";

        checkX = curX + 2 * DirectionToVector(currentDir).x + DirectionToVector(RightDirection()).x;
        checkY = curY + 2 * DirectionToVector(currentDir).y + DirectionToVector(RightDirection()).y;
        data += SimulateSensorData(checkX, checkY, currentDir);
        data += ",";

        checkX = curX + DirectionToVector(currentDir).x + 2 * DirectionToVector(LeftDirection()).x;
        checkY = curY + DirectionToVector(currentDir).y + 2 * DirectionToVector(LeftDirection()).y;
        data += SimulateSensorData(checkX, checkY, LeftDirection());
        data += ",";

        checkX = curX + DirectionToVector(currentDir).x + 2 * DirectionToVector(RightDirection()).x;
        checkY = curY + DirectionToVector(currentDir).y + 2 * DirectionToVector(RightDirection()).y;
        data += SimulateSensorData(checkX, checkY, RightDirection());

        return data;
    }
    public static Direction LeftDirection() {
        switch (currentDir) {
            case Direction.NORTH:
                return Direction.WEST;
            case Direction.WEST:
                return Direction.SOUTH;
            case Direction.SOUTH:
                return Direction.EAST;
            case Direction.EAST:
                return Direction.NORTH;
            default:
                return currentDir;
        }
    }

    public static Direction RightDirection() {
        switch (currentDir) {
            case Direction.NORTH:
                return Direction.EAST;
            case Direction.WEST:
                return Direction.NORTH;
            case Direction.SOUTH:
                return Direction.WEST;
            case Direction.EAST:
                return Direction.SOUTH;
            default:
                return currentDir;
        }
    }

    public static Direction OppositeDirection() {
        switch (currentDir) {
            case Direction.NORTH:
                return Direction.SOUTH;
            case Direction.WEST:
                return Direction.EAST;
            case Direction.SOUTH:
                return Direction.NORTH;
            case Direction.EAST:
                return Direction.WEST;
            default:
                return currentDir;
        }
    }

    static bool BlockFree(int x, int y) {
        try {
            if (grids[x, y].gs == GridStatus.EMPTY || grids[x, y].gs == GridStatus.VIRTUAL_WALL) { return true; }
        } catch { }
        return false;
    }
    static bool BlockNotWall(int x, int y) {
        try {
            if (grids[x, y].gs == GridStatus.EMPTY || grids[x, y].gs == GridStatus.VIRTUAL_WALL || grids[x, y].gs == GridStatus.UNEXPLORED) { return true; }
        } catch { }
        return false;
    }

    static bool BlockUnexplored(int x, int y) {
        try {
            if (grids[x, y].gs == GridStatus.UNEXPLORED) { return true; }
        } catch { }
        return false;
    }

    // Check if grid is empty or unexplored
    static bool RowNotWall(Direction dir, int offset) {
        bool flag = false;

        int newX = currentPos.x + offset * DirectionToVector(dir).x;
        int newY = currentPos.y + offset * DirectionToVector(dir).y;

        if (BlockNotWall(newX, newY)) {
            switch (dir) {
                case Direction.NORTH:
                case Direction.SOUTH:
                    flag = BlockNotWall(newX + DirectionToVector(Direction.EAST).x, newY + DirectionToVector(Direction.EAST).y)
                        && BlockNotWall(newX + DirectionToVector(Direction.WEST).x, newY + DirectionToVector(Direction.WEST).y);
                    break;
                case Direction.EAST:
                case Direction.WEST:
                    flag = BlockNotWall(newX + DirectionToVector(Direction.NORTH).x, newY + DirectionToVector(Direction.NORTH).y)
                        && BlockNotWall(newX + DirectionToVector(Direction.SOUTH).x, newY + DirectionToVector(Direction.SOUTH).y);
                    break;
            }
        }
        return flag;
    }

    static bool RoomCheck(Direction dir) {
        int newX, newY, count, numOfOpenings;
        numOfOpenings = 0;
        count = 2;
        bool prevFlag = false;
        bool currFlag = true;
        bool isRoom = true;

        // Final value of count is the number of grids to check
        while (currFlag) {
            currFlag = RowNotWall(dir, count);
            count++;
        }
        count -= 1;

        // Check left side of 3x3; Not room if any grid is unexplored or if there're 3 grids in a row that the robot can go through
        for (int i = 0; i < count; i++) {
            newX = currentPos.x + i * DirectionToVector(dir).x;
            newY = currentPos.y + i * DirectionToVector(dir).y;

            switch (dir) {
                case Direction.NORTH:
                case Direction.SOUTH:
                    currFlag = BlockNotWall(newX + 2 * DirectionToVector(Direction.WEST).x, newY + 2 * DirectionToVector(Direction.WEST).y);
                    if (BlockUnexplored(newX + 2 * DirectionToVector(Direction.WEST).x, newY + 2 * DirectionToVector(Direction.WEST).y))
                        return false;
                    break;

                case Direction.EAST:
                case Direction.WEST:
                    currFlag = BlockNotWall(newX + 2 * DirectionToVector(Direction.SOUTH).x, newY + 2 * DirectionToVector(Direction.SOUTH).y);
                    if (BlockUnexplored(newX + 2 * DirectionToVector(Direction.SOUTH).x, newY + 2 * DirectionToVector(Direction.SOUTH).y))
                        return false;
                    break;
            }

            if (prevFlag && currFlag) {
                numOfOpenings += 1;
                if (numOfOpenings == 2)
                    isRoom = false;
            } else {
                numOfOpenings = 0;
            }
            prevFlag = currFlag;
        }

        numOfOpenings = 0;
        prevFlag = false;
        currFlag = false;

        // Check right side of 3x3; Not room if any grid is unexplored or if there're 3 grids in a row that the robot can go through
        // same to previous lines, just the direction inside currFlag is different
        for (int i = 0; i < count; i++) {
            newX = currentPos.x + i * DirectionToVector(dir).x;
            newY = currentPos.y + i * DirectionToVector(dir).y;

            switch (dir) {
                case Direction.NORTH:
                case Direction.SOUTH:
                    currFlag = BlockNotWall(newX + 2 * DirectionToVector(Direction.EAST).x, newY + 2 * DirectionToVector(Direction.EAST).y);
                    if (BlockUnexplored(newX + 2 * DirectionToVector(Direction.EAST).x, newY + 2 * DirectionToVector(Direction.EAST).y))
                        return false;
                    break;
                case Direction.EAST:
                case Direction.WEST:
                    currFlag = BlockNotWall(newX + 2 * DirectionToVector(Direction.NORTH).x, newY + 2 * DirectionToVector(Direction.NORTH).y);
                    if (BlockUnexplored(newX + 2 * DirectionToVector(Direction.NORTH).x, newY + 2 * DirectionToVector(Direction.NORTH).y))
                        return false;
                    break;
            }

            if (prevFlag && currFlag) {
                numOfOpenings += 1;
                if (numOfOpenings == 2)
                    isRoom = false;
            } else {
                numOfOpenings = 0;
            }
            prevFlag = currFlag;
        }
        return isRoom;
    }

    public static bool CalibratedFrontWall() {
        //return false; // TODO: hardcoded to remove front calibrate
        try {
            if (BlockFree(
                currentPos.x + 2 * DirectionToVector(currentDir).x + DirectionToVector(LeftDirection()).x,
                currentPos.y + 2 * DirectionToVector(currentDir).y + DirectionToVector(LeftDirection()).y) ||
                BlockFree(
                currentPos.x + 2 * DirectionToVector(currentDir).x,
                currentPos.y + 2 * DirectionToVector(currentDir).y) ||
                BlockFree(
                currentPos.x + 2 * DirectionToVector(currentDir).x + DirectionToVector(RightDirection()).x,
                currentPos.y + 2 * DirectionToVector(currentDir).y + DirectionToVector(RightDirection()).y)) {
                return false;
            }
        } catch { } // it's a wall!
        return true;
    }
    static bool CanCalibrateByRightTurn() {
        try {
            if (BlockFree(
                    currentPos.x + 2 * DirectionToVector(RightDirection()).x + DirectionToVector(currentDir).x,
                    currentPos.y + 2 * DirectionToVector(RightDirection()).y + DirectionToVector(currentDir).y) ||
                BlockFree(
                    currentPos.x + 2 * DirectionToVector(RightDirection()).x,
                    currentPos.y + 2 * DirectionToVector(RightDirection()).y) ||
                BlockFree(
                    currentPos.x + 2 * DirectionToVector(RightDirection()).x + DirectionToVector(OppositeDirection()).x,
                    currentPos.y + 2 * DirectionToVector(RightDirection()).y + DirectionToVector(OppositeDirection()).y)) {
                return false;
            }
        } catch { } // it's a wall!
        return true;
    }


    // static void ExitLoop(object source, ElapsedEventArgs e) {
    //     _usingRightHandWall = false;
    // }

    static bool ExploredLimitFulfilled(int limit) {
        int explored = 0;

        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 20; j++) {
                if (grids[i, j].gs != GridStatus.UNEXPLORED) {
                    explored++;
                }
            }
        }

        return explored >= limit;
    }

    public static void SnapPicture() {
        int checkX = currentPos.x + 2 * DirectionToVector(currentDir).x;
        int checkY = currentPos.y + 2 * DirectionToVector(currentDir).y;
        string msg = "";
        try {
            if (grids[checkX, checkY].gs == GridStatus.WALL) {
                if (checkX <= 9) {
                    msg = msg + "0" + checkX.ToString();
                } else {
                    msg += checkX.ToString();
                }

                if (checkY <= 9) {
                    msg = msg + "0" + checkY.ToString();
                } else {
                    msg += checkY.ToString();
                }
                SendPictureCoordinates(msg);
            }
        } catch { }
    }

    //static bool HasWall(Direction dir, Pos currPos) {
    //    int newX = currPos.x + 2 * DirectionToVector(dir).x;
    //    int newY = currPos.y + 2 * DirectionToVector(dir).y;
    //    if (BlockFree(newX, newY)) {
    //        switch (dir) {
    //            case Direction.NORTH:
    //            case Direction.SOUTH:
    //                if ((!BlockFree(newX + DirectionToVector(Direction.EAST).x, newY + DirectionToVector(Direction.EAST).y))
    //                    || (!BlockFree(newX + DirectionToVector(Direction.WEST).x, newY + DirectionToVector(Direction.WEST).y))) {
    //                    return true;
    //                }
    //                break;
    //            case Direction.EAST:
    //            case Direction.WEST:
    //                if ((!BlockFree(newX + DirectionToVector(Direction.NORTH).x, newY + DirectionToVector(Direction.NORTH).y))
    //                    || (!BlockFree(newX + DirectionToVector(Direction.SOUTH).x, newY + DirectionToVector(Direction.SOUTH).y))) {
    //                    return true;
    //                }
    //                break;
    //        }
    //    } else { return true; }
    //    return false;
    //}

    //static void MoveToNearestWall() {
    //    Direction closestDir;
    //    int leastSteps = 50;

    //    foreach (Direction dir in directionsForExplore) {

    //        Pos tempPos = currentPos;

    //        for (int i = 0; i < 20; i++) {
    //            // Loops till we find a position next to an obstacle or wall
    //            if (HasWall(dir, tempPos)) {
    //                break;
    //            }

    //            tempPos = new Pos(currentPos.x + DirectionToVector(dir).x, currentPos.y + DirectionToVector(dir).y);

    //            if (i < leastSteps) {
    //                leastSteps = i;
    //                closestDir = dir;
    //            }
    //        }

    //        for (int i = 0; i < leastSteps; i++) {
    //            MoveByNextDirection(currentDir, dir);
    //        }
    //    }
    //}

    /// <summary>
    /// @tailRec
    /// </summary>
    //static void AdjustForRightHandWall() {
    //    if (!HasWall(RightDirection(), currentPos)) {
    //        if (HasWall(currentDir, currentPos)) {
    //            //Turn Left
    //            SendUpdateUponAct(Instruction.TurnLeft);
    //            Act(Instruction.TurnLeft);
    //            UpdateBoard(GetSensorBuffer());
    //        } else if (HasWall(OppositeDirection(), currentPos)) {
    //            //Turn Right
    //            SendUpdateUponAct(Instruction.TurnRight);
    //            Act(Instruction.TurnRight);
    //            UpdateBoard(GetSensorBuffer());
    //        } else if (HasWall(LeftDirection(), currentPos)) {
    //            //Turn Around
    //            SendUpdateUponAct(Instruction.TurnRight);
    //            Act(Instruction.TurnRight);
    //            UpdateBoard(GetSensorBuffer());
    //            SendUpdateUponAct(Instruction.TurnRight);
    //            Act(Instruction.TurnRight);
    //            UpdateBoard(GetSensorBuffer());
    //        } else {
    //            MoveToNearestWall();
    //            AdjustForRightHandWall(); // Adjust again because after moving will be facing wall
    //        }
    //    } // If already have right wall don't need to move
    //}


    static bool CheckHasUnexploredGridAfterInstru(Instruction instru) {
        List<Pos> lp = new List<Pos>();
        switch (instru) {
            case Instruction.Forward:
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(currentDir).x + 2 * DirectionToVector(LeftDirection()).x,
                    currentPos.y + 2 * DirectionToVector(currentDir).y + 2 * DirectionToVector(LeftDirection()).y,
                    LeftDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(currentDir).x + 2 * DirectionToVector(RightDirection()).x,
                    currentPos.y + 2 * DirectionToVector(currentDir).y + 2 * DirectionToVector(RightDirection()).y,
                    RightDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + 5 * DirectionToVector(currentDir).x + DirectionToVector(LeftDirection()).x,
                    currentPos.y + 5 * DirectionToVector(currentDir).y + DirectionToVector(LeftDirection()).y,
                    RightDirection()));
                break;
            case Instruction.TurnLeft:
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(LeftDirection()).x,
                    currentPos.y + 2 * DirectionToVector(LeftDirection()).y,
                    LeftDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(LeftDirection()).x + DirectionToVector(OppositeDirection()).x,
                    currentPos.y + 2 * DirectionToVector(LeftDirection()).y + DirectionToVector(OppositeDirection()).y,
                    LeftDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + DirectionToVector(LeftDirection()).x + 2 * DirectionToVector(OppositeDirection()).x,
                    currentPos.y + DirectionToVector(LeftDirection()).y + 2 * DirectionToVector(OppositeDirection()).y,
                    OppositeDirection()));
                break;
            case Instruction.TurnRight:
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(RightDirection()).x,
                    currentPos.y + 2 * DirectionToVector(RightDirection()).y,
                    RightDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(RightDirection()).x + DirectionToVector(OppositeDirection()).x,
                    currentPos.y + 2 * DirectionToVector(RightDirection()).y + DirectionToVector(OppositeDirection()).y,
                    RightDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + DirectionToVector(RightDirection()).x + 2 * DirectionToVector(OppositeDirection()).x,
                    currentPos.y + DirectionToVector(RightDirection()).y + 2 * DirectionToVector(OppositeDirection()).y,
                    OppositeDirection()));
                break;
            case Instruction.TurnBack:
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(LeftDirection()).x + DirectionToVector(OppositeDirection()).x,
                    currentPos.y + 2 * DirectionToVector(LeftDirection()).y + DirectionToVector(OppositeDirection()).y,
                    LeftDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + DirectionToVector(LeftDirection()).x + 2 * DirectionToVector(OppositeDirection()).x,
                    currentPos.y + DirectionToVector(LeftDirection()).y + 2 * DirectionToVector(OppositeDirection()).y,
                    OppositeDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(RightDirection()).x + DirectionToVector(OppositeDirection()).x,
                    currentPos.y + 2 * DirectionToVector(RightDirection()).y + DirectionToVector(OppositeDirection()).y,
                    RightDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + DirectionToVector(RightDirection()).x + 2 * DirectionToVector(OppositeDirection()).x,
                    currentPos.y + DirectionToVector(RightDirection()).y + 2 * DirectionToVector(OppositeDirection()).y,
                    OppositeDirection()));
                lp.AddRange(TripletOfPos(
                    currentPos.x + 2 * DirectionToVector(OppositeDirection()).x,
                    currentPos.y + 2 * DirectionToVector(OppositeDirection()).y,
                    OppositeDirection()));
                break;
        }
        foreach (Pos p in lp) {
            if (BlockUnexplored(p.x, p.y)) {
                return true;
            }
        }
        return false;
    }

    static List<Pos> TripletOfPos(int x, int y, Direction dir) {
        List<Pos> result = new List<Pos>();
        for (int i = 0; i < 3; i++) {
            result.Add(new Pos(x, y));
            x += DirectionToVector(dir).x;
            y += DirectionToVector(dir).y;
        }
        return result;
    }



    #endregion




    public static void Init(Pos currentPos, Direction currentDir) {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 20; j++) {
                grids[i, j].UpdateStatus(GridStatus.UNEXPLORED);
            }
        }
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                grids[i, j].UpdateStatus(GridStatus.EMPTY);
                grids[i + 12, j + 17].confidence = -9000;
            }
        }
        for (int i = -1; i < 2; i++) {
            switch (currentDir) {
                case Direction.NORTH:
                case Direction.SOUTH:
                    grids[currentPos.x + i, currentPos.y + 2 * DirectionToVector(currentDir).y].UpdateStatus(GridStatus.EMPTY);
                    break;
                case Direction.EAST:
                case Direction.WEST:
                    grids[currentPos.x + 2 * DirectionToVector(currentDir).x, currentPos.y + i].UpdateStatus(GridStatus.EMPTY);
                    break;
            }
        }

        totalSteps = 0;
        for (int i = 0; i < 8; i++) {
            lastPoses[i] = new Pos(-1, -1);
            lastDirs[i] = currentDir;
        }
        totalSteps = 0;
        _lastExploredCount = 0;
        Algo.currentPos = currentPos;
        Algo.currentDir = currentDir;
        _testing = false;
    }

    public static void Init(Pos currentPos, Direction currentDir, GridStatus[,] testBoard) {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 20; j++) {
                grids[i, j].UpdateStatus(GridStatus.UNEXPLORED);
            }
        }
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                grids[i, j].UpdateStatus(GridStatus.EMPTY);
                grids[i + 12, j + 17].confidence = -9000;
            }
        }
        for (int i = -1; i < 2; i++) {
            switch (currentDir) {
                case Direction.NORTH:
                case Direction.SOUTH:
                    grids[currentPos.x + i, currentPos.y + 2 * DirectionToVector(currentDir).y].UpdateStatus(GridStatus.EMPTY);
                    break;
                case Direction.EAST:
                case Direction.WEST:
                    grids[currentPos.x + 2 * DirectionToVector(currentDir).x, currentPos.y + i].UpdateStatus(GridStatus.EMPTY);
                    break;
            }
        }
        totalSteps = 0;
        _lastExploredCount = 0;
        Algo.currentPos = currentPos;
        Algo.currentDir = currentDir;
        _testing = true;
        _testBoard = testBoard;
    }

    private static string GetSensorBuffer() {
        string result;
        while (true) {
            _bufferMutex.WaitOne();
            if (!_hasNewSensorData) {
                _bufferMutex.ReleaseMutex();
                continue;
            }
            result = _sensorBuffer;
            _hasNewSensorData = false;
            _bufferMutex.ReleaseMutex();
            //CheckCalibrate(result);
            break;
        }
        return result;
    }

    public static void SetSensorBuffer(string incomingMsg) {
        _bufferMutex.WaitOne();
        _hasNewSensorData = true;
        _sensorBuffer = incomingMsg;
        _bufferMutex.ReleaseMutex();
    }

    public static void CheckAndAddCalibrate() {
        if (CalibratedFrontWall()) {
            // should have calibrated from Arduino: calibrateInstru = Instruction.CalibrateFrontWall;
            calibrateInstru = Instruction.CalibrateFront; // right wall calibration
            rightWallCounter = STEPS_EVERY_CALIBRATE;
            overallCalibrateCounter = STEPS_EVERY_CALIBRATE * 2;
        } else if (CanCalibrateByRightTurn()) {
            Debug.Log("Calibrate counters: " + rightWallCounter + ", " + overallCalibrateCounter);
            if (rightWallCounter < 1 || overallCalibrateCounter < 1) {
                rightWallCounter = STEPS_EVERY_CALIBRATE;
                overallCalibrateCounter = STEPS_EVERY_CALIBRATE * 2;
                calibrateInstru = Instruction.CalibrateRightWall; // right wall calibration
            }
            rightWallCounter -= 1;
        }
        overallCalibrateCounter -= 1;
        if (calibrateInstru != Instruction.Stop) {
            outgoingInstructions.Add(calibrateInstru);
            calibrateInstru = Instruction.Stop;
        }
    }


    //==========================
    // Sensor Formation
    // 
    //   abc
    //  dXXXe
    //   XXX
    //   XXX
    //
    // String : "a, b, c, d, e"
    //==========================
    static void UpdateBoard(string rawData) {

        int[] data = Array.ConvertAll(rawData.Split(','), int.Parse);
        int updateX = currentPos.x + DirectionToVector(currentDir).x;
        int updateY = currentPos.y + DirectionToVector(currentDir).y;

        FillBoard(data[3], LeftDirection(),
            updateX + 2 * DirectionToVector(LeftDirection()).x,
            updateY + 2 * DirectionToVector(LeftDirection()).y,
            true);
        FillBoard(data[4], RightDirection(),
            updateX + 2 * DirectionToVector(RightDirection()).x,
            updateY + 2 * DirectionToVector(RightDirection()).y,
            false);

        updateX = updateX + DirectionToVector(currentDir).x + DirectionToVector(LeftDirection()).x;
        updateY = updateY + DirectionToVector(currentDir).y + DirectionToVector(LeftDirection()).y;
        for (int i = 0; i < 3; i++) {
            FillBoard(data[i], currentDir, updateX, updateY, false);
            updateX += DirectionToVector(RightDirection()).x;
            updateY += DirectionToVector(RightDirection()).y;
        }
    }

    static void FillBoard(int data, Direction dir, int updateX, int updateY, bool fromLeftSensor) {
        if (data == -1) {
            data = 4;
        }
        for (int i = 1; i < 4; i++) {
            UpdateGridStatus(updateX, updateY, i, i == data, fromLeftSensor);
            if (i == data) {
                break; // shouldn't update those behind a known block
            }
            updateX += DirectionToVector(dir).x;
            updateY += DirectionToVector(dir).y;
        }
    }

    private static void UpdateGridStatus(int updateX, int updateY, int i, bool hasWall, bool fromLeftSensor) {
        int delta;
        if (i == 1) { delta = 20; } else if (i == 2) { delta = 10; } else { delta = 1; }
        //if (fromLeftSensor && i > 1) { delta += 1000; } // our long-range left sensor is good! Not distance 1 though
        //if (fromLeftSensor && i == 1) { delta = 10; } // our long-range left sensor is good! Not distance 1 though
        //(int)Math.Pow(10, 3 - i); // 1: 100, 2: 10, 3: 1
        if (!hasWall) {
            delta = -delta;
        }
        try { grids[updateX, updateY].UpdateStatus(delta); } catch { }
    }

    static bool CanMove(Direction dir) {
        if (RoomCheck(dir)) {
            return false;
        }

        bool flag = false;
        int newX = currentPos.x + 2 * DirectionToVector(dir).x;
        int newY = currentPos.y + 2 * DirectionToVector(dir).y;
        if (BlockFree(newX, newY)) {
            switch (dir) {
                case Direction.NORTH:
                case Direction.SOUTH:
                    flag = BlockFree(newX + DirectionToVector(Direction.EAST).x, newY + DirectionToVector(Direction.EAST).y)
                        && BlockFree(newX + DirectionToVector(Direction.WEST).x, newY + DirectionToVector(Direction.WEST).y);
                    break;
                case Direction.EAST:
                case Direction.WEST:
                    flag = BlockFree(newX + DirectionToVector(Direction.NORTH).x, newY + DirectionToVector(Direction.NORTH).y)
                        && BlockFree(newX + DirectionToVector(Direction.SOUTH).x, newY + DirectionToVector(Direction.SOUTH).y);
                    break;
            }
        }
        return flag;
    }

    static Direction DecideNextMove() {
        Direction nextMove;

        if (CanMove(RightDirection())) {
            nextMove = RightDirection();
        } else if (CanMove(currentDir)) {
            nextMove = currentDir;
        } else if (CanMove(LeftDirection())) {
            nextMove = LeftDirection();
        } else {
            nextMove = OppositeDirection();
        }
        return nextMove;
    }

    static void MoveRobot(Pos targetPos) {
        CreateVirtualWall(false);
        List<Direction> dirs = FastestPath.AStarSearch(currentPos, targetPos, grids, false);
        if (null != dirs) {
            foreach (Direction nextMove in dirs) {
                Debug.Log("TargetPos = " + targetPos.ToString() + ", " +
                    "currentPos = " + currentPos.ToString() + ", " +
                    "currentDir = " + currentDir + ", " +
                    "next move is: " + nextMove);
                MoveByNextDirection(currentDir, nextMove);
            }
        }
        Debug.Log("Exit FP, robot at: " + currentPos.x + ", " + currentPos.y);
        //AdjustForRightHandWall();
    }

    // all instructions to Arduino comes from here!
    static void MoveByNextDirection(Direction currentDir, Direction nextMove) {
        if (!_testing) {
            foreach (Instruction instru in GetInstructions(currentDir, nextMove, false)) {
                CheckAndAddCalibrate();

                if (CheckHasUnexploredGridAfterInstru(instru)) {
                    // check side calibrate instruction
                    outgoingInstructions.Add(instru);
                    // last new instru will require sensor reading, so suplement a Instruction.ReadSensor to the whole list
                    outgoingInstructions.Add(Instruction.ReadSensor);

                    SendFastestPathInstructions(outgoingInstructions);
                    outgoingInstructions.Clear();
                    Act(instru);
                    UpdateBoard(GetSensorBuffer());

                } else { // concat instrus
                    outgoingInstructions.Add(instru);
                    Act(instru);
                }

            }
        } else {
            foreach (Instruction instru in GetInstructions(currentDir, nextMove, false)) {
                Act(instru);
                Thread.Sleep((int)(1000 * timePerMove));
            }
            UpdateBoard(UpdateSensorData());
        }
    }

    // // version without concat instructions to save sensor time
    //static void MoveByNextDirection(Direction currentDir, Direction nextMove) {
    //    if (!_testing) {
    //        foreach (Instruction instru in GetInstructions(currentDir, nextMove, false)) {
    //            UpdateUponAct(instru);
    //            Act(instru);
    //            UpdateBoard(GetSensorBuffer());
    //        }
    //    } else {
    //        foreach (Instruction instru in GetInstructions(currentDir, nextMove, false)) {
    //            Act(instru);
    //            Thread.Sleep((int)(1000 * timePerMove));
    //        }
    //        UpdateBoard(UpdateSensorData());
    //    }
    //}

    static void StopExploration(object source, ElapsedEventArgs e) {
        _underSixMinutes = false;
        //UpdateArduinoInstruction(Instruction.Stop);
        //Algo.Act(Instruction.Stop);
    }

    public static void ExplorationAlgoTimeLimit() {

        int limit = (int)(timeLimit / timePerMove);
        int steps = 0;

        while (Contains2D(grids, GridStatus.UNEXPLORED) && (steps < limit)) {
            string rawData = UpdateSensorData();
            UpdateBoard(rawData);
            Direction nextMove = DecideNextMove();
            foreach (Instruction instru in GetInstructions(currentDir, nextMove, false)) {
                steps++;
                Act(instru);
                if (inSimulation) {
                    Thread.Sleep((int)(1000 * timePerMove));
                }
            }
        }

        inExploration = false;
        inSimulation = false;
    }

    public static void ExplorationAlgoCoverage() {
        // coverageLimit is in % of explored squares
        int limit = 3 * coveragePercentage; // 300*(coverageLimit/100) = 3 * coverageLimit

        while (Contains2D(grids, GridStatus.UNEXPLORED)) {
            string rawData = UpdateSensorData();
            UpdateBoard(rawData);
            if (ExploredLimitFulfilled(limit)) {
                break;
            }
            Direction nextMove = DecideNextMove();
            foreach (Instruction instru in GetInstructions(currentDir, nextMove, false)) {
                Act(instru);
                if (inSimulation) {
                    Thread.Sleep((int)(1000 * timePerMove));
                }
            }
        }

        inExploration = false;
        inSimulation = false;
    }

    public static void ExplorationAlgo() {

        // Set Timer for 5 minutes and 45 seconds
        endTimer = new System.Timers.Timer(345000);
        endTimer.Elapsed += StopExploration;
        endTimer.Enabled = true;

        Pos nextPos;
        Direction nextMove = currentDir;

        if (_testing) {
            UpdateBoard(UpdateSensorData());
        }

        while (!ExploredLimitFulfilled(300) && _underSixMinutes) {
            // as if we call nextMove = DecideNextMove() first
            if (_usingRightHandWall) {
                MoveByNextDirection(currentDir, nextMove);
                CheckSwitchToFastestPath();
            } else {
                nextPos = NextFringeToVisit();
                Debug.Log("Inside Fastest. Target: " + nextPos.x + ", " + nextPos.y);
                if (nextPos.x == -1) { // if null
                    break;
                }
                MoveRobot(nextPos);
                _usingRightHandWall = true;
                _lastExploredCount = CountExploredGrids();
                if (_lastExploredCount <= 30) {
                    for (int i = 0; i < 4; ++i) {
                        for (int j = 0; j < 4; ++j) {
                            grids[i, j].gs = GridStatus.EMPTY;
                        }
                    }
                }
            }
            nextMove = DecideNextMove();
        }

        if (_underSixMinutes) {
            MoveRobot(new Pos(1, 1)); // move back to start
        }
        if (!_testing) {
            SendMDF();
            //UpdateArduinoInstruction(Instruction.CalibrateStart);
            Act(Instruction.CalibrateStart); // arduino's hardcode after start calibration
        }
        CreateVirtualWall(true); // this is after sending MDF!
        hasCompleteMap = true;

        inExploration = false;
        inSimulation = false;
        Debug.Log("Exploration finished! Robot at: " + currentPos.x + ", " + currentPos.y +
                ". Total Steps = " + totalSteps);
    }

}
