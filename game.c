/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "ledmatrix.h"
#include "terminalio.h"
#include "buttons.h" //MY INCLUSION
#include "serialio.h" //MY INCLUSION
#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include "timer0.h"



static const uint8_t track[TRACK_LENGTH] = {0x00,
	0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,            
	0x04, 0x40, 0x08, 0x80, 0x00, 0x00, 0x04, 0x02,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x80, 0x04, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
	0x04, 0x40, 0x02, 0x08, 0x80, 0x00, 0x02, 0x01,
	0x04, 0x40, 0x08, 0x80, 0x04, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x12, 0x20, 0x00, 0x00, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x01, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};

// 0, 0, 0, 4, 4, 4, 0, 3, 2, 3
int greenNotes[] = {0, 0, 0, 0, 0, 0, 0, 0};

uint8_t beat;
uint8_t ghost_note;
int terminal_score = 0;
uint8_t play_note_called = 0;
uint8_t game_over = 0;
uint8_t beat_paused = 0;
uint8_t beat_increment = 0;
uint8_t beat_increment_called = 0;
uint8_t seven_seg[10] = { 63,6,91,79,102,109,125,7,127,111};
uint8_t next_index;
uint8_t manual_on = 0;

	// Initialise the game by resetting the grid and beat
void initialise_game(void)
{
	// initialise the display we are using.
	default_grid();
	beat = 0;
	printf("\rscore is: %3d", 0);
	printf("\n Manual Mode is Off \n");
	printf("\n Game Speed is set to Normal");
	
	
}


void longNoteInProgress(void)
{
	
	// if value of the next index is 0x20, 0x40 or 0x80, 
	// then colour all pixels between the current index and the next index
	// 
}




void seven_seg_display(uint8_t number, uint8_t digit)
{
	PORTA = digit;
	PORTC = seven_seg[number];
}

void display_tens(uint8_t tens) {
	// Drive the tens digit
	PORTA = 1; // Assuming PORTA is used to select the digit.
	PORTC = seven_seg[tens];
	_delay_ms(15); // Adjust the delay as needed
}

void display_ones(uint8_t ones) {
	// Drive the ones digit
	PORTA = 0; // Select the other digit
	PORTC = seven_seg[ones];
		
}
	

int display_score(void) {

	DDRC = 0xFF; // Assuming PORTC is used for the seven-segment display.
	DDRA = 1;   // Assuming PORTA is used to select the digit.

		
	uint8_t tensDigit = (terminal_score / 10)%10;
	uint8_t onesDigit = terminal_score % 10;

	// Display ten's digit and then one's digit in quick succession
	if (tensDigit > 0)
	{
		display_tens(tensDigit);
		display_ones(onesDigit);	
	}
	display_ones(onesDigit);
	return 0;
}

void display_terminal_score(int terminal_score)
{

}

	// Play a note in the given lane
void play_note(uint8_t lane)
{
	// YOUR CODE HERE
	clear_terminal();
	// Reset the greenNotes array for this beat
	if (play_note_called== 1) {
		return; // Skip this call
	}
	// Check if there are notes in the scoring area of the given lane for this beat
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		
		
		
		if ((col>=10) && (track[index] & (1<<lane)))
		{
			greenNotes[lane] = 1;
			
		}
		
		
		if ((track[index] & (1<<lane))&&(col==10))
		{
			terminal_score += 1;
		}

		if ((track[index] & (1<<lane))&&(col==11))
		{
			terminal_score += 1;
		}

		if ((track[index] & (1<<lane))&&(col==12))
		{
			terminal_score += 1;
		}
		if ((track[index] & (1<<lane))&&(col==13))
		{
			terminal_score -= 1;
		}


	
	// 		if (greenNotes[lane]==0)
	// 		{
	// 			terminal_score -= 1;
	// 		}

		
	}
	
	printf("\r\n score is: %3d", terminal_score);
	
	
}
	
		

	
		

		
	
	
	// possible steps:
	// a) check if there is a note in the scoring area of the given lane -
	//    look at advance_note below for a hint to the logic
	// b) if there is, immediately turn those notes green
	// c) set up a variable to remember if notes should be green,
	//    and trigger it simultaneously with b)
	// d) in advance_note below, poll that variable, and choose COLOUR_GREEN
	//    instead of COLOUR_RED for ledmatrix_update_pixel if required
	// e) depending on your implementation, clear the variable in
	//    advance_note when a note disappears from the screen


