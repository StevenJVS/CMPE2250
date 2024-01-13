/********************************************************************/
// HC12 Program: ICA12 - I2C DAC
// Processor: MC9S12XDP512
// Bus Speed: 20 MHz
// Author: Steven Santiago
// Details: Demonstrate use of barometer for I2C protocol
// Date: Dec 12, 2023
// Revision History:

/********************************************************************/
// Includes
/********************************************************************/
#include <hidef.h>      // Common defines and macros
#include "derivative.h" // Derivative-specific definitions

#include <string.h>
#include <stdio.h>

#include "clock.h"
#include "rti.h"
#include "sw_led.h"
#include "segs.h"
#include "sci.h"
#include "lcd.h"
#include "i2c.h"
#include "pit.h"

/********************************************************************/
// Local Prototypes
/********************************************************************/
void rtiCallback(void);

/********************************************************************/
// Global Variables
/********************************************************************/
unsigned int milliSecondsCounter = 0;
char shouldUpdate = 0;
char testResult;
char readStatus = 0;

unsigned char pressureMSB, pressureCSB, pressureLSB;
unsigned long pressureData;
float pressurePascals;
unsigned char fractionalBits;

char temperatureMSB;
char temperatureLSB;
unsigned long temperatureData;
float temperatureCelsius;
unsigned char temperatureFractionalBits;

unsigned char bufferOne[128];
unsigned char bufferTwo[128];

/********************************************************************/
// Constants
/********************************************************************/

/********************************************************************/
// Main Entry
/********************************************************************/
void main(void)
{
  // Main entry point
  _DISABLE_COP();
  EnableInterrupts;

  /********************************************************************/
  // Initializations
  /********************************************************************/
  Clock_Set20MHZ();
  RTI_InitCallback(rtiCallback);
  SWL_Init();
  lcd_Init();
  lcd_DispControl(0, 0);
  Segs_Init();
  sci0_Init(115200, 1);

  I2C0_InitBus(I2CBus400); // 400kHz

  // Initialize thermometer/barometer/temp sensor
  // CTRL_REG1(0x26) -> 0b00111000 - Barometer mode, 128 Over Sampling, everything else 0
  testResult = I2C0_RegWrite(MPL3115A2_ADD, CTRL_REG1, 0x38, IIC0_STOP);

  // PT_DATA_CFG(0x13) -> 0b00000111 - Data ready Event flag, Event or Pressure/Alt, Event on Temp
  testResult = I2C0_RegWrite(MPL3115A2_ADD, PT_DATA_CFG, 0x07, IIC0_STOP);

  // Activate device (out of stand-by mode)
  // CTRL_REG1(0x26) -> 0b00111001
  testResult = I2C0_RegWrite(MPL3115A2_ADD, CTRL_REG1, 0x39, IIC0_STOP);

  /********************************************************************/
  // Main program loop
  /********************************************************************/
  for (;;)
  {
    if (shouldUpdate)
    {
      shouldUpdate = 0;
      SWL_TOG(SWL_YELLOW);

      // Read Temperature Bytes (Moved to the top)
      testResult = I2C0_RegRead(&temperatureMSB, MPL3115A2_ADD, 0x04);
      temperatureData = temperatureMSB;
      testResult = I2C0_RegRead(&temperatureLSB, MPL3115A2_ADD, 0x05);
      temperatureLSB >>= 4;

      // Extract integer and fractional parts
      temperatureFractionalBits = temperatureLSB & 0xF;
      temperatureCelsius = temperatureData + (temperatureFractionalBits * 0.0625);

      // Display on LCD (Moved to the top)
      lcd_StringXY(0, 0, "Current Conditions:");
      (void)sprintf(bufferOne, "Temp: +%2.1f%cC", temperatureCelsius, 0xDF);
      lcd_StringXY(0, 1, bufferOne);

      testResult = I2C0_RegRead(&readStatus, MPL3115A2_ADD, DR_STATUS);

      if (readStatus & DR_STATUS_PTDR_MASK)
      {
        SWL_TOG(SWL_GREEN);

        // Reset variables
        pressureMSB = pressureCSB = pressureLSB = 0;
        pressureData = fractionalBits = 0;
        pressurePascals = 0.0;

        // Read Pressure Bytes (Moved to the bottom)
        testResult = I2C0_RegRead(&pressureMSB, MPL3115A2_ADD, 0x01);
        pressureData = pressureMSB;
        pressureData <<= 8;
        testResult = I2C0_RegRead(&pressureCSB, MPL3115A2_ADD, 0x02);
        pressureData |= pressureCSB;
        pressureData <<= 2;
        testResult = I2C0_RegRead(&pressureLSB, MPL3115A2_ADD, 0x03);
        pressureLSB >>= 6;

        // Extract fractional bits
        fractionalBits = pressureLSB & 0x3;

        // Combine pressure data with fractional part
        pressurePascals = pressureData + (fractionalBits * 0.25);

        sci0_txStr("READ successful!\r\n");
      }
      else
      {
        SWL_OFF(SWL_GREEN);
        SWL_ON(SWL_RED);
        sci0_txStr("NO READ\r\n");
      }

      // Display on LCD (Moved to the bottom)
      (void)sprintf(bufferTwo, "Press: %3.1f kPa", (pressurePascals + 7783) / 1000.0);
      lcd_StringXY(0, 2, bufferTwo);
    }
  }
}

/********************************************************************/
// Functions
/********************************************************************/
void rtiCallback(void)
{
  if (++milliSecondsCounter > 249)
  {
    milliSecondsCounter = 0;
    shouldUpdate = 1;
  }
}

/********************************************************************/
// Interrupt Service Routines
/********************************************************************/