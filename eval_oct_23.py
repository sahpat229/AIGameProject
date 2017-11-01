		myFeatures, otherFeatures = self.computeFeatures(myMapper, otherMapper, currentTurn, currdepth)
		myScore = 0
		otherScore = 0
		for key, value in myFeatures.items():
			myScore += value * self.featureWeights[key]
		for key, value in otherFeatures.items():
			otherScore += value * self.featureWeights[key]

		multiplierAdd = int(len(myMapper) * 300 / len(otherMapper))
		if multiplierAdd > 300:
			myScore += multiplierAdd
			otherScore -= multiplierAdd
		elif multiplierAdd < 300:
			otherScore += multiplierAdd
			myScore -= multiplierAdd

		return ((myScore - otherScore) << 4) + np.random.randint(low=0, high=15)