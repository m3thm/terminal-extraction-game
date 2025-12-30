#include <stdio.h>
#include "game.h"
#include "input.h"
#include "ui.h"

static int readGridSize(void)
{
    int N;
    do {
        printf("Enter grid size N (5-15): ");
        if (scanf("%d", &N) != 1) {
            // clear bad input
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            N = 0;
        }
    } while (N < 5 || N > 15);

    // consume newline left by scanf
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}

    return N;
}

int main(void)
{
    GameState game;
    GameConfig cfg;

    cfg.N = readGridSize();
    cfg.numPlayers = 1;
    cfg.startingLives = 3;
    cfg.requiredIntel = 3;
    cfg.numIntelItems = 3;
    cfg.numLifeItems  = 2;
    cfg.wallCount = cfg.N;      // simple default; tweak later
    cfg.seed = 0;               // 0 = time-based in initGame

    if (!initInput()) {
        printf("Error: could not enable raw input mode.\n");
        return 1;
    }

    if (!initGame(&game, &cfg, "game_log.txt")) {
        shutdownInput();
        printf("Error: game failed to initialize.\n");
        return 1;
    }

    while (!game.gameOver) {
        renderGame(&game);

        char key = (char)read_key();     // if you exposed read_key()
        // If you did NOT expose read_key(), use your public function:
        // char key = input_read_char();  // whatever you named it

        TurnOutcome out = ApplyTurn(&game, key);
        (void)out; // for now (UI/log already handled in game.c)
    }

    renderEndScreen(&game);

    freeGame(&game);
    shutdownInput();
    return 0;
}

