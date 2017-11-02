#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <sstream>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <math.h>
#include <algorithm>
#include <signal.h>
#include <sys/time.h>
#include <signal.h>
#include <ctime>
#include <chrono>
#include <stdlib.h>

#include "stateModel.h"

using namespace std;
using Coord = pair<int, int>;

void handleSIGALRM(int signum) {
    cout << "THROWING" << endl;
    throw 20;
}

StateModel::StateModel(int mode, int color, int pos, vector<pair<string, int>> redPieces,
                       vector<pair<string, int>> yellowPieces, int timeLimit) {
    this->mode = mode;
    this->color = color;
    this->currentTurns[0] = "yellow";
    this->currentTurns[1] = "red";

    vector<Coord> redDefaults = { {1, 0}, {3, 0}, {5, 0}, {7, 0}, {0, 1}, {2, 1}, {4, 1},
        {6, 1}, {1, 2}, {3, 2}, {5, 2}, {7, 2}
    };
    vector<Coord> yellowDefaults = { {0, 7}, {2, 7}, {4, 7}, {6, 7}, {1, 6}, {3, 6}, {5, 6},
        {7, 6}, {0, 5}, {2, 5}, {4, 5}, {6, 5}
    };

    if (pos == 1) {
        for (Coord coord : redDefaults) {
            this->redMapper[coord] = 0;
        }
        for (Coord coord : yellowDefaults) {
            this->yellowMapper[coord] = 0;
        }
    }

    else {
        for (pair<string, int> &tup : redPieces) {
            this->redMapper[this->getLocation(tup.first)] = tup.second;
        }
        for (pair<string, int> &tup : yellowPieces) {
            this->yellowMapper[this->getLocation(tup.first)] = tup.second;
        }
    }
    this->timeLimit = timeLimit;
    this->diag = { {7, 0}, {6, 1}, {5, 2}, {4, 3}, {3, 4},
            {2, 5}, {1, 6}, {0, 7}
        };
    this->doubleDiag = { {1, 0}, {2, 1}, {3, 2}, {4, 3}, {5, 4}, {6, 5}, {7, 6},
            {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}
        };
    this->doubleCorner = { {1, 0}, {0, 1}, {6, 7}, {7, 6} };
}

void StateModel::printEmpty(bool background) {
    string color = "";
    if (background) {
        color = "44";
    } else {
        color = "47";
    }
    cout << "    ";
    for (int i = 0; i < 8; ++i) {
        cout << "|";
        cout << "\033["+color+"m     \033[0m";
        if (background) {
            color = "47";
        } else {
            color = "44";
        }
        background = !background;
    }
    cout << "|" << endl;
}

void StateModel::printBoard() {
    string divider = "    +-----+-----+-----+-----+-----+-----+-----+-----+";
    bool background = true;
    string color = "";
    if (background) {
        color = "44";
    } else {
        color = "47";
    }
    for (int y = 0; y < 8; ++y) {
        cout << divider << endl;
        bool orig_background = background;
        this->printEmpty(orig_background);
        cout << y + 1 << " : ";
        for (int x = 0; x < 8; ++x) {
            if (background) {
                color = "44";
            } else {
                color = "47";
            }
            cout << "|";
            auto r_got = this->redMapper.find(make_pair(x, y));
            auto y_got = this->yellowMapper.find(make_pair(x, y));
            if (r_got != this->redMapper.end()) {
                int king = this->redMapper[make_pair(x, y)];
                if (king == 1) {
                    cout << "\033["+color+";31;1m" + "  K  " + "\033[0m";
                }
                else {
                    cout << "\033["+color+";31;1m" + "  O  " + "\033[0m";
                }
            } else if (y_got != this->yellowMapper.end()) {
                int king = this->yellowMapper[make_pair(x, y)];
                if (king == 1) {
                    cout << "\033["+color+";30;1m" + "  K  " + "\033[0m";
                }
                else {
                    cout << "\033["+color+";30;1m" + "  O  " + "\033[0m";
                }
            } else {
                cout << "\033["+color+";30;1m" + "     " + "\033[0m";
            }
            background = !background;
        }
        cout << "|" << endl;
        this->printEmpty(orig_background);
        background = !background;
    }
    cout << divider << endl;
    cout << "       A     B     C     D     E     F     G     H  " << endl;
}

pair<int, int> StateModel::getLocation(const string &location) {
    return make_pair(location[0] - 'A', location[1] - '1');
}

bool StateModel::coordInBoard(Coord coord) {
    if (coord.first >= 0 && coord.first <= 7 && coord.second >= 0 && coord.second <= 7) return true;
    return false;
}

