#include "game.h"
#include <stdlib.h>  
#include <time.h>

int initGame(GameState *game, const GameConfig *config, const char *logFilename)
{
    if (!game || !config) return 0;
    if (config->numPlayers < 1 || config->numPlayers > MAX_PLAYERS) return 0;

    /* ---------- Initialise bookkeeping ---------- */
    game->numPlayers    = config->numPlayers;
    game->currentPlayer = 0;
    game->turnNumber    = 0;
    game->gameOver      = 0;
    game->winnerIndex   = -1;
    game->requiredIntel = config->requiredIntel;
    game->logFile       = NULL;

    /* ---------- Seed RNG once for the whole game ---------- */
    if (config->seed != 0) {
        srand(config->seed);
    } else {
        srand((unsigned)time(NULL));
    }

    /* ---------- Initialise map ---------- */
    if (!initMap(&game->map, config->N)) {
        return 0;
    }

    /* ---------- Generate base map contents ---------- */
    /* Walls first so items don't overwrite them */
    placeRandom(&game->map, WALL, config->wallCount);

    /* Place collectibles and extraction point */
    placeRandom(&game->map, INTEL, config->numIntelItems);
    placeRandom(&game->map, LIFE,  config->numLifeItems);

    /* Extraction: exactly one */
    int extractionRow, extractionCol;
    if (!findRandomEmpty(&game->map, &extractionRow, &extractionCol)) {
        freeMapData(&game->map);
        return 0;
    }
    setCellType(&game->map, extractionRow, extractionCol, EXTRACTION);
    game->extractionRow = extractionRow;
    game->extractionCol = extractionCol;

    /* ---------- Initialise players ---------- */
    for (int i = 0; i < game->numPlayers; i++) {
        int playerRow, playerCol;

        /* Find a valid empty starting position */
        if (!findRandomEmpty(&game->map, &playerRow, &playerCol)) {
            freeMapData(&game->map);
            return 0;
        }

        /* Assign symbols deterministically */
        char symbol = (i == 0) ? '@' : (i == 1) ? '&' : '$';

        initPlayer(&game->players[i],
                   symbol,
                   HUMAN,        /* can change later for AI */
                   playerRow, playerCol,
                   config->startingLives);
    }

    /* ---------- Open log file (required by spec) ---------- */
    if (logFilename) {
        game->logFile = fopen(logFilename, "w");
        if (!game->logFile) {
            freeMapData(&game->map);
            return 0;
        }
    }

    return 1;
}

void freeGame(GameState *game)
{
    if (!game) return;

    /* Close log file if opened */
    if (game->logFile) {
        fclose(game->logFile);
        game->logFile = NULL;
    }

    /* Free map memory */
    freeMapData(&game->map);

    /* Clear remaining state (defensive, not strictly required) */
    game->numPlayers    = 0;
    game->currentPlayer = 0;
    game->turnNumber    = 0;
    game->gameOver      = 0;
    game->winnerIndex   = -1;
}


