#include "game.h"
#include <stdlib.h>  
#include <time.h>
#include <ctype.h>

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

/* ---------- Internal helpers (private to game.c) ---------- */

static void commandToDelta(char cmd, int *dr, int *dc, MoveResult *outResult)
{
    *dr = 0;
    *dc = 0;

    cmd = (char)toupper((unsigned char)cmd);

    switch (cmd) {
        case 'W': *dr = -1; *dc =  0; *outResult = OK;        break;
        case 'A': *dr =  0; *dc = -1; *outResult = OK;        break;
        case 'S': *dr =  1; *dc =  0; *outResult = OK;        break;
        case 'D': *dr =  0; *dc =  1; *outResult = OK;        break;
        case 'Q': *outResult = EXIT_GAME;                     break;
        default:  *outResult = BAD_INPUT;                     break;
    }
}

static void logTurn(GameState *game, int playerIndex, char inputChar, const TurnOutcome *outcome)
{
    if (!game || !game->logFile || !outcome) return;

    Player *p = &game->players[playerIndex];

    fprintf(game->logFile,
            "Turn %d | Player %c | Input '%c' | Result %d | Pos (%d,%d) | Lives %d | Intel %d\n",
            game->turnNumber,
            p->symbol,
            inputChar,
            outcome->result,
            p->row, p->col,
            p->lives,
            p->intel);

    /* Optional: dump the board after the move (good for debugging/viva) */
    for (int r = 0; r < game->map.N; r++) {
        for (int c = 0; c < game->map.N; c++) {
            /* overlay players in the log the same way UI does */
            char printed = 0;
            for (int i = 0; i < game->numPlayers; i++) {
                Player *pp = &game->players[i];
                if (pp->active && pp->row == r && pp->col == c) {
                    fputc(pp->symbol, game->logFile);
                    printed = 1;
                    break;
                }
            }
            if (!printed) {
                CellType t;
                getCellType(&game->map, r, c, &t);
                fputc(cellToChar(t), game->logFile);
            }
        }
        fputc('\n', game->logFile);
    }

    fputs("----\n", game->logFile);
    fflush(game->logFile);
}

/* ---------- Required functions ---------- */

int countActivePlayers(const GameState *game)
{
    if (!game) return 0;

    int count = 0;
    for (int i = 0; i < game->numPlayers; i++) {
        if (game->players[i].active) count++;
    }
    return count;
}

int advanceToNextPlayer(GameState *game)
{
    if (!game || game->numPlayers <= 0) return 0;

    /* If nobody is active, nothing to advance to */
    if (countActivePlayers(game) == 0) return 0;

    int start = game->currentPlayer;
    int idx = start;

    do {
        idx = (idx + 1) % game->numPlayers;
        if (game->players[idx].active) {
            game->currentPlayer = idx;
            return 1;
        }
    } while (idx != start);

    return 0;
}

TurnOutcome ApplyTurn(GameState *game, char inputChar)
{
    TurnOutcome out;
    out.result = BAD_INPUT;
    out.moved = 0;
    out.collectedIntel = 0;
    out.collectedLife = 0;
    out.becameInactive = 0;
    out.won = 0;

    if (!game || game->gameOver) return out;

    int pIndex = game->currentPlayer;
    Player *p = &game->players[pIndex];

    /* If current player is inactive, skip them (main loop can call advance too) */
    if (!p->active) {
        out.result = BAD_INPUT;
        return out;
    }

    int dr, dc;
    MoveResult parseResult;
    commandToDelta(inputChar, &dr, &dc, &parseResult);

    /* Handle quitting */
    if (parseResult == EXIT_GAME) {
        p->active = 0;
        out.result = EXIT_GAME;
        out.becameInactive = 1;

        /* Part 1: game ends if the only player quits */
        if (game->numPlayers == 1) {
            game->gameOver = 1;
        } else {
            /* Part 2/3 rule: if only one active remains, they win automatically */
            if (countActivePlayers(game) == 1) {
                for (int i = 0; i < game->numPlayers; i++) {
                    if (game->players[i].active) {
                        game->winnerIndex = i;
                        game->gameOver = 1;
                        break;
                    }
                }
            }
        }

        logTurn(game, pIndex, inputChar, &out);
        game->turnNumber++;
        return out;
    }

    /* Invalid direction input counts as invalid move => lose 1 life */
    if (parseResult == BAD_INPUT) {
        p->lives -= 1;
        out.result = BAD_INPUT;

        if (p->lives <= 0) {
            p->active = 0;
            out.becameInactive = 1;
            if (game->numPlayers == 1) game->gameOver = 1;
        }

        logTurn(game, pIndex, inputChar, &out);
        game->turnNumber++;
        return out;
    }

    /* Compute target position */
    int nr = p->row + dr;
    int nc = p->col + dc;

    /* Out of bounds => invalid move => lose 1 life */
    if (!isWithinBounds(&game->map, nr, nc)) {
        p->lives -= 1;
        out.result = OUT_OF_BOUNDS;

        if (p->lives <= 0) {
            p->active = 0;
            out.becameInactive = 1;
            if (game->numPlayers == 1) game->gameOver = 1;
        }

        logTurn(game, pIndex, inputChar, &out);
        game->turnNumber++;
        return out;
    }

    /* Wall => invalid move => lose 1 life */
    if (isCellWall(&game->map, nr, nc)) {
        p->lives -= 1;
        out.result = HIT_WALL;

        if (p->lives <= 0) {
            p->active = 0;
            out.becameInactive = 1;
            if (game->numPlayers == 1) game->gameOver = 1;
        }

        logTurn(game, pIndex, inputChar, &out);
        game->turnNumber++;
        return out;
    }

    /* Valid move: update player position */
    p->row = nr;
    p->col = nc;
    out.moved = 1;
    out.result = OK;

    /* Resolve landing cell */
    CellType t;
    getCellType(&game->map, nr, nc, &t);

    if (t == INTEL) {
        p->intel += 1;
        out.collectedIntel = 1;
        setCellType(&game->map, nr, nc, EMPTY);  /* remove item after collection */
    } else if (t == LIFE) {
        p->lives += 1;
        out.collectedLife = 1;
        setCellType(&game->map, nr, nc, EMPTY);  /* remove item after collection */
    } else if (t == EXTRACTION) {
        /* If player reaches X without required intel, they become inactive :contentReference[oaicite:1]{index=1} */
        if (p->intel >= game->requiredIntel) {
            out.won = 1;
            game->winnerIndex = pIndex;
            game->gameOver = 1;
        } else {
            p->active = 0;
            out.becameInactive = 1;

            if (game->numPlayers == 1) {
                game->gameOver = 1; /* Part 1 loss condition :contentReference[oaicite:2]{index=2} */
            } else {
                /* Part 2/3: check automatic win if only one active remains :contentReference[oaicite:3]{index=3} */
                if (countActivePlayers(game) == 1) {
                    for (int i = 0; i < game->numPlayers; i++) {
                        if (game->players[i].active) {
                            game->winnerIndex = i;
                            game->gameOver = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* After any valid turn in multiplayer, auto-win can trigger :contentReference[oaicite:4]{index=4} */
    if (!game->gameOver && game->numPlayers > 1) {
        if (countActivePlayers(game) == 1) {
            for (int i = 0; i < game->numPlayers; i++) {
                if (game->players[i].active) {
                    game->winnerIndex = i;
                    game->gameOver = 1;
                    break;
                }
            }
        }
    }

    logTurn(game, pIndex, inputChar, &out);

    game->turnNumber++;

    /* Move to next player for multi-player modes */
    if (!game->gameOver && game->numPlayers > 1) {
        advanceToNextPlayer(game);
    }

    return out;
}

