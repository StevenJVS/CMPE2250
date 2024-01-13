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
#include "lcd.h"

#include <stdio.h>  //for sprintf
#include <string.h> //for memcpy, memset
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
SwState anyState, left, right, up, down, ctr;
unsigned int duty = 500;

char update = 1;
char lcdmessage[25];

char message[5][9] = {"0.1%", "1%", "10%", "Polarity", "Reset"};
// sprintf();
int index = 0;
char on = 1;
char pol = 0;

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
  lcd_Init();
  // Working @20[MHz]/4 = 200ns
  //-> 200us/200ns = 1000
  // no prescaling

  // use B clock
  PWMPRCLK |= PWMPRCLK_PCKB1_MASK;

  // concatenate PWM6 and PWM7
  PWMCTL_CON67 = 1;

  // PWM positive polarity
  PWMPOL_PPOL7 = 0; // PWM conf: Low at the beginning of the period

  // set period
  PWMPER67 = 1000;

  // set Duty cycle to 25%
  PWMDTY67 = duty; // Define Pulse width

  PWME |= PWME_PWME7_MASK; // Enable channel 7
  // main program loop
  // sprintf(message[0]);
  // sprintf(message[1]);
  // sprintf(message[2]);
  // sprintf(message[4]);
  /********************************************************************/
  for (;;)
  {
    if (Sw_Process(&anyState, SWL_ANY) == Pressed)
    {
      update = 1;
    }
    if (Sw_Process(&left, SWL_LEFT) == Pressed)
    {
      if (--index < 0)
        index = 4;
    }
    if (Sw_Process(&right, SWL_RIGHT) == Pressed)
    {
      if (++index > 4)
        index = 0;
    }
    if (Sw_Process(&up, SWL_UP) == Pressed)
    {
      switch (index)
      {
      case 0:
        if (duty < 999)
          duty += 1;
        PWMDTY67 = duty;
        break;
      case 1:
        if (duty < 989)
          duty += 10;
        PWMDTY67 = duty;
        break;
      case 2:
        if (duty < 899)
          duty += 100;
        PWMDTY67 = duty;
        break;
      case 3:
          PWMPOL_PPOL7 = 1;
          pol = 1;
        break;
      case 4:
        index = 0;
        PWMPOL_PPOL7 = 1;
        duty = 500;
        PWMDTY67 = duty; // Define Pulse width
        break;
      default:
        break;
      }
    }
    if (Sw_Process(&down, SWL_DOWN) == Pressed)
    {
      switch (index)
      {
      case 0:
        if (duty > 1)
          duty -= 1;
        PWMDTY67 = duty;
        break;
      case 1:
        if (duty > 11)
          duty -= 10;
        PWMDTY67 = duty;
        break;
      case 2:
        if (duty > 111)
          duty -= 100;
        PWMDTY67 = duty;
        break;
      case 3:
          PWMPOL_PPOL7 = 0;
          pol = 0;
        break;
      case 4:
        index = 0;
        PWMPOL_PPOL7 = 1;
        duty = 500;
        PWMDTY67 = duty; // Define Pulse width
        break;
      default:
        break;
      }
    }
    if (Sw_Process(&ctr, SWL_CTR) == Pressed)
    {
      if (PWME_PWME7 == 1)
      {
        PWME_PWME7 = 0;
        on = 0;
      }
      else
      {
        PWME_PWME7 = 1;
        on = 1;
      }
    }
    if (update)
    {
      lcd_Clear();
      sprintf(lcdmessage, "Adjust: %s", message[index]);
      lcd_StringXY(0, 0, lcdmessage);
      sprintf(lcdmessage, "Duty %.2f%%", (float)((duty / 1000.0) * 100.0));
      lcd_StringXY(0, 1, lcdmessage);
      if (pol)
      {
        sprintf(lcdmessage, "Polarity: +");
        lcd_StringXY(0, 2, lcdmessage);
      }
      else
      {
        sprintf(lcdmessage, "Polarity: -");
        lcd_StringXY(0, 2, lcdmessage);
      }
      if (on)
      {
        sprintf(lcdmessage, "PWM Chan 7: On");
        lcd_StringXY(0, 3, lcdmessage);
      }
      else
      {
        sprintf(lcdmessage, "PWM Chan 7: Off");
        lcd_StringXY(0, 3, lcdmessage);
      }
      update = 0;
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/

/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
