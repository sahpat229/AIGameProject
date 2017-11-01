    def evaluateBoard(self, myMapper, otherMapper, currentTurn, currdepth, whoseWeights=None):
        myFeatures, otherFeatures = self.computeFeatures(myMapper, otherMapper, currentTurn, currdepth)
        #print("MyFeatures:", myFeatures, "OtherFeatures:", otherFeatures, "whoseWeights:", whoseWeights)
        myScore = 0
        otherScore = 0
        if whoseWeights is None:
            whoseWeights = self.featureWeights
        for key, value in myFeatures.items():
            myScore += value * whoseWeights[key]
        for key, value in otherFeatures.items():
            otherScore += value * whoseWeights[key]

        multiplierAdd = int(len(myMapper) * 300 / len(otherMapper))
        if multiplierAdd > 300:
            myScore += multiplierAdd
            otherScore -= multiplierAdd
        elif multiplierAdd < 300:
            otherScore += multiplierAdd
            myScore -= multiplierAdd

        return ((myScore - otherScore) << 4) + random.randint(0, 15)