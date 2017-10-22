import copy
import numpy as np
import re
import signal
import sys
from termcolor import cprint, colored
import time

class TimeoutException(Exception):
	pass

def timeout_handler(signum, frame):
	raise TimeoutException

class StateModel():
	def __init__(self, mode, color, startPosDefault, startPosRed, startPosYellow, timeLimit):
		"""
		mode - 1: Player vs. Player, 2: Player vs Computer, 3: Computer vs Computer
		color - 1: Red, 2: Yellow
		startPosDefault - True: use default starting boards for checkers, False: look at startPosRed and
			startPosYellow for the starting boards
		startPosRed - An array of a tuple of values indicating where the red pieces are.  The tuple is defined as:
			(X_coordY_coord, King?)
		startPosYellow - Same as startPosRed but for the yellow pieces
		timeLimit - the time limit afforded to the model for alpha-beta search
		"""
		self.mode = mode
		self.color = color
		self.reachedStates = []
		if startPosDefault == 1:
			#red is lower indices, yellow is higher indices
			self.redMapper = {'identifier': 'red', (1, 0) : 0, (3, 0) : 0, (5, 0) : 0, (7, 0) : 0,
				(0, 1) : 0, (2, 1) : 0, (4, 1) : 0, (6, 1) : 0,
				(1, 2) : 0, (3, 2) : 0, (5, 2) : 0, (7, 2) : 0}
			self.yellowMapper = {'identifier': 'yellow', (0, 7) : 0, (2, 7) : 0, (4, 7) : 0, (6, 7) : 0,
				(1, 6) : 0, (3, 6) : 0, (5, 6) : 0, (7, 6) : 0,
				(0, 5) : 0, (2, 5) : 0, (4, 5) : 0, (6, 5): 0}
		else:
			self.redMapper = {}
			self.yellowMapper = {}
			for tup in startPosRed:
				coord = self.getLocation(tup[0])
				if coord[1] == 7:
					self.redMapper[coord] = 1
				else:
					self.redMapper[coord] = tup[1]
			for tup in startPosYellow:
				coord = self.getLocation(tup[0])
				if coord[1] == 0:
					self.yellowMapper[coord] = 1
				else:
					self.yellowMapper[coord] = tup[1]

		if len(self.redMapper) == 0 or len(self.yellowMapper) == 0:
			self.gameIsFinished = True
		else:
			self.gameIsFinished = False

		self.diag = set([(7, 0), (6, 1), (5, 2), (4, 3), (3, 4),
			(2, 5), (1, 6), (0, 7)])
		self.doublediag = set([(1, 0), (2, 1), (3, 2), (4, 3), (5, 4), (6, 5), (7, 6),
			(0, 1), (1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 7)])
		self.doublecorner = set([(1, 0), (0, 1), (6, 7), (7, 6)])
		self.timeLimit = timeLimit

	def getLocation(self, location):
		return (ord(location[0]) - ord('A'), int(location[1]) - 1)

	def printEmpty(self, backgroundStart):
		background = backgroundStart
		print('    ', end="")
		for _ in range(8):
			print('|', end="")
			cprint('     ', 'yellow', background, end="")
			if background == 'on_grey':
				background = 'on_blue'
			else:
				background = 'on_grey'
		print('|')

	def printBoard(self):
		divider = '    ' + '+-----'*8 + '+'
		background = 'on_grey'
		for rowindex in range(8):
			print(divider)
			orig_background = background
			self.printEmpty(orig_background)
			print(str(rowindex+1) + " : ", end="")
			for colindex in range(8):
				print('|', end="")
				if (colindex, rowindex) in self.redMapper:
					if self.redMapper[(colindex, rowindex)] == 0:
						cprint('  O  ', 'red', background, end="")
					else:
						cprint('  K  ', 'red', background, end="")
				elif (colindex, rowindex) in self.yellowMapper:
					if self.yellowMapper[(colindex, rowindex)] == 0:
						cprint('  0  ', 'yellow', background, end="")
					else:
						cprint('  K  ', 'yellow', background, end="")
				else:
					cprint('     ', 'yellow', background, end="")
				if background == 'on_grey':
					background = 'on_blue'
				else:
					background = 'on_grey'
			print('|')
			self.printEmpty(orig_background)
		print(divider)
		print('    ' + '   A  ' + '   B  ' + '   C  ' + '   D  ' + '   E  ' + '   F  ' + '   G  ' + '   H  ')

	def coordInBoard(self, coord):
		if 0 <= coord[0] <= 7 and 0 <= coord[1] <= 7:
			return True
		else:
			return False

	def getValidMovesAfterJump(self, currentTurn, coord, king, myMapper, otherMapper):
		validMoves = []
		if currentTurn == 'red':
			possibilities = []
			possibilities.append((coord[0] - 1, coord[1] + 1))
			possibilities.append((coord[0] + 1, coord[1] + 1))
			if king == 1:
				possibilities.append((coord[0] - 1, coord[1] - 1))
				possibilities.append((coord[0] + 1, coord[1] - 1))
		else:
			possibilities = []
			possibilities.append((coord[0] - 1, coord[1] - 1))
			possibilities.append((coord[0] + 1, coord[1] - 1))
			if king == 1:
				possibilities.append((coord[0] - 1, coord[1] + 1))
				possibilities.append((coord[0] + 1, coord[1] + 1))

		for possibility in possibilities:
			if self.coordInBoard(possibility):
				if possibility in otherMapper:
					#if the hop is a different color
					if possibility[0] < coord[0]:
						if possibility[1] > coord[1]:
							#approached it by going southwest
							newCoord = (possibility[0] - 1, possibility[1] + 1)
						else:
							#approached it by going northwest
							newCoord = (possibility[0] - 1, possibility[1] - 1)
					else:
						if possibility[1] > coord[1]:
							#approached it by going southeast
							newCoord = (possibility[0] + 1, possibility[1] + 1)
						else:
							#approached it by going northeast
							newCoord = (possibility[0] + 1, possibility[1] - 1)

					if self.coordInBoard(newCoord):
						#if the new coordinate is also in the board
						if newCoord not in myMapper and newCoord not in otherMapper:
							#the new coordinate is a blank
							newOtherMapper = dict(otherMapper)
							del newOtherMapper[possibility]
							startCoord = chr(ord('A')+coord[0]) + chr(ord('1')+coord[1])
							endCoord = chr(ord('A')+newCoord[0]) + chr(ord('1')+newCoord[1])
							currentMove = startCoord + '->' + endCoord
							newMoves = self.getValidMovesAfterJump(currentTurn, newCoord, king, myMapper, newOtherMapper)
							if len(newMoves) == 0:
								validMoves.append(currentMove)
							else:
								for move in newMoves:
									validMoves.append(currentMove[:-2]+move)
		return validMoves

	def getValidMovesForCoord(self, currentTurn, coord, king, myMapper, otherMapper, jumpFound=False):
		validMoves = []
		if currentTurn == 'red':
			possibilities = []
			possibilities.append((coord[0] - 1, coord[1] + 1))
			possibilities.append((coord[0] + 1, coord[1] + 1))
			if king == 1:
				possibilities.append((coord[0] - 1, coord[1] - 1))
				possibilities.append((coord[0] + 1, coord[1] - 1))
		else:
			possibilities = []
			possibilities.append((coord[0] - 1, coord[1] - 1))
			possibilities.append((coord[0] + 1, coord[1] - 1))
			if king == 1:
				possibilities.append((coord[0] - 1, coord[1] + 1))
				possibilities.append((coord[0] + 1, coord[1] + 1))

		for possibility in possibilities:
			if self.coordInBoard(possibility):
				if possibility not in myMapper:
					# the hop doesn't land on a same piece
					if possibility not in otherMapper:
						# the hop lands on a blank, add that to the valid moveset and possible moves for the AI
						startCoord = chr(ord('A')+coord[0]) + chr(ord('1')+coord[1])
						endCoord = chr(ord('A')+possibility[0]) + chr(ord('1')+possibility[1])
						if not jumpFound:
							validMoves.append(startCoord + '->' + endCoord)
					else:
						# the hop lands on a different piece, so check if further nodes are blank
						if possibility[0] < coord[0]:
							if possibility[1] > coord[1]:
								#approached it by going southwest
								newCoord = (possibility[0] - 1, possibility[1] + 1)
							else:
								#approached it by going northwest
								newCoord = (possibility[0] - 1, possibility[1] - 1)
						else:
							if possibility[1] > coord[1]:
								#approached it by going southeast
								newCoord = (possibility[0] + 1, possibility[1] + 1)
							else:
								#approached it by going northeast
								newCoord = (possibility[0] + 1, possibility[1] - 1)

						if self.coordInBoard(newCoord):
							#if the new coordinate is also in the board
							if newCoord not in myMapper and newCoord not in otherMapper:
								#the new coordinate is a blank
								if not jumpFound:
									validMoves = []
									jumpFound = True

								newOtherMapper = dict(otherMapper)
								del newOtherMapper[possibility]
								startCoord = chr(ord('A')+coord[0]) + chr(ord('1')+coord[1])
								endCoord = chr(ord('A')+newCoord[0]) + chr(ord('1')+newCoord[1])
								currentMove = startCoord + '->' + endCoord
								newMoves = self.getValidMovesAfterJump(currentTurn, newCoord, king, myMapper, newOtherMapper)
								if len(newMoves) == 0:
									validMoves.append(currentMove)
								else:
									for move in newMoves:
										validMoves.append(currentMove[:-2]+move)
		return validMoves, jumpFound

	def getValidMoves(self, currentTurn, myMapper, otherMapper):
		nonJumpMoves = []
		jumpMoves = []
		for coord, king in myMapper.items():
			if type(coord) is str:
				continue
			moves, jumpFound = self.getValidMovesForCoord(currentTurn, coord, king, myMapper, otherMapper)
			if jumpFound:
				jumpMoves += moves
			else:
				nonJumpMoves += moves

		if len(jumpMoves) != 0:
			return jumpMoves
		else:
			return nonJumpMoves

	def startGame(self):
		if self.mode == 1:
			self.turnStyle = {'red': 'player', 'yellow': 'player'}
		elif self.mode == 2 and self.color == 1:
			self.turnStyle = {'red': 'player', 'yellow': 'computer'}
		elif self.mode == 2 and self.color == 2:
			self.turnStyle = {'red': 'computer', 'yellow': 'player'}
		else:
			self.turnStyle = {'red': 'computer', 'yellow': 'computer'}

		self.currentTurn = 'red'
		while not self.gameIsFinished:
			self.executeTurn()

		if self.currentTurn == 'red':
			text = colored('yellow won', 'yellow')
		else:
			text = colored('red won', 'red')
		print(text)

	def coordOnBottom(self, coord, pieceColor):
		if pieceColor == 'red':
			if coord[1] == 0:
				return True
			else:
				return False
		else:
			if coord[1] == 7:
				return True
			else:
				return False

	def coordOnSide(self, coord):
		return coord[0] == 0 or coord[1] == 7

	def coordAttacking(self, coord, pieceColor):
		if pieceColor == 'red':
			if 5 <= coord[1] <= 7:
				return True
			else:
				return False
		else:
			if 0 <= coord[1] <= 2:
				return True
			else:
				return False

	def computeFeatures(self, myMapper, otherMapper, currentTurn, currdepth):
		if currentTurn == 'red':
			redMapper = myMapper
			yellowMapper = otherMapper
		else:
			redMapper = otherMapper
			yellowMapper = myMapper

		redFeatures = {'pawns': 0, 'kings': 0, 'safePawns': 0, 'safeKings': 0,
			'moveablePawns': 0, 'moveableKings': 0, 'distanceToKing': 0, 'defenderPieces': 0,
			'kingLine': 0, 'centerPieces': 0}
		yellowFeatures = {'pawns': 0, 'kings': 0, 'safePawns': 0, 'safeKings': 0,
			'moveablePawns': 0, 'moveableKings': 0, 'distanceToKing': 0, 'defenderPieces': 0,
			'kingLine': 0, 'centerPieces': 0}

		for coord, king in redMapper.items():
			nonJumpMoves, jumpFound = self.getValidMovesForCoord(currentTurn,
				coord, king, myMapper, otherMapper, jumpFound=False)
			if king == 0:
				redFeatures['pawns'] += 1
				if coord[0] == 0 or coord[0] == 7:
					redFeatures['safePawns'] += 1
				if !jumpFound:
					redFeatures['moveablePawns'] += 1
				redFeatures['distanceToKing'] += 7 - coord[0]
			else:
				redFeatures['kings'] += 1
				if coord[0] == 0 or coord[0] == 7:
					redFeatures['safeKings'] += 1
				if !jumpFound:
					redFeatures['moveableKings'] += 1
			if coord[1] == 0:
				redFeatures['kingLine'] += 1
			if 0 <= coord[1] <= 1:
				redFeatures['defenderPieces'] += 1
			if 2 <= coord[0] <= 5 and 2 <= coord[1] <= 5:
				redFeatures['centerPieces'] += 1
				


	def evaluateBoard(self, myMapper, otherMapper, currentTurn, currdepth):


	def findBestMove(self):
		if self.currentTurn == 'red':
			myMapper = self.redMapper
			otherMapper = self.yellowMapper
		else:
			myMapper = self.yellowMapper
			otherMapper = self.redMapper

		timeEnd = time.time() + self.timeLimit
		depthLimit = 2
		move = None
		signal.signal(signal.SIGVTALRM, timeout_handler)
		signal.setitimer(signal.ITIMER_VIRTUAL, self.timeLimit, 1000000)
		try:
			while True:
				move = self.alphaBetaSearch(self.currentTurn, myMapper, otherMapper, depthLimit)
				#print("Move:", move)
				# if move != None:
				# 	print("Here")
				depthLimit += 1
		except TimeoutException:
			signal.setitimer(signal.ITIMER_VIRTUAL, 0)
			if move != None:
				print("Depth:", depthLimit - 1)
				return move
			else:
				raise ValueError('Best move is None!')

	def alphaBetaSearch(self, currentTurn, myMapper, otherMapper, depthLimit):
		v, move = self.maxValue(currentTurn, myMapper, otherMapper, -float("inf"), float("inf"), 1, depthLimit)
		return move

	def maxValue(self, currentTurn, myMapper, otherMapper, alpha, beta, currdepth, depthLimit):
		#try:
		if currdepth == depthLimit:
			return self.evaluateBoard(myMapper, otherMapper, currentTurn, currdepth), None
		v = -sys.maxsize + 1
		maxmove = None
		moves = self.getValidMoves(currentTurn, myMapper, otherMapper)
		if len(moves) == 0:
			return -sys.maxsize + 1 + currdepth, None
		if len(otherMapper) == 0:
			return sys.maxsize - currdepth, None

		for move in moves:
			# if currdepth == 1:
			# 	print("V:", v, "Maxmove:", maxmove, "Move:", move, "Currturn:", currentTurn)
			newMyMapper = dict(myMapper)
			newOtherMapper = dict(otherMapper)
			self.executeMove(currentTurn, move, newMyMapper, newOtherMapper)
			if currentTurn == 'red':
				#max is red
				nextCurrentTurn = 'yellow'
			else:
				nextCurrentTurn = 'red'
			newV, newmove = self.minValue(nextCurrentTurn, newOtherMapper, newMyMapper, alpha, beta, currdepth+1, depthLimit)
			if newV > v:
				v = newV
				maxmove = move
			# elif newV == v:
			# 	if len(newmove) < maxmove:
			# 		maxmove = newmove
			if v >= beta:
				return v+1, maxmove
			alpha = max(alpha, v)
		return v, maxmove
		#except Exception:
		#	print("Currdepth", currdepth, "Move:", move, "myMapper:", myMapper, "otherMapper:", otherMapper)
		#	raise ValueError("error")

	def minValue(self, currentTurn, myMapper, otherMapper, alpha, beta, currdepth, depthLimit):
		#try:
		if currdepth == depthLimit:
			return self.evaluateBoard(otherMapper, myMapper, currentTurn, currdepth), None
		v = sys.maxsize
		minmove = None
		moves = self.getValidMoves(currentTurn, myMapper, otherMapper)
		if len(moves) == 0:
			return sys.maxsize - currdepth, None
		if len(otherMapper) == 0:
			return -sys.maxsize + 1 + currdepth, None

		for move in moves:
			# if currdepth == 2:
			# 	print("Minv:", v, "minmove:", minmove, "move:", move, "all moves:", moves)
			newMyMapper = dict(myMapper)
			newOtherMapper = dict(otherMapper)
			self.executeMove(currentTurn, move, newMyMapper, newOtherMapper)
			if currentTurn == 'red':
				#min is red
				nextCurrentTurn = 'yellow'
			else:
				nextCurrentTurn = 'red'
			newV, newmove = self.maxValue(nextCurrentTurn, newOtherMapper, newMyMapper, alpha, beta, currdepth+1, depthLimit)
			if newV < v:
				v = newV
				minmove = move
			# elif newV == v:
			# 	if len(newmove) < maxmove:
			# 		minmove = newmove
			if v <= alpha:
				return v-1, minmove
			beta = min(beta, v)
		return v, minmove
		#except Exception:
		#	print("Currdepth:", currdepth, "Move:", move, "myMapper:", myMapper, "otherMapper:", otherMapper)
		#	raise ValueError("error")

	def executeMove(self, currentTurn, move, myMapper, otherMapper):
		moves = re.split('->', move)
		startLocation = self.getLocation(moves[0])
		endLocation = self.getLocation(moves[-1])
		king = myMapper[startLocation]
		del myMapper[startLocation]
		myMapper[endLocation] = king
		if currentTurn == 'red':
			if endLocation[1] == 7:
				myMapper[endLocation] = 1
		else:
			if endLocation[1] == 0:
				myMapper[endLocation] = 1

		if len(moves) > 2:
			# a jump was involved
			for index in range(len(moves)-1):
				startLocation = self.getLocation(moves[index])
				endLocation = self.getLocation(moves[index+1])
				midLocation = (int((startLocation[0] + endLocation[0])/2), int((startLocation[1] + endLocation[1])/2))
				del otherMapper[midLocation]
		else:
			if abs(startLocation[0] - endLocation[0]) == 2:
				#a jump was involved
				midLocation = (int((startLocation[0] + endLocation[0])/2), int((startLocation[1] + endLocation[1])/2))
				del otherMapper[midLocation]

	def executeTurn(self):
		self.printBoard()
		text = colored(self.currentTurn + "'s turn", self.currentTurn)
		print(text)

		if self.currentTurn == 'red':
			myMapper = self.redMapper
			otherMapper = self.yellowMapper
		else:
			myMapper = self.yellowMapper
			otherMapper = self.redMapper

		self.validMoves = self.getValidMoves(self.currentTurn, myMapper, otherMapper)
		if len(self.validMoves) == 0 or len(myMapper) == 0 or len(otherMapper) == 0:
			self.gameIsFinished = True
			return

		if self.turnStyle[self.currentTurn] == 'player':
			#player Turn
			move = self.handlePlayerTurn()
			if move == 'U':
				try:
					self.redMapper, self.yellowMapper, self.currentTurn = self.reachedStates.pop()
				except Exception:
					print("No moves to undo!")
				return
			elif move == 'UU':
				try:
					if not len(self.reachedStates) >= 2:
						print("Not enough moves to undo")
						return
					self.reachedStates.pop()
					self.redMapper, self.yellowMapper, self.currentTurn = self.reachedStates.pop()
				except Exception:
					print("No moves to undo!")
				return
			elif move == 'AI':
				if len(self.validMoves) == 1:
					ai_move = self.validMoves[0]
				else:
					ai_move = self.findBestMove()
				print(ai_move)
				return
			self.reachedStates.append((dict(self.redMapper), dict(self.yellowMapper), self.currentTurn))
			self.executeMove(self.currentTurn, move, myMapper, otherMapper)
		else:
			#computer Turn
			self.reachedStates.append((dict(self.redMapper), dict(self.yellowMapper), self.currentTurn))
			if len(self.validMoves) == 1:
				move = self.validMoves[0]
			else:
				move = self.findBestMove()
			self.executeMove(self.currentTurn, move, myMapper, otherMapper)

		if self.currentTurn == 'red':
			self.currentTurn = 'yellow'
		else:
			self.currentTurn = 'red'

	def handlePlayerTurn(self):
		while True:
			try:
				move = input("Enter a move like follows - A5->B6 - or enter in 'H' for a" \
					+ "list of all possible moves or 'U' for undoing your move once or 'UU' for twice:" \
					+ "or AI for seeing what the AI would do in your spot: ")
				if move == 'H':
					print('\n'.join(self.validMoves))
				elif move == 'U':
					break
				elif move == 'UU':
					break
				elif move == 'AI':
					break
				else:
					if not move in self.validMoves:
						raise ValueError("Please enter a valid, properly formatted move.")
					break
			except Exception:
				print("Please enter a valid, properly formatted move.")
		print("Returning move:", move)
		return move

