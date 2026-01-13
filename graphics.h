#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>
#include <stddef.h>

typedef enum TerminalColor {
    Black = 0,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
} TerminalColor;

typedef struct GraphicsCell {
    char ch;
    TerminalColor fg;
    TerminalColor bg;
} GraphicsCell;

typedef struct GraphicsBuffer {
    int width;
    int height;
    int outFd;

    GraphicsCell *cells;           
    unsigned char *dirtyBits;
    int dirtyByteCount;

    int firstDirty;                
    int lastDirty;                 

    int cursorX;
    int cursorY;

    bool colorCached;
    bool useBrightColors;

    TerminalColor currentFg;
    TerminalColor currentBg;
} GraphicsBuffer;

bool init(GraphicsBuffer *gfx, int width, int height);
void shutdown(GraphicsBuffer *gfx);

void clearScreen(GraphicsBuffer *gfx, char ch, TerminalColor fg, TerminalColor bg);
void setCursor(GraphicsBuffer *gfx, int x, int y);
void writeChar(GraphicsBuffer *gfx, char ch, TerminalColor fg, TerminalColor bg);
void setCell(GraphicsBuffer *gfx, int x, int y, char ch, TerminalColor fg, TerminalColor bg);
void drawText(GraphicsBuffer *gfx, int x, int y, const char *text, TerminalColor fg, TerminalColor bg);

void render(GraphicsBuffer *gfx);

void hideCursor(GraphicsBuffer *gfx, bool hide);
void setBrightColors(GraphicsBuffer *gfx, bool enable);


#endif
