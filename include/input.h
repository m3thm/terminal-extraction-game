
#ifndef INPUT_H
#define INPUT_H

#include "common.h"

int initInput();
int read_key();
void shutdownInput();
Command readInputCmd();
void clear_screen();

#endif
