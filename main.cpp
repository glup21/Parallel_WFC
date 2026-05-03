#include "wfc_generator.h"
#include "tileset.h"
#include "grid.h"
#include "image_generator.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <filesystem>

using std::unique_ptr;
using namespace std;
int main(int argc, char *argv[])
{
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <tileset name> <grid width> <grid height> <mode>\n";
        return 1;
    }

    int gridWidth = std::stoi(argv[2]);
    int gridHeight = std::stoi(argv[3]);
    string strMode = argv[4];

    string path = "../Tiles/" + std::string(argv[1]) + "/config.json";
        cout << "CWD: " << std::filesystem::current_path() << endl;
    cout << "Path: " << path << endl;
    //string path = std::string(argv[1]);
    Tileset ts(path);
    
    //std::cout << ts << std::endl;

    unique_ptr<Grid> grid;
    do {

        grid.reset(new Grid(ts, gridWidth, gridHeight));

        WFCGenerator generator(*grid, ts, strMode);
        auto start = std::chrono::high_resolution_clock::now();

        generator.collapseGrid();

        auto end = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "Time taken to collapse grid: " << elapsed.count() << " seconds\n";
        std::cout << std::endl;
        
    } while(!grid->isValid());
    ImageGenerator::saveImage(*grid, ts, "image.png");
    
    return 0;
}