
#ifndef GAME_H
#define GAME_H

#include <stdio.h>     
#include "map.h"
#include "player.h"

typedef struct {
    int N;                 
    int numPlayers;       
    int startingLives;    
    int requiredIntel;    
    int numIntelItems;    
    int numLifeItems;      
    int wallCount;         
    unsigned int seed;     
} GameConfig;

typedef enum {
    OK,
    OUT_OF_BOUNDS,
    HIT_WALL,
    BAD_INPUT,
    EXIT_GAME
} MoveResult;

typedef struct {
    MoveResult result;
    int moved;            
    int collectedIntel;    
    int collectedLife;     
    int becameInactive;    
    int won;               
} TurnOutcome;

typedef struct {
    Map map;

    Player players[MAX_PLAYERS];
    int numPlayers;

    int currentPlayer;
    int turnNumber;

    int gameOver;
    int winnerIndex;      

    int requiredIntel;

    int extractionRow;
    int extractionCol;

    FILE *logFile;
} GameState;

int initGame(GameState *game, const GameConfig *config, const char *logFilename);
void freeGame(GameState *game);
TurnOutcome ApplyTurn(GameState *game, char inputChar);
int advanceToNextPlayer(GameState *game);
int countActivePlayers(const GameState *game);

#endif