while True:
	try:
		mode = int(input("Choose Mode: (1)Player vs. Player, (2)Player vs. Computer, (3)Computer vs Computer: "))
		if not 0 < mode < 4:
			raise ValueError('Mode should be in the range [1, 3]')
		break
	except Exception:
		print("Please enter an integer value for the mode in the range [1, 3]")

color = 1
if mode != 3:
	while True:
		try:
			color = int(input("Choose Color: (1)Red, (2)Yellow. Red goes first: "))
			if not 0 < color < 3:
				raise ValueError('Color should be in the range [1, 2]')
			break
		except Exception:
			print("Please enter an integer value for the color in the range [1, 2]")

while True:
	try:
		startPosDefault = int(input("Starting Position: (1)Default or (2)Custom: "))
		if not 0 < startPosDefault < 3:
			raise ValueError('Choose a number in the range [1, 2]')
		break
	except Exception:
		print("Please enter an integer value within in the range [1, 2]")

def locationCheck(location):
	letters = set(['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'])
	positions = set(range(1, 9))
	if len(location) != 2:
		return False
	if location[0] not in letters:
		return False
	try:
		if int(location[1]) not in positions:
			return False
	except Exception:
		return False
	return True

startPosRed = None
startPosYellow = None
if startPosDefault != 1:
	while True:
		try:
			redPositionsItems = re.split(' ', input("Input starting locations for Red: "))
			redLocations = redPositionsItems[0::2]
			redKings = list(map(int, redPositionsItems[1::2]))
			if len(set(redLocations)) != len(redLocations):
				raise ValueError('Each location should be unique')
			if len(redLocations) > 12:
				raise ValueError('Each player can only have a maximum of 12 pieces')
			for location in redLocations:
				if not locationCheck(location):
					raise ValueError("The locations aren't specified properly.")
			miniking = min(redKings)
			maxking = max(redKings)
			if miniking != 0 and miniking != 1:
				raise ValueError('King indicator should be either 0 or 1')
			if maxking != 0 and maxking != 1:
				raise ValueError('King indicator should be either 0 or 1')
			startPosRed = list(zip(redLocations, redKings))
			break
		except Exception:
			print("Please enter a maximum of 12 unique locations and king indicators in the format \
				Location1 KingIndicator1 Location2 KingIndicator2 ...  The king indicators are either \
				0 for 'not a king' or 1 for 'king'")

	while True:
		try:
			yellowPositionsItems = re.split(' ', input("Input starting locations for Yellow: "))
			yellowLocations = yellowPositionsItems[0::2]
			yellowKings = list(map(int, yellowPositionsItems[1::2]))
			if len(set(yellowLocations)) != len(yellowLocations):
				raise ValueError('Each location should be unique')
			if len(yellowLocations) > 12:
				raise ValueError('Each player can only have a maximum of 12 pieces')
			for location in yellowLocations:
				if not locationCheck(location):
					raise ValueError("The locations aren't specified properly.")
				if location in redLocations:
					raise ValueError("One of the locations was already used by red.")
			miniking = min(yellowKings)
			maxking = max(yellowKings)
			if miniking != 0 and miniking != 1:
				raise ValueError('King indicator should be either 0 or 1')
			if maxking != 0 and maxking != 1:
				raise ValueError('King indicator should be either 0 or 1')
			startPosYellow = list(zip(yellowLocations, yellowKings))
			break
		except Exception:
			print("Please enter a maximum of 12 unique locations and king indicators in the format \
				Location1 KingIndicator1 Location2 KingIndicator2 ...  The king indicators are either 0 \
				for 'not a king' or 1 for 'king'")

while True:
	try:
		timeLimit = float(input("Enter the time limit for AI search: "))
		if not timeLimit > 0:
			raise ValueError('Enter a time limit greater than 0 seconds.')
		break
	except Exception:
		print("Please enter a float value greater than 0 seconds.")

stateModel = StateModel(mode=mode,
	color=color,
	startPosDefault=startPosDefault,
	startPosRed=startPosRed,
	startPosYellow=startPosYellow,
	timeLimit=timeLimit)

stateModel.startGame()