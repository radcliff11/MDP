'''
Author: Huangfu Qingchuan (c160135@ntu.edu.sg)
NTU SCSE MDP AY1920 S1 Group 24

'''

import sys

wall_config = '20_15_test.txt'
list = []
try:
	with open(wall_config, 'r') as f:
		file = f.readlines()
		for i in range(20):
			x = []
			for j in range(15):
				if int(file[i][j]) == 0:
					flag = 0
				else:
					flag = 1
				x.append(flag)
			list.append(x)
		
		for i in range(20):
			print(list[i])
		print()

		for i in range(20):
			for j in range(15):
				if i ==0 or i == 19 or j == 0 or j == 14:
					if list[i][j] == 0:
						list[i][j] = 2
				elif list[i][j] == 1:
					for x1 in range(-1,2):
						for x2 in range(-1,2):
							if list[i+x1][j+x2] == 0:
								list[i+x1][j+x2] = 2
			
except Exception as e:
	print(e)
	sys.exit()

for i in range(20):
	print(list[i] )
