#ifndef WFCGENERATOR_H
#define WFCGENERATOR_H

#include "tileset.h"
#include "grid.h"
#include <random>
#include "sides.h"

class Grid;

enum Mode
{
    SEQUENTIAL,
    CHUNK,
    PARALLEL
};

struct GridChunk
{
    int startX;
    int endX;

    int startY;
    int endY;

    int phase;
};

Mode stringToMode(const std::string& str);

class WFCGenerator
{
    Grid& grid;
    Tileset& ts;
    Mode mode;

    Cell* initGrid();
    void updateGrid(Cell* changed_cell, GridChunk chunk);
    Cell* collapseLeastEnthropy(GridChunk chunk);

    void sequentialGridCollapse();
    void chunkGridCollapse();

    vector<GridChunk> splitGridIntoChunks();

    int chunkLowerBound = 4;
    int chunkHigherBound = 20;

public:
    WFCGenerator(Grid& grid, Tileset& ts, const string& strMode);
    ~WFCGenerator() = default;

    void collapseGrid();
    
};

#endif