
#ifndef MAP_H
#define MAP_H

/*
 *  Map Module:
 *  responsible for dynamically allocating a N x N grid,
 *  storing the initial cell type of each cell on the grid,
 *  and providing helper functions that give access to the grid
 * */

/*
 * Cell types stored in the grid
 * */
typedef enum {
    EMPTY,
    WALL,
    INTEL,
    LIFE,
    EXTRACTION
} CellType;

/*
 *  The Data Structure of the Map.
 *  N - the dimension of the grid
 *  grid - a 2D array of rows and columns
 *  data - all the data in the grid stored in a contigious 1D form (easier for memory management)
 * */
typedef struct {
    int N;
    CellType **grid;
    CellType *data;
} Map;

/*
 *  Initialize the map of size N x N by allocating memory for the grid and data, and assign empty cell types
 *  to each cell on the grid.
 *  Returns 1 if succesful, 0 if failed.
 * */
int initMap(Map *m, int N);

/*
 *  Frees all memory that was associated with the map. Then sets the data structures to NULL.
 * */
void freeMapData(Map *m);

/*
 *  Returns 1 if the cell at row and col are within map bounds, 0 if otherwise.
 * */
int isWithinBounds(const Map *m, int row, int col);

/*
 *  Get the cell type of the cell at row and col. 
 *  Return 1 if succesful, 0 if otherwise.
 * */
int getCellType(const Map *m, int row, int col, CellType *out);

/*
 *  Set the cell type of the cell at row and col.
 *  Return 1 if succesful, 0 if otherwise.
 * */
int setCellType(Map *m, int row, int col, CellType type);

/*
 *  Checks if the cell is an empty cell.
 *  Return 1 if true, 0 if false.
 * */
int isCellEmpty(const Map *m, int row, int col);

/*
 *  Checks if the cell is a Wall.
 *  Return 1 if true, 0 if false.
 * */
int isCellWall(const Map *m, int row, int col);

/*
 *  Fill the entire map with a single cell type.
 * */
void fillMap(Map *m, CellType type);

/*
 *  Count how many cells exist inside the grid of a given cell type.
 * */
int countCellType(const Map *m, CellType type);

/*
 *  Find a random empty cell inside the map. Write out it's coordinates to out_row and out_col.
 * */
int findRandomEmpty(const Map *m, int *out_row, int *out_col);

/*
 *  Place some amount of cells on the map at random, determined by count.
 * */
int placeRandom(Map *m, CellType type, int count);

/*
 *  Convert a given cell type to its display character.
 * */
char cellToChar(CellType type);

#endif 
