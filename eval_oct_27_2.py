        myFeatures, otherFeatures = self.computeFeatures(myMapper, otherMapper, currentTurn, currdepth)

        if len(myMapper) + len(otherMapper) <= 3:
            whoseWeights = self.endGameWeights
            if len(myMapper) > len(otherMapper):
                myScore = 0
                otherScore = 0
                for key, value in myFeatures.items():
                    myScore += value * whoseWeights[key]
                for key, value in otherFeatures.items():
                    otherScore += value * whoseWeights[key]
                coords = list(myMapper.keys())
                target = list(otherMapper.keys())[0]
                for coord in coords:
                    myScore -= (abs(coord[0] - target[0]) + abs(coord[1] - target[1])) * 25

                if len(coords) > 1:
                    myScore -= (abs(coords[0][0] - coords[1][0]) + abs(coords[0][1] - coords[1][1])) * 25
            else:
                myScore = 0
                otherScore = 0
                for key, value in myFeatures.items():
                    myScore += value * whoseWeights[key]
                for key, value in otherFeatures.items():
                    otherScore += value * whoseWeights[key]
                coords = list(otherMapper.keys())
                target = list(myMapper.keys())[0]
                for coord in coords:
                    otherScore -= (abs(coord[0] - target[0]) + abs(coord[1] - target[1]))
                if len(coords) > 1:
                    otherScore -= (abs(coords[0][0] - coords[1][0]) + abs(coords[0][1] - coords[1][1])) * 25
        else:
            if whoseWeights is None:
                whoseWeights = self.featureWeights
            myScore = 0
            otherScore = 0
            for key, value in myFeatures.items():
                myScore += value * whoseWeights[key]
            for key, value in otherFeatures.items():
                otherScore += value * whoseWeights[key]

        multiplierAdd = int(len(myMapper) * 150 // len(otherMapper))
        if multiplierAdd > 150:
            myScore += multiplierAdd
            otherScore -= multiplierAdd
        elif multiplierAdd < 150:
            otherScore += multiplierAdd
            myScore -= multiplierAdd

        return ((myScore - otherScore) << 4) + np.random.randint(0, 15)