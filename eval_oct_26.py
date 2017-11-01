    def evaluateBoard(self, myMapper, otherMapper, currentTurn, currdepth, whoseWeights=None):
        if currentTurn == 'red':
            redMapper = myMapper
            yellowMapper = otherMapper
            otherTurn = 'yellow'
        else:
            yellowMapper = myMapper
            redMapper = otherMapper
            otherTurn = 'red'

        redScore = 0
        yellowScore = 0
        pawnScore = 600
        kingScore = 1000
        doubleCornerScore = 150
        bottomScore = 125
        centerScore = 80
        sideScore = 70
        diagScore = 60
        pawnAdvance = 20
        for coord, king in redMapper.items():
            if type(coord) is str:
                continue
            addedValue = 0
            if coord in self.doublecorner:
                addedValue = doubleCornerScore
            elif coord[1] == 0: # coord's on the bottom
                addedValue = bottomScore
            elif 2 <= coord[0] <= 5 and 2 <= coord[1] <= 5:
                addedValue = centerScore
            elif coord[0] == 0 or coord[0] == 7:
                addedValue = sideScore
            elif coord in self.diag:
                addedValue = diagScore
            sub = (7 - coord[1]) * pawnAdvance

            if king == 0:
                redScore += pawnScore + addedValue - sub
            else:
                redScore += kingScore + addedValue

        for coord, king in yellowMapper.items():
            if type(coord) is str:
                continue
            addedValue = 0
            if coord in self.doublecorner:
                addedValue = doubleCornerScore
            elif coord[1] == 7: # coord's on the bottom
                addedValue = bottomScore
            elif 2 <= coord[0] <= 5 and 2 <= coord[1] <= 5:
                addedValue = centerScore
            elif coord[0] == 0 or coord[0] == 7:
                addedValue = sideScore
            elif coord in self.diag:
                addedValue = diagScore
            sub = coord[1] * pawnAdvance

            if king == 0:
                yellowScore += pawnScore + addedValue - sub
            else:
                yellowScore += kingScore + addedValue

        if currentTurn == 'red':
            myScore = redScore
            otherScore = yellowScore
        else:
            myScore = yellowScore
            otherScore = redScore

        multiplierAdd = int(len(myMapper) * 300 / len(otherMapper))
        if multiplierAdd > 300:
            myScore += multiplierAdd
            otherScore -= multiplierAdd
        elif multiplierAdd < 300:
            otherScore += multiplierAdd
            myScore -= multiplierAdd

        return ((myScore - otherScore) << 4) + np.random.randint(0, 15)