using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Arena;
using static Algo;

public class FastestPath {
    public static Pos startState;
    public static Pos goal;
    public static Grid[,] walls;

    static bool IsGoalState(Pos state) {
        return state.x == goal.x && state.y == goal.y;
    }

    struct Successor {
        public Pos state;
        public Direction action;
        public float cost;
        public Successor(Pos state, Direction action, float cost) {
            this.state = state;
            this.action = action;
            this.cost = cost;
        }
    }
    static List<Successor> GetSuccessors(Pos state) {
        List<Successor> successors = new List<Successor>();
        foreach (Direction action in Arena.directionsForExplore) {
            int nextx = state.x + DirectionToVector(action).x;
            int nexty = state.y + DirectionToVector(action).y;
            try {
                if (walls[nextx, nexty].gs == GridStatus.EMPTY
                    //|| walls[nextx, nexty].gs == GridStatus.UNEXPLORED // accommondate exploration
                    ) {
                    Pos nextState = new Pos(nextx, nexty);
                    // cost = 1 by default
                    successors.Add(new Successor(nextState, action, 1));
                }
            } catch { continue; } // when index out of range, ignore
        }

        return successors;
    }

    static float ManhattanHeuristic(Pos goal_position, Pos position) {
        // The Manhattan distance heuristic
        return Math.Abs(position.x - goal_position.x) + Math.Abs(position.y - goal_position.y);
    }

    static float DiagonalAdjustment(Direction curr_action, Direction prev_action, bool diagonal) {
        if (curr_action == prev_action) {
            if (diagonal) { return 0.5f; } 
            else { return -0.5f; }
        }
        return 0;
    }

    struct StateSolutionPair {
        public Pos state;
        public List<Direction> solution;
        public StateSolutionPair(Pos state, List<Direction> solution) {
            this.state = state;
            this.solution = solution;
        }
    }

    public static List<Direction> AStarSearch(
        Grid[,] wallsInfo,
        bool diagonal) {
        return AStarSearch(new Pos(1, 1), new Pos(13, 18), wallsInfo, diagonal);
    }
    public static List<Direction> AStarSearch(
        Pos startPos, Pos goalPos, Grid[,] wallsInfo,
        bool diagonal) {
        // Search the node that has the lowest combined action cost and heuristic first.

        startState = startPos;
        goal = goalPos;
        walls = wallsInfo;

        List<Direction> solution = new List<Direction>();
        PriorityQueue fringe = new PriorityQueue();
        HashSet<Pos> visited = new HashSet<Pos>();

        float cumulativeCost = 0;
        Direction lastAction = currentDir;
        Pos curr = startState;
        visited.Add(curr);
        while (!IsGoalState(curr)) {
            foreach (Successor s in GetSuccessors(curr)) {
                if (!visited.Contains(s.state)) {
                    float f_cost = s.cost + ManhattanHeuristic(goal, s.state);
                    f_cost += DiagonalAdjustment(s.action, lastAction, diagonal);
                    List<Direction> newSolution = new List<Direction>(solution) { s.action };
                    fringe.Enqueue(cumulativeCost + f_cost, s.state, newSolution);
                    visited.Add(s.state);
                }
            }

            if (fringe.Count() > 0) {
                Tuple<float, Pos, List<Direction>> next = fringe.Dequeue();  // get smallest value first
                cumulativeCost = next.Item1;
                curr = next.Item2;
                solution = next.Item3;
                lastAction = solution[solution.Count - 1];
            } else {
                Debug.LogError("Search exited with empty fringe. ");
                return null;
            }
        }
        return solution;
    }

}