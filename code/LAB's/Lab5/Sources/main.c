/********************************************************************/
// HC12 Program:  Lab05 - RTC
// Processor:     MC9S12XDP512
// Bus Speed:     20 MHz
// Author:        Steven Santiago
// Details:       RTC

// Date:          December, 12th 2023
// Revision History :

/********************************************************************/
// Constant Defines
/********************************************************************/

/********************************************************************/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
/********************************************************************/
// Library includes
/********************************************************************/
// your includes go here
#include <string.h>
#include <stdio.h>

#include "clock.h"
#include "rti.h"
#include "sw_led.h"
#include "segs.h"
#include "lcd.h"
#include "sci.h"
#include "i2c.h"
#include "rti.h"

typedef enum Header
 {
 None = 0,
 UnixTime = 0xA1,
 LocalTime = 0xA2,
 RGB = 0xA3,
 Servo = 0xA4
 }Header;


/********************************************************************/
// Local Prototypes
/********************************************************************/
void RTI_Callback(void);
void ReadRtc(RTC_Read *);
void WriteRtc(RTC_Read);
void DisplayRtc(RTC_Read, char);
/********************************************************************/
// Global Variables
/********************************************************************/
int msCounter = 0;
char rtiUpdate = 0, display[128];
int test;

RTC_Read readRTC, haltedTime, clockTime;
char readData, SecondsReg, AlarmHourReg;
char Array[8] = {0};
char MSB;
char LSB;
Header rxData;
/********************************************************************/
// Constants
/********************************************************************/
const unsigned char *DaysArray[] =
    {
        "NA", // 0
        "Su", // 1
        "Mo", // 2
        "Tu", // 3
        "We", // 4
        "Th", // 5
        "Fr", // 6
        "Sa", // 7
};

const unsigned char *MonthsArray[] =
    {
        "NON", // 0
        "Jan", // 1
        "Feb", // 2
        "Mar", // 3
        "Apr", // 4
        "May", // 5
        "Jun", // 6
        "Jul", // 7
        "Aug", // 8
        "Sep", // 9
        "Oct", // 10
        "Nov", // 11
        "Dec", // 12
};
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
  RTI_InitCallback(RTI_Callback);
  SWL_Init();
  lcd_Init();
  lcd_DispControl(0, 0);
  Segs_Init();
  (void)sci0_Init(115200, 1);

  I2C0_InitBus(I2CBus400); // for 400KHz

  // Configuring the M41T81S: "RTC" *****************************

  // Check for STOP flag
  test = I2C0_RegRead(&SecondsReg, RTC_ADD, RTC_SECONDS);
  if (SecondsReg & RTC_SECONDS_ST)
  {
    SWL_ON(SWL_RED);
    SWL_OFF(SWL_GREEN);
    I2C0_RegWrite(RTC_ADD, RTC_SECONDS, SecondsReg & ~RTC_SECONDS_ST, IIC0_STOP);
  }
  else
  {
    SWL_ON(SWL_GREEN);
    SWL_OFF(SWL_RED);
  }

  // Check for HALT flag
  test = I2C0_RegRead(&AlarmHourReg, RTC_ADD, RTC_AL_HOUR);
  if (AlarmHourReg & RTC_AL_HOUR_HT)
  {
    SWL_ON(SWL_YELLOW);
    I2C0_RegWrite(RTC_ADD, RTC_AL_HOUR, AlarmHourReg & ~RTC_AL_HOUR_HT, IIC0_STOP);
    ReadRtc(&readRTC);
    DisplayRtc(readRTC, 'h');
  }
  else
  {
    SWL_OFF(SWL_YELLOW);
  }

  /********************************************************************/
  // main program loop
  /********************************************************************/
  for (;;)
  {
    if (rtiUpdate)
    {
      rtiUpdate = 0;
      ReadRtc(&readRTC);
      DisplayRtc(readRTC, 'c');
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void RTI_Callback()
{
  if (++msCounter > 999)
  {
    msCounter = 0;
    rtiUpdate = 1;
  }
}
void ReadRtc(RTC_Read *time)
{
  test = I2C0_RegRead(&Array[0], RTC_ADD, RTC_HSECONDS);
  time->HuSeconds = 0.1 * (Array[0] >> 4) + 0.01 * (Array[0] & 0x0F);

  test = I2C0_RegRead(&Array[1], RTC_ADD, RTC_SECONDS);
  time->Seconds = 10 * ((Array[1] & 0x70) >> 4) + (Array[1] & 0x0F);

  test = I2C0_RegRead(&Array[2], RTC_ADD, RTC_MINUTES);
  time->Minutes = 10 * ((Array[2] & 0x70) >> 4) + (Array[2] & 0x0F);

  test = I2C0_RegRead(&Array[3], RTC_ADD, RTC_CHOUR);
  time->Hours = 10 * ((Array[3] & 0x30) >> 4) + (Array[3] & 0x0F);

  test = I2C0_RegRead(&Array[4], RTC_ADD, RTC_DAY);
  time->Day = ((Array[4] & 0x0F) & 0x7);

  test = I2C0_RegRead(&Array[5], RTC_ADD, RTC_DATE);
  time->MonthDay = 10 * ((Array[5] & 0x30) >> 4) + (Array[5] & 0x0F);

  test = I2C0_RegRead(&Array[6], RTC_ADD, RTC_MONTH);
  time->Month = 10 * ((Array[6] & 0x10) >> 4) + (Array[6] & 0x0F);

  test = I2C0_RegRead(&Array[7], RTC_ADD, RTC_YEAR);
  time->Year = 2000 + (10 * (Array[7] >> 4)) + (Array[7] & 0x0F);
}
void WriteRtc(RTC_Read time)
{
}
void DisplayRtc(RTC_Read time, char actualReading)
{
  if (actualReading == 'c')
  {
    (void)sprintf(display, "%s %s %d ,%d", DaysArray[time.Day], MonthsArray[time.Month], time.MonthDay, time.Year);
    lcd_StringXY(0, 0, display);
    (void)sprintf(display, "%02d:%02d:%02d - Curr Time", (int)time.Hours, (int)time.Minutes, (int)time.Seconds);
    lcd_StringXY(0, 1, display);
  }
  else if (actualReading == 'h')
  {
    (void)sprintf(display, "%s %s %d ,%d", DaysArray[time.Day], MonthsArray[time.Month], time.MonthDay, time.Year);
    lcd_StringXY(0, 2, display);
    (void)sprintf(display, "%02d:%02d:%02d - Halt Time", (int)time.Hours, (int)time.Minutes, (int)time.Seconds);
    lcd_StringXY(0, 3, display);
  }
}
/********************************************************************/
// Interrupt Service Routines
/********************************************************************/
interrupt VectorNumber_Vsci0 void Vsci0_ISR(void)
{
  if (SCI0SR1 & SCI0SR1_RDRF_MASK)
  {
    rxData = SCI0DRL;
    // if (current == Waiting || current == Loading)//only use data when Waiting or Loading only
    // {
    //   current = Loading;
    //   loadingtimer = 0;
    //   rxData = SCI0DRL;
    //   if (rxData == '\r')//process if carriage return is recieved
    //   {
    //     send = 1;
    //     tempnum = rcounter;
    //     rcounter = 0;
    //   }
    //   else
    //   {
    //     rxBuffer[index++] = rxData;
    //     pcounter++;
    //     rcounter++;

    //     if (index > 32)//check if index is greater than 33
    //     {
    //       current = Overrun;
    //       send = 1;
    //     }
    //   }
    // }
  }
}