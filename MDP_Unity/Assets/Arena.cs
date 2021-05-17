using System.Collections.Generic;
using System.IO;
using UnityEngine;
using static Algo;

// Main support class Arena, contains Pos, Direction, and GameObject references
public class Arena {

    public static GameObject robot;
    public static GameObject[,] gridCubes;
    public static List<GameObject> obstacles;

    private static Vector2 referencePos = new Vector2(5, 1.5f);

    // Unity Editor references
    public static GameObject gridPrefab;
    public static GameObject robotPrefab;
    public static Material gridEmpty;
    public static Material gridVirtualWall;
    public static Transform gridCubesContainer;
    public static Transform gridCubeWallsContainer;

    public struct Pos {
        public int x;
        public int y;
        public Pos(int x, int y) {
            this.x = x;
            this.y = y;
        }
        public override string ToString() {
            return x + ", " + y;
        }
    }

    public enum GridStatus { UNEXPLORED, WALL, EMPTY, VIRTUAL_WALL, IMAGE };
    public enum Direction { NORTH, SOUTH, EAST, WEST, STOP, NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST };
    public enum Instruction {
        Forward, ForwardD,
        TurnLeft, TurnRight, TurnLeftD, TurnRightD, TurnBack,
        CalibrateStart, CalibrateRightWall, CalibrateFront,
        Backward,
        TurnLeftF, TurnRightF, ReadSensor,
        Stop, // as null value
    };
    public static char InstructionToChar(Instruction instru) {
        char instruChar = 'P';
        switch (instru) {
            case Instruction.TurnLeftF: instruChar = 'N'; break;
            case Instruction.TurnRightF: instruChar = 'M'; break;
            case Instruction.TurnBack: instruChar = 'X'; break;
            case Instruction.CalibrateRightWall: instruChar = 'V'; break;
            case Instruction.CalibrateStart: instruChar = 'F'; break;
            case Instruction.ReadSensor: instruChar = 'O'; break;
            case Instruction.TurnLeftD: instruChar = 'Q'; break;
            case Instruction.TurnRightD: instruChar = 'E'; break;
            case Instruction.ForwardD: instruChar = 'R'; break;

            case Instruction.Stop: instruChar = 'P'; break;
            case Instruction.Forward: instruChar = 'W'; break;
            case Instruction.Backward: instruChar = 'S'; break;
            case Instruction.TurnLeft: instruChar = 'A'; break;
            case Instruction.TurnRight: instruChar = 'D'; break;
            case Instruction.CalibrateFront: instruChar = 'C'; break;
        }
        return instruChar;
    }

    public static Direction[] directionsForExplore = { Direction.NORTH, Direction.EAST, Direction.SOUTH, Direction.WEST };
    public static Dictionary<Direction, Pos> DirectionToPos = new Dictionary<Direction, Pos>() {
        { Direction.NORTH, new Pos(0, 1) },
        { Direction.SOUTH, new Pos(0, -1) },
        { Direction.EAST, new Pos(1, 0) },
        { Direction.WEST, new Pos(-1, 0) },
        { Direction.STOP, new Pos(0, 0) },
        { Direction.NORTHEAST, new Pos(1, 1) },
        { Direction.NORTHWEST, new Pos(-1, 1) },
        { Direction.SOUTHEAST, new Pos(1, -1) },
        { Direction.SOUTHWEST, new Pos(-1, -1) },
    };
    public static Pos DirectionToVector(Direction direction) { return DirectionToPos[direction]; }
    public static Direction VectorToDirection(Pos vector) {
        int dx = vector.x;
        int dy = vector.y;
        if (dy > 0) {
            if (dx > 0) return Direction.NORTHEAST;
            else if (dx < 0) return Direction.NORTHWEST;
            else return Direction.NORTH;
        } else if (dy < 0) {
            if (dx > 0) return Direction.SOUTHEAST;
            else if (dx < 0) return Direction.SOUTHWEST;
            else return Direction.SOUTH;
        } else {
            if (dx > 0) return Direction.EAST;
            else if (dx < 0) return Direction.WEST;
            else return Direction.STOP;
        }
    }
    public static int DirectionToEuler(Direction direction) {
        switch (direction) {
            case Direction.STOP:
            case Direction.NORTH: return 0;
            case Direction.NORTHEAST: return 45;
            case Direction.EAST: return 90;
            case Direction.SOUTHEAST: return 135;
            case Direction.SOUTH: return 180;
            case Direction.SOUTHWEST: return 225;
            case Direction.WEST: return 270;
            case Direction.NORTHWEST: return 315;
        }
        return 0;
    }

    public static Direction EulerToDirection(int direction) {
        switch (direction) {
            case 0: return Direction.NORTH;
            case -315:
            case 45: return Direction.NORTHEAST;
            case -270:
            case 90: return Direction.EAST;
            case -225:
            case 135: return Direction.SOUTHEAST;
            case -180:
            case 180: return Direction.SOUTH;
            case -135:
            case 225: return Direction.SOUTHWEST;
            case -90:
            case 270: return Direction.WEST;
            case -45:
            case 315: return Direction.NORTHWEST;
        }
        return Direction.NORTH;
    }

