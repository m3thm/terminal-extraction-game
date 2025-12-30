#include "map.h"
#include <stdlib.h>
#include <stddef.h>

int initMap(Map *map, int N)
{
    if (!map || (N < 5 || N > 15)) return 0;

    map->N = N;
    map->grid = NULL;
    map->data = NULL;

    map->grid = (CellType**)malloc((size_t)N * sizeof(CellType*));
    if (!map->grid) return 0;

    map->data = (CellType*)malloc((size_t)N * (size_t)N * sizeof(CellType));
    if (!map->data) {
        free(map->grid);
        map->grid = NULL;
        return 0;
    }

    for (int r = 0; r < N; r++) {
        map->grid[r] = &map->data[r * N];
    }

    fillMap(map, EMPTY);

    return 1;
}

void freeMapData(Map *map)
{
    if (!map) return;

    free(map->data);
    free(map->grid);

    map->data = NULL;
    map->grid = NULL;
    map->N = 0;
}

int isWithinBounds(const Map *map, int row, int col)
{
    if (!map) return 0;
    return (row >= 0 && col >= 0 && row < map->N && col < map->N);

}

int getCellType(const Map *map, int row, int col, CellType *out)
{
    if (!out || !isWithinBounds(map, row, col)) return 0;
    *out = map->grid[row][col];   
    return 1;
}

int setCellType(Map *map, int row, int col, CellType type)
{
    if (!isWithinBounds(map, row, col)) return 0;
    map->grid[row][col] = type;
    return 1;
}

int isCellEmpty(const Map *map, int row, int col)
{
    if (!isWithinBounds(map, row, col)) return 0;
    return  map->grid[row][col] == EMPTY;
}

int isCellWall(const Map *map, int row, int col)
{
    if (!isWithinBounds(map, row, col)) return 0;
    return  map->grid[row][col] == WALL;
}

void fillMap(Map *map, CellType type)
{
    if (!map || !map->data) return;

    int totalCells = map->N * map->N;
    for (int i = 0; i < totalCells; i++) {
        map->data[i] = type;
    }
}

int countCellType(const Map *map, CellType type)
{
    if (!map || !map->data) return 0;

    int count = 0;
    int totalCells = map->N * map->N;
    for (int i = 0; i < totalCells; i++) {
        if (map->data[i] == type) count++;
    }
    return count;
}

int findRandomEmpty(const Map *map, int *out_row, int *out_col)
{
    if (!map || !map->data || !out_row || !out_col) return 0;

    int totalCells = map->N * map->N;
    int *empty = (int*)malloc((size_t)totalCells * sizeof(int));
    if (!empty) return 0;

    int emptyCount = 0;
    for (int i = 0; i < totalCells; i++) {
        if (map->data[i] == EMPTY) {
            empty[emptyCount++] = i;
        }
    }

    if (emptyCount == 0) {
        free(empty);
        return 0;
    }

    int result = empty[rand() % emptyCount];
    free(empty);

    *out_row = result / map->N;
    *out_col = result % map->N;
    return 1;
}

int placeRandom(Map *map, CellType type, int count)
{
    if (!map || count <= 0) return 0;

    int placed = 0;
    for (int i = 0; i < count; i++) {
        int row, col;
        if (!findRandomEmpty(map, &row, &col)) break;
        setCellType(map, row, col, type);
        placed++;
    }
    return placed;
}

char cellToChar(CellType type)
{
    switch (type) {
        case EMPTY:      return '.';
        case WALL:       return '#';
        case INTEL:      return 'I';
        case LIFE:       return 'L';
        case EXTRACTION: return 'X';
        default:              return '?';
    }
}
