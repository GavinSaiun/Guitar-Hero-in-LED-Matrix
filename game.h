/*
 * game.h
 *
 * Author: Jarrod Bennett, Cody Burnett
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */


#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

#define TRACK_LENGTH 129

extern uint8_t beat_paused; //my inclusion
extern uint8_t beat_increment; //
extern uint8_t beat_increment_called;


// Initialise the game by resetting the grid and beat
void initialise_game(void);

// Play a note in the given lane
void play_note(uint8_t lane);

// Advance the notes one row down the display
void advance_note(void);

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void);

#endif
