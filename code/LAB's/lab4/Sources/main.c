/********************************************************************/
// HC12 Program:  Lab 4
// Processor:     MC9S12XDP512
// Bus Speed:     20 MHz
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
#include "i2c.h"

/********************************************************************/
// Local Prototypes
/********************************************************************/
void RTI_Callback(void);
/********************************************************************/
// Global Variables
/********************************************************************/
char rxData;
unsigned int msCounter = 0;

char convert = 0, update = 0;

int valueA = 0;

int output;
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

  // 200[us] event -> 20E6 x 200us = 10,000

  PIT_InitChannel(PIT_CH0, PIT_MT0, PIT_IDIS);
  PITMTLD0 = 0;
  PITLD0 = 9999;
  PIT_Start();

  I2C0_InitBus(I2CBus400); // for 400KHz
  // ADC Settings

  // Power up - Fast flag clear - ATD Power Down in wait mode
  ATD0CTL2 = ATD0CTL2_ADPU_MASK | ATD0CTL2_AFFC_MASK | ATD0CTL2_AWAI;

  // Number of conversion per sequence (1), finish current conversion then freeze
  ATD0CTL3 = ATD0CTL3_S1C_MASK | ATD0CTL3_FRZ1_MASK;
  // ATD0CTL3_S1C = 1;
  // ATD0CTL3_FRZ1 = 1;

  // 20MHz, divide by 20, ADC to 1MHz
  // 10-bit resolution, 4 A/D coversion clock periods, prescaler to 20 -> 1MHZ ADC-Clock
  ATD0CTL4 = ATD0CTL4_SMP0 | ATD0CTL4_PRS3_MASK | ATD0CTL4_PRS0_MASK;

  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (PITTF & PITTF_PTF0_MASK)
    {
      PITTF = PITTF_PTF0_MASK; // clear flag;
      convert = 0;
      ATD0CTL5 = ATD0CTL5_DJM_MASK; // triggers conversion
      while (!(ATD0STAT0 & ATD0STAT0_SCF_MASK))
        ;                   // wait for conversion to complete
      valueA = ATD0DR0 * 5; // read value, every step (LSB) is 5[mV], vREF = 5.115 /5.12 [V]
      Segs_16D(valueA, Segs_LineTop);

      output = (int)(0.8 * valueA);
      I2C0_WriteDAC(output, DAC_A);

      I2C0_WriteDAC((4095 - output), DAC_B);
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/

/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
