using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using UnityEditor;
using System.Collections.Generic;
using System.Threading;
using System;
using static Algo;
using static Arena;

// Main non-static class of Algorithm
public class AlgoMono : MonoBehaviour {

    public GameObject gridPrefab;
    public GameObject robotPrefab;
    public Material gridEmpty;
    public Material gridVirtualWall;

    private Toggle toggleDiagonal;
    private Toggle toggleTemp;
    private Slider sliderCoverage;
    private Slider sliderInterval;
    private InputField inputTimeLimit;


    // ---------------------------------------------- //
    // ------------    Receive Message    ----------- //
    // ---------------------------------------------- //
    public void Receive(string incomingMsg) {
        try {
            switch (incomingMsg.Substring(0, 2)) {
                case "A2": // Arduino sensor data
                    Exploration.SetSensorBuffer(incomingMsg.Substring(3)); break;
                case "B1": // Android initiate
                    SetInitiateInstruction(int.Parse(incomingMsg.Substring(3, 1)));
                    break;
                case "B2": // Android set starting position or waypoint
                    Int32.TryParse(incomingMsg.Substring(4, 2), out int x);
                    Int32.TryParse(incomingMsg.Substring(6, 2), out int y);
                    Pos coordinate = new Pos(x, y);

                    if (incomingMsg.Substring(3, 1).Equals("0")) {
                        startingPos = coordinate;
                    } else if (incomingMsg.Substring(3, 1).Equals("1")) {
                        waypoint = coordinate;
                    }
                    break;
                case "B3": // Android set starting direction
                    switch (incomingMsg.Substring(3, 1)) {
                        case "0": startingDir = Direction.NORTH; break;
                        case "1": startingDir = Direction.EAST; break;
                        case "2": startingDir = Direction.SOUTH; break;
                        case "3": startingDir = Direction.WEST; break;
                    }
                    break;
            }
        } catch (Exception exception) {
            Debug.Log("Decode message fail: " + exception.ToString());
        }
    }

    // ---------------------------------------------- //
    // ------------   Buttons Functions   ----------- //
    // ---------------------------------------------- //

    public void RunExploration(bool simulation) {
        Arena.ClearBoard();

        if (simulation) {
            string path = DEFAULT_MAP_PATH; ;
            if (Application.isEditor) {
                //path = EditorUtility.OpenFilePanel("Load map file: ", "Assets/", "txt");
            } 
            if (path.Length > 0) {
                Arena.CreateBoard(false);
                startingPos = new Pos(1, 1);
                startingDir = Direction.EAST;
                Exploration._sensorBuffer = "-1,-1,-1,-1,1";
                Exploration.Init(startingPos, startingDir, LoadMap(path));
            } else {
                return; // didn't load map
            }
        } else {
            if (!connected) {
                return; // not connected to RPI
            }
            Arena.CreateBoard(false);
            Exploration.Init(startingPos, startingDir);
        }
        if (coveragePercentage < 100) { // simulation: coverage
            explorationThread = new Thread(new ThreadStart(Exploration.ExplorationAlgoCoverage));
        } else if (hasTimeLimit) { // simulation: time limit
            explorationThread = new Thread(new ThreadStart(Exploration.ExplorationAlgoTimeLimit));
        } else { // real run & plain simulation
            explorationThread = new Thread(new ThreadStart(Exploration.ExplorationAlgo));
        }
        inExploration = true;
        inSimulation = simulation;
        hasCompleteMap = false;
        explorationThread.Start();
    }

    public void RunFastestPath() {
        if (!hasCompleteMap) {
            Debug.LogError("No active complete map, can't do Fastest Path! ");
            return;
        }
        inFastestPath = true;
        inSimulation = true;
        InitRobot();
        List<Direction> solution;
        if (waypoint.x != -1) { // if has waypoint
            solution = FastestPath.AStarSearch(new Pos(1, 1), waypoint, grids, diagonal);
            if (solution.Count < 1) {
                Debug.Log("Cannot find a path to waypoint, aborting.");
                inFastestPath = false;
                inSimulation = false;
                return;
            }
            Direction initialDir = currentDir;
            currentDir = solution[solution.Count - 1];
            solution.AddRange(FastestPath.AStarSearch(waypoint, new Pos(13, 18), grids, diagonal));
            currentDir = initialDir;
        } else {
            solution = FastestPath.AStarSearch(grids, diagonal);
        }
        foreach (Direction d in solution) {
            Debug.Log(d);
        }
        List<Instruction> li = ConvertToInstructionList(solution, diagonal);
        try {
            SendFastestPathInstructions(li);
        } catch { }

        // for Unity display
        // StartCoroutine() prevents this from being static!
        StartCoroutine("ShowPath", li);
    }
    IEnumerator ShowPath(List<Instruction> solution) {
        while (inFastestPath) {
            if (!(solution.Count > 0)) {
                inFastestPath = false;
                inSimulation = false;
                UpdateAndroidMap();
                StopAllCoroutines();
                yield return null;
            }
            Act(solution[0]);
            solution.RemoveAt(0);
            //RenderUpdateAll();
            yield return new WaitForSeconds(timePerMove);
        }
    }

