#include "ConnectFourIA.h"
#include <iostream>
#include "type.h"
#include "Vector.h"

using namespace tim::core;

void ConnectFourIA::printGrid() const
{
    for(int j=GRID_Y-1 ; j>=0 ; --j)
    {
        std::cout << "\n";
        for(int i=0 ; i<GRID_X ; ++i)
        {
            std::cout << _grid.grids[i][j];
        }
    }
    std::cout << std::endl;
}

ConnectFourIA::ConnectFourIA(bool isP1, int maxDepth) : _humanP1(isP1), _maxDepth(maxDepth)
{
    static_assert(ConnectFourIA::GRID_X == 7, "ConnectFourIA will not work with GRID_X != 7");
    static_assert(ConnectFourIA::GRID_Y == 6, "ConnectFourIA will not work with GRID_Y != 6");
}

int ConnectFourIA::getVerticalIndex(int col) const
{
    if(col >= 0)
        return _grid.stackSize[col];
    else return -1;
}

bool ConnectFourIA::humanPlay(int col)
{
    if((_humanP1 && !_P1_turn) || (!_humanP1 && _P1_turn))
        return false;

    if(col < 0 || col >= GRID_X || _grid.stackSize[col] >= GRID_Y)
        return false;

    _grid.grids[col][_grid.stackSize[col]] = _humanP1 ? Slot::P1 : Slot::P2;
    _grid.stackSize[col]++;

    _P1_turn = !_P1_turn;

    return true;
}

int ConnectFourIA::computerPlay()
{
    if((_humanP1 && _P1_turn) || (!_humanP1 && !_P1_turn))
        return -1;

    int bestMove = -1;
    int score = -INF;

    for(int i=0 ; i<GRID_X ; ++i)
    {
        if(!_grid.checkCol(i))
            continue;


        int curScore = evalComputerPlay(_grid, i, 0, {-INF,INF});
        if(curScore >= score)
        {
            bestMove = i;
            score = curScore;
        }
    }

    if(bestMove < 0 || bestMove >= GRID_X || _grid.stackSize[bestMove] >= GRID_Y)
        return -1;

    _grid.grids[bestMove][_grid.stackSize[bestMove]] = _humanP1 ? Slot::P2 : Slot::P1;
    _grid.stackSize[bestMove]++;

    _P1_turn = !_P1_turn;

    return bestMove;
}

ConnectFourIA::Slot ConnectFourIA::checkWinner() const
{
    return _grid.checkWinner();
}

int ConnectFourIA::computeScore(bool player1) const
{
    return _grid.eval(player1);
}

ConnectFourIA::Grid ConnectFourIA::Grid::play(bool P1_turn, int col) const
{
    Grid g = *this;
    g.grids[col][g.stackSize[col]] = P1_turn ? Slot::P1 : Slot::P2;
    g.stackSize[col]++;
    return g;
}

ConnectFourIA::Slot ConnectFourIA::Grid::checkWinner() const
{
    for(int i=0 ; i<GRID_X ; ++i)
    {
        for(int j=0 ; j<GRID_Y ; ++j)
        {
            if(read(i,j) == EMPTY)
                continue;

            if(read(i,j) == P1 && checkWinner(i,j, true))
                return P1;
            else if(read(i,j) == P2 && checkWinner(i,j, false))
                return P2;
        }
    }

    return EMPTY;
}

bool ConnectFourIA::Grid::checkWinner(int x, int y, bool player1) const
{
    Slot lookingFor = player1 ? P1:P2;

    int vertical=1, horizontal=1, diag1=1, diag2=1;

    for(int i=x+1; i<GRID_X && grids[i][y]==lookingFor ; ++i, ++horizontal);
    for(int i=x-1; i>=0 && grids[i][y]==lookingFor ; --i, ++horizontal);

    if(horizontal >= 4)
        return true;

    for(int i=y+1; i<GRID_Y && grids[x][i]==lookingFor ; ++i, ++vertical);
    for(int i=y-1; i>=0 && grids[x][i]==lookingFor ; --i, ++vertical);

    if(vertical >= 4)
        return true;

    for(int i=x+1, j=y+1; i<GRID_X && j<GRID_Y && grids[i][j]==lookingFor ; ++i, ++j, ++diag1);
    for(int i=x-1, j=y-1; i>=0 && j>=0 && grids[i][j]==lookingFor ; --i, --j, ++diag1);

    if(diag1 >= 4)
        return true;

    for(int i=x+1, j=y-1; i<GRID_X && j>=0 && grids[i][j]==lookingFor ; ++i, --j, ++diag2);
    for(int i=x-1, j=y+1; i>=0 && j<GRID_Y && grids[i][j]==lookingFor ; --i, ++j, ++diag2);

    if(diag2 >= 4)
        return true;

    return false;
}