vector<vector<Coord> *> *StateModel::getValidMoves(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                                 unordered_map<Coord, int, pair_hash> &otherMapper) {
    vector<vector<Coord> *> *nonJumpMoves = new vector<vector<Coord> *>;
    vector<vector<Coord> *> *jumpMoves = new vector<vector<Coord> *>;

    for (const pair<const Coord, int> &piece : myMapper) {
        pair<vector<vector<Coord> *> *, bool> validMoves = this->getValidMovesForCoord(currentTurn,
                                                                                       piece.first,
                                                                                       piece.second,
                                                                                       myMapper,
                                                                                       otherMapper);
        if (validMoves.second) { // jumpFound
            jumpMoves->insert(jumpMoves->end(), validMoves.first->begin(), validMoves.first->end());
        }
        else {
            nonJumpMoves->insert(nonJumpMoves->end(), validMoves.first->begin(), validMoves.first->end());
        }
    }
    if (jumpMoves->size() > 0) {
        for (auto it = nonJumpMoves->begin(); it != nonJumpMoves->end(); ++it) {
            delete *it;
        }
        nonJumpMoves->clear();
        delete nonJumpMoves;
        return jumpMoves;
    }
    else {
        jumpMoves->clear();
        delete jumpMoves;
        return nonJumpMoves;
    }
}

