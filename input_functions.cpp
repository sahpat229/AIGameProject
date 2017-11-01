/*
Input functions cpp file
*/

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <iterator>
#include "input_functions.h"

using namespace std;

int modeInput() {
    int mode = -1;
    while (true) {
        cout << "Choose Mode: (1)Player vs. Player, (2)Player vs. Computer, (3)Computer vs Computer: ";
        cin >> mode;
        if (mode >= 4 || mode <= 0) {
            cout << "Please enter an integer value for the mode in the range [1, 3]" << endl;
            continue;
        }
        break;
    }
    return mode;
}

int colorInput(int mode) {
    if (mode == 3) return 1;
    int color = -1;
    while (true) {
        cout << "Choose Color: (1)Red, (2)Yellow. Red goes first: ";
        cin >> color;
        if (color <= 0 || color >= 3) {
            cout << "Color should be in the range [1, 2]" << endl;
            continue;
        }
        break;
    }
    return color;
}

int posInput() {
    int def = -1;
    while (true) {
        cout << "Starting Position: (1)Default or (2)Custom: ";
        cin >> def;
        if (def <= 0 || def >= 3) {
            cout << "Choose a number in the range [1, 2]" << endl;
            continue;
        }
        break;
    }
    return def;
}

bool locationCheck(const string &location)
{
    if (location.length() != 2) {
        return false;
    }
    int horizontal = location[0] - 'A';
    int vertical = location[1] - '1';
    if (horizontal < 0 || horizontal > 7) {
        return false;
    }
    if (vertical < 0 || vertical > 7) {
        return false;
    }
    return true;
}

template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

vector<pair<string, int>> customLocations(int def, string color) {
    if (def == 1) {
        return vector<pair<string, int>>();
    }
    string s_locations;
    vector<string> items;
    cin.ignore();
    while (true) {
        cout << "Input starting locations for " + color + ": ";
        getline(cin, s_locations);
        items = split(s_locations, ' ');
        bool one_is_false = false;
        for (auto it = items.begin(); it != items.end(); it+=2) {
            if (!locationCheck(*it)) {
                one_is_false = true;
            }
        }
        if (one_is_false) {
            cout << "Enter valid starting locations" << endl;
            continue;
        }
        break;
    }
    int index = 0;
    vector<pair<string, int>> return_list = vector<pair<string, int>>();
    while (index < items.size()) {
        return_list.push_back(make_pair(items[index], stoi(items[index+1])));
        index += 2;
    }
    return return_list;
}

int timeInput() {
    int time_input = -1;
    while (true) {
        cout << "Entire time: ";
        cin >> time_input;
        if (time_input <= 0) {
            cout << "Please enter a number greater than zero" << endl;
            continue;
        }
        break;
    }
    return time_input;
}