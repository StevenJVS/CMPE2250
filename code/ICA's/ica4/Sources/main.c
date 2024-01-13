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
#include "rti.h"
#include <stdio.h>  //for sprintf
#include "sw_led.h" // include switch library
#include "clock.h"  //include clock library
#include "segs.h"
#include "lcd.h"
#include "sci.h"

/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here

/********************************************************************/
// Local Prototypes
/********************************************************************/
void ledtog(void);
void lcdratio(void);
/********************************************************************/
// Global Variables
/********************************************************************/
int mycounter = 0;
unsigned long myHexcounter = 0;
unsigned long myLoopcounter = 0;
float ratio;
unsigned char startingchar = 'a';
unsigned char currentchar;
int switchcase = 1;
int charrecieved = 0;
unsigned char rxData; // data to store
unsigned char charVAl[40];
unsigned char charHex[40];
unsigned char charinput[40];
unsigned char anyinput;
unsigned char ratiobuffer[40];
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
  RTI_Init();
  Segs_Init();
  lcd_Init();
  SWL_Init();
  lcd_Clear();
  sci0_Init(9600, 1);//set baud rate and inable interrupts
  RTI_InitCallback(&ledtog);//function will trigger every 1ms
  currentchar = startingchar;
  lcd_String("Loops/Char Ratio:");
  lcd_StringXY(0, 3, "RX:");
  Segs_Clear();
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    Segs_16H(myLoopcounter++, Segs_LineTop);
    lcdratio();

    if (charrecieved)
    {
      // check if the character recieved is a valid character
      if (rxData >= 'a' && rxData <= 'z' || (rxData - 32) >= 'A' && (rxData - 32) <= 'Z')
      {
        startingchar = rxData;
      }
      // store recieved charater from the register
      sprintf(charVAl, "%c", anyinput);
      sprintf(charHex, "%2x", anyinput);

      // reset flag
      charrecieved = 0;
    }

    // if current character is 'z' pr 'Z'
    if (currentchar == 123 || currentchar == 91)
    {
      // then line feed and carriage return
      sci0_txStr("\r\n");
      currentchar = startingchar;
    }

    if (SCI0SR1 & SCI0SR1_TDRE_MASK) // if data is recieved (register, flag)
    {
      Segs_16H(myHexcounter++, Segs_LineBottom);

      if (switchcase)
      {
        if (currentchar >= 'a' && currentchar <= 'z') // check if current character is between a and z
        {
          // set current character to switchcased character
          currentchar -= 32;
        }
      }
      else
      {
        if (currentchar >= 'A' && currentchar <= 'Z') // check if current character is between A and Z
        {
          // set current character to switchcased character
          currentchar += 32;
        }
      }
      //feed the character to the register, then increment
      SCI0DRL = currentchar++;
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void ledtog()
{
  mycounter++;
  if (mycounter == 1000)
  {
    // switch case every 1s
    mycounter = 0;
    switchcase ^= 1;
    // update display every 500ms
    lcd_StringXY(4, 3, charVAl);
    lcd_StringXY(6, 3, ":");
    lcd_StringXY(8, 3, charHex);
    lcd_StringXY(0, 1, ratiobuffer);
  }
  // update display every 500ms
  if (mycounter == 500)
  {
    lcd_StringXY(4, 3, charVAl);
    lcd_StringXY(6, 3, ":");
    lcd_StringXY(8, 3, charHex);
    lcd_StringXY(0, 1, ratiobuffer);
  }
}
void lcdratio()
{
  // calculate ratio and display
  ratio = (float)myLoopcounter / (float)myHexcounter;
  sprintf(ratiobuffer, "%2.2f", ratio);
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vsci0 void Vsci0_ISR(void)
{
  if (SCI0SR1 & SCI0SR1_RDRF_MASK)//check flag if data is received
  {
    SWL_TOG(SWL_RED);
    // reads character
    rxData = SCI0DRL;
    charrecieved = 1;
    if (SCI0DRL == 13 || SCI0DRL == 10)
    {
      // if a carriage return is recieved, do nothing
    }
    else
    {
      // display store any character recieved from the terminal
      anyinput = SCI0DRL;
    }
  }
}