int ConnectFourIA::Grid::eval(bool player1) const
{
    int scoreP1=0, scoreP2=0;

    for(int i=0 ; i<GRID_X ; ++i)
    {
        for(int j=0 ; j<GRID_Y ; ++j)
        {
            if(read(i,j) != EMPTY)
                continue;

            scoreP1 += innerEvalPos(i,j, P1);
            scoreP2 += innerEvalPos(i,j, P2);
        }
    }

    return player1 ? scoreP1-scoreP2 : scoreP2-scoreP1;
}

int ConnectFourIA::Grid::innerEvalPos(int x, int y, Slot lookingFor) const
{
    Slot lineBreaker = lookingFor==P1?P2:P1;

    int vertical=0, horizontal=0, diag1=0, diag2=0;

    for(int i=x+1; i<GRID_X && grids[i][y]!=lineBreaker && horizontal<3; ++i, ++horizontal);
    for(int i=x-1; i>=0 && grids[i][y]!=lineBreaker  && horizontal<3; --i, ++horizontal);

    for(int i=y+1; i<GRID_Y && grids[x][i]!=lineBreaker && vertical<3; ++i, ++vertical);
    for(int i=y-1; i>=0 && grids[x][i]!=lineBreaker && vertical<3; --i, ++vertical);

    for(int i=x+1, j=y+1; i<GRID_X && j<GRID_Y && grids[i][j]!=lineBreaker && diag1<3; ++i, ++j, ++diag1);
    for(int i=x-1, j=y-1; i>=0 && j>=0 && grids[i][j]!=lineBreaker && diag1<3; --i, --j, ++diag1);

    for(int i=x+1, j=y-1; i<GRID_X && j>=0 && grids[i][j]!=lineBreaker && diag2<3; ++i, --j, ++diag2);
    for(int i=x-1, j=y+1; i>=0 && j<GRID_Y && grids[i][j]!=lineBreaker && diag2<3; --i, ++j, ++diag2);

    int score=0;
    if(vertical>=3) ++score;
    if(horizontal>=3) ++score;
    if(diag1>=3) ++score;
    if(diag2>=3) ++score;

    return score;
}

/* minimax algo */
int ConnectFourIA::evalComputerPlay(const Grid& grid, int col, int depth, core::ivec2 a_b)
{
    Grid newGrid = grid.play(!_humanP1, col);
    if(grid.checkWinner(col, grid.stackSize[col], !_humanP1))
        return INF;
    else if(depth >= _maxDepth)
        return newGrid.eval(!_humanP1);
    else
    {
        // check all the plays the human will do and suppose he minimize the score
        int minimizer = INF;
        for(int i=0 ; i<GRID_X ; ++i)
        {
            if(newGrid.checkCol(i))
            {
                minimizer = std::min(evalHumanPlay(newGrid, i, depth+1, a_b), minimizer);
                if(minimizer <= a_b.x())
                    return minimizer;
                a_b.y() = std::min(a_b.y(), minimizer);
            }
        }

        return minimizer;
    }
}

int ConnectFourIA::evalHumanPlay(const Grid& grid, int col, int depth, core::ivec2 a_b)
{
    Grid newGrid = grid.play(_humanP1, col);
    if(grid.checkWinner(col, grid.stackSize[col], _humanP1))
        return -INF;
    else if(depth >= _maxDepth) // eval leaf
       return newGrid.eval(!_humanP1);
    else
    {
        // check all the plays the computer will do and suppose he maximize the score
        int maximizer = -INF;
        for(int i=0 ; i<GRID_X ; ++i) // eval leaf
        {
            if(newGrid.checkCol(i))
            {
                maximizer = std::max(evalComputerPlay(newGrid, i, depth+1, a_b), maximizer);
                if(maximizer >= a_b.y())
                    return maximizer;
                a_b.x() = std::max(a_b.x(), maximizer);
            }
        }

        return maximizer;
    }
}

bool ConnectFourIA::humanTurn() const
{
    return ((_humanP1 && _P1_turn) || (!_humanP1 && !_P1_turn));
}



