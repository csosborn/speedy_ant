#include <string>
#include <iostream>
#include <set>
#include <algorithm>
#include <chrono>

enum Direction {
    N,
    E,
    S,
    W
};

const char* directionName(uint8_t d) {
    switch (d) {
        case 0:
            return "north";
            break;
        case 1:
            return "east";
            break;
        case 2:
            return "south";
            break;
        case 3:
            return "west";
            break;
        default:
            return "invalid direction";     
        }
}

const char* colorName(bool color) {
    return color ? "black" : "white";
}

void gogoant(uint64_t n) {
    using namespace std::chrono;

    uint64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    int max_x = 0;
    int max_y = 0;
    int min_x = 0;
    int min_y = 0;

    std::pair<int, int> loc = {0,0};
    uint8_t dir = 0;
    bool color = 0;
    std::set<std::pair<int, int>> trail;

    for (int i = 0; i < n; i++) {

        min_x = std::min(min_x, loc.first);
        min_y = std::min(min_y, loc.second);
        max_x = std::max(max_x, loc.first);
        max_y = std::max(max_y, loc.second);

        color = trail.find(loc) != trail.end();
        // color = getColor(x, y);
        if (color) {
            dir -= 1;
            color = false;
        } else {
            dir += 1;
            color = true;
        }
        dir = dir % 4;

        if (color) {
            trail.insert(loc);
        } else {
            trail.erase(loc);
        }

        switch (dir) {
        case 0:
            loc.second++;
            break;
        case 1:
            loc.first++;
            break;
        case 2:
            loc.second--;
            break;
        case 3:
            loc.first--;
            break;     
        }

        // if (i % 1000 == 0) {
        if (i == n-1) {
            std::cout << "Step " << i << ": (" << loc.first << "," << loc.second << ") facing ";
            std::cout << directionName(dir) << " on a " << colorName(color) << " square. ";
            std::cout << "Bounding box: " << min_x << ", " << min_y << " -> " << max_x << ", " << max_y << std::endl;
        }
    }

    uint64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    float speed = (n*10000.0) / (end - start);
    std::cout << "Done. " << n << " iterations in " << end - start << "ms, or " << speed << " it/s." << std::endl;
}

int main(int argc, char** argv) {

    gogoant(10000000);

    return 0;
}