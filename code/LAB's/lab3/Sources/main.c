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
unsigned int conversion;
char update = 0;
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
  // Working @20[MHz] = 50ns
  // no prescaling
  PWMPRCLK = 0;

  // concatenate PWM6 and PWM7
  PWMCTL_CON67 = 1;

  // PWM positive polarity
  PWMPOL_PPOL7 = 1; // PWM conf: High at the beginning of the period

  // set period
  PWMPER67 = 512;

  // set Duty cycle to 50%
  PWMDTY67 = 256; // Define Pulse width

  PWME |= PWME_PWME7_MASK; // Enable channel 7
                           //////////////////////////////////////////////////////////////////////////////
  // Power up - Fast flag clear - Interrupt enable
  ATD0CTL2 = ATD0CTL2_ADPU_MASK | ATD0CTL2_AFFC_MASK | ATD0CTL2_ASCIE_MASK;

  // Number of conversion per sequence (1)
  ATD0CTL3 = ATD0CTL3_S1C_MASK;

  // 10-bit resolution, 4 A/D coversion clock periods, prescaler to 20 -> 1MHZ ADC-Clock
  ATD0CTL4 = ATD0CTL4_SMP0 | ATD0CTL4_PRS3_MASK;

  ATD0CTL0 = 0; // no effect wrapping

  // Right justified, Continuos conversion
  ATD0CTL5 = ATD0CTL5_DJM_MASK | ATD0CTL5_SCAN_MASK;
  /********************************************************************/
  // initializations
  /********************************************************************/
  Clock_Set20MHZ();
  SWL_Init();
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (update)
    {
      PWMDTY67 = conversion/2;
      update = 0;
    }
  }
}
interrupt VectorNumber_Vatd0 void Vatd0_ISR(void)
{
  // ATD0STAT0 = ATD0STAT0_SCF_MASK; //flag clearing, not necessary here
  SWL_TOG(SWL_GREEN);
  conversion = ATD0DR0;
  update = 1;
}

/********************************************************************/
// Functions
/********************************************************************/

/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
