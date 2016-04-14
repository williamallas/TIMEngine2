#ifndef PASCALETRIANGLE_H_INCLUDED
#define PASCALETRIANGLE_H_INCLUDED

#include "type.h"

namespace tim
{
namespace core
{

    class PascaleTriangle
    {
    private:
        vector<vector<int>> _data; // This is the actual data

    public:
        PascaleTriangle(uint dummy)
        {
            if (dummy > 0)
            {
                vector<int> row;
                _data.resize(dummy);
                // The first row
                row.resize(1);
                row.at(0) = 1;
                _data.at(0) = row;
                // The second row
                if (_data.size() > 1){
                    row.resize(2);
                    row.at(0) = 1; row.at(1) = 1;
                    _data.at(1) = row;
                }
                // The other rows
                if (_data.size() > 2)
                {
                    for (uint i = 2; i < _data.size(); i++)
                    {
                        row.resize(i + 1); // Theoretically this should work faster than consecutive push_back()s
                        row.front() = 1;
                        for (uint j = 1; j < row.size() - 1; j++)
                            row.at(j) = _data.at(i - 1).at(j - 1) + _data.at(i - 1).at(j);
                        row.back() = 1;
                        _data.at(i) = row;
                    }
                }
            }
        }

        int getCoeff(uint dummy1, uint dummy2) const
        {
            int result = 0;
            if ((dummy1 < _data.size()) && (dummy2 < _data.at(dummy1).size()))
                    result = _data.at(dummy1).at(dummy2);
            return result;
        }

        const vector<int>& getRow(uint dummy) const
        {
            return _data[dummy];
        }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // PASCALETRIANGLE_H_INCLUDED
