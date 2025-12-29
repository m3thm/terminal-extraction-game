
#ifndef COMMON_H
#define COMMON_H

#define MAX_PLAYERS 3 

typedef struct {
    int r; 
    int c;
} Position; 

typedef enum {
    HUMAN,
    COMPUTER
} PlayerType; 

typedef enum {
    UP,
    LEFT,
    DOWN,
    RIGHT,
    QUIT,
    INVALID
} Command; 

#endif