    public void CalibrateStart() {
        if (connected) {
            client.SendMessage("A1", InstructionToChar(Instruction.CalibrateStart).ToString());
        } else {
            Debug.LogError("Not connected! Cannot send start calibrate.");
        }
    }

    public void LoadMapFromFile() {
        Arena.ClearBoard();
        string path = DEFAULT_MAP_PATH;
        if (Application.isEditor) {
            //path = EditorUtility.OpenFilePanel("Load map file: ", "Assets/", "txt");
        } 
        if (path.Length > 0) {
            gridStatuses = Arena.LoadMap(path);
            CreateVirtualWall(true);
        } else {
            Debug.LogError("Not able to open file! Loading default test map file. ");
            gridStatuses = Arena.LoadMap(DEFAULT_MAP_PATH);
        }
        Arena.CreateBoard(true);
    }

    public void RunImageRecognition() {
        // TODO:
    }

    public void ConnectToRPI() {
        client.Init(this);
    }


    // ---------------------------------------------- //
    // ------------      UI Component      ---------- //
    // ---------------------------------------------- //

    public void ToggleDiagonal() {
        Algo.diagonal = toggleDiagonal.isOn;
    }
    public void ToggleTemp() {
        Algo.toggleTempFlag = toggleTemp.isOn;
    }
    public void ValueChangeCoverage() {
        coveragePercentage = (int)sliderCoverage.value;
        GameObject.Find("Slider_Coverage_Text").GetComponent<Text>().text = "Coverage% : " + coveragePercentage + "%";
    }
    public void ValueChangeInterval() {
        timePerMove = 1 / sliderInterval.value;
        GameObject.Find("Slider_Interval_Text").GetComponent<Text>().text = "Move per second: " + sliderInterval.value.ToString("0.0");
    }
    public void ValueChangeTimeLimit() {
        try {
            int minValue;
            int secValue = 0;
            if (Int32.TryParse(inputTimeLimit.text.Substring(0, 2), out minValue)) {
                Int32.TryParse(inputTimeLimit.text.Substring(3, 2), out secValue);
            } else if (Int32.TryParse(inputTimeLimit.text.Substring(0, 1), out minValue)) {
                Int32.TryParse(inputTimeLimit.text.Substring(2, 2), out secValue);
            }
            hasTimeLimit = true;
            timeLimit = minValue * 60 + secValue;
            GameObject.Find("Text_Limit").GetComponent<Text>().text = "Time Left: " + timeLimit + "s";
        } catch { }

    }

    // ---------------------------------------------- //
    // ------------ MonoBehavior Component ---------- //
    // ---------------------------------------------- //

