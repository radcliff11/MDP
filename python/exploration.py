'''
Author: Huangfu Qingchuan (c160135@ntu.edu.sg)
NTU SCSE MDP AY1920 S1 Group 24

'''
import sys, math

class ExplorationProblem:
    def __init__(self, ):
        self.board = [[2]*15]*20 # 0: explored safe 1: wall 2: undetermined
        self.current_position = [0,0] # position is tuple in shortest path
        self.current_direction = Directions.EAST # TODO: should have flexibility in UI

    def read_sensor(self, sensor_data):
        # Assume sensor_data is array of integers
        pass
        # TODO Update explored data
        # TODO Update walls if any

    def right_empty(self):
        '''
            Function written with the assumption:
            board [y][x]
            position [x,y]
        '''
        switch = {Directions.EAST:[1,-1], Directions.NORTH:[0,1], Directions.WEST:[1,1], Directions.SOUTH:[0,-1]}
        right_coordinate = self.current_position
        right_coordinate[switch[self.current_direction][0]] += switch[self.current_direction][0]

        return self.board[right_coordinate[1]][right_coordinate[0]] == 0
    
    def can_move(self, dir):
        '''
            Function written with the assumption:
            board [y][x]
            position [x,y]
        '''
        if dir == 'right':
            switch = {Directions.EAST:[1,-1], Directions.NORTH:[0,1], Directions.WEST:[1,1], Directions.SOUTH:[0,-1]}
        elif dir == 'left':
            switch = {Directions.EAST:[1,1], Directions.NORTH:[0,-1], Directions.WEST:[1,-1], Directions.SOUTH:[0,1]}
        elif dir == 'front':
            switch = {Directions.EAST:[0,1], Directions.NORTH:[1,1], Directions.WEST:[0,-1], Directions.SOUTH:[1,-1]}
        elif dir == 'back':
            switch = {Directions.EAST:[0,-1], Directions.NORTH:[1,-1], Directions.WEST:[0,1], Directions.SOUTH:[1,1]}

        next_coordinate = self.current_position
        next_coordinate[switch[self.current_direction][0]] += switch[self.current_direction][0]

        if self.board[next_coordinate[1]][next_coordinate[0]] == 0:
            return True, next_coordinate
        return False, self.current_position

    def determine_direction(self):
        directions = ['right', 'front', 'left', 'back']
        right = {Directions.EAST: Directions.SOUTH, Directions.SOUTH: Directions.WEST, Directions.WEST: Directions.NORTH, Directions.NORTH: Directions.EAST}
        front = {Directions.EAST: Directions.EAST, Directions.SOUTH: Directions.SOUTH, Directions.WEST: Directions.WEST, Directions.NORTH: Directions.NORTH}
        left = {Directions.EAST: Directions.NORTH, Directions.SOUTH: Directions.EAST, Directions.WEST: Directions.SOUTH, Directions.NORTH: Directions.WEST}
        back = {Directions.EAST: Directions.WEST, Directions.SOUTH: Directions.NORTH, Directions.WEST: Directions.EAST, Directions.NORTH: Directions.SOUTH}

        for d in directions:
            flag, coor = self.can_move(d)
            if flag:
                cardinal_direction = eval(d)
                return cardinal_direction[self.current_direction], coor
        
        return self.current_direction, self.current_position
    
    def fully_explored(self):
        for i in range(20):
            for j in range(15):
                if self.board[i][j] == 2:
                    return False
        return True

    def explore(self):
        sensor_data = [0,0,0,0]
        # TODO: current_position and current_direction should have flexibility in UI
        current_position = [0,0] # position is tuple in shortest path
        current_direction = Directions.EAST # TODO: should have flexibility in UI
        
        while (True):
            # receiving message
            self.read_sensor(sensor_data) 
            # wall has been updated in the map
            current_direction, next_coordinate = self.determine_direction()
            current_position = next_coordinate
            if (self.fully_explored()):
                break
        
        #TODO return board
        return "TODO explored board"
        

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

if __name__ == '__main__':

    explore = ExplorationProblem()
    result = explore.explore()

    print(result)