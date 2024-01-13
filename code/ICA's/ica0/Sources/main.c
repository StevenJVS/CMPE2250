/********************************************************************/
// HC12 Program:  PIT LCD Demo
// Processor:     MC9S12XDP512
// Bus Speed:     20 MHz
// Author:        Carlos Estay
// Details:       Simple LCD Demo
// Date:          Apr 11, 2023
// Revision History :
// March - 17th, 2023: Adapted for Winter 2023

/********************************************************************/
// Constant Defines
/********************************************************************/

/********************************************************************/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include <stdio.h>
#include "sw_led.h"
#include "clock.h"
#include "pit.h"
#include "segs.h"
#include "lcd.h"

/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here

/********************************************************************/
// Local Prototypes
/********************************************************************/
void intToBinary(int display, char *b_buffer);
/********************************************************************/
// Global Variables
/********************************************************************/
char myMessage[] = "CMPE2250";
char myName[] = "Santiago";
char tl_buffer[40]; // visible count in hex
char tr_buffer[40]; // count down
char b_buffer[16];  // visible count in binary
int counter = 0;    // first counter
int display = 0;    // second counter
int invert = 10000;  // constant for inverted value
SwState leftstate, ctrstate, rightstate;
int i;
/********************************************************************/
// Constants
/********************************************************************/

/********************************************************************/
// Main Entry
/********************************************************************/
void main(void)
{
  // main entry point
  _DISABLE_COP();
  // EnableInterrupts;
  /********************************************************************/
  // initializations
  /********************************************************************/
  Clock_Set20MHZ();
  SWL_Init();
  Segs_Init();

  // Set non-blocking event - No interrupts
  PIT_InitChannel(PIT_CH0, PIT_MT0, PIT_IDIS);

  // 5[ms] event -> 20E6 x 5E-3 = 100,000
  // 100,000/ 5 = 20,000
  PITMTLD0 = 4;
  PITLD0 = 19999;
  PIT_Start();
  lcd_Init();
  // LCD INIT************************
  // Port Initialization-------------------------------------------

  // WRITE DATA***************************

  // display name and class on lcd
  lcd_StringXY(0, 2, myMessage);
  lcd_StringXY(12, 1, myName);
  Segs_Clear();
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    // reset counter to 0 if greater than 9999
    if (display > 9999)
    {
      display = 0;
    }

    // visible counter on lcd and bottom segs
    if (counter >= 100)
    {
      counter = 0; // reset counter
      display++;
      sprintf(tl_buffer, "%04X", display);
      lcd_StringXY(0, 0, tl_buffer); // print on lcd
      invert = 9999 - display;

      sprintf(tr_buffer, "%04d", invert);
      lcd_StringXY(16, 0, tr_buffer); // print on lcd

      intToBinary(display, b_buffer); // print on lcd

      lcd_StringXY(2, 3, b_buffer); // print on lcd
    }
    if (PITTF & PITTF_PTF0_MASK) // check mask
    {
      counter++;               // add to first counter
      PITTF = PITTF_PTF0_MASK; // clear flag;
    }
    if (Sw_Process(&leftstate, SWL_LEFT) == Held)
    {
      Segs_16H(display, Segs_LineTop); // visible counter in hex
    }
    else if (Sw_Process(&ctrstate, SWL_CTR) == Held)
    {
      if (invert < 0) // reset inverted value if less than 0
      {
        invert = 9999;
      }
      Segs_16D(invert, Segs_LineTop);
    }
    else if (Sw_Process(&rightstate, SWL_RIGHT) == Held)
    {
      // extract numbers and display on segs accordingly
      int ones = display % 10;
      int tens = (display / 10) % 10;
      int hund = (display / 100) % 10;
      int thou = (display / 1000) % 10;

      Segs_Custom(0, thou + 128);
      Segs_Custom(1, hund + 128);
      Segs_Custom(2, tens + 128);
      Segs_Custom(3, ones + 128);
    }
    else
    {
      Segs_ClearLine(Segs_LineTop); // clear if released
    }
    Segs_16D(display, Segs_LineBottom); // bottom segment
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void intToBinary(int display, char *b_buffer)
{
  for (i = 16; i >= 0; i--)
  {
    // bit shift each position and check if the binary value is either 1 or 0
    b_buffer[16 - i] = (display >> i) & 1 ? '1' : '0';
  }
  b_buffer[17] = '\0';
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
