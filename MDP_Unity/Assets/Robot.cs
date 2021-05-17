using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Arena;
using static Algo;

public class Robot : MonoBehaviour
{
    public Direction direction;

    void Update()
    {
        robot.transform.position = ConvertPosToWorld(currentPos.x, currentPos.y);
        robot.GetComponent<Robot>().direction = currentDir;
        transform.rotation = Quaternion.Euler(0, DirectionToEuler(direction) ,0);
    }

}
