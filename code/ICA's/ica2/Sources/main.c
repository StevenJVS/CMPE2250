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
#include "sw_led.h"     // include switch library
#include "clock.h"      //include clock library
#include <stdio.h>
#include "segs.h"
#include "pit.h"
#include "lcd.h"
#include "string.h"
/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here

/********************************************************************/
// Local Prototypes
/********************************************************************/

/********************************************************************/
// Global Variables
/********************************************************************/
volatile long count = 0;
long maincount = 0;
char myName[] = "S. Santiago";
char tx_buffert[40]; // visible count in hex
char tx_bufferl[40]; // visible count in hex
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
  SWL_Init();
  Segs_Init();
  PIT_InitChannel(PIT_CH0, PIT_MT0, PIT_IEN);

  // 50[ms] event -> 20E6 x 50E-3 = 100,000
  // 100,000 = 5 x 20,000
  PITMTLD0 = 4;
  PITLD0 = 19999;

  PIT_Start();
  lcd_Init();
  lcd_StringXY(0, 3, myName);
  lcd_StringXY(0, 0, "Ticks:");
  lcd_StringXY(0, 1, "Loops:");
  /********************************************************************/
  // main program loop,
  /********************************************************************/
  for (;;)
  {
    SWL_TOG(SWL_GREEN);

    sprintf(tx_buffert, "%lu", count);
    lcd_StringXY(20 - strlen(tx_buffert), 0, tx_buffert); // print on lcd
    sprintf(tx_bufferl, "%lu", maincount);
    lcd_StringXY(20 - strlen(tx_bufferl), 1, tx_bufferl); // print on lcd
    maincount++;
    
    
    PIT_Sleep(PIT_CH1, 50);
  }
}

/********************************************************************/
// Functions
/********************************************************************/

/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vpit0 void Vpit0_ISR(void)
{
  PITTF = PITTF_PTF0_MASK; // clear flag;
  count++;
  SWL_TOG(SWL_YELLOW);
}