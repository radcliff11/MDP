using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Arena;
using static Algo;

public class Grid : MonoBehaviour {
    public GridStatus gs = GridStatus.UNEXPLORED;
    public bool explored = false;
    private GameObject obstacle;
    public int confidence = 0;
    public int x;
    public int y;

    private void Awake() {
        obstacle = transform.GetChild(0).gameObject;
        obstacle.SetActive(false);
    }
    public void InitPos(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void UpdateStatus(GridStatus gs) {
        this.gs = gs;
        switch (gs) {
            case GridStatus.UNEXPLORED: confidence = 0; break;
            case GridStatus.EMPTY: confidence = -9000; break;
            case GridStatus.WALL: confidence = 9000; break;
            //case GridStatus.IMAGE: obstacle.SetActive(true); break;
        }
    }
    public void UpdateStatus(int delta) {
        confidence += delta;
        if (confidence > 0) {
            gs = GridStatus.WALL;
        } else if (confidence < 0) {
            gs = GridStatus.EMPTY;
        } else {
            gs = GridStatus.UNEXPLORED;
        }
    }
    private void Update() {
        switch (gs) {
            case GridStatus.UNEXPLORED: break;
            case GridStatus.EMPTY: GetComponent<MeshRenderer>().material = gridEmpty; obstacle.SetActive(false); break;
            case GridStatus.WALL:
            case GridStatus.IMAGE: obstacle.SetActive(true); break;
            case GridStatus.VIRTUAL_WALL: GetComponent<MeshRenderer>().material = gridVirtualWall; break;
        }
    }
}
