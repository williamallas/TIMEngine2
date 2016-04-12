#include "FiniteGrid.h"

FiniteGrid::FiniteGrid(const uivec2& s) : _SIZE(s), _map(std::unique_ptr<Case[]>(new Case[s.dot(s)]))
{

}

const Case& FiniteGrid::operator()(uint x, uint y) const
{
    return _map.get()[convertPos(x,y)];
}

Case& FiniteGrid::operator()(uint x, uint y)
{
    return _map.get()[convertPos(x,y)];
}

