#include "input.h"

#include <stdio.h>      
#include <unistd.h>     
#include <termios.h>    
#include <stdlib.h>     
#include <ctype.h> 

/* Store original terminal settings so we can restore them */
static struct termios original_termios;
static int raw_mode_enabled = 0;

static void disable_raw_mode(void);

/*
 * Enables raw mode:
 * - ICANON off: no line buffering (input arrives instantly)
 * - ECHO off: don't print typed characters
 */
static void enable_raw_mode()
{
    if (raw_mode_enabled) return;

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        return; // caller checks via input_init()
    }

    atexit(disable_raw_mode);

    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    /* Apply changes immediately; flush pending input */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return;
    }

    raw_mode_enabled = 1;
}

static void disable_raw_mode(void)
{
    if (!raw_mode_enabled) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    raw_mode_enabled = 0;
}

int initInput()
{
    enable_raw_mode();
    return raw_mode_enabled ? 1 : 0;
}

void shutdownInput()
{
    disable_raw_mode();
}

/* ANSI clear + cursor home */
void clear_screen()
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

/* Read one byte from stdin (raw mode makes this immediate) */
int read_key()
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n == 1) return (int)c;
    return -1;
}

/* Convert character to Command */
static Command parse_key(int key)
{
    if (key < 0) return INVALID;

    char ch = (char)toupper((unsigned char)key);

    switch (ch) {
        case 'W': return UP;
        case 'A': return LEFT;
        case 'S': return DOWN;
        case 'D': return RIGHT;
        case 'Q': return QUIT;
        default:  return INVALID;
    }
}

Command readInputCmd()
{
    int key = read_key();
    return parse_key(key);
}

