/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by <YOUR NAME HERE>
 */

#include <stdio.h>

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

uint16_t game_speed;


// GAME COUNTDOWN CHARACTERS
const uint8_t pixel_typeface[][16]= {
	{0b00000000,0b01111110,0b01000000,0b01000000,0b01000000,0b01000000,0b01000000,0b01111110,0b01000000,0b01000000,0b01000000,0b01000000,0b01000000,0b01111110,0b00000000,0b00000000}, // "3"
	{0b00000000,0b01111110,0b01000000,0b01000000,0b01000000,0b01000000,0b01000000,0b01111110,0b00000010,0b00000010,0b00000010,0b00000010,0b00000010,0b01111110,0b00000000,0b00000000}, // "2"
	{0b00000000,0b00010000,0b00011000,0b00010100,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b00000000,0b00000000}, // "1"
	{0b00000000,0b01111100,0b00000010,0b00000010,0b00000010,0b01000010,0b01100010,0b01011100,0b00000000,0b00111100,0b01000010,0b01000010,0b01000010,0b01000010,0b00111100,0b00000000}  // "GO"
};

char speed_word[10];


void display_pixel_typeface(uint8_t character_index)
{
	
	for (int row = 0; row < 16; row++)
	{
		for (int pixel_column = 0; pixel_column < 8; pixel_column++)
		{
			if (pixel_typeface[character_index][row] & (1<<pixel_column))
			{
				ledmatrix_update_pixel(row, pixel_column, COLOUR_LIGHT_RED);
			}
		}
	}
	_delay_ms(500); //FIX THIS LATER, WILL CAUSE PROBLEMS FOR GAME PAUSE
}

void display_character(void)
{
	for (int character = 0; character < 4; character++) // accessing '3', '2', '1', 'GO'
	{
		ledmatrix_clear();  // Clear matrix before showing characters
		display_pixel_typeface(character);
		
	}
}


/////////////////////////////// main //////////////////////////////////
int main(void)
{
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();
	
	// Loop forever and continuously play the game.
	while(1)
	{
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void)
{
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	init_timer0();
	init_timer1();
	init_timer2();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void)
{
	// Clear terminal screen and output a message
	clear_terminal();
	show_cursor();
	clear_terminal();
	hide_cursor();
	set_display_attribute(FG_WHITE);
	move_terminal_cursor(10,4);
	printf_P(PSTR("  ______   __     __  _______         __    __"));
	move_terminal_cursor(10,5);
	printf_P(PSTR(" /      \\ |  \\   |  \\|       \\       |  \\  |  \\"));
	move_terminal_cursor(10,6);
	printf_P(PSTR("|  $$$$$$\\| $$   | $$| $$$$$$$\\      | $$  | $$  ______    ______    ______"));
	move_terminal_cursor(10,7);
	printf_P(PSTR("| $$__| $$| $$   | $$| $$__| $$      | $$__| $$ /      \\  /      \\  /      \\"));
	move_terminal_cursor(10,8);
	printf_P(PSTR("| $$    $$ \\$$\\ /  $$| $$    $$      | $$    $$|  $$$$$$\\|  $$$$$$\\|  $$$$$$\\"));
	move_terminal_cursor(10,9);
	printf_P(PSTR("| $$$$$$$$  \\$$\\  $$ | $$$$$$$\\      | $$$$$$$$| $$    $$| $$   \\$$| $$  | $$"));
	move_terminal_cursor(10,10);
	printf_P(PSTR("| $$  | $$   \\$$ $$  | $$  | $$      | $$  | $$| $$$$$$$$| $$      | $$__/ $$"));
	move_terminal_cursor(10,11);
	printf_P(PSTR("| $$  | $$    \\$$$   | $$  | $$      | $$  | $$ \\$$     \\| $$       \\$$    $$"));
	move_terminal_cursor(10,12);
	printf_P(PSTR(" \\$$   \\$$     \\$     \\$$   \\$$       \\$$   \\$$  \\$$$$$$$ \\$$        \\$$$$$$"));
	move_terminal_cursor(10,14);
	// change this to your name and student number; remove the chevrons <>
	printf_P(PSTR("CSSE2010/7201 A2 by Gavin Sun - 48036131"));
	printf("\n Manual Mode is Off          \n");
	printf("\n Game Speed is set to normal    \n");
	
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();
	
	uint8_t frame_number = 0;
	game_speed = 1000;

	

	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1)
	{
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		
		if (serial_input == '1')
		{
			game_speed = 1000;
			printf("\n Game Speed is set to normal    \n");
		}
		
		if (serial_input == '2')
		{
			game_speed = 500;
			printf("\n Game Speed is set to fast    \n");
		}
		
		if (serial_input == '3')
		{
			game_speed = 250;
			printf("\n Game Speed is set to extreme    \n");
		}
		
		if ((serial_input == 'm')||(serial_input == 'M'))
		{
			beat_paused = (beat_paused == 0) ? 1 : 0;
			
			if (beat_paused == 0)
			{
				printf("\n\rManual Mode is Off          \n");
				
			}
			
			else
			{
				printf("\n\rManual Mode is On          \n");
			}
			
		}
		
		
		
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S')
		{
			show_start_screen();
		}
		

		
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED)
		{
			break;
		}

		// every 200 ms, update the animation
		current_time = get_current_time();
		if (current_time - last_screen_update > game_speed/5)
		{
			update_start_screen(frame_number);
			frame_number = (frame_number + 1) % 32;
			last_screen_update = current_time;
		}
	}
	

	
}










