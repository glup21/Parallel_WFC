# Project Wave Function Collapse procedure texture generation

This program generates new textures from small samples (Tiles).\
Read https://github.com/mxgmn/WaveFunctionCollapse for more.

## Terminology
Tile: image with set of rules, which define which tiles are allowed to be neighbours of this one.\
Tileset: set of Tiles.\
Cell: cell which contains possible Tiles. Can be in collapsed and undefined state. In undefined\
    state cell contains more than one possible Tile or none (which means contradiction and error).\
    In collapsed state Cell has only one possible Tile.\
Grid: 2D array of Cells. \
Enthropy: number of possible states of Cell.

## Prerequisites
You will need opencv for image handling and nlohmann json parser for config parsing.

Ubuntu:
> sudo apt update\
> sudo apt install libopencv-dev\
> sudo apt-get install nlohmann-json-dev


Arch:
> sudo pacman -S opencv
> sudo pacman -S nlohmann-json

### Installing
> mkdir build \
> cd build\
> cmake ..\
> make -j

### Running

Program is launched from bash.\
Syntax of launching:
* > ./WFCGenerator "Tileset name" "grid width" "grid height" "mode"
* > Example: ./WFCGenerator circuit 20 20 chunk

Tileset name is defined by folder name in folder Tiles!\
F.e if you want to use Tileset "circuit" you should create ./Tiles/circuit.\
This folder must contain config.json with Tileset settings!

**Parallelisation** can be turned on by changing the **mode** in which the program operates. Exists two options:
1. seq - sequential generation
2. chunk - parallel processing using chunk split

## Program functioning and structure.

Program starts with reading config.json in "./Tiles/ Tileset name /" \
From there it reads tiles: path to their images, their side structure and parameter "rotate", which\
defines if program should automatically create rotated versions of this tile (see config structure below)\
After reading config Tileset initializes and saves shared_ptr to each Tile. shared_ptr is used for\
further accessing to them from another parts of program, such as Grid or Cell.

After initializing Tileset Grid is created. Grid creates Cells which contain vector of shared_ptr to Tiles in Tileset.\
\
After that WFCGenerator class starts generating image by collapsing random Cell in Grid.\
After that cycle is started:
1. Update Grid enthropies.
2. Collapse Cell with least enthropy.
3. Repeat 1-2 untill all cells are collapsed.

Grid is updated by BFS. Each cell, after updating its state, updates all its neighbours, by sending them all possible states of this one.\
When Cell is updating it recieves requirments with all possible Tiles it can be, and if this Cell has Tiles which are not
in those requirments it would delete them. \
By constantly repeating this process all cells are collapsed to satisfy rules of another neighbouring cells.

After collapsing a Grid it is checked for errors (f.e Cell has 0 possible Tiles). If contradiction occured - whole\
generation starts over.

After successfully collapsing Grid it is exported as image.png in folder in which program started.

## Config structure
Config starts with node "tiles" which contain array of nodes, which contain Tiles of Tileset.\
Keys of those nodes are perceived as ID for tiles by a program.\
Those nodes must contain:
* "address" key with address to a Tile image starting from build directory of a program.
* "sides" key with array of 4 elements which contain string description of a Tile image.
Each Tile should be divided into 2D grid, where each cell is assigned some symbol to represent it. 
F.e image of '+' symbol 3x3 pixels should be divided into grid 3x3, where 1 represents white pixel
and 0 - black one. Sides are read in clockwise order UP-RIGHT-DOWN-LEFT: ["010", "010", "010", "010]. Any symbols or string can be used for side representation, symbol-by-pixel is just recommended for non-symmetrical tile support.
* "rotate" key which defines should be rotated by a program (NOTE: it cant diferentiate should tile be rotated two or four
times, which can cause performance issues if done incorrectly, so its recommended to provide rotated tiles beforehand, or\
avoid using Tiles which can be rotated only two times!).

## Future
In the future is planned:
- [ ] Backtracking for resolving contradictions.
- [ ] Automatic Tile generation from sample image.
- [ ] GUI