    private void Awake() {

        // UI setup
        Algo algo = GetComponent<Algo>();

        GameObject.Find("Button_FastestPath").GetComponent<Button>().onClick.AddListener(RunFastestPath);
        GameObject.Find("Button_Exploration").GetComponent<Button>().onClick.AddListener(delegate { RunExploration(false); });
        GameObject.Find("Button_ExplorationTest").GetComponent<Button>().onClick.AddListener(delegate { RunExploration(true); });
        GameObject.Find("Button_Image").GetComponent<Button>().onClick.AddListener(RunImageRecognition);
        GameObject.Find("Button_RPI").GetComponent<Button>().onClick.AddListener(ConnectToRPI);
        GameObject.Find("Button_Calibrate_Start").GetComponent<Button>().onClick.AddListener(CalibrateStart);
        //GameObject.Find("Button_Calibrate_Start").GetComponent<Button>().onClick.AddListener(LoadMapFromFile);
        sliderCoverage = GameObject.Find("Slider_Coverage").GetComponent<Slider>();
        sliderInterval = GameObject.Find("Slider_Interval").GetComponent<Slider>();
        toggleDiagonal = GameObject.Find("Toggle_Diagonal").GetComponent<Toggle>();
        toggleTemp = GameObject.Find("Toggle_Temp").GetComponent<Toggle>();
        inputTimeLimit = GameObject.Find("InputField_TimeLimit").GetComponent<InputField>();
        sliderCoverage.onValueChanged.AddListener(delegate { ValueChangeCoverage(); });
        sliderInterval.onValueChanged.AddListener(delegate { ValueChangeInterval(); });
        inputTimeLimit.onEndEdit.AddListener(delegate { ValueChangeTimeLimit(); });
        toggleDiagonal.onValueChanged.AddListener(delegate { ToggleDiagonal(); });
        toggleTemp.onValueChanged.AddListener(delegate { ToggleTemp(); });

        Arena.gridCubesContainer = GameObject.Find("GridCubes").transform;
        Arena.gridCubeWallsContainer = GameObject.Find("GridCubeWalls").transform;
        Arena.gridPrefab = gridPrefab;
        Arena.robotPrefab = robotPrefab;
        Arena.gridEmpty = gridEmpty;
        Arena.gridVirtualWall = gridVirtualWall;

    }

    //public void RenderUpdate(Pos p) {
    //    gridCubes[p.x, p.y].GetComponent<Grid>().UpdateStatus(grids[p.x, p.y]);
    //}
    //public void RenderUpdateAll() {
        //robot.transform.position = ConvertPosToWorld(currentPos.x, currentPos.y);
        //robot.GetComponent<Robot>().direction = currentDir;
    //}

    void CheckAndroidInitiate() {
        int i = GetInitiateInstruction();
        switch (i) {
            case 0:
                SetInitiateInstruction(-1);
                if (inExploration) { return; }
                RunExploration(false);
                break;
            case 1:
                SetInitiateInstruction(-1);
                if (inFastestPath) { return; }
                RunFastestPath();
                break;
            case 2:
                SetInitiateInstruction(-1);
                RunImageRecognition();
                break;
        }
    }
    private void OnDestroy() {
        StopAllCoroutines();
        try {
            explorationThread.Abort();
            Algo.client.Disconnect();
            SocketClient.socketConnection.GetStream().Close();
            SocketClient.socketConnection.Close();

        } catch { }
    }

    void Update() {
        if (connected) {
            CheckAndroidInitiate();
        }


        if (Input.GetKeyDown(KeyCode.Escape)) {
            Debug.Log("Exit.");
            Application.Quit();
        }
        if (Input.GetKeyDown(KeyCode.W)) {
            UpdateArduinoInstruction(Instruction.Forward);
        }
        if (Input.GetKeyDown(KeyCode.R)) {
            UpdateArduinoInstruction(Instruction.ForwardD);
        }
        if (Input.GetKeyDown(KeyCode.Q)) {
            UpdateArduinoInstruction(Instruction.TurnLeftD);
        }
        if (Input.GetKeyDown(KeyCode.E)) {
            UpdateArduinoInstruction(Instruction.TurnRightD);
        }
        if (Input.GetKeyDown(KeyCode.A)) {
            Act(Instruction.TurnLeft);
            UpdateArduinoInstruction(Instruction.TurnLeft);
        }
        if (Input.GetKeyDown(KeyCode.D)) {
            Act(Instruction.TurnRight);
            UpdateArduinoInstruction(Instruction.TurnRight);
        }
        if (Input.GetKeyDown(KeyCode.C)) {
            UpdateArduinoInstruction(Instruction.CalibrateFront);
        }
        if (Input.GetKeyDown(KeyCode.F)) {
            UpdateArduinoInstruction(Instruction.CalibrateStart);
        }
        if (Input.GetKeyDown(KeyCode.M)) {
            //client.SendMessage("B4", "09031AAA9A802AAA6A00AAAAAAA0AAAAAA81AAAAAA00AAAA00009AA800021AA00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
            SendMDF();
        }
        //if (Input.GetKeyDown(KeyCode.I)) {
        //    client.SendMessage("D2", "1110");
        //}
        if (Input.GetKeyDown(KeyCode.P)) {
            client.SendMessage("C1", "9N6N2M4M6N8");
        }
        //if (Input.GetKeyDown(KeyCode.O)) {
        //    client.SendMessage("A1", "2O");
        //}


    }
}
