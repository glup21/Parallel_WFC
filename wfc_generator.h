#ifndef WFCGENERATOR_H
#define WFCGENERATOR_H

#include "tileset.h"
#include "grid.h"
#include <random>
#include "sides.h"

enum Mode
{
    SEQUENTIAL,
    CHUNK
};

Mode stringToMode(const std::string& str);

class WFCGenerator
{
    Grid& grid;
    Tileset& ts;
    Mode mode;

    Cell* initGrid();
    void updateGrid(Cell* changed_cell);
    Cell* collapseLeastEnthropy();

    void sequentialGridCollapse();
    void chunkGridCollapse();

public:
    WFCGenerator(Grid& grid, Tileset& ts, const string& strMode);
    ~WFCGenerator() = default;

    void collapseGrid();
    
};

#endif