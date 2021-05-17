'''
Author: Huangfu Qingchuan (c160135@ntu.edu.sg)
NTU SCSE MDP AY1920 S1 Group 24

'''
import sys, math

class SearchProblem:
    def __init__(self, wall_config='20_15_virtual_wall_test.txt'):
        self.startState = (1, 1)
        self.goal = (13, 18)
        self.walls = self.loadWalls(wall_config)

    def isGoalState(self, state):
        return state[0] == self.goal[0] and state[1] == self.goal[1]

    def getSuccessors(self, state):
        successors = []
        for action in [Directions.NORTH, Directions.SOUTH, Directions.EAST, Directions.WEST]:
            x, y = state
            dx, dy = Actions.directionToVector(action)
            nextx, nexty = int(x + dx), int(y + dy)
            try:
                if not self.walls[nextx][nexty]:
                    nextState = (nextx, nexty)
                    cost = 1
                    successors.append((nextState, action, cost))
            except: # List index out of range as walls
                pass

        return successors

    def manhattanHeuristic(goal_position, position):
        "The Manhattan distance heuristic"
        xy1 = position
        xy2 = goal_position
        return abs(xy1[0] - xy2[0]) + abs(xy1[1] - xy2[1])

    def nullHeuristic(self, state): return 0

    def diagonal_adjustment(self, curr_action, prev_action):
        if curr_action == prev_action: return 0.05
        return 0

    def loadWalls(self, wall_config):
        list = []
        try:
            with open(wall_config, 'r') as f:
                file = f.readlines()
                for i in range(20):
                    x = []
                    for j in range(15):
                        if int(file[i][j]) == 0:
                            flag = False
                        else:
                            flag = True
                        x.append(flag)
                    list.append(x)
            transposed = [[list[j][i] for j in range(len(list))] for i in range(len(list[0]))]

        except Exception as e:
            print(e)
            sys.exit()

        return transposed

    def diagonalAStarSearch(self, diagonal, heuristic=manhattanHeuristic):
        """Search the node that has the lowest combined action cost and heuristic first."""
        from queue import PriorityQueue

        solution = []
        fringe = PriorityQueue()
        visited = set([])

        cumulative_cost = 0
        last_action = Directions.STOP
        curr = self.startState
        visited.add(str(curr))
        while not self.isGoalState(curr):
            for s in self.getSuccessors(curr):
                if str(s[0]) not in visited:
                    f_cost = s[2] + heuristic(self.goal, s[0])
                    if diagonal:
                        f_cost = f_cost + self.diagonal_adjustment(s[1], last_action)
                    fringe.put((cumulative_cost + f_cost, (s, solution + [s[1]])))
                    visited.add(str(s[0]))

            if fringe:
                next = fringe.get()  # get smallest value first
                next_successor = next[1][0]
                curr = next_successor[0]
                solution = next[1][1]
                cumulative_cost = next[0]
                last_action = solution[-1]

            else:
                print("Warning! Search exited with empty fringe. ")
                break

        # print(solution)
        return solution

    def convert_to_diagonal(self, seq):
        ''' Convert adjacent perpendicular move into diagonal. '''
        solution = []
        solution.append(seq[0])
        for i in range(1, len(seq)):
            xy1 = Actions.directionToVector(seq[i])
            xy2 = Actions.directionToVector(seq[i-1])
            comb = (xy1[0] + xy2[0], xy1[1] + xy2[1])
            dist = comb[0] ** 2 + comb[1] ** 2
            if 1 < dist < 4:
                solution.pop()
                solution.append(Actions.vectorToDirection(comb))
            else:
                solution.append(seq[i])
        return solution

class Directions:
    NORTH = 'North'
    SOUTH = 'South'
    EAST = 'East'
    WEST = 'West'
    STOP = 'Stop'
    NORTHEAST = 'NorthEast'
    SOUTHEAST = 'SouthEast'
    SOUTHWEST = 'SouthWest'
    NORTHWEST = 'NorthWest'

    # LEFT =       {NORTH: WEST,
    #                SOUTH: EAST,
    #                EAST:  NORTH,
    #                WEST:  SOUTH,
    #                STOP:  STOP}

    # RIGHT =  dict([(y,x) for x, y in LEFT.items()])

    # REVERSE = {NORTH: SOUTH,
    #            SOUTH: NORTH,
    #            EAST: WEST,
    #            WEST: EAST,
    #            STOP: STOP}

class Actions:

    _directions = {Directions.NORTH: (0, 1),
                   Directions.SOUTH: (0, -1),
                   Directions.EAST:  (1, 0),
                   Directions.WEST:  (-1, 0),
                   Directions.STOP:  (0, 0),
                   Directions.NORTHEAST: (1, 1),
                   Directions.NORTHWEST: (1, -1),
                   Directions.SOUTHEAST:  (-1, 1),
                   Directions.SOUTHWEST:  (-1, -1)}

    def directionToVector(direction): return Actions._directions[direction]

    def vectorToDirection(vector):
        dx, dy = vector
        if dy > 0:
            if dx > 0: 
                return Directions.NORTHEAST
            elif dx < 0 :
                return Directions.NORTHWEST
            else:
                return Directions.NORTH
        elif dy < 0:
            if dx > 0: 
                return Directions.SOUTHEAST
            elif dx < 0 :
                return Directions.SOUTHWEST
            else:
                return Directions.SOUTH
        else:
            if dx > 0: 
                return Directions.EAST
            elif dx < 0 :
                return Directions.WEST
            else:
                return Directions.STOP


if __name__ == '__main__':

    diagonal_enabled = False
    try:
        diagonal_enabled = sys.argv[1] == 'True' or sys.argv[1] == 'true'
    except:
        pass

    search = SearchProblem()
    if diagonal_enabled:
        seq_result = search.diagonalAStarSearch(True)
        result = search.convert_to_diagonal(seq_result)
    else:
        result = search.diagonalAStarSearch(False)

    print(result)




'''
/*
 List<Direction> solution = new List<Direction>();
        List<StateSolutionCostPair> fringe = new List<StateSolutionCostPair>();
        HashSet<Pos> visited = new HashSet<Pos>();

        float cumulative_cost = 0;
        Direction last_action = Direction.STOP;
        Pos curr = startState;
        visited.Add(curr);
        while (!IsGoalState(curr))
        {
            foreach (Successor s in GetSuccessors(curr))
            {
                if (!visited.Contains(s.state))
                {
                    float f_cost_delta = s.cost + ManhattanHeuristic(goal, s.state);
                    if (diagonal)
                    {
                        f_cost_delta += DiagonalAdjustment(s.action, last_action);
                    }
                    List<Direction> newSolution = new List<Direction>(solution) { s.action };
                    fringe.Add(new StateSolutionCostPair(s.state, newSolution, cumulative_cost + f_cost_delta));
                    visited.Add(s.state);
                }
            }
     
     */
'''