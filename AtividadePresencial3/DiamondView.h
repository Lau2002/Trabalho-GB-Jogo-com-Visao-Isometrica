#ifndef DiamondView_h
#define DiamondView_h

#include "TilemapView.h"
#include <iostream>
using namespace std;

class DiamondView : public TilemapView {
public:
    void computeDrawPosition(const int col, const int row, const float tw, const float th, float& targetx, float& targety) const {
        targetx = col * tw / 2 + row * tw / 2;
        targety = col * th / 2 - row * th / 2;
    }

    void computeMouseMap(int& col, int& row, const float tw, const float th, const float mx, const float my) const {
        float tw2 = tw / 2.0f;
        float th2 = th / 2.0f;

        row = my / th;
        col = mx / tw;
        cout << "dest: " << col << "," << row << endl;
        cout << "tw/th " << tw << "," << th << endl;
        cout << "mx/my " << mx << "," << my << endl;

    }

    void computeTileWalking(int& col, int& row, const int direction) const {
        switch (direction) {
        case DIRECTION_NORTH:
            row--;
            col++;
            break;
        case DIRECTION_EAST:
            col--;
            row--;
            break;
        case DIRECTION_WEST:
            row++;
            col++;
            break;
        case DIRECTION_SOUTH:
            row++;
            col--;
            break;
        case DIRECTION_NORTHWEST:
            row--;
            break;
        case DIRECTION_NORTHEAST:
            col++;
            break;
        case DIRECTION_SOUTHWEST:
            col--;
            break;
        case DIRECTION_SOUTHEAST:
            row++;
        }
        
    }

};

#endif /* DiamondView_h */
