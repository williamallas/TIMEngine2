#ifndef CONNECTFOURIA_H
#define CONNECTFOURIA_H

#include <array>
#include "Vector.h"
using namespace tim;

class ConnectFourIA
{

public:
    enum Slot { EMPTY=0, P1, P2 };

    const static int GRID_X = 7;
    const static int GRID_Y = 6;

    ConnectFourIA(const ConnectFourIA&) = default;
    ConnectFourIA& operator=(const ConnectFourIA&) = default;

    ConnectFourIA(bool isP1=true, int maxDepth=3);

    bool humanTurn() const;
    bool humanPlay(int col);
    int computerPlay();
    Slot checkWinner() const;

    int getVerticalIndex(int col) const;

    void printGrid() const;
    int computeScore(bool P1) const;

private:
    const static int INF = 10000000;

    bool _humanP1;
    bool _P1_turn = true;
    int _maxDepth;

    struct Grid
    {
        Slot grids[GRID_X][GRID_Y] = {{EMPTY}};
        int stackSize[GRID_X] = {0};

        Grid play(bool P1_turn, int col) const;
        Slot checkWinner() const;
        Slot read(int x, int y) const { return grids[x][y]; }
        bool checkCol(int col) const { return stackSize[col] < GRID_Y; }
        bool checkPos(int x, int y) const { return x>=0 && x<GRID_X && y>=0 && y<GRID_Y; }
        bool checkWinner(int x, int y, bool player1) const;
        int eval(bool player1) const;

    private:
        int innerEvalPos(int x, int y, Slot) const;
    };
    Grid _grid;

    /* recusrive function for minimax algo */
    int evalHumanPlay(const Grid& grid, int col, int depth, core::ivec2 a_b);
    int evalComputerPlay(const Grid& grid, int col, int depth, core::ivec2 a_b);
};

#endif // CONNECTFOURIA_H