// Advance the notes one row down the display
void advance_note(void)
{

	if (track[next_index] == 0x20 || track[next_index] == 0x40 || track[next_index] == 0x80)
	{
		/*printf("Long Note should be flagged");*/
		
	}
	
	// remove all the current notes; reverse of below
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		if (index >= TRACK_LENGTH)
		{
			
			break;
		}
		if ((future+beat) % 5)
		{
			continue;
		}
		for (uint8_t lane = 0; lane < 4; lane++)
		{
			if (track[index] & (1<<lane))
			{
				PixelColour colour;
				// yellows in the scoring area
				if (col==11 || col == 15)
				{
					colour = COLOUR_QUART_YELLOW;
				}
				else if (col==12 || col == 14)
				{
					colour = COLOUR_HALF_YELLOW;
				}
				else if (col==13)
				{
					colour = COLOUR_YELLOW;
				}
				else
				{
					colour = COLOUR_BLACK;
				}
				ledmatrix_update_pixel(col, 2*lane, colour);
				ledmatrix_update_pixel(col, 2*lane+1, colour);
			}
		}
	}
	
	
	// increment the beat

	// MANUAL MODE, if 'm', beat stops, if 'n', beat increments
	if (!beat_paused)
	{
		beat++;
		
		
		display_score();
		
	}
	
	if (beat_paused)  //if you press 'm'
	{
		manual_on = 1;
		if (beat_increment)  //if you press 'n'
		{
			if (!beat_increment_called) // guarantees to loop through once
			{
				
				beat_increment_called = 1;
				
				beat = beat+ 1;
			}
			
		}
	}



	
/*	beat++;*/
	
	// GHOST NOTES
	
	uint16_t future = 20;  //show ghost note when real note is 4 pixels down
	// Calculate the index where ghost notes should appear
	uint16_t index = (future + beat) / 5;
	next_index = index;
	if (index < TRACK_LENGTH) {
		for (uint8_t lane = 0; lane < 4; lane++) {
			if (track[index] & (1 << lane)) {
				ledmatrix_update_pixel(0, 2 * lane, COLOUR_LIGHT_RED);
				ledmatrix_update_pixel(0, 2 * lane + 1, COLOUR_LIGHT_RED);
			}
			else
			{
				future++;
			}
		}
	}

	// draw the new notes
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		// col counts from one end, future from the other
		uint8_t future = MATRIX_NUM_COLUMNS-1-col;  // this is equal to 15
		// notes are only drawn every five columns
		if ((future+beat)%5)
		{
			continue;
		}
		
		// index of which note in the track to play
		uint8_t index = (future+beat)/5;
		
		// if the index is beyond the end of the track,
		// no note can be drawn
// 		if (index >= TRACK_LENGTH)
// 		{
// 			for (int i = 0; i < 4; i++)
// 			{
// 				greenNotes[i] = 0;
// 			}
// 		}
		// iterate over the four paths
		for (uint8_t lane=0; lane<4; lane++)
		{
			// check if there's a note in the specific path
			
			
			if (track[index] & (1<<lane))
			{
				// if so, colour the two pixels red
				
				ledmatrix_update_pixel(col, 2*lane, COLOUR_RED);
				ledmatrix_update_pixel(col, 2*lane+1, COLOUR_RED);			
			}
			
			while((track[next_index] & (1<<lane)) == 0x20 || (track[next_index] & (1<<lane)) == 0x40 || (track[next_index] & (1<<lane)) == 0x80)
			{
				ledmatrix_update_pixel(col-1, 2*lane, COLOUR_RED);
				ledmatrix_update_pixel(col-1, 2*lane+1, COLOUR_RED);
				if (next_index == index)
				{
					break;
				}
			}
			
			if ((track[index] & (1<<lane)) && (greenNotes[lane] == 1)&&(col>=11))
			{
					
				ledmatrix_update_pixel(col, 2*lane, COLOUR_GREEN);
				ledmatrix_update_pixel(col, 2*lane+1, COLOUR_GREEN);
			}
			
			

			
			
			
			
			
			//once the note reaches the end of the matrix, reset play_note
			if ((track[index] & (1<<lane))&&(col>=15))
			{
				play_note_called = 0;
				
				// reset the GreenNotes flag
				int i;
				for (i=0; i<4; i++)
				{
					greenNotes[i] = 0;
				}
			}

			
			
		}

		
		
	}

// 	int i;
// 	for (i=0; i<4; i++)
// 	{
// 		greenNotes[i] = 0;
// 	}
	
	
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void)
{
	// YOUR CODE HERE
	// Detect if the game is over i.e. if a player has won.
	//uint8_t index = (future + beat) / 5;
// 	if (index == 132)
// 	{
// 		printf("Game Over\n");
// 	}
	if (beat == TRACK_LENGTH)
	{
		printf("Game Over");
		handle_game_over();
	}

	
	
	return 0;
}





