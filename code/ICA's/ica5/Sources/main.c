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
void manualConv(void);
void contConv(void);

/********************************************************************/
// Global Variables
/********************************************************************/
char rxData;
unsigned int msCounter = 0;

char manual = 0, cont = 0, display = 0;

unsigned int valueA = 0;
unsigned int conversions[8];
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
  sci0_Init(115200, 1);

  // Set PJ0 and PJ1 as inputs
  DDRJ &= ~(DDRJ_DDRJ1_MASK | DDRJ_DDRJ0_MASK);
  // Set edges for PJ0 and PJ1
  PPSJ |= PPSJ_PPSJ1_MASK; // rising edge, PRESS
  PPSJ |= PPSJ_PPSJ0_MASK; // falling edge, RELEASE

  // Enable Interrupts for PJ1 and PJ0
  PIEJ |= PIEJ_PIEJ1_MASK | PIEJ_PIEJ0_MASK;

  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (display)
    {
      if (manual)
      {
        ATD0CTL5 = ATD0CTL5_DJM_MASK; // triggers conversion
        while (!(ATD0STAT0 & ATD0STAT0_SCF_MASK))
          ;                   // wait for conversion to complete
        valueA = ATD0DR0 * 5; // read value, every step (LSB) is 5[mV], vREF = 5.115 /5.12 [V]

        // at AtoD module can only convert digital signals in 10 bits
        Segs_16D(valueA, Segs_LineTop);
        Segs_16H(ATD0DR0, Segs_LineBottom);
        // 5.02. the theoretical maximum value is suppose to be 5.12V but since we are only displaying the
        // signal every 100ms it rolls aver before it can show the actual max value.

        // since the period is only 50s, every second the ramp generator will produce 100mV.
      }
      if (cont)
      {
        Segs_16D(conversions[0] * 5, Segs_LineTop);
        Segs_16H(conversions[0], Segs_LineBottom);
        // sprintf(line1, "%04d", conversions[5] * 5);
        // lcd_StringXY(0,0, line1);

        // no its not incrementing in every 1 second anymore, because
      }
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
void manualConv(void)
{
  // reset regiters
  ATD0CTL2 = 0;
  ATD0CTL5 = 0;

  // Power up - Fast flag clear - ATD Power Down in wait mode
  ATD0CTL2 = ATD0CTL2_ADPU_MASK | ATD0CTL2_AFFC_MASK | ATD0CTL2_AWAI;

  // Number of conversion per sequence (1), finish current conversion then freeze
  ATD0CTL3 = ATD0CTL3_S1C_MASK | ATD0CTL3_FRZ1_MASK;
  // ATD0CTL3_S1C = 1;
  // ATD0CTL3_FRZ1 = 1;

  // 20MHz, divide by 20, ADC to 1MHz
  // 10-bit resolution, 4 A/D coversion clock periods, prescaler to 20 -> 1MHZ ADC-Clock
  ATD0CTL4 = ATD0CTL4_SMP0 | ATD0CTL4_PRS3_MASK | ATD0CTL4_PRS0_MASK;
}
void contConv(void)
{
  // reset regiters
  ATD0CTL2 = 0;

  // Power up - Fast flag clear - Interrupt enable
  ATD0CTL2 = ATD0CTL2_ADPU_MASK | ATD0CTL2_AFFC_MASK | ATD0CTL2_ASCIE_MASK;

  // Number of conversion per sequence (8)
  ATD0CTL3 = 0;

  // 20MHz, divide by 20, ADC to 1MHz
  // 10-bit resolution, 4 A/D coversion clock periods, prescaler to 20 -> 1MHZ ADC-Clock
  ATD0CTL4 = ATD0CTL4_SMP0 | ATD0CTL4_PRS3_MASK | ATD0CTL4_PRS0_MASK;

  // ATD0CTL0 = 0 ; //no effect wrapping

  // Right justified, multi channel, Continuos conversion
  ATD0CTL5 = ATD0CTL5_DJM_MASK | ATD0CTL5_MULT_MASK | ATD0CTL5_SCAN_MASK;
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vsci0 void Vsci0_ISR(void)
{
  if (SCI0SR1 & SCI0SR1_RDRF_MASK)
  {
    rxData = SCI0DRL; // read character
  }
}
interrupt VectorNumber_Vatd0 void Vatd0_ISR(void)
{
  SWL_ON(SWL_GREEN);
  // ATD0STAT0 = ATD0STAT0_SCF_MASK; //flag clearing, not necessary here
  conversions[0] = ATD0DR0;
  conversions[1] = ATD0DR1;
  conversions[2] = ATD0DR2;
  conversions[3] = ATD0DR3;
  conversions[4] = ATD0DR4;
  conversions[5] = ATD0DR5;
  conversions[6] = ATD0DR6;
  conversions[7] = ATD0DR7;
  SWL_OFF(SWL_GREEN);
}
interrupt VectorNumber_Vportj void Vportj_ISR(void)
{
  SWL_TOG(SWL_RED);
  if (PIFJ & PIFJ_PIFJ0_MASK) // PJ0 was detected an edge
  {
    manual = 1;
    cont = 0;
    manualConv();

    lcd_Clear();
    lcd_StringXY(0, 0, "Manual Conv");
    PIFJ = PIFJ_PIFJ0_MASK; // clear flag;
  }
  if (PIFJ & PIFJ_PIFJ1_MASK) // PJ1 detected an edge
  {
    manual = 0;
    cont = 1;
    contConv();

    lcd_Clear();
    lcd_StringXY(0, 0, "Cont Conv");
    PIFJ = PIFJ_PIFJ1_MASK; // clear flag;
  }
}
