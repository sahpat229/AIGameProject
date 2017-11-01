/*
Input functions
*/

#include <vector>
#include <string>
#include <utility>

using namespace std;

int modeInput();
int colorInput(int mode);
int posInput();
bool locationCheck(const string &location);
vector<pair<string, int>> customLocations(int def, string color);
int timeInput();

template<typename Out>
void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);