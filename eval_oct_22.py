		baseScore = 300
		kingScore = 500

		myScore = 0
		otherScore = 0
		defenders = 0
		attackers = 0
		for coord, king in myMapper.items():
			addedValue = 0
			if type(coord) is str:
				continue
			if self.coordOnBottom(coord, currentTurn):
				addedValue = 150
			elif coord in self.doublecorner:
				addedValue = 120
			elif self.coordOnSide(coord):
				addedValue = 80
			elif coord in self.diag:
				addedValue = 50
			elif coord in self.doublediag:
				addedValue = 40
			# elif self.coordAttacking(coord, currentTurn):
			# 	addedValue = 90
			if currentTurn == 'red': #myMapper is red
				sub = (7 - coord[1])*20
				if coord[1] <= 1:
					defenders += 1
				if coord[1] >= 5 and king == 0:
					attackers += 1
			else:
				sub = coord[1]*20
				if coord[1] >= 6:
					defenders += 1
				if coord[1] <= 2 and king == 0:
					attackers += 1
			if king == 0:
				myScore += baseScore + addedValue - sub
			else:
				myScore += kingScore + addedValue
		# myScor += 30 *e defenders
		# myScore += 30 * attackers

		defenders = 0
		attackers = 0
		for coord, king in otherMapper.items():
			addedValue = 0
			colorToUse = 'red' if currentTurn == 'yellow' else 'yellow'
			if type(coord)is str:
				continue
			if self.coordOnBottom(coord, colorToUse):
				addedValue = 150
			elif coord in self.doublecorner:
				addedValue = 120
			elif self.coordOnSide(coord):
				addedValue = 80
			elif coord in self.diag:
				addedValue = 50
			elif coord in self.doublediag:
				addedValue = 40
			# elif self.coordAttacking(coord, currentTurn):
			# 	addedValue = 90
			if currentTurn == 'red': #otherMapper is yellow
				sub = coord[1]*20
				if coord[1] >= 6:
					defenders += 1
				if coord[1] <= 2 and king == 0:
					attackers += 1
			else:
				sub = (7 - coord[1])*20
				if coord[1] <= 1:
					defenders += 1
				if coord[1] >= 5 and king == 0:
					attackers += 1
			if king == 0:
				otherScore += baseScore + addedValue - sub
			else:
				otherScore += kingScore + addedValue
		# otherScore += 30 * defenders
		# otherScore += 30 * attackers

		multiplierAdd = int(len(myMapper) * 300 / len(otherMapper))
		if multiplierAdd > 300:
			myScore += multiplierAdd
			otherScore -= multiplierAdd
		elif multiplierAdd < 300:
			otherScore += multiplierAdd
			myScore -= multiplierAdd
		
		return ((myScore - otherScore) << 4) + np.random.randint(low=0, high=15)