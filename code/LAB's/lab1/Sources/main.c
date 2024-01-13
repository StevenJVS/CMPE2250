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
#include "sci.h"
#include "pit.h"
#include "segs.h"
#include "lcd.h"
#include "rti.h"
#include <stdio.h> //for sprintf

/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here
typedef enum Status // enum
{
  Waiting,
  Loading,
  Timeout,
  Overrun,
  Halted
} Status;

/********************************************************************/
// Local Prototypes
/********************************************************************/
void redledtog(void);// ISR funtion
/********************************************************************/
// Global Variables
/********************************************************************/
int send = 0;
Status current;
int timer = 0;
int waitcounter = 0;

unsigned char rxData;               //data to store
unsigned char txBuffer[40];         // char array to send to sci
unsigned char rxBuffer[40];         // char array received from sci
unsigned char ProccessedBuffer[40]; // proccessed buffer
int index = 0;

//counters 
unsigned int rcounter = 0;
unsigned int pcounter = 0;
unsigned int tempnum; // used to display rcounter before cleared

//timers
int loadingtimer = 0;
int displaytimer = 0;
unsigned int haltcounter = 0;

unsigned char message[40];//processed message
int trash;//used to clear sci0 flag when current is not loading or waiting
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

  current = Waiting;

  /********************************************************************/
  // initializations
  /********************************************************************/
  Clock_Set20MHZ();
  RTI_Init();
  SWL_Init();

  // 100[us] event -> 8E6 x 100E-6 = 2,000
  PITMTLD0 = 0;
  PITLD0 = 1999;

  PIT_InitChannel(PIT_CH0, PIT_MT0, PIT_IEN);

  PIT_Start();
  lcd_Init();
  lcd_Clear();
  sci0_Init(115200, 1);         // set baud rate and enable interrupts
  RTI_InitCallback(&redledtog); // function will trigger every 1ms
  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (current == Waiting)
    {
      SWL_OFF(SWL_GREEN);
      if (timer == 1000)
      {
        sci0_txStr("Waiting\r\n");
        timer = 0;
      }
      if (waitcounter == 100)
      {
        SWL_TOG(SWL_RED);
        waitcounter = 0;
        lcd_StringXY(0, 0, "Waiting");
      }
    }
    if (current == Loading)
    {
      if (send)//send to sci when message is needed to be sent
      {
        sprintf(message, "Processed:%s\r\n", rxBuffer);
        sci0_txStr(message);
        (void)memset(message, 0, sizeof(message));
        (void)memset(rxBuffer, 0, sizeof(rxBuffer));
        index = 0;//clear index here to ensure data is being sent to the user

        loadingtimer = 0;
        send ^= 1;
      }

      if (displaytimer >= 100)// update lcd every 100ms
      {
        lcd_StringXY(0, 0, "Loading");
        SWL_OFF(SWL_RED); // ensures red led is off
        SWL_TOG(SWL_GREEN);

        sprintf(txBuffer, "Received:%d", tempnum);
        lcd_StringXY(0, 2, txBuffer);
        (void)memset(txBuffer, 0, sizeof(txBuffer));

        sprintf(txBuffer, "Processed:%d", pcounter);
        lcd_StringXY(0, 3, txBuffer);
        (void)memset(txBuffer, 0, sizeof(txBuffer));

        displaytimer = 0;
      }
      if (loadingtimer == 2000)
      {
        current = Timeout;
        loadingtimer = 0;
        send = 1;
      }
    }
    if (current == Overrun)
    {
      if (send)
      {
        SWL_ON(SWL_RED);
        SWL_ON(SWL_GREEN);
        lcd_StringXY(0, 0, "Buffer Overrun");
        sci0_txStr("Buffer Overrun\r\n");
        send ^= 1;
      }
      if (haltcounter >= 2000)
      {
        current = Halted;
        haltcounter = 0;
      }
    }
    if (current == Timeout)
    {
      if (send)
      {
        SWL_OFF(SWL_GREEN);
        SWL_ON(SWL_RED);
        lcd_StringXY(0, 0, "Timeout");
        sci0_txStr("Timeout\r\n");
        send ^= 1;
      }
      if (haltcounter >= 2000)
      {
        current = Halted;
        haltcounter = 0;
      }
    }
    if (current == Halted)
    {
      (void)memset(rxBuffer, 0, sizeof(rxBuffer));
      pcounter = 0;
      rcounter = 0;
      index = 0;
      tempnum = 0;
      haltcounter = 0;
      if (SWL_Pushed(SWL_CTR))
      {
        current = Waiting;
        lcd_Clear();
      }
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void redledtog()
{
  if (current == Waiting)
  {
    waitcounter++;
    timer++;
  }

  if (current == Loading)
  {
    loadingtimer++;
    displaytimer++;
  }

  if (current == Timeout || current == Overrun)
  {
    haltcounter++;
  }
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vpit0 void Vpit0_Handler(void)
{
  PITTF = PITTF_PTF0_MASK; // clear flag;
  SWL_TOG(SWL_YELLOW);
}
interrupt VectorNumber_Vsci0 void Vsci0_ISR(void)
{
  if (SCI0SR1 & SCI0SR1_RDRF_MASK)
  {
    trash = SCI0DRL;
    if (current == Waiting || current == Loading)//only use data when Waiting or Loading only
    {
      current = Loading;
      loadingtimer = 0;
      rxData = SCI0DRL;
      if (rxData == '\r')//process if carriage return is recieved
      {
        send = 1;
        tempnum = rcounter;
        rcounter = 0;
      }
      else
      {
        rxBuffer[index++] = rxData;
        pcounter++;
        rcounter++;

        if (index > 32)//check if index is greater than 33
        {
          current = Overrun;
          send = 1;
        }
      }
    }
  }
}