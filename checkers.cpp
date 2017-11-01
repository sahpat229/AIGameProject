#include <iostream>
#include "input_functions.h"
#include "stateModel.h"

using namespace std;

int main() {
    int mode_input = modeInput();
    int color_input = colorInput(mode_input);
    int pos_input = posInput();
    vector<pair<string, int>> redPieces = customLocations(pos_input, "Red");
    vector<pair<string, int>> yellowPieces = customLocations(pos_input, "Yellow");
    int time_input = timeInput();

    StateModel stateModel = StateModel(mode_input, color_input, pos_input, 
                                       redPieces, yellowPieces, time_input);
    stateModel.startGame();
}