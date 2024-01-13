#include "derivative.h"
#include "rti.h"

void Dummy(void);
void (*pRTI)(void) = &Dummy;
volatile unsigned int counter = 0;

void RTI_Init(void)
{
    RTICTL = 0b10010111;
    CRGINT |= CRGINT_RTIE_MASK; // 0b10000000, Enable real time Interrupt
}

void RTI_InitCallback(void (*function)(void))
{
    RTI_Init();
    pRTI = function;
}
void RTI_Delay_ms(unsigned int timeout)
{
    counter = timeout;
    while (counter);
}
void Dummy(void)
{
    ;
}

interrupt VectorNumber_Vrti void Vrti_ISR(void)
{                            // This ISR happens every 1[ms]
  CRGFLG = CRGFLG_RTIF_MASK; // clear flag;
  // Perform some action here
  pRTI();
  --counter;
}
