/*
 * display.h
 *
 * Authors: Luke Kamols, Jarrod Bennett, Martin Ploschner, Cody Burnett,
 * Renee Nightingale
 */ 

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "pixel_colour.h"

// Initialise the display for the board, this creates the display
// for an empty board.
void default_grid(void);

// Initialise the display for the board, this creates the display
// for an empty board.
void initialise_display(void);

// Shows a starting display.
void show_start_screen(void);

// Update dynamic start screen based on frame number (0-11)
void update_start_screen(uint8_t frame_number);

#endif /* DISPLAY_H_ */
