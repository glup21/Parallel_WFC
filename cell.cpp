#include "cell.h"
#include <iostream>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <future>
using std::unordered_set;
Cell::Cell(vector<shared_ptr<Tile>> tiles, int x, int y, Tileset& ts):
 tiles(tiles), enthropy(tiles.size()), 
 x(x), y(y), ts(ts)
{
    omp_init_lock(&lock);
}

Cell::~Cell()
{
    omp_destroy_lock(&lock);
}

void Cell::collapse()
{
    omp_set_lock(&lock);
    if(tiles.empty())
    {
        std::cout << "Cell is empty!\n";

        omp_unset_lock(&lock);
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, tiles.size()-1);


    enthropy = 1;
    shared_ptr<Tile> new_tile = tiles.at(distr(gen));
    tiles = vector<shared_ptr<Tile>> {new_tile};

    omp_unset_lock(&lock);
}

void Cell::resetTiles(vector<shared_ptr<Tile>> newTiles)
{
    this->tiles = newTiles;
    this->enthropy = newTiles.size();
}

vector<shared_ptr<Tile>> Cell::getTiles() 
{
    omp_set_lock(&lock);
    auto res = tiles;
    omp_unset_lock(&lock);
    return res;
}

bool Cell::update(vector<shared_ptr<Tile>> neigh_tiles, int direction)
{
    omp_set_lock(&lock);
    
    bool updated = false;
    int checking_side = rotateSide(direction); // Side to which we compare neighbouring tiles
    unordered_set<string> valid_ids; // Store valid tile IDs for quick lookup

    for (shared_ptr<Tile> neigh_tile : neigh_tiles)
    {
        // Convert the rules to an unordered_set for quick lookup
        const vector<string>& rule = neigh_tile->getRules().at(direction);
        valid_ids.insert(rule.begin(), rule.end());
    }

    unordered_set<shared_ptr<Tile>> new_tiles;
    for (shared_ptr<Tile> possible_tile : tiles)
    {
        // Check if the possible tile ID is in the valid IDs set
        if (valid_ids.find(possible_tile->getId()) != valid_ids.end())
        {
            new_tiles.insert(possible_tile);
        }
    }

    if (tiles.size() == new_tiles.size())
    {
        omp_unset_lock(&lock);
        return false; // No change in possible tiles
        
    }

    tiles.assign(new_tiles.begin(), new_tiles.end()); // Update possible tiles
    enthropy = tiles.size(); // Update entropy
    omp_unset_lock(&lock);
    return true;
}


// bool Cell::update(vector<shared_ptr<Tile>> neigh_tiles, int direction)
// {
//     bool updated = false;
//     int checking_side = rotateSide(direction); //Side to which we compare neighbouring tiles
//     unordered_set<shared_ptr<Tile>> new_tiles;

//     for(shared_ptr<Tile> neigh_tile : neigh_tiles) //We take pointer to each possible tile from neighbopuring cell
//     {
//         vector<string> rule = neigh_tile->getRules().at(direction); // Take possible neighbours for possible neigh tile

//         for(shared_ptr<Tile> possible_tile : tiles) //For each possible tile in this cell
//         {
//             if(std::find(rule.begin(), rule.end(), possible_tile->getId()) != rule.end())
//                 new_tiles.insert(possible_tile); //We take it to an updated cell        
            
//         }
//     }

//     if(tiles.size() == new_tiles.size())
//         return false;

//     tiles.assign(new_tiles.begin(), new_tiles.end());

//     enthropy = tiles.size();
//     return true;
// }
