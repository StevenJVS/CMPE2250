/********************************************************************/
// HC12 Program:  YourProg - MiniExplanation
// Processor:     MC9S12XDP512
// Bus Speed:     8 MHz
// Author:        Steven Santiago
// Details:       A more detailed explanation of the program is entered here

// Date:          Date Created
// Revision History :
//  each revision will have a date + desc. of changes

/********************************************************************/
// Constant Defines
/********************************************************************/

/********************************************************************/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include "rti.h"
#include <stdio.h>
#include "sw_led.h" // include switch library
#include "clock.h"  //include clock library
#include "segs.h"
#include "pit.h"
#include "lcd.h"
/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here

/********************************************************************/
// Local Prototypes
/********************************************************************/
void ledtog(void);
/********************************************************************/
// Global Variables
/********************************************************************/
int mycounter = 0;
int secondcounter = 0;
int displaycounter = 0;
char d_buffer[40];
char h_buffer[40];
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
  EnableInterrupts;

  /********************************************************************/
  // initializations
  /********************************************************************/
  Clock_Set20MHZ();
  RTI_Init();
  Segs_Init();
  lcd_Init();
  SWL_Init();
  lcd_Init();
  lcd_Clear();
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    RTI_Delay_ms(1000);

    if (displaycounter > 9999)
    {
      displaycounter = 0;
    }
    sprintf(d_buffer, "DEC: %04d", displaycounter);
    sprintf(h_buffer, "HEX: %04X", displaycounter);
    lcd_StringXY(0, 0, d_buffer);
    lcd_StringXY(0, 1, h_buffer);
    Segs_16D(displaycounter++, Segs_LineTop); // top segment
    RTI_InitCallback(&ledtog);
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void ledtog()
{
  mycounter++;
  if (mycounter % 500 == 0)
  {
    SWL_TOG(SWL_RED);
  }
  if (mycounter % 250 == 0)
  {
    SWL_TOG(SWL_YELLOW);
  }
  if (mycounter % 125 == 0)
  {
    SWL_TOG(SWL_GREEN);
  }
  if (mycounter == 1000)
  {
    mycounter = 0;
    secondcounter++;
    Segs_16D(secondcounter, Segs_LineBottom); // bottom segment
  }
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
