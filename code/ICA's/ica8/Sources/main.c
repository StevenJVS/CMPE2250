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
#include "segs.h"
#include "rti.h"
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
void RTI_Callback(void);
/********************************************************************/
// Global Variables
/********************************************************************/
unsigned int period = 156;
float duty;
SwState anyState, left, right, up, down, ctr;

unsigned int msCounter = 0;
char update = 0;
char lcdmessage[25];

int index = 0;

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
  RTI_InitCallback(RTI_Callback);
  // Working @20[MHz]/8 = 400ns
  //-> 1ms/400ns = 2500
  PWMPRCLK |= PWMPRCLK_PCKB1_MASK;

  // use SB clock
  PWMCLK_PCLK3 = 1;
  /*Formula
    2500 = 8 x 2 x 156.25
  */

  // PWM positive polarity
  PWMPOL_PPOL3 = 1; // PWM conf: High at the beginning of the period

  // set period
  PWMPER3 = 156;

  // set Duty cycle to 25%
  PWMDTY3 = 39; // Define Pulse width

  PWME |= PWME_PWME3_MASK; // Enable channel 3

  //@20[MHz]/16 = 800ns
  // 200us/784.31ns = 255
  PWMPRCLK |= PWMPRCLK_PCKA2_MASK;

  // default to clock a
  PWMCLK_PCLK0 = 0;
  PWMCLK_PCLK1 = 0;
  PWMCLK_PCLK4 = 0;

  PWMPOL_PPOL0 = 1; // PWM conf: High at the beginning of the period
  PWMPOL_PPOL1 = 1; // PWM conf: High at the beginning of the period
  PWMPOL_PPOL4 = 1; // PWM conf: High at the beginning of the period

  // set period
  PWMPER0 = 255;
  // set period
  PWMPER1 = 255;
  // set period
  PWMPER4 = 255;

  // set Duty cycle to 25%
  PWMDTY0 = 255; // Define Pulse width
  PWMDTY1 = 255; // Define Pulse width
  PWMDTY4 = 255; // Define Pulse width

  PWME |= PWME_PWME0_MASK; // Enable channel 3
  PWME |= PWME_PWME1_MASK; // Enable channel 3
  PWME |= PWME_PWME4_MASK; // Enable channel 3
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (Sw_Process(&anyState, SWL_ANY) == Pressed)
    {
      period = 156;
      PWMDTY3 = period;
    }
    if (Sw_Process(&left, SWL_LEFT) == Pressed)
    {
      if (index < 0)
        index = 2;
      else
        index--;
    }
    if (Sw_Process(&right, SWL_RIGHT) == Pressed)
    {
      if (index > 2)
        index = 0;
      else
        index++;
    }
    if (Sw_Process(&up, SWL_UP) == Pressed)
    {
      switch (index)
      {
      case 0:
        if (PWMDTY0 < 255)
          PWMDTY0++;
        break;
      case 1:
        if (PWMDTY1 < 255)
          PWMDTY1++;
        break;
      case 2:
        if (PWMDTY4 < 255)
          PWMDTY4++;
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
        if (PWMDTY0 > 0)
          PWMDTY0--;
        break;
      case 1:
        if (PWMDTY1 > 0)
          PWMDTY1--;
        break;
      case 2:
        if (PWMDTY4 > 0)
          PWMDTY4--;
        break;
      default:
        break;
      }
    }
    if (Sw_Process(&ctr, SWL_CTR) == Pressed)
    {
      switch (index)
      {
      case 0:
        if (PWMDTY0 < 128 && PWMDTY0 < 255)
          PWMDTY0 += 20;
        else if (PWMDTY0 > 128 && PWMDTY0 > 0)
          PWMDTY0 -= 20;
        break;
      case 1:
        if (PWMDTY1 < 128 && PWMDTY1 < 255)
          PWMDTY1 += 20;
        else if (PWMDTY1 > 128 && PWMDTY1 > 0)
          PWMDTY1 -= 20;
        break;
      case 2:
        if (PWMDTY4 < 128 && PWMDTY4 < 255)
          PWMDTY4 += 20;
        else if (PWMDTY4 > 128 && PWMDTY4 > 0)
          PWMDTY4 -= 20;
        break;
      default:
        break;
      }
    }
    if (update)
    {
      lcd_Clear();
      switch (index)
      {
      case 0:
        sprintf(lcdmessage, "B: %d", PWMDTY0);
        lcd_StringXY(0, 0, lcdmessage);
        lcd_StringXY(0, 1, "Blue");
        break;
      case 1:
        sprintf(lcdmessage, "G: %d", PWMDTY1);
        lcd_StringXY(0, 0, lcdmessage);
        lcd_StringXY(0, 1, "Green");
        break;
      case 2:
        sprintf(lcdmessage, "R: %d", PWMDTY4);
        lcd_StringXY(0, 0, lcdmessage);
        lcd_StringXY(0, 1, "Red");
        break;

      default:
        break;
      }
      if (period > 39)
      {
        period -= 2;
        PWMDTY3 = period; // Define Pulse width
      }
      duty = ((float)period / 156) * 100;
      sprintf(lcdmessage, "Duty Cycle: %.2f%%", duty);
      lcd_StringXY(0, 3, lcdmessage);

      update = 0;
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void RTI_Callback(void)
{
  SWL_TOG(SWL_RED);
  if (++msCounter > 49)
  {
    msCounter = 0;
    update = 1;
  }
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
