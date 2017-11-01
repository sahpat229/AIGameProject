#ifndef _STATEMODEL
#define _STATEMODEL

#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "input_functions.h"
#include <chrono>

using namespace std;
using Coord = pair<int, int>;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;  
    }
};

class StateModel {
public:
    StateModel(int mode, int color, int pos, vector<pair<string, int>> redPieces,
               vector<pair<string, int>> yellowPieces, int timeLimit);
    pair<int, int> getLocation(const string &location);
    void printBoard();
    bool coordInBoard(pair<int, int> coord);
    vector<vector<Coord> *> *getValidMovesAfterJump(bool currentTurn, const Coord &coord, int king,
                                                  unordered_map<Coord, int, pair_hash> &myMapper,
                                                  unordered_map<Coord, int, pair_hash> &otherMapper);
    pair<vector<vector<Coord> *> *, bool> getValidMovesForCoord(bool currentTurn, const Coord &coord, int king,
                                                             unordered_map<Coord, int, pair_hash> &myMapper,
                                                             unordered_map<Coord, int, pair_hash> &otherMapper);
    vector<vector<Coord> *> *getValidMoves(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                         unordered_map<Coord, int, pair_hash> &otherMapper);

    void startGame();
    void executeTurn();
    vector<Coord> *handlePlayerTurn(vector<vector<Coord> *> *validMoves);
    void executeMove(bool currentTurn, vector<Coord> *move, unordered_map<Coord, int, pair_hash> &myMapper,
                     unordered_map<Coord, int, pair_hash> &otherMapper);
    void displayMove(vector<Coord> *move);
    vector<Coord> *convertToCoord(vector<string> coords);
    unordered_map<Coord, int, pair_hash> *getMyMapper();
    unordered_map<Coord, int, pair_hash> *getOtherMapper();
    void printEmpty(bool background);
    vector<Coord> *alphaBetaSearch(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                   unordered_map<Coord, int, pair_hash> &otherMapper,
                                   int depthLimit, bool debug, chrono::system_clock::time_point &endTime); 
    pair<int, vector<Coord> *>maxValue(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                       unordered_map<Coord, int, pair_hash> &otherMapper,
                                       int alpha, int beta, int currdepth, int depthLimit,
                                       vector<Coord> *moveTaken, bool debug, chrono::system_clock::time_point &endTime);
    pair<int, vector<Coord> *>minValue(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                       unordered_map<Coord, int, pair_hash> &otherMapper,
                                       int alpha, int beta, int currdepth, int depthLimit,
                                       vector<Coord> *moveTaken, bool debug, chrono::system_clock::time_point &endTime);
    int evaluateBoard(bool currentTurn,
                      unordered_map<Coord, int, pair_hash> &myMapper,
                      unordered_map<Coord, int, pair_hash> &otherMapper,
                      int currdepth, bool fromMax);
    vector<Coord> *findBestMove();
    void displayMoveInline(vector<Coord> *move);
    int computeManhattan(vector<Coord> &largerCoords, vector <Coord> &smallerCoords);

private:
    int mode;
    int color;
    unordered_map<Coord, int, pair_hash> redMapper;
    unordered_map<Coord, int, pair_hash> yellowMapper;
    bool gameIsFinished;
    unordered_set<Coord, pair_hash> diag;
    unordered_set<Coord, pair_hash> doubleDiag;
    unordered_set<Coord, pair_hash> doubleCorner;
    int timeLimit;
    bool currentTurn; // true for red, false for yellow
    string turnStyles[2];
    string currentTurns[2];
};

#endif