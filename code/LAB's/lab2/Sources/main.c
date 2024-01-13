/********************************************************************/
// HC12 Program:  SMultiple Interrupts demo
// Processor:     MC9S12XDP512
// Bus Speed:     20 MHz
// Author:        Carlos Estay
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
/********************************************************************/
// Library includes
/********************************************************************/
#include <stdio.h>  //for sprintf
#include <string.h> //for memcpy, memset

// your includes go here
#include "clock.h"
#include "sw_led.h"
#include "segs.h"
#include "rti.h"
#include "lcd.h"
#include "sci.h"
#include "pit.h"

/********************************************************************/
// Local Prototypes
/********************************************************************/
void RTI_Callback(void);

/********************************************************************/
// Global Variables
/********************************************************************/
unsigned int msCounter = 0;

char display = 0;

unsigned int conversion;
unsigned int period = 0;

unsigned int lcdperiod = 0;
unsigned int lastCaptureTicks = 0;
char lcdmessage[25];
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
  Clock_Set20MHZ(); // Set clock to 20MHz
  SWL_Init();
  Segs_Init();
  lcd_Init();
  RTI_InitCallback(RTI_Callback);

    // Power up - Fast flag clear - Interrupt enable
  ATD0CTL2 = ATD0CTL2_ADPU_MASK | ATD0CTL2_AFFC_MASK | ATD0CTL2_ASCIE_MASK;

  // Number of conversion per sequence (8)
  ATD0CTL3 = 1;

  // 20MHz, divide by 20, ADC to 1MHz
  // 10-bit resolution, 4 A/D coversion clock periods, prescaler to 20 -> 1MHZ ADC-Clock
  ATD0CTL4 = ATD0CTL4_SMP0 | ATD0CTL4_PRS3_MASK | ATD0CTL4_PRS0_MASK;

  ATD0CTL0 = 0 ; //no effect wrapping

  // Right justified, Continuos conversion
  ATD0CTL5 = ATD0CTL5_DJM_MASK | ATD0CTL5_SCAN_MASK | ATD0CTL5_CC_MASK;

  /*
  ECT CONFIGURATION********************
  */

  /*1.1.1 Set Tick Time*/
  TSCR1_PRNT = 1; // ECT Precision Timer
  // 20MHz / 20 = 1MHz
  // 1/1Mhz = 1us
  PTPSR = 19;

  /*1.1.2 Set CH1 as Output and CH3 as Input Capture*/
  TIOS_IOS1 = 1; // Enable channel 1 (TC1) as output
  TIOS_IOS3 = 0; // Enable channel 3 (TC1) as input

  TCTL2_OM1 = 0; // Set TC1 operation to toggle
  TCTL2_OL1 = 1; // Set TC1 operation to toggle

    // Capture TC3 in rising edge
  TCTL4_EDG3B = 0;
  TCTL4_EDG3A = 1;

  TIE_C1I = 1; // Enable interrupt for TC1
  TIE_C3I = 1; // Enable interrupt for TC3

  TSCR1 |= TSCR1_TEN_MASK; // enable timer
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (display)
    {
      Segs_16D(conversion, Segs_LineTop);
      Segs_16D(period, Segs_LineBottom);

      sprintf(lcdmessage,"Volt: %d [mV]", conversion);
      lcd_StringXY(0,0,lcdmessage);

      sprintf(lcdmessage,"Per: %d [us]", lcdperiod);
      lcd_StringXY(0,1,lcdmessage);
      display = 0;
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
    display = 1;
  }
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vatd0 void Vatd0_ISR(void)
{
  // ATD0STAT0 = ATD0STAT0_SCF_MASK; //flag clearing, not necessary here
  SWL_ON(SWL_GREEN);
  conversion = ATD0DR0 * 5;
  SWL_OFF(SWL_GREEN);
}
interrupt VectorNumber_Vtimch1 void Vtimch1_ISR(void)
{
  TFLG1 = TFLG1_C1F_MASK;// clears flag, not using fast flag clearing this time
  period = conversion;  // set period to current ADC Value
  TC1 += period/2; // set TC1 counter to new output compare value
}
interrupt VectorNumber_Vtimch3 void Vtimch3_ISR(void)
{
  TFLG1 = TFLG1_C3F_MASK;// clears flag, not using fast flag clearing this time
  lcdperiod = (TC3 - lastCaptureTicks); // Calculate period in ticks
  lastCaptureTicks = TC3;     
}

/*
Part d:
1. The period of green led is the time it takes to run the code in the ATD ISR and the 
time for a conversion to complete, where as the positive duty cycle is only the length
of the code in the ISR.

2. The period of the green led is the same when coverting all 8 channels compared to 
1 channel

3. They are diretly propotional, meaning that the frequency will be the inverse.
*/
