/********************************************************************/
// HC12 Program:  YourProg - Input Capture Mode
// Processor:     MC9S12XDP512
// Bus Speed:     20 MHz
// Author:        Carlos Estay
// Details:       Read a signal period based on tick counts

// Date:          November 1st, 2023
// Revision History :
//

/********************************************************************/
// Constant Defines
/********************************************************************/

/********************************************************************/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */

/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here
#include <stdio.h>
#include "clock.h"
#include "rti.h"
#include "sw_led.h"
#include "segs.h"
#include "lcd.h"

/********************************************************************/
// Local Prototypes
/********************************************************************/
void RTI_Callback(void);
/********************************************************************/
// Global Variables
/********************************************************************/
int msCounter = 0; // ms counter to be used with RTI callback
unsigned int edgeCounter = 0;

char update = 0; // Seg display update flag
char pacProcess = 0;

unsigned int lastCaptureTicks = 0, periodTicks = 0;
float edgeFrequency = 0; // keep track of the last capture ticks and period in ticks

unsigned char lcdMessage[25];
float frequency = 0;
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
  RTI_InitCallback(RTI_Callback);
  SWL_Init();
  Segs_Init();

  /*
 ECT CONFIGURATION********************
 */

  /*2.1.1 Set Tick Time*/
  TSCR1_PRNT = 1; // ECT Precision Timer
  PTPSR = 20 - 1; // No prescaler, 1[us] ticks

  /*2.1.2 Set CH0 and CH7 as Input Capture*/
  TIOS_IOS0 = 0; // Enable channel 0 (TC0) as input
  TIOS_IOS7 = 0; // Enable channel 7 (TC7) as input

  /*1.1.3 Configure the edge detection*/

  /*****************************************
  //Capture settings - TCTL3/TCTL4
  EDGxB   EDGxA
    0       0  Capture disabled
    0       1  Capture on rising edges only
    1       0  Capture on falling edges only
    1       1  Capture on any edge (rising or falling)
  *****************************************/

  // Capture TC0 in rising edge
  TCTL4_EDG0B = 0;
  TCTL4_EDG0A = 1;

  /*2.1.4 Enable interrupts*/
  TIE_C0I = 1; // Enable interrupt for TC0

  /*2.1.5 Enable fast flag clear and start timer*/
  TSCR1 |= TSCR1_TFFCA_MASK | TSCR1_TEN_MASK; // Fast flag clearing, enable timer

  PACTL = PACTL_PAEN_MASK; // Pulse accumulator A enable

  lcd_Init();

  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (update)
    {
      update = 0;
      Segs_16D(periodTicks, Segs_LineBottom);
      frequency = 1000000.0 / periodTicks;
    }
    if (pacProcess)
    {
      pacProcess = 0;
      Segs_16D(edgeFrequency, Segs_LineTop);
      (void)sprintf(lcdMessage, "%.2f[Hz]    ", edgeFrequency);
      lcd_StringXY(0, 0, lcdMessage);

      //in the seven segment display it doesnt show the frequecy properly after 10kHz because the number 
      //doesnt fit. In the lcd display the pulse accumulator doesnt work after 66 khz because the its 
      //the maximum amount of ticks before it resets to 0.
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void RTI_Callback(void)
{
  if (++msCounter > 99)
  {
    msCounter = 0;
    update = 1;
  }
  if (++edgeCounter > 999)
  {
    edgeFrequency = PACN32; // read PAC 16-bit counter
    PACN32 = 0;
    edgeCounter = 0;
    pacProcess = 1;
  }
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vtimch0 void Vtimch0_ISR(void)
{
  // TFLG1 = TFLG1_C1F_MASK;                   //clears flag, not using fast flag clearing this time
  periodTicks = (TC0 - lastCaptureTicks); // Calculate period in ticks
  lastCaptureTicks = TC0;                 // save capture for next subtraction
}
