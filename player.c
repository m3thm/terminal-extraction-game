#include "player.h"

void initPlayer(Player *player, char symbol, PlayerType type,
                int startRow, int startCol, int startLives)
{
    if (!player) return;

    player->symbol = symbol;
    player->type = type;
    player->row = startRow;
    player->col = startCol;
    player->lives = startLives;
    player->intel = 0;
    player->active = 1;
}

void deactivatePlayer(Player *p)
{
    if (!player) return;
    player->active = 0;
}

int isPlayerActive(const Player *player)
{
    if (!player) return 0;
    return player->active;
}

