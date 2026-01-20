#include "graphics.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Private Helper Functions
 * these functions are static inline. static means these cannot be called elsewhere.
 * inline means the compiler will replace the function call with the function body. 
 * inlining is useful for small helper functions like this that gets called frequently.
 * */

static inline int cellCount(const GraphicsBuffer *gfx) 
{
    return gfx->width * gfx->height; //returns the current total cell count rendered on the terminal window.
}

static inline bool withinBounds(const GraphicsBuffer *gfx, int x, int y) 
{
    return x >= 0 && y >= 0 && x < gfx->width && y < gfx->height; //returns true if the given x and y coordinates lie within the terminal window
}

static inline int cellIndex(const GraphicsBuffer *gfx, int x, int y) 
{
    return y * gfx->width + x; //finds the index of the cell given it's x and y coordinates
}

/*
 * Quick Explanation on this Engine's Main Data Structure.
 * this graphics engine has one major problem to solve. that is storing all the cells being rendered on the screen (terminal window)
 * instead of storing all cells (N cells = width * height) and rendering every cell every frame even though some cells havent changed,
 * we only store the cells that needs to be changed or redrawn (dirty cells). (we do store every cell inside the terminal in another array 
 * but that is used for different purposes, not for rendering)
 * the data structure we use to store the dirty cells is called the 'Frame Buffer'. we could store the dirty cells directly as a 1D array 
 * (1 byte per cell) which is wasteful. so instead we use this: 'unsigned char *dirtyBits;'. an unsigned char is 1 byte long. 1 byte has 
 * 8 bits in it. therefore we can store 8 cells in 1 byte. This way we can store 8 times more cells in the same amount of space.
 * this is why we compute dirty byte count like this: dirtyByteCount = (nCells + 7) / 8;
 *
 * */

/*
 * Working with this Data Structure using Bitwise Operators.
 * since we store cell indexes at the bit level, in the following functions we will be using bitwise operators to compute certain values 
 * firstly we will store every cell index inside the terminal in an array (GraphicsCell cells[width * height];). then we use this function
 * idx = y * width + x; to give every cell a unique index, which ranges from 0 to (x*y-1).
 * the frame buffer inside our graphics buffer is a pointer to an unsigned char. we make it a pointer because we want to allocate the size of 
 * the frame buffer at runtime. this still means at runtime, the frame buffer will be an array of bytes, each containing 8 bits.
 *
 * Why we need ByteIndex and BitIndex.
 * the byte index is basically the index of a given element in the frame buffer array. the bit index is the index of the bit inside the byte 
 * given by the byte index. We need these two indices to update the framebuffer when the engine is running. We need them to be able to only edit 
 * the value of a specific bit inside a specific byte inside the frame buffer.
 *
 * How we calculate the ByteIndex and BitIndex.
 * each byte holds 8 cells. dirtybist[0] holds cells 0 to 7. dirtybits[1] holds cells 8 to 15 and so on. we can use integer division to calculate
 * this index. dividing the cell index by 8 gives the byte index. because integer division discards the remainder, cells 0 to 7, divided by 8 gives 0.
 * cells 8 to 15 divided by 8 gives 1 and so on. but this is not an optimized way to calculate it. We can use BitWise operators instead to make it faster.
 * because using bitwise operators only costs a single cpu instruction whereas / and % costs few dozen. So, to achieve the same result as / 8, we use
 * idx >> 3. this means right shift the binary value of the index by 3 bits, (for example: 00001101 -> 00000001) which is equivalent because 8 is 2³. 
 * Right shifting by 3 removes the lowest 3 bits of the binary number, which is the same as performing integer division by 8. 
 *
 * the bitIndex is calculated using idx & 7, which extracts the lowest 3 bits. this is possible because the binary of 7 is 00000111. an AND operation between
 * cell index and 00000111 will result in keeping the cell index's lower 3 bits and the rest turned to 0. these lower 3 bits corresponds to the remainder that is
 * discarded when doing integer division. in other words % 8 is the same as & 7.
 *
 * */

/*
 * Creating a Mask using the BitIndex.
 * the mask in this case is an array of 8 bits (00000100) corresponding to a byte in the frame buffer. in this mask only the bit that corresponds to the bit index is 1, 
 * and the rest is 0. (for example BitIndex = 1 -> 00000010, BitIndex = 3 -> 00001000). this is useful for updating the frame buffer. this mask is created using
 * (1u << bitIndex) we left shift 1u (00000001) by a bitIndex amount. we must also cast it to an unsigned char because (1u << bitIndex) is natively an unsigned integer. 
 *
 * */

static inline void markDirty(GraphicsBuffer *gfx, int idx) { //mark a specific cell as dirty. 
    int byteIndex = idx >> 3
    int bitIndex  = idx & 7      
    
    unsigned char mask = (unsigned char)(1u << bitIndex);
    gfx->dirtyBits[byteIndex] |= mask; //bitwise or is used so that only the bit we want to update gets updated inside the frame buffer.

    if (idx < gfx->firstDirty) gfx->firstDirty = idx; //this function also updates the first and last dirty bits. these are used to create a range of dirty bits.
    if (idx > gfx->lastDirty)  gfx->lastDirty  = idx; //the engine only scans for cells within this range for efficiency.
}

static inline bool isDirty(const GraphicsBuffer *gfx, int idx) { ///check if cell is dirty
    int byteIndex = idx / 8;
    int bitIndex  = idx % 8;
    
    unsigned char mask = (unsigned char)(1u << bitIndex);
    return (gfx->dirtyBits[byteIndex] & mask) != 0; //returns 1 if both bits inside the frame buffer and mask are 1.
}

static inline void clearDirty(GraphicsBuffer *gfx, int idx) { //marks a cell as not dirty
    int byteIndex = idx / 8;
    int bitIndex  = idx % 8;
    
    unsigned char mask = (unsigned char)(1u << bitIndex);
    gfx->dirtyBits[byteIndex] &= ~mask; // ~ means invert the mask. bitwise and keeps the rest of the bits as is and sets the bit to 0 inside the frame buffer. 
    
}




