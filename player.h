
#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

typedef struct {
    char symbol;       
    PlayerType type;   
    int row;
    int col;
    int lives;
    int intel;
    int active;        
} Player;

void initPlayer(Player *player, char symbol, PlayerType type, int startRow, int startCol, int startLives);
void deactivatePlayer(Player *player);
int isPlayerActive(const Player *player);

#endif
