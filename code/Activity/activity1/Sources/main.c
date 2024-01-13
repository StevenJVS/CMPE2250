/********************************************************************/
// HC12 Program:  SCI demo w/timeout
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

/********************************************************************/
// Local Prototypes
/********************************************************************/

/********************************************************************/
// Global Variables
/********************************************************************/
unsigned char rxData; // data to store
SwState leftstate = Idle;
SwState rightstate = Idle;
SwState ctrstate = Idle;
int index = 0;

unsigned char txBuffer[40]; // char array to send to sci
unsigned char rxBuffer[40]; // char array recieved from sci

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
  sci0_Init(9600, 0);

  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {

    if (sci0_rxByte(&rxData))
    {
      if (rxData == '\r')
      { // process
        switch (rxBuffer[0])
        {
        case 'y':
          if (rxBuffer[1] == '1')
          {
            SWL_ON(SWL_YELLOW);
          }
          else if (rxBuffer[1] == '0')
            SWL_OFF(SWL_YELLOW);
          break;

        case 'g':
          if (rxBuffer[1] == '1')
          {
            SWL_ON(SWL_GREEN);
          }
          else if (rxBuffer[1] == '0')
            SWL_OFF(SWL_GREEN);
          break;
        case 'r':
          if (rxBuffer[1] == '1')
          {
            SWL_ON(SWL_RED);
          }
          else if (rxBuffer[1] == '0')
            SWL_OFF(SWL_RED);
          break;
        default:
          break;
        }
        // clear buffer
        (void)memset(rxBuffer, 0, sizeof(rxBuffer));
        index = 0;
      }
      else
      {
        rxBuffer[index++] = rxData;
      }
    }

    if (Sw_Process(&leftstate, SWL_LEFT) == Released)
    {
      sci0_txStr("r0\n\r");
    }
    if (Sw_Process(&ctrstate, SWL_CTR) == Released)
    {
      sci0_txStr("y0\n\r");
    }
    if (Sw_Process(&rightstate, SWL_RIGHT) == Released)
    {
      sci0_txStr("g0\n\r");
    }
    /////////////////////////////////////////////////////////////////
    if (Sw_Process(&leftstate, SWL_LEFT) == Pressed)
    {
      sci0_txStr("r1\n\r");
    }
    if (Sw_Process(&ctrstate, SWL_CTR) == Pressed)
    {
      sci0_txStr("y1\n\r");
    }
    if (Sw_Process(&rightstate, SWL_RIGHT) == Pressed)
    {
      sci0_txStr("g1\n\r");
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/