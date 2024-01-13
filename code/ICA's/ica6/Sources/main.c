/********************************************************************/
// HC12 Program:  YourProg - ECT Demo 2 - TC7/TC0 PWM
// Processor:     MC9S12XDP512
// Bus Speed:     8 MHz
// Author:        This B. You
// Details:       50[us] period signal of 20% duty cycle

// Date:          March 1, 2023
// Revision History :
// October 25th, 2023 - Updated

/********************************************************************/
// Constant Defines
/********************************************************************/

/********************************************************************/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include "clock.h"
#include "sw_led.h"

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
SwState left, right, up, down;
unsigned int period = 5000;
unsigned int duty;
unsigned int temp;
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
  duty = period / 2;
  Clock_Set20MHZ();
  SWL_Init();

  /*
  ECT CONFIGURATION********************
  */

  /*1.1.1 Set Tick Time*/
  TSCR1_PRNT = 1; // ECT Precision Timer
  PTPSR = 19;

  /*1.1.2 Set CH7 as Output Compare*/
  TIOS_IOS7 = 1;  // Enable channel 7 (TC7) as output
  TSCR2_TCRE = 1; // TC7 event auto resets the counter (special CH7 feature)

  TIOS_IOS0 = 1; // Enable channel 0 (TC0) as output
  /*1.1.3 Set timer ticks to compare against*/
  /*
  Set a 5[ms] @50[ns] ticks => ticks = 5[ms]/1[us] = 5000
  */
  TC7 = period; // TC7 event to 5[ms]
  TC0 = duty;   // TC7 event to 2.5[ms] (50% duty cycle)

  /*1.1.4 Set pin action to TOGGLE*/

  /*****************************************
  Compare result Action - TCTL1/TCTL2
  OMx   OLx
  0     0  Timer Disconnected from pin
  0     1  Toggle OCx output line
  1     0  Clear OCx output lize to zero
  1     1  Set OCx output line to one
  *****************************************/
  TCTL1_OM7 = 0; // Set TC7 operation to toggle
  TCTL1_OL7 = 1; // Set TC7 operation to toggle

  TCTL2_OM0 = 0; // Set TC0 operation to toggle
  TCTL2_OL0 = 1; // Set TC0 operation to toggle

  /*1.1.5 Decide if TC7 event affects other channels*/
  OC7M_OC7M0 = 1;     // OC7D0 BIT will be transfered to CH0 on TC7 event
  OC7D |= OC7M_OC7M0; // Transfer a HIGH(1) on TC7 event

  /*1.1.6 Enable interrupts if necessary*/
  TIE_C7I = 0; // No interrupt

  /*1.1.7 Enable fast flag clear if interrupting and start timer*/
  TSCR1 |= TSCR1_TEN_MASK; // Only start timer, NO fast flag clearing (TFFCA)
  // TSCR1 = TSCR1_TEN_MASK | TSCR1_TFFCA_MASK;
  // TSCR1_TEN = 1;

  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (Sw_Process(&left, SWL_LEFT) == Pressed)
    {
      if (period <= 9999 && period > duty + 50)
      {
        period += 50;
        //duty += 25;
        TC7 = period;
        //TC0 = duty;
      }
    }
    if (Sw_Process(&right, SWL_RIGHT) == Pressed)
    {
      if (period >= 249 && period > duty + 50)
      {
        period -= 50;
        //duty -= 25;
        TC7 = period;
        //TC0 = duty;
      }
    }
    if (Sw_Process(&up, SWL_UP) == Pressed)
    {
      temp = (int)((double)period * 0.05);
      if (duty <= period - 251)
      {
        duty += temp;
        TC0 = duty;
      }
    }
    if (Sw_Process(&down, SWL_DOWN) == Pressed)
    {
      temp = (int)((double)period * 0.05);
      if (duty >= 499)
      {
        duty -= temp;
        TC0 = duty;
      }
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/

/********************************************************************/
// Interrupt Service Routines
/********************************************************************/