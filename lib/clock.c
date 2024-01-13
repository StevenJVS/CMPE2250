#include "clock.h"
#include "derivative.h"

// PLLCLK = 2 x OSCCLK x ([SYNR + 1] / [REFDV + 1])
// BUS  CLK =  PLLCLK / 2
// BUS CLK = OSCCLK / 2
unsigned long Clockspeed = DEF_BUS_CLOCK;

void Clock_Set8MHZ(void) // Set clock to 8MHZ using PLL
{
  CLKSEL &= ~CLKSEL_PLLSEL_MASK;

  SYNR = 1;
  REFDV = 3;

  PLLCTL = PLLCTL_PLLON_MASK | PLLCTL_AUTO_MASK;

  while (!(CRGFLG & CRGFLG_LOCK_MASK))
    ;

  CLKSEL |= CLKSEL_PLLSEL_MASK;

  ECLKCTL &= ~ECLKCTL_NECLK_MASK;
  Clockspeed = 8000000;
}
void Clock_Set24MHZ(void) // Set clock to 24MHZ using PLL
{
  CLKSEL &= ~CLKSEL_PLLSEL_MASK; // disable pll as system clock

  // We set SYNR = 5, REFDV = 3
  // SYNR = SYNR_SYN2_MASK | SYNR_SYN0_MASK; //0x5, set multiplier (SYNR +1) to: 5+1 = 6
  SYNR = 5;
  // REFDV = REFDV_REFDV1_MASK | REFDV_REFDV0_MASK; //0x3, set divider (REFDV +1) to: 3+1 = 4
  REFDV = 3;

  // Turn PLL ON and select AUTO
  PLLCTL = PLLCTL_PLLON_MASK | PLLCTL_AUTO_MASK; // PLL ON and AUTO

  // Wait for PLL to lock into the frequency
  while (!(CRGFLG & CRGFLG_LOCK_MASK))
    ; // Wait for PLL to lock

  // enable pll as system clock
  CLKSEL |= CLKSEL_PLLSEL_MASK;

  // Enable clock output
  // ECLKCTL &= 0b01111111;
  ECLKCTL &= ~ECLKCTL_NECLK_MASK;
  Clockspeed = 24000000;
}
void Clock_Set20MHZ(void) // Set clock to 20MHZ using PLL
{
  CLKSEL &= ~CLKSEL_PLLSEL_MASK;

  SYNR = 4;
  REFDV = 3;

  PLLCTL = PLLCTL_PLLON_MASK | PLLCTL_AUTO_MASK;

  while (!(CRGFLG & CRGFLG_LOCK_MASK))
    ;

  CLKSEL |= CLKSEL_PLLSEL_MASK;

  ECLKCTL &= ~ECLKCTL_NECLK_MASK;
  Clockspeed = 20000000;
}
void Clock_EnableOutput(unsigned char div) // Enable output specifying divider for it
{
  ECLKCTL &= ~0x3;
  switch (div)
  {
  case 4:
    ECLKCTL |= ECLKCTL_EDIV1_MASK | ECLKCTL_EDIV0_MASK;
    break;

  case 3:
    ECLKCTL |= ECLKCTL_EDIV1_MASK;
    break;

  case 2:
    ECLKCTL |= ECLKCTL_EDIV0_MASK;
    break;

  default:
    break;
  }
}
unsigned long Clock_GetBusSpeed(void) // Get Current Clock Speed
{
  return Clockspeed;
}
