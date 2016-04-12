#ifndef FINITEGRID_H
#define FINITEGRID_H

#include "core.h"
#include "Case.h"

class FiniteGrid
{
public:
    FiniteGrid(const uivec2& s = {512,512});

    uivec2 size() const;

    const Case& operator()(uint, uint) const;
    Case& operator()(uint, uint);

private:
    const uivec2 _SIZE;
    std::unique_ptr<Case[]> _map;

    uint convertPos(uint x, uint y) const { return y*_SIZE.x()+x; }
};

inline uivec2 FiniteGrid::size() const { return _SIZE; }

#endif // FINITEGRID_H