    public static GridStatus[,] LoadMap(string wall_path) {
        GridStatus[,] list = new GridStatus[15, 20];
        try {
            using (StreamReader sr = new StreamReader(wall_path)) {
                for (int i = 0; i < 20; i++) {
                    string s = sr.ReadLine();
                    for (int j = 0; j < 15; j++) {
                        if ((s[j]) == '0') {
                            list[j, 19 - i] = GridStatus.EMPTY;
                        } else if ((s[j]) == '1') {
                            list[j, 19 - i] = GridStatus.WALL;
                        } else if ((s[j]) == '2') {
                            list[j, 19 - i] = GridStatus.VIRTUAL_WALL;
                        }
                    }
                }
            }
        } catch { Debug.LogError("File read fail! "); }

        return list;
    }

    public static void CreateBoard(bool hasSetMap) {
        if (!hasSetMap) { gridStatuses = new GridStatus[15, 20]; }
        gridCubes = new GameObject[15, 20];

        for (int j = 0; j < 20; j++) {
            for (int i = 0; i < 15; i++) {
                Vector3 pos = ConvertPosToWorld(i, j);
                GameObject go = GameObject.Instantiate(gridPrefab, pos, Quaternion.identity);
                gridCubes[i, j] = go;
                grids[i, j] = go.GetComponent<Grid>();
                go.transform.parent = gridCubesContainer;
                grids[i, j].InitPos(i, j);
                if (hasSetMap) {
                    grids[i, j].UpdateStatus(gridStatuses[i, j]);
                }
            }
        }

        // Borders
        for (int i = 0; i < 17; i++) {
            for (int m = 0; m < 2; m++) {
                for (int k = 0; k < 3; k++) {
                    Vector3 pos = new Vector3(referencePos.x + 1 - m * 21, k, referencePos.y - 1 + i);
                    GameObject go = GameObject.Instantiate(gridPrefab, pos, Quaternion.identity);
                    go.transform.parent = gridCubeWallsContainer;
                }
            }
        }
        for (int j = 0; j < 22; j++) {
            for (int m = 0; m < 2; m++) {
                for (int k = 0; k < 3; k++) {
                    Vector3 pos = new Vector3(referencePos.x + 1 - j, k, referencePos.y - 1 + m * 16);
                    GameObject go = GameObject.Instantiate(gridPrefab, pos, Quaternion.identity);
                    go.transform.parent = gridCubeWallsContainer;
                }
            }
        }

        if (hasSetMap) {
            hasCompleteMap = true;
        } else { // Robot
            InitRobot();
        }
    }
    public static void CreateVirtualWall(bool changeUnexploredToEmpty) {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 20; j++) {
                if (changeUnexploredToEmpty && grids[i, j].gs == GridStatus.UNEXPLORED) {
                    grids[i, j].gs = GridStatus.EMPTY;
                }
                if (i == 0 || i == 14 || j == 0 || j == 19) {
                    if (grids[i, j].gs == GridStatus.EMPTY) {
                        grids[i, j].gs = GridStatus.VIRTUAL_WALL;
                    }
                }
                if (grids[i, j].gs == GridStatus.WALL) {
                    for (int a = -1; a < 2; a++) {
                        for (int b = -1; b < 2; b++) {
                            try {
                                if (grids[i + a, j + b].gs == GridStatus.EMPTY) {
                                    grids[i + a, j + b].gs = GridStatus.VIRTUAL_WALL;
                                }
                            } catch { }
                        }
                    }
                }
            }
        }
    }

    public static void InitRobot() {
        try {
            GameObject.Destroy(GameObject.Find("Robot(Clone)"));
        } catch { }

        Vector3 robotPos = ConvertPosToWorld(1, 1);
        currentPos = new Pos(1, 1);
        robot = GameObject.Instantiate(robotPrefab, robotPos, Quaternion.identity);
    }


    public static Vector3 ConvertPosToWorld(int i, int j) {
        return new Vector3(referencePos.x - j, 0, referencePos.y + i);
    }

    public static bool Contains2D(Grid[,] arr2d, GridStatus value) {
        for (int i = 0; i < arr2d.GetLength(0); i++) {
            for (int j = 0; j < arr2d.GetLength(1); j++) {
                if (arr2d[i, j].gs.Equals(value)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static void ClearBoard() {
        // remove all prefabs from arena
        try {
            for (int i = 0; i < gridCubes.GetLength(0); i++) {
                for (int j = 0; j < gridCubes.GetLength(1); j++) {
                    GameObject.Destroy(gridCubes[i, j].GetComponent<Grid>().gameObject);
                }
            }
            gridStatuses = new GridStatus[15, 20];
            grids = new Grid[15, 20];
        } catch { }
        hasCompleteMap = false;
    }
}
