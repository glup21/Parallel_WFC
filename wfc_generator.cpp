#include "wfc_generator.h"
#include <queue>
#include <memory>
#include <unordered_set>
#include <future>
#include <mutex>
#include <thread>
#include <omp.h>
using std::queue, std::unordered_set;
using namespace std;
Mode stringToMode(const std::string& str)
{
    if (str == "seq")
        return SEQUENTIAL;
    else if (str == "chunk")
        return CHUNK;
    else
        return SEQUENTIAL;
}

WFCGenerator::WFCGenerator(Grid& grid, Tileset& ts, const string& strMode): grid(grid), ts(ts), mode(stringToMode(strMode))
{

}


void WFCGenerator::collapseGrid()
{
    if(mode == SEQUENTIAL)
    {
        sequentialGridCollapse();
    }
    else if(mode == CHUNK)
    {
        chunkGridCollapse();
    }
}

void WFCGenerator::sequentialGridCollapse()
{
    Cell* changed_cell = initGrid();
    int steps = 0;
    // grid.printGridEnthropy();
    GridChunk gridChunk{
        0,
        grid.getX(),
        0,
        grid.getY(),
        0
    };
    while(true)
    {

        updateGrid(changed_cell, gridChunk);

        //std::cout << "Step: " << steps << "\n";
        steps++;
        //grid.printGridEnthropy();
        
        changed_cell = collapseLeastEnthropy(gridChunk);

        if(changed_cell == nullptr)
            break;


    }
    std::cout << std::endl;
}

void WFCGenerator::chunkGridCollapse()
{
    initGrid();
    vector<GridChunk> chunks = splitGridIntoChunks();

    vector<GridChunk> blackChunks, whiteChunks;
    for(auto& chunk : chunks)
    {
        if(chunk.phase == 0) blackChunks.push_back(chunk);
        else whiteChunks.push_back(chunk);
    }

    // Phase 1 - black chunks, fully independent
    #pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < blackChunks.size(); i++)
    {
        Cell* changed_cell = collapseLeastEnthropy(blackChunks[i]);
        while(changed_cell != nullptr)
        {
            updateGrid(changed_cell, blackChunks[i]);
            changed_cell = collapseLeastEnthropy(blackChunks[i]);
        }
    }

    // Phase 2 - white chunks, borders are fully determined by phase 1
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < whiteChunks.size(); i++)
    {
        // while (true)
        // {
            // auto chunkBackup = grid.getChunkTiles(whiteChunks[i]);

            Cell* changed_cell = collapseLeastEnthropy(whiteChunks[i]);

            while (changed_cell != nullptr)
            {
                updateGrid(changed_cell, whiteChunks[i]);
                changed_cell = collapseLeastEnthropy(whiteChunks[i]);
            }

            // if (grid.isValid(whiteChunks[i]))
            // {
            //     break; // success
            // }

            // // failure → restore and retry same chunk
            // grid.resetChunk(whiteChunks[i], chunkBackup);
        // }
    }
}

vector<GridChunk> WFCGenerator::splitGridIntoChunks()
{
    vector<GridChunk> res;
    int cores = omp_get_num_procs();
    int oversubscription = 1;

    int totalChunks = cores * oversubscription * 2;
    int chunksPerAxis = (int)ceil(sqrt((float)totalChunks));

    int paddedX = ((grid.getX() + chunksPerAxis - 1) / chunksPerAxis) * chunksPerAxis;
    int paddedY = ((grid.getY() + chunksPerAxis - 1) / chunksPerAxis) * chunksPerAxis;

    int chunkSizeX = paddedX / chunksPerAxis;
    int chunkSizeY = paddedY / chunksPerAxis;

    cout << "Chunks per axis: " << chunksPerAxis << endl;
    cout << "Chunk size: " << chunkSizeX << "x" << chunkSizeY << endl;

    // Warn if chunks are below lower bound
    if (chunkSizeX < 4 || chunkSizeY < 4)
        cout << "WARNING: chunk size below lower bound, consider rule propagation instead" << endl;

    for (int cy = 0; cy < chunksPerAxis; cy++)
    {
        for (int cx = 0; cx < chunksPerAxis; cx++)
        {
            GridChunk chunk;
            chunk.startX = cx * chunkSizeX;
            chunk.startY = cy * chunkSizeY;

            chunk.endX = min(chunk.startX + chunkSizeX, grid.getX());
            chunk.endY = min(chunk.startY + chunkSizeY, grid.getY());

            chunk.phase = (cx + cy) % 2;
            res.push_back(chunk);
        }
    }

    return res;
}

void WFCGenerator::updateGrid(Cell* changed_cell, GridChunk chunk)
{
    std::queue<Cell*> cells_to_update;
    cells_to_update.push(changed_cell);

    unordered_set<Cell*> visited_cells;

    while (!cells_to_update.empty()) {
        
        Cell* current_cell = cells_to_update.front();
        cells_to_update.pop();

        if(visited_cells.find(current_cell)!= visited_cells.end())
            continue;

        visited_cells.insert(current_cell);

        // grid.printGridEnthropy();
        // std::cout << "\n";

        int current_x = current_cell->getX();
        int current_y = current_cell->getY();

        for (int i : const_dir) {
            Cell* neighbor = nullptr;
            switch (i) {
                case UP:
                    neighbor = grid.getCell(current_x, current_y-1);
                    break;
                case RIGHT:
                    neighbor = grid.getCell(current_x+1, current_y);
                    break;
                case DOWN:
                    neighbor = grid.getCell(current_x, current_y+1);
                    break;
                case LEFT:
                    neighbor = grid.getCell(current_x-1, current_y);
                    break;
            }

            if (neighbor == nullptr || neighbor->getEnthropy() <= 1)
                continue;

            // if (neighbor->getX() < chunk.startX || neighbor->getX() >= chunk.endX ||
            //     neighbor->getY() < chunk.startY || neighbor->getY() >= chunk.endY)
            //     continue;

            bool updated = neighbor->update(current_cell->getTiles(), i);

            if (updated) {
                cells_to_update.push(neighbor);
            }
        }
    }
}

Cell* WFCGenerator::initGrid()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> distr(0, grid.getX()-1);
    int x = distr(gen);
    distr = std::uniform_int_distribution<>(0, grid.getY()-1);
    int y = distr(gen);

    Cell* collapsed_cell = grid.getCell(x,y);

    if(collapsed_cell == nullptr)
    {
        std::cout << "Cell not found!" << std::endl;
    }
    else
        collapsed_cell->collapse();

    return collapsed_cell;

}
Cell* WFCGenerator::collapseLeastEnthropy(GridChunk chunk)
{
    Cell* res = grid.getLeastEnthropy(chunk);

    if(res == nullptr)
        return res;
    res->collapse();

    return res;
}