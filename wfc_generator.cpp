#include "wfc_generator.h"
#include <queue>
#include <memory>
#include <unordered_set>
#include <future>
#include <mutex>
#include <thread>
using std::queue, std::unordered_set;

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
    grid.printGridEnthropy();
    while(true)
    {

        updateGrid(changed_cell);

        //std::cout << "Step: " << steps << "\n";
        steps++;
        //grid.printGridEnthropy();
        
        changed_cell = collapseLeastEnthropy();

        if(changed_cell == nullptr)
            break;


    }
    std::cout << std::endl;
}

void WFCGenerator::chunkGridCollapse()
{
    Cell* changed_cell = initGrid();
    int steps = 0;
    grid.printGridEnthropy();
    while(true)
    {

        updateGrid(changed_cell);

        //std::cout << "Step: " << steps << "\n";
        steps++;
        //grid.printGridEnthropy();
        
        changed_cell = collapseLeastEnthropy();

        if(changed_cell == nullptr)
            break;


    }
    std::cout << std::endl;
}


void WFCGenerator::updateGrid(Cell* changed_cell)
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

Cell* WFCGenerator::collapseLeastEnthropy()
{
    Cell* res = grid.getLeastEnthropy();

    if(res == nullptr)
        return res;
    res->collapse();

    return res;
}