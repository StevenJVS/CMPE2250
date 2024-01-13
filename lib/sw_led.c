#include "sw_led.h"
#include "derivative.h"
#include "pit.h"

void SWL_Init()
{
  // 3 most sig. bits->outputs (1). 5 least bits are inputs (0)
  DDR1AD1 = 0b11100000;
  // Enable switches
  ATD1DIEN1 |= 0b00011111;
}
void SWL_ON(SWL_LEDColour led)
{
  PT1AD1 |= led;
}
void SWL_OFF(SWL_LEDColour led)
{
  PT1AD1 &= ~led;
}
void SWL_TOG(SWL_LEDColour led)
{
  PT1AD1 ^= led;
}
int SWL_Pushed(SWL_SwitchPos pos)
{
  return PT1AD1 & pos;
}
int SWL_Any()
{
  return PT1AD1 & SWL_ANY;
}
SwState Sw_ProcessD(SwState *state, SWL_SwitchPos pos)
{
  unsigned char cSample1 = 1;
  unsigned char cSample2 = 0;

  if (SWL_Pushed(pos))
  {
    while (cSample1 != cSample2)
    {
      cSample1 = PT1AD1 & 0b00011111; // sampling all switches
      PIT_Sleep(PIT_CH3, 10);
      cSample2 = PT1AD1 & 0b00011111;
    }
    if (*state == Idle)
    {
      *state = Pressed;
    }
    else
    {
      *state = Held;
    }
  }
  else
  {
    if (*state == Held)
    {
      *state = Released;
    }
    else
    {
      *state = Idle;
    }
  }
  return *state;
}
// change
SwState Sw_Process(SwState *state, SWL_SwitchPos pos)
{
  if (SWL_Pushed(pos))
  {
    if (*state == Idle)
    {
      *state = Pressed;
    }
    else
    {
      *state = Held;
    }
  }
  else
  {
    if (*state == Held)
    {
      *state = Released;
    }
    else
    {
      *state = Idle;
    }
  }
  return *state;
}
