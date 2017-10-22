	def evaluateBoard(self, myMapper, otherMapper, currentTurn, currdepth):
		baseScore = 300
		kingScore = 500

		myScore = 0
		otherScore = 0
		for coord, king in myMapper.items():
			addedValue = 0
			if type(coord) is str:
				continue
			if self.coordOnBottom(coord, currentTurn):
				addedValue = 100
			if self.coordOnMajDiag(coord):
				addedValue = 80
			if currentTurn == 'red':
				sub = (7 - coord[1])*20
			else:
				sub = coord[1]*20
			if king == 0:
				myScore += baseScore + addedValue - sub
			else:
				myScore += kingScore + addedValue

		for coord, king in otherMapper.items():
			addedValue = 0
			if type(coord)is str:
				continue
			if self.coordOnBottom(coord, currentTurn):
				addedValue = 100
			if self.coordOnMajDiag(coord):
				addedValue = 80
			if currentTurn == 'red':
				sub = coord[1]*20
			else:
				sub = (7 - coord[1])*20
			if king == 0:
				otherScore += baseScore + addedValue - sub
			else:
				otherScore += kingScore + addedValue

		multiplierAdd = int(len(myMapper) * 300 / len(otherMapper))
		if multiplierAdd > 300:
			myScore += multiplierAdd
			otherScore -= multiplierAdd
		elif multiplierAdd < 300:
			otherScore += multiplierAdd
			myScore -= multiplierAdd
		
		return ((myScore - otherScore) << 8) + np.random.randint(low=0, high=5)