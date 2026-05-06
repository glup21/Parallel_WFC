#ifndef GRID_H
#define GRID_H

#include <vector>
#include "cell.h"
#include "tileset.h"
#include "wfc_generator.h"
using std::vector;
class GridChunk;

class Grid
{
    int xSize; // column
    int ySize; // row
    vector<vector<Cell>> cells;

public:
    Grid(Tileset& ts, int x, int y);
    ~Grid() = default;

    Cell* getCell(int x, int y);
    int getX()const{return xSize;}
    int getY()const{return ySize;}
    bool isCollapsed();
    bool isValid()const;
    bool isValid(GridChunk chunk) const;
    Cell* getLeastEnthropy(GridChunk chunk);
    vector<vector<Cell>>& getCells(){return cells;}
    void resetChunk(GridChunk chunk, vector< vector<vector<shared_ptr<Tile> >>> resetConfiguration);
    vector< vector<vector<shared_ptr<Tile> >>> getChunkTiles(GridChunk chunk);


    void printGridEnthropy();
    void printGridTiles();
    void printGridCollapsedTiles();

};

#endif