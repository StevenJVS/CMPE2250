#include "derivative.h"
#include "pit.h"
#include "clock.h"

/// @brief Configures a channel (ch0 - ch3)
/// @param ch The channel in question (PIT_CH0 - PIT_CH3)
/// @param mt The micro-timer to be connected to (MT1 or MT0(default))
/// @param ie Enables or disables interrupt for the channel
void PIT_InitChannel(PIT_Channel ch, PIT_MicroTimer mt, PIT_Interrupt ie)
{
  if (mt == PIT_MT1)
  {
    PITMUX |= ch; // connect channel to micro timer 1
  }
  else
    PITMUX &= ~ch; // Connect channel to Micro Timer 0
  if (ie == PIT_IEN)
  {
    PITINTE |= ch; // enable interrupt for channel
  }
  else
    PITINTE &= ~ch; // else disable interrupt
  PITCE |= ch;      // enable desired channel
}

/// @brief Configures the channel to a 1[ms] event, fix connection to micro-timer1
/// @param ch The channel to be configured
void PIT_Set1msDelay(PIT_Channel ch)
{
  int counter;
  if (!(PITCE & ch))
  {
    PIT_InitChannel(ch, PIT_MT1, PIT_IDIS); // connect to timer 1
  }
  PITMTLD1 = 1 - 1;                             // load 0 counter to micro timer1
  counter = ((Clock_GetBusSpeed() / 1000) - 1); // get ticks for channel counter
  switch (ch)
  {
  case PIT_CH0: // load channel 0
    PITLD0 = counter;
    break;
  case PIT_CH1: // load channel 1
    PITLD1 = counter;
    break;
  case PIT_CH2: // load channel 2
    PITLD2 = counter;
    break;
  case PIT_CH3: // load channel 3
    PITLD3 = counter;
    break;
  default:
    break;
  }
}

/// @brief A blocking delay function in milliseconds
/// @param ch The channel to use with the delay
/// @param ms The number of milliseconds to delay
void PIT_Sleep(PIT_Channel ch, unsigned int ms)
{
  int i;
  PIT_Set1msDelay(ch); // set delay
  PITFLT |= ch;        // force load wanted channel
  PITTF = ch;          // Clear flag
  if (!(PITCFLMT & PITCFLMT_PITE_MASK))
  {
    PIT_Start();
  }
  for (i = 0; i < ms; i++)
  {
    while (!(PITTF & ch));
    PITTF = ch; // clear flag
  }
}

/// @brief This enables the PIT. It is recommened to be called last.
///        Nothing will work if this is not called

void PIT_Start()
{
  PITCFLMT |= PITCFLMT_PITE_MASK; // enable PIT Timer (Global)
}

/// @brief Optional. Reasonable for anything above 20us
/// @param ch The channel to use with the delay
/// @param us The number of microseconds to delay
void PIT_Delay_us(PIT_Channel ch, unsigned int usDelay)
{
    unsigned  int timeTicks = Clock_GetBusSpeed() / 1000000 * usDelay;
     //Check if channel is not already enabled
    if(!(PITCE & ch))
    {
        //If not enabled, connect to micro-timer1 with interrupt disabled
        PIT_InitChannel(ch, PIT_MT1, PIT_IDIS);
    }     
    switch (ch)
    {
    case PIT_CH0:
        PITLD0 = (unsigned int)timeTicks - 1;
        break;
    case PIT_CH1:
        PITLD1 = (unsigned int)timeTicks - 1;
        break;
    case PIT_CH2:
        PITLD2 = (unsigned int)timeTicks - 1;
        break;
    case PIT_CH3:
        PITLD3 = (unsigned int)timeTicks - 1;
        break;                        
    
    default:
        break; 
    }
    PITFLT |= ch;     //Force load counter
    PITTF = ch;       //clear flag, in case already active;
    PIT_Start();
    while(!(PITTF & ch));
}
