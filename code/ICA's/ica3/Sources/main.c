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
#include "pit.h"
#include "lcd.h"
#include "segs.h"
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
char myName[] = "Steven Santiago";
volatile int count = 0;
int ss = 0;
int min = 0;
char tx_buffer[40]; // visible count
int tog = 1;
int run = 1;
volatile int holdcount = 0;
int holdcheck = 0;
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

  // 100[ms] event -> 20E6 x 100E-3 = 2,000,000
  // 2,000,000 = 50 x 40,000
  PITMTLD0 = 49;
  PITLD0 = 39999;

  PIT_Start();

  DDRJ &= ~(DDRJ_DDRJ1_MASK | DDRJ_DDRJ0_MASK);
  // Set edges for PJ0 and PJ1
  PPSJ |= PPSJ_PPSJ1_MASK;  // rising edge, PRESS
  PPSJ &= ~PPSJ_PPSJ0_MASK; // falling edge, RELEASE

  // Enable Interrupts for PJ1 and PJ0
  PIEJ |= PIEJ_PIEJ1_MASK | PIEJ_PIEJ0_MASK;

  lcd_Init();
  lcd_String(myName);//print name
  lcd_StringXY(0,1,"Time:");//print time
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (count == 10)//increase seconds after 10 ms
    {
      count = 0;
      ss++;
    }
    if (ss == 60)//increase min after 60 s
    {
      ss = 0;
      min++;
    }
    if (tog)//check if counter started
    {
      if (run)//check if lcd is running
      {
        sprintf(tx_buffer, "%02d:%02d:%01d", min, ss, count);
        lcd_StringXY(5, 1, tx_buffer); // print on lcd
      }
    }
    if (holdcheck)//check if botton i held
    {
      if (holdcount > 30)//check if held over 30ms == 3s
      {
        //reset count
        count = 0;
        ss = 0;
        min = 0;
        sprintf(tx_buffer, "%02d:%02d:%01d", min, ss, count);
        lcd_StringXY(5, 1, tx_buffer); // print on lcd

        //enable lcd to run and reset timer
        run = 1;
        tog = 0;

        //reset hold count and disable botton being held
        holdcount = 0;
        holdcheck ^= 1;
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
interrupt VectorNumber_Vpit0 void Vpit0_ISR(void)
{
  PITTF = PITTF_PTF0_MASK; // clear flag;
  if (tog)//run timer if toggled
  {
    count++;
    SWL_TOG(SWL_RED);
  }
  if (holdcheck)//if button is held, increment
  {
    holdcount++;
  }
}

interrupt VectorNumber_Vportj void Vportj_ISR(void)
{
  if (PIFJ & PIFJ_PIFJ0_MASK) // PJ0 was detected an edge
  {
    if (!run && tog)//run display if display was paused
    {
      run ^= 1;
      tog ^= 1;
    }
    PIFJ = PIFJ_PIFJ0_MASK; // clear flag;
    tog ^= 1;
  }
  if (PIFJ & PIFJ_PIFJ1_MASK) // PJ1 detected an edge
  {
    PIFJ = PIFJ_PIFJ1_MASK; // clear flag;
    holdcheck = 1;//botton is being held

    if (PPSJ & PPSJ_PPSJ1_MASK) // PJ1 if its a rising edge
    {
      PPSJ &= ~PPSJ_PPSJ1_MASK; // falling edge, RELEASE
      run ^= 1;//tog lcd
    }
    else
    {
      //if botton was a falling edge tog hold to 0 and reset count
      SWL_TOG(SWL_GREEN);
      PPSJ |= PPSJ_PPSJ1_MASK; // rising edge, PRESS
      holdcheck ^= 1;
      holdcount = 0;
    }
  }
}