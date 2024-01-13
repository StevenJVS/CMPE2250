#include "derivative.h"
#include "segs.h"
#include "clock.h"

// setup ports to speak with 7-segs
void Segs_Init(void)
{
    // PA0 and PA1 to outputs (DDR register)
    DDRA |= PORTA_PA1_MASK | PORTA_PA0_MASK;
    // Set Entire port B to output
    DDRB = 0xFF;
}
// show HEX decoding (changes all of display to HEX, w/wo dp)
/* Params: (address, data, dp) */
void Segs_Normal(unsigned char addr, unsigned char data, Segs_DPOption dp)
{
    PORTB = 0b01011000;
    /*          ||||||||_____Addr
                |||||________Bank A
                ||||_________Normal
                |||__________Decode
                ||___________Decode as HEX
                |____________No Data coming
    */
    PORTB |= (addr & 0x7); // make addres only 3-lsb

    PORTA |= PORTA_PA1_MASK;  // Set to Control Mode
    PORTA &= ~PORTA_PA0_MASK; // Latch

    // Add delay for faster than 8MHZ
    PORTA |= PORTA_PA0_MASK;

    PORTB = data;
    if (dp == Segs_DP_OFF)
    {
        PORTB |= 0x80;
    }

    PORTA &= ~PORTA_PA1_MASK; // Set to Data Mode
    PORTA &= ~PORTA_PA0_MASK; // Latch

    // Add delay for faster than 8MHZ
    PORTA |= PORTA_PA0_MASK;
}

// control segs manually
/* Params: (address, data) */
void Segs_Custom(unsigned char addr, unsigned char data)
{
    // CONTROL BYTE******************
    // Port B
      PORTB = 0b01111000;
    /*          ||||||||_____Addr
                |||||________Bank A
                ||||_________Normal
                |||__________No Decode
                ||___________Decode as HEX
                |____________No Data coming
    */
    PORTB |= (addr & 0x7); // make addres only 3-lsb
    // Port A
    PORTA |= PORTA_PA1_MASK; // Set to Control Mode
    // LATCH
    PORTA &= ~PORTA_PA0_MASK;
    // Add delay for faster than 8MHZ
    PORTA |= PORTA_PA0_MASK;

    // Send DATA BYTE
    // Port B
    // PORTB = 0x80; //Clear digit
    PORTB = data;

    // Port A
    PORTA &= ~PORTA_PA1_MASK; // Set to Data Mode

    // LATCH
    PORTA &= ~PORTA_PA0_MASK;
    // Add delay for faster than 8MHZ
    PORTA |= PORTA_PA0_MASK;
}

// clear the display
/* Params: (address) */
void Segs_ClearDigit(unsigned char addr)
{
    Segs_Custom(addr, 0x80);
}

// clear the entire display
void Segs_Clear(void)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        Segs_ClearDigit(i);
    }
}

// clear display  upper or lower line
/* Params: (line) */
void Segs_ClearLine(Segs_LineOption line)
{
    int i;
    int j;
    switch (line)
    {
    case Segs_LineTop:
        for (i = 0; i < 4; i++)
        {
            Segs_ClearDigit(i);
        }
        break;
    case Segs_LineBottom:
        for (j = 4; i < 8; i++)
        {
            Segs_ClearDigit(i);
        }
        break;

    default:
        break;
    }
}

// show a 16-bit value on the upper or lower display
/* Params: (value, line) */
void Segs_16H(unsigned int value, Segs_LineOption line)
{
    switch (line)
    {
    case Segs_LineTop:
        Segs_Normal(0, (unsigned char)(value >> 12), Segs_DP_OFF);
        Segs_Normal(1, (unsigned char)(value >> 8), Segs_DP_OFF);
        Segs_Normal(2, (unsigned char)(value >> 4), Segs_DP_OFF);
        Segs_Normal(3, (unsigned char)(value >> 0), Segs_DP_OFF);
        break;
    case Segs_LineBottom:
        Segs_Normal(4, (unsigned char)(value >> 12), Segs_DP_OFF);
        Segs_Normal(5, (unsigned char)(value >> 8), Segs_DP_OFF);
        Segs_Normal(6, (unsigned char)(value >> 4), Segs_DP_OFF);
        Segs_Normal(7, (unsigned char)(value >> 0), Segs_DP_OFF);
    default:
        break;
    }
}

// show a decimal value on the first or second line of the 7-segs
/* Params: (value, line) */
void Segs_16D(unsigned int value, Segs_LineOption line)
{
    int ones = value % 10;
    int tens = (value / 10) % 10;
    int hund = (value / 100) % 10;
    int thou = (value / 1000) % 10;

    switch (line)
    {
    case Segs_LineTop:
        Segs_Normal(0, thou, Segs_DP_OFF);
        Segs_Normal(1, hund, Segs_DP_OFF);
        Segs_Normal(2, tens, Segs_DP_OFF);
        Segs_Normal(3, ones, Segs_DP_OFF);
        break;
    case Segs_LineBottom:
        Segs_Normal(4, thou, Segs_DP_OFF);
        Segs_Normal(5, hund, Segs_DP_OFF);
        Segs_Normal(6, tens, Segs_DP_OFF);
        Segs_Normal(7, ones, Segs_DP_OFF);
    default:
        break;
    }
}
// show the 8-bit value starting on the digit as addr (0-6)
/* Params: (addr, value) */
void Segs_8H(unsigned char addr, unsigned char value)
{
    Segs_Normal(addr, (unsigned char)(value >> 4), Segs_DP_OFF);
    Segs_Normal(addr+1, (unsigned char)(value >> 0), Segs_DP_OFF);
}

// say Err on the appropriate line
/* Params: (line) */
void Segs_SayErr(Segs_LineOption line)
{
    switch (line)
    {
    case Segs_LineTop:
        Segs_Custom(0x0, 0b11001111);
        Segs_Custom(0x1, 0b10001100);
        Segs_Custom(0x2, 0b10001100);

        break;
    case Segs_LineBottom:
        Segs_Custom(0x4, 0b11001111);
        Segs_Custom(0x5, 0b10001100);
        Segs_Custom(0x6, 0b10001100);

    default:
        break;
    }
}