void new_game(void)
{
	// Clear the serial terminal
	clear_terminal();
	
	display_character();
	// Initialise the game and display
	initialise_game();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void)
{

	
	uint32_t last_advance_time, current_time;
	int8_t btn; // The button pushed
	
	last_advance_time = get_current_time();
	
	// We play the game until it's over
	while (!is_game_over())
	{
				
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		// Checkout the function comment in `buttons.h` and the implementation
		// in `buttons.c`.
		btn = button_pushed();
		
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		
		if ((btn == BUTTON0_PUSHED)||(serial_input == 'f') || (serial_input == 'F'))
		{
			// If button 0 play the lowest note (right lane)
			
			play_note(3);
		}

		if ((btn == BUTTON1_PUSHED)||(serial_input == 'd') || (serial_input == 'D'))
		{
			// If button 0 play the lowest note (right lane)
			play_note(2);
		}
		
		if ((btn == BUTTON2_PUSHED)||(serial_input == 's') || (serial_input == 'S'))
		{
			// If button 0 play the lowest note (right lane)
			play_note(1);
		}
		//REMOVE LATER
		if ((btn == BUTTON3_PUSHED)||(serial_input == 'a') || (serial_input == 'A'))
		{
			// If button 0 play the lowest note (right lane)
			play_note(0);
		}
		
		if ((serial_input == 'm')||(serial_input == 'M'))
		{
			beat_paused = (beat_paused == 0) ? 1 : 0;
		}
		
		if ((serial_input == 'n')||(serial_input == 'N'))
		{
			beat_increment = (beat_increment == 0) ? 1 : 0;
			if (beat_increment) // if n is pressed
			{
				beat_increment_called = 0; // Reset the flag
			}
			
		}
		
		// REMOVE LATER
		
		current_time = get_current_time();
		if (current_time >= last_advance_time + game_speed/5)
		{
			// 200ms (0.2 second) has passed since the last time we advance the
			// notes here, so update the advance the notes
			advance_note();
			
			// Update the most recent time the notes were advance
			last_advance_time = current_time;
		}
	}
	
	// We get here if the game is over.
}

void handle_game_over()
{
	int8_t btn;
	clear_terminal();
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf("Game Speed was %d", game_speed);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
	// MY WORK
	while(1)
	{
		btn = button_pushed();
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		
		if (serial_input == 's' || serial_input == 'S'||btn == BUTTON1_PUSHED|| btn == BUTTON0_PUSHED || btn == BUTTON2_PUSHED || btn == BUTTON3_PUSHED)
		{
			show_start_screen();
			start_screen();
			
		}
	}
	// MY WORK
	
	
	
	// Do nothing until a button is pushed. Hint: 's'/'S' should also start a
	// new game
	
	while (button_pushed() == NO_BUTTON_PUSHED)
	{
		; // wait
	}
}

