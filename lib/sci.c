#include "derivative.h"
#include "sci.h"
#include "clock.h"
#include "math.h"

//when called, return to nothing
//cast method as void
unsigned long sci0_Init(unsigned long ulBaudRate, int iRDRF_Interrupt)
{
  float SBR_PRE = (Clock_GetBusSpeed()/(16.0*ulBaudRate));
		
    //round setter baud rate to nearest int
    unsigned long SBR = ceil(SBR_PRE*2)/2 ;

    SCI0CR1 = 0;//do nothing in this register

		//enable transmitting and recieving bit
    SCI0CR2 |= SCI0CR2_TE_MASK | SCI0CR2_RE_MASK;

    //enalble interrupts
    if(iRDRF_Interrupt){
      SCI0CR2 |= SCI0CR2_RIE_MASK;
    }

    SCI0BD = SBR;//set baud rate

    return (Clock_GetBusSpeed()/(16*SCI0BD));//return actaul baud rate
}

// blocking byte read
// waits for a byte to arrive and returns it
unsigned char sci0_bread(void)
{
  //check for flag if there is data to be read
while(!(SCI0SR1 & SCI0SR1_RDRF_MASK));
return SCI0DRL; //return value to register
}

//non blocking read byte
// read a byte, non-blocking
// returns 1 if byte read, 0 if not
unsigned char sci0_rxByte(unsigned char * pData)
{
if(SCI0SR1 & SCI0SR1_RDRF_MASK)//check flag if there is data to be read
{
  *pData = SCI0DRL;//read data and store data to an address 
  return 1;//return true
}
else
return 0;//return false
}

// send a byte over SCI
void sci0_txByte (unsigned char data)
{
    while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
    SCI0DRL = data;
}

// send a null-terminated string over SCI
void sci0_txStr (char const * straddr)
{
    while(*straddr)
    {
      sci0_txByte(*straddr++);
    }
}

// receive a string from the SCI
// up to buffer size-1 (string always NULL terminated)
// number of characters is BufferSize minus one for null
// once user enters the max characters, null terminate and return
// if user enters 'enter ('\r')' before-hand, return with current entry (null terminated)
// echo valid characters (non-enter) back to the terminal
// return -1 on any error, otherwise string length
//int sci0_rxStr (char * const pTarget, int BufferSize);