pair<vector<vector<Coord> *> *, bool> StateModel::getValidMovesForCoord(bool currentTurn, const Coord &coord, int king,
                                                                        unordered_map<Coord, int, pair_hash> &myMapper,
                                                                        unordered_map<Coord, int, pair_hash> &otherMapper) {
    bool jumpFound = false;
    vector<vector<Coord> *> *validMoves = new vector<vector<Coord> *>;
    vector<Coord> possibilities;
    if (currentTurn) {
        possibilities.push_back(make_pair(coord.first - 1, coord.second + 1));
        possibilities.push_back(make_pair(coord.first + 1, coord.second + 1));
        if (king == 1) {
            possibilities.push_back(make_pair(coord.first - 1, coord.second - 1));
            possibilities.push_back(make_pair(coord.first + 1, coord.second - 1));
        }
    }
    else {
        possibilities.push_back(make_pair(coord.first - 1, coord.second - 1));
        possibilities.push_back(make_pair(coord.first + 1, coord.second - 1));
        if (king == 1) {
            possibilities.push_back(make_pair(coord.first - 1, coord.second + 1));
            possibilities.push_back(make_pair(coord.first + 1, coord.second + 1));
        }
    }

    for (Coord &possibility : possibilities) {
        if (this->coordInBoard(possibility)) {
            auto got = myMapper.find(possibility);
            if (got == myMapper.end()) {
                auto o_got = otherMapper.find(possibility);
                if (o_got == otherMapper.end()) {
                    Coord start_coord = make_pair(coord.first, coord.second);
                    Coord end_coord = make_pair(possibility.first, possibility.second);
                    if (!jumpFound) {
                        vector<Coord> *move = new vector<Coord>;
                        move->push_back(start_coord);
                        move->push_back(end_coord);
                        validMoves->push_back(move);
                    }
                }
                else {
                    Coord newCoord;
                    if (possibility.first < coord.first) {
                        if (possibility.second > coord.second) {
                            newCoord = make_pair(possibility.first - 1, possibility.second + 1);
                        }
                        else {
                            newCoord = make_pair(possibility.first - 1, possibility.second - 1);
                        }
                    }
                    else {
                        if (possibility.second > coord.second) {
                            newCoord = make_pair(possibility.first + 1, possibility.second + 1);
                        }
                        else {
                            newCoord = make_pair(possibility.first + 1, possibility.second - 1);
                        }
                    }
                    if (this->coordInBoard(newCoord)) {
                        auto m_got = myMapper.find(newCoord);
                        auto o_got_2 = otherMapper.find(newCoord);

                        if (m_got == myMapper.end() && o_got_2 == otherMapper.end()) {

                            if (!jumpFound) {
                                for (auto it = validMoves->begin(); it != validMoves->end(); ++it) {
                                    delete *it;
                                }
                                validMoves->clear();
                                jumpFound = true;
                            }

                            unordered_map<Coord, int, pair_hash> newOtherMapper = otherMapper;
                            newOtherMapper.erase(possibility);
                            Coord startCoord = make_pair(coord.first, coord.second);
                            Coord endCoord = make_pair(newCoord.first, newCoord.second);
                            vector<Coord> *currentMove = new vector<Coord>;
                            currentMove->push_back(startCoord);
                            currentMove->push_back(endCoord);

                            vector<vector<Coord> *> *newMoves = this->getValidMovesAfterJump(currentTurn,
                                                                                             newCoord,
                                                                                             king,
                                                                                             myMapper,
                                                                                             newOtherMapper);
                            if (newMoves->size() == 0) {
                                validMoves->push_back(currentMove);
                            }
                            else {
                                currentMove->pop_back();
                                for (vector<Coord> *move : *newMoves) {
                                    vector<Coord> *thisMove = new vector<Coord>;
                                    thisMove->insert(thisMove->end(), currentMove->begin(), currentMove->end());
                                    thisMove->insert(thisMove->end(), move->begin(), move->end());
                                    validMoves->push_back(thisMove);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return make_pair(validMoves, jumpFound);
}

vector<vector<Coord> *> *StateModel::getValidMovesAfterJump(bool currentTurn, const Coord &coord, int king,
                                                            unordered_map<Coord, int, pair_hash> &myMapper,
                                                            unordered_map<Coord, int, pair_hash> &otherMapper) {
    vector<vector<Coord> *> *validMoves = new vector<vector<Coord> *>;
    vector<Coord> possibilities;
    if (currentTurn) {
        possibilities.push_back(make_pair(coord.first - 1, coord.second + 1));
        possibilities.push_back(make_pair(coord.first + 1, coord.second + 1));
        if (king == 1) {
            possibilities.push_back(make_pair(coord.first - 1, coord.second - 1));
            possibilities.push_back(make_pair(coord.first + 1, coord.second - 1));
        }
    }
    else {
        possibilities.push_back(make_pair(coord.first - 1, coord.second - 1));
        possibilities.push_back(make_pair(coord.first + 1, coord.second - 1));
        if (king == 1) {
            possibilities.push_back(make_pair(coord.first - 1, coord.second + 1));
            possibilities.push_back(make_pair(coord.first + 1, coord.second + 1));
        }
    }

    for (Coord &possibility : possibilities) {
        if (this->coordInBoard(possibility)) {
            auto got = otherMapper.find(possibility);
            if (got != otherMapper.end()) {
                Coord newCoord;
                if (possibility.first < coord.first) {
                    if (possibility.second > coord.second) {
                        newCoord = make_pair(possibility.first - 1, possibility.second + 1);
                    }
                    else {
                        newCoord = make_pair(possibility.first - 1, possibility.second - 1);
                    }
                }
                else {
                    if (possibility.second > coord.second) {
                        newCoord = make_pair(possibility.first + 1, possibility.second + 1);
                    }
                    else {
                        newCoord = make_pair(possibility.first + 1, possibility.second - 1);
                    }
                }
                if (this->coordInBoard(newCoord)) {
                    auto m_got = myMapper.find(newCoord);
                    auto o_got = otherMapper.find(newCoord);
                    if (m_got == myMapper.end() && o_got == otherMapper.end()) {
                        unordered_map<Coord, int, pair_hash> newOtherMapper = otherMapper;
                        newOtherMapper.erase(possibility);
                        Coord startCoord = make_pair(coord.first, coord.second);
                        Coord endCoord = make_pair(newCoord.first, newCoord.second);
                        vector<Coord> *currentMove = new vector<Coord>;
                        currentMove->push_back(startCoord);
                        currentMove->push_back(endCoord);

                        vector<vector<Coord> *> *newMoves = this->getValidMovesAfterJump(currentTurn,
                                                                                         newCoord,
                                                                                         king,
                                                                                         myMapper,
                                                                                         newOtherMapper);
                        if (newMoves->size() == 0) {
                            validMoves->push_back(currentMove);
                        }
                        else {
                            currentMove->pop_back();
                            for (vector<Coord> *move : *newMoves) {
                                vector<Coord> *thisMove = new vector<Coord>;
                                thisMove->insert(thisMove->end(), currentMove->begin(), currentMove->end());
                                thisMove->insert(thisMove->end(), move->begin(), move->end());
                                validMoves->push_back(thisMove);
                            }
                        }
                    }
                }
            }
        }
    }
    return validMoves;
}

void StateModel::startGame() {
    switch (this->mode) {
        case 1:
            this->turnStyles[1] = "player";
            this->turnStyles[0] = "player";
            break;
        case 2:
            if (this->color == 1) {
                this->turnStyles[1] = "player";
                this->turnStyles[0] = "computer";     
            } else {
                this->turnStyles[1] = "computer";
                this->turnStyles[0] = "player";
            }
            break;
        case 3:
            this->turnStyles[1] = "computer";
            this->turnStyles[0] = "computer";
    }
    this->currentTurn = false;
    while (!this->gameIsFinished) {
        this->executeTurn();
    }
    cout << "Game finished " << endl;
}

void StateModel::executeTurn() {
    this->printBoard();
    vector<string> colors{"33", "31"};
    cout << "\033[1;"+ colors[this->currentTurn] + "m" + 
        this->currentTurns[this->currentTurn] + "\'s turn\033[0m\n" << endl;

    unordered_map<Coord, int, pair_hash> &myMapper = *(this->getMyMapper());
    unordered_map<Coord, int, pair_hash> &otherMapper = *(this->getOtherMapper());

    vector<vector<Coord> *> *validMoves = this->getValidMoves(this->currentTurn,
                                                              myMapper,
                                                              otherMapper);
    if ((validMoves->size() == 0) || (myMapper.size() == 0) || (otherMapper.size() == 0)) {
        this->gameIsFinished = true;
        for (auto it = validMoves->begin(); it != validMoves->end(); ++it) {
            delete *it;
        }
        validMoves->clear();
        delete validMoves;
        return;
    }

    // for (vector<Coord> *move : *validMoves) {
    //     this->displayMove(move);
    // }

    if (this->turnStyles[this->currentTurn] == "player") {
        vector<Coord> *move = this->handlePlayerTurn(validMoves);
        this->executeMove(this->currentTurn, move, myMapper, otherMapper);
    }
    else {
        // computer Turn
        vector<Coord> *move = NULL;
        if (validMoves->size() == 1) {
            move = validMoves->at(0);
        } else {
            move = this->findBestMove();
        }
        this->executeMove(currentTurn, move, myMapper, otherMapper);
    }

    for (auto it = validMoves->begin(); it != validMoves->end(); ++it) {
        delete *it;
    }
    validMoves->clear();
    delete validMoves;

    this->currentTurn = !this->currentTurn;
}

vector<Coord> *StateModel::handlePlayerTurn(vector<vector<Coord> *> *validMoves) {
    string s_move;
    while (true) {
        cout << "Enter a move like follows - A5>B6 - or enter in 'H' for a list of all possible moves: ";
        cin >> s_move;
        if (s_move == "H") {
            for (vector<Coord> *mv : *validMoves) {
                this->displayMove(mv);
            };
        }
        else {
            vector<Coord> *move = this->convertToCoord(split(s_move, '>'));
            for (vector<Coord> *mv : *validMoves) {
                if (mv->size() != move->size()) continue;
                bool is_correct = true;
                for (int index = 0; index < mv->size(); ++index) {
                    Coord firstCoord = mv->at(index);
                    Coord secondCoord = move->at(index);
                    if ((firstCoord.first != secondCoord.first) || (firstCoord.second != secondCoord.second)) {
                        is_correct = false;
                    }
                }
                if (is_correct) return move;
            }
            move->clear();
            delete move;
            cout << "Enter a valid move please." << endl;
        }
    }
}

vector<Coord> *StateModel::convertToCoord(vector<string> coords) {
    vector<Coord> *move = new vector<Coord>;
    for (string &coord : coords) {
        move->push_back(make_pair(coord[0] - 'A', coord[1] - '1'));
    }
    return move;
}

void StateModel::displayMove(vector<Coord> *move) {
    for (Coord &coord : *move) {
        cout << char('A' + coord.first) << char('1' + coord.second) << " ";
    }
    cout << endl;
}
void StateModel::displayMoveInline(vector<Coord> *move) {
    for (Coord &coord : *move) {
        cout << char('A' + coord.first) << char('1' + coord.second) << " ";
    }
}

unordered_map<Coord, int, pair_hash> *StateModel::getMyMapper() {
    if (this->currentTurn) return &(this->redMapper);
    return &(this->yellowMapper);
}

unordered_map<Coord, int, pair_hash> *StateModel::getOtherMapper() {
    if (this->currentTurn) return &(this->yellowMapper);
    return &(this->redMapper);
}

void StateModel::executeMove(bool currentTurn, vector<Coord> *move, unordered_map<Coord, int, pair_hash> &myMapper,
                             unordered_map<Coord, int, pair_hash> &otherMapper) {
    Coord &startCoord = move->at(0);
    Coord &endCoord = *(move->end()-1);
    int king = myMapper[startCoord];
    myMapper.erase(startCoord);
    myMapper[endCoord] = king;
    if (currentTurn) {
        if (endCoord.second == 7) {
            myMapper[endCoord] = 1;
        }
    } else {
        if (endCoord.second == 0) {
            myMapper[endCoord] = 1;
        }
    }
    if (move->size() > 2) {
        int index = 0;
        while (index < move->size() - 1) {
            Coord &beginCoord = move->at(index);
            Coord &finishCoord = move->at(index+1);
            Coord midCoord = make_pair((beginCoord.first + finishCoord.first) / 2, 
                                       (beginCoord.second + finishCoord.second) / 2);
            otherMapper.erase(midCoord);
            index += 1;
        }
    } else {
        if (abs(startCoord.first - endCoord.first) == 2) {
            Coord midCoord = make_pair((startCoord.first + endCoord.first) / 2,
                                       (startCoord.second + endCoord.second) / 2);
            otherMapper.erase(midCoord);
        }
    }
}

vector<Coord> *StateModel::alphaBetaSearch(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                           unordered_map<Coord, int, pair_hash> &otherMapper,
                                           int depthLimit, bool debug, chrono::system_clock::time_point &endTime) {
    //cout << "ALPHABETA: " << depthLimit << endl;
    return this->maxValue(currentTurn, myMapper, otherMapper, INT_MIN, INT_MAX, 
                          1, depthLimit, NULL, debug, endTime).second;
}

pair<int, vector<Coord> *>StateModel::maxValue(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                               unordered_map<Coord, int, pair_hash> &otherMapper,
                                               int alpha, int beta, int currdepth, int depthLimit,
                                               vector<Coord> *moveTaken, bool debug, 
                                               chrono::system_clock::time_point &endTime) {
    //cout << "MAX: " << currdepth << endl;
    vector<Coord> *mT = NULL;

    if (chrono::system_clock::now() > endTime) return make_pair(INT_MIN, mT);

    if (myMapper.size() == 0) {
        return make_pair(INT_MIN + 10*currdepth*currdepth, mT);
    }
    if (otherMapper.size() == 0) {
        return make_pair(INT_MAX - 10*currdepth*currdepth, moveTaken);
    }
    if (currdepth == depthLimit) {
        int evaluated = this->evaluateBoard(currentTurn, myMapper, otherMapper, currdepth, true);
        if (debug) {
            for (int i = 0; i < currdepth; ++i) {
                cout << "\t\t";
            }
            cout << "MoveTaken: ";
            if (moveTaken != NULL) this->displayMoveInline(moveTaken);
            cout << "Evaled: " << evaluated << endl;
        }
        return make_pair(evaluated, mT);
    }
    int v = INT_MIN;
    vector<Coord> *maxmove = NULL;
    vector<vector<Coord> *> *moves = this->getValidMoves(currentTurn, myMapper, otherMapper);
    if (moves->size() == 0) {
        return make_pair(INT_MIN + 10*currdepth*currdepth, mT);
    }
    for (vector<Coord> *move : *moves) {
        unordered_map<Coord, int, pair_hash> newMyMapper = myMapper;
        unordered_map<Coord, int, pair_hash> newOtherMapper = otherMapper;
        this->executeMove(currentTurn, move, newMyMapper, newOtherMapper);
        bool nextCurrentTurn = !currentTurn;
        pair<int, vector<Coord> *>newPair = this->minValue(nextCurrentTurn, newOtherMapper, newMyMapper, 
                                                           alpha, beta, currdepth+1,
                                                           depthLimit, move, debug, endTime);
        if (newPair.first == INT_MIN) {
            for (auto it = moves->begin(); it != moves->end(); ++it) {
                delete *it;
            }
            moves->clear();
            delete moves;
            return make_pair(INT_MIN, mT);
        }

        if (newPair.first > v) {
            v = newPair.first;
            maxmove = move;
        }

        if (newPair.second != NULL) delete newPair.second;

        if (v >= beta) {
            vector<Coord> *returnMove = new vector<Coord>;
            returnMove->insert(returnMove->end(), maxmove->begin(), maxmove->end());
            for (auto it = moves->begin(); it != moves->end(); ++it) {
                delete *it;
            }
            moves->clear();
            delete moves;

            if (debug) {
                for (int i = 0; i < currdepth; ++i) {
                    cout << "\t\t";
                }
                cout << "PRUNE MoveTaken: ";
                if (moveTaken != NULL) this->displayMoveInline(moveTaken);
                cout << "V: " << v+1 << "MAXMOVE: ";
                if (returnMove != NULL) this->displayMoveInline(returnMove);
                cout << endl;
            }

            return make_pair(v+1, returnMove);
        }
        alpha = max(alpha, v);
    }
    vector<Coord> *returnMove = new vector<Coord>;
    returnMove->insert(returnMove->end(), maxmove->begin(), maxmove->end());

    for (auto it=moves->begin(); it != moves->end(); ++it) {
        delete *it;
    }
    moves->clear();
    delete moves;

    if (debug) {
        for (int i = 0; i < currdepth; ++i) {
            cout << "\t\t";
        }
        cout << "MoveTaken: ";
        if (moveTaken != NULL) this->displayMoveInline(moveTaken);
        cout << "V: " << v+1 << "MAXMOVE: ";
        if (returnMove != NULL) this->displayMoveInline(returnMove);
        cout << endl;
    }
    if (currdepth == 1) {
        cout << "V: " << v << endl;
    }

    return make_pair(v, returnMove);
}


pair<int, vector<Coord> *>StateModel::minValue(bool currentTurn, unordered_map<Coord, int, pair_hash> &myMapper,
                                               unordered_map<Coord, int, pair_hash> &otherMapper,
                                               int alpha, int beta, int currdepth, int depthLimit,
                                               vector<Coord> *moveTaken, bool debug,
                                               chrono::system_clock::time_point &endTime) {
    //cout << "MIN: " << currdepth << endl;
    vector<Coord> *mT = NULL;

    if (chrono::system_clock::now() > endTime) return make_pair(INT_MIN, mT);

    if (myMapper.size() == 0) {
        return make_pair(INT_MAX - 10*currdepth*currdepth, mT);
    }
    if (otherMapper.size() == 0) {
        return make_pair(INT_MIN + 10*currdepth*currdepth, moveTaken);
    }
    if (currdepth == depthLimit) {
        int evaluated = this->evaluateBoard(currentTurn, myMapper, otherMapper, currdepth, false);
        if (debug) {
            for (int i = 0; i < currdepth; ++i) {
                cout << "\t\t";
            }
            cout << "MoveTaken: ";
            if (moveTaken != NULL) this->displayMoveInline(moveTaken);
            cout << "Evaled: " << evaluated << endl;
        }
        return make_pair(evaluated, mT);
    }
    int v = INT_MAX;
    vector<Coord> *minmove = NULL;
    vector<vector<Coord> *> *moves = this->getValidMoves(currentTurn, myMapper, otherMapper);
    if (moves->size() == 0) {
        return make_pair(INT_MAX - 10*currdepth*currdepth, mT);
    }
    for (vector<Coord> *move : *moves) {
        unordered_map<Coord, int, pair_hash> newMyMapper = myMapper;
        unordered_map<Coord, int, pair_hash> newOtherMapper = otherMapper;
        this->executeMove(currentTurn, move, newMyMapper, newOtherMapper);
        bool nextCurrentTurn = !currentTurn;
        pair<int, vector<Coord> *> newPair = this->maxValue(nextCurrentTurn, newOtherMapper, newMyMapper, 
                                                            alpha, beta, currdepth+1,
                                                            depthLimit, move, debug, endTime);
        if (newPair.first == INT_MIN) {
            for (auto it = moves->begin(); it != moves->end(); ++it) {
                delete *it;
            }
            moves->clear();
            delete moves;
            return make_pair(INT_MIN, mT);
        }

        if (newPair.first < v) {
            v = newPair.first;
            minmove = move;
        }
        if (newPair.second != NULL) delete newPair.second;

        if (v <= alpha) {
            vector<Coord> *returnMove = new vector<Coord>;
            returnMove->insert(returnMove->end(), minmove->begin(), minmove->end());
            for (auto it = moves->begin(); it != moves->end(); ++it) {
                delete *it;
            }
            moves->clear();
            delete moves;

            if (debug) {
                for (int i = 0; i < currdepth; ++i) {
                    cout << "\t\t";
                }
                cout << "PRUNE MoveTaken: ";
                if (moveTaken != NULL) this->displayMoveInline(moveTaken);
                cout << "V: " << v+1 << "MINMOVE: ";
                if (returnMove != NULL) this->displayMoveInline(returnMove);
                cout << endl;
            }

            return make_pair(v-1, returnMove);
        }
        beta = min(beta, v);
    }

    vector<Coord> *returnMove = new vector<Coord>;
    returnMove->insert(returnMove->end(), minmove->begin(), minmove->end());
    for (auto it=moves->begin(); it != moves->end(); ++it) {
        delete *it;
    }
    moves->clear();
    delete moves;

    if (debug) {
        for (int i = 0; i < currdepth; ++i) {
            cout << "\t\t";
        }
        cout << "MoveTaken: ";
        if (moveTaken != NULL) this->displayMoveInline(moveTaken);
        cout << "V: " << v+1 << "MINMOVE: ";
        if (returnMove != NULL) this->displayMoveInline(returnMove);
        cout << endl;
    }

    return make_pair(v, returnMove);
}

int StateModel::computeManhattan(vector<Coord> &largerCoords, vector <Coord> &smallerCoords) {
    int distance = 0;
    for (Coord &coord : largerCoords) {
        for (Coord &targetCoord : smallerCoords) {
            distance += abs(coord.first - targetCoord.first) + abs(coord.second - targetCoord.second);
        }
    }
    return distance;
}

int StateModel::evaluateBoard(bool currentTurn,
                              unordered_map<Coord, int, pair_hash> &myMapper,
                              unordered_map<Coord, int, pair_hash> &otherMapper,
                              int currdepth, bool fromMax) {

    unordered_map<Coord, int, pair_hash> &redMapper = currentTurn ? myMapper : otherMapper;
    unordered_map<Coord, int, pair_hash> &yellowMapper = currentTurn ? otherMapper : myMapper;

    int redScore = 0;
    int yellowScore = 0;

    int pawnScore = 600;
    int kingScore = 1000;
    int safeScore = 80;
    int distancePenalty = -20;
    int centerScore = 140;
    int doubleCornerScore = 150;
    int kingLine = 250;
    int diagonalScore = 50;

    int multiplierCoefficient = 150;

    // for (pair<const Coord, int> &piece : redMapper) {
    //     if (piece.second == 0) {
    //         redScore += pawnScore;
    //     } else redScore += kingScore;
    // }

    // for (pair<const Coord, int> &piece : yellowMapper) {
    //     if (piece.second == 0) {
    //         yellowScore += pawnScore;
    //     } else yellowScore += kingScore;
    // }

    if (redMapper.size() + yellowMapper.size() < 7) {
        pawnScore = 600;
        kingScore = 1100;
        centerScore = 200;
        doubleCornerScore = 200;
        kingLine = 0;
        diagonalScore = 50;

        multiplierCoefficient = 500;

        vector<Coord> redCoords;
        vector<Coord> yellowCoords;
        for (pair<const Coord, int> &piece : redMapper) {
            redCoords.push_back(piece.first);
        }
        for (pair<const Coord, int> &piece : yellowMapper) {
            yellowCoords.push_back(piece.first);
        }

        if (redCoords.size() > yellowCoords.size()) {
            int distance = this->computeManhattan(redCoords, yellowCoords);
            redScore -= distance * 15;
        } else if (yellowCoords.size() > redCoords.size()) {
            int distance = this->computeManhattan(yellowCoords, redCoords);
            yellowScore -= distance * 15;
        }
    }

    if (redMapper.size() + yellowMapper.size() < 4) {
        pawnScore = 600;
        kingScore = 1000;
        safeScore = -50;
        distancePenalty = -20;
        centerScore = 200;
        doubleCornerScore = 500;
        kingLine = 0;
        diagonalScore = 80;

        vector<Coord> redCoords;
        vector<Coord> yellowCoords;
        for (pair<const Coord, int> &piece : redMapper) {
            redCoords.push_back(piece.first);
        }
        for (pair<const Coord, int> &piece : yellowMapper) {
            yellowCoords.push_back(piece.first);
        }

        if (redMapper.size() > yellowMapper.size()) {
            Coord &target = yellowCoords[0];
            for (Coord &coord : redCoords) {
                redScore -= (abs(coord.first - target.first) + abs(coord.second - target.second)) * 25;
            }
            if (redCoords.size() > 1) {
                redScore -= (abs(redCoords[0].first - redCoords[1].first) + abs(redCoords[0].second - redCoords[1].second)) * 25;
            }
        } else if (yellowMapper.size() > redMapper.size()) {
            Coord &target = redCoords[0];
            for (Coord &coord : yellowCoords) {
                yellowScore -= (abs(coord.first - target.first) + abs(coord.second - target.second)) * 25;
            }
            if (yellowCoords.size() > 1) {
                yellowScore -= (abs(yellowCoords[0].first - yellowCoords[1].first) + abs(yellowCoords[0].second - yellowCoords[1].second)) * 25;
            }
        }
    }

    for (pair<const Coord, int> &piece : redMapper) {
        const Coord &coord = piece.first;
        if (piece.second == 0) {
            redScore += pawnScore;
            redScore += (7 - coord.second) * distancePenalty;
        }
        else redScore += kingScore;
        if ((coord.first == 0) || (coord.first == 7)) redScore += safeScore;
        if (this->doubleCorner.find(coord) != this->doubleCorner.end()) redScore += doubleCornerScore;
        if (this->doubleDiag.find(coord) != this->doubleDiag.end()) redScore += diagonalScore;
        if (coord.second == 0) redScore += kingLine;
        if ((2 <= coord.first) && (coord.first <= 5) && (2 <= coord.second) && (coord.second <= 5))
            redScore += centerScore;

    }
    for (pair<const Coord, int> &piece : yellowMapper) {
        const Coord &coord = piece.first;
        if (piece.second == 0) {
            yellowScore += pawnScore;
            yellowScore += coord.second * distancePenalty;
        }
        else yellowScore += kingScore;
        if ((coord.first == 0) || (coord.first == 7)) yellowScore += safeScore;
        if (this->doubleCorner.find(coord) != this->doubleCorner.end()) yellowScore += doubleCornerScore;
        if (this->doubleDiag.find(coord) != this->doubleDiag.end()) yellowScore += diagonalScore;
        if (coord.second == 7) yellowScore += kingLine;
        if ((2 <= coord.first) && (coord.first <= 5) && (2 <= coord.second) && (coord.second <= 5))
            yellowScore += centerScore;
    }

    int myScore = currentTurn ? redScore : yellowScore;
    int otherScore = currentTurn ? yellowScore : redScore;

    int multiplierAdd = myMapper.size() * multiplierCoefficient / otherMapper.size();
    if (multiplierAdd > multiplierCoefficient) {
        myScore += multiplierAdd;
        otherScore -= multiplierAdd;
    } else if (multiplierAdd < multiplierCoefficient) {
        myScore -= multiplierAdd;
        otherScore += multiplierAdd;
    }

    if (fromMax) return ((myScore - otherScore) << 5) + (rand() % 8);
    else return ((otherScore - myScore) << 5) + (rand() % 8);

    // if (fromMax) return (myScore - otherScore);
    // else return (otherScore - myScore);
}

vector<Coord> *StateModel::findBestMove() {
    unordered_map<Coord, int, pair_hash> &myMapper = *(this->getMyMapper());
    unordered_map<Coord, int, pair_hash> &otherMapper = *(this->getOtherMapper());

    vector<Coord> *move = NULL;
    vector<Coord> *returnMove = NULL;
    int depthLimit = 3;

    chrono::duration<double> elapsed_seconds;
    auto startTime = chrono::system_clock::now();
    auto endTime = startTime + chrono::seconds(this->timeLimit) - chrono::milliseconds(int(double(this->timeLimit)*0.06));

    do {
        move = this->alphaBetaSearch(this->currentTurn, myMapper, otherMapper, depthLimit, false, endTime);
        if (move != NULL) {
            delete returnMove;
            returnMove = move;
            elapsed_seconds = chrono::system_clock::now() - startTime;
        }
        depthLimit += 1;
    } while (move != NULL);

    cout << "Time Used: " << elapsed_seconds.count() << "\tDepth reached: " << depthLimit - 1 << endl;
    return returnMove;
    
    // move = this->alphaBetaSearch(this->currentTurn, myMapper, otherMapper, 10, false);
    // elapsed_seconds = chrono::system_clock::now() - startTime;
    // cout << "Time Used: " << elapsed_seconds.count() << endl;
    // return move;

    // try {
    //     setitimer(ITIMER_VIRTUAL, &timer, &old_value);
    //     cout << "setting timer" << endl;
    //     while (true) {
    //         cout << "HERE" << endl;
    //         vector<Coord> *oldMove = move;
    //         move = this->alphaBetaSearch(this->currentTurn, myMapper, otherMapper, depthLimit, false);
    //         if (move == NULL) {
    //             cout << "HERE's the error" << endl;
    //         }
    //         this->displayMove(move);
    //         delete oldMove;
    //         elapsed_seconds = chrono::system_clock::now() - startTime;
    //         depthLimit += 1;
    //     }
    // } catch (int param) {
    //     sigprocmask(SIG_UNBLOCK, &sig, NULL);
    //     //setitimer(ITIMER_REAL, &end_timer, &old_value);
    //     cout << "Caught" << endl;
    //     if (move != NULL) {
    //         cout << "Depth: " << depthLimit - 1 << " Time Used: " << elapsed_seconds.count() << endl;
    //         return move;
    //     }
    // }
}