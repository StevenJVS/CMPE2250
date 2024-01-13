#include "derivative.h"
#include "lcd.h"
#include "clock.h"
#include "pit.h"

void lcd_Init(void)
{
    // PH7-PH0 -> D7 - D0 (data Lines)
    DDRH = 0xFF;
    // PK2-PK0 -> Control Lines (output)
    DDRK |= 0b00000111;
    // 1st delay - Delay 40+ [ms]
    PIT_Sleep(PIT_CH2, 45);
    // Present Data on PTH
    PTH = 0b00111000; // Function Set
    /*             ||||||_____(don't care)
                   |||||______(don't care)
                   ||||_______font:  5x8 matrix (LOW)
                   |||________lines: 2 (HIGH)
                   ||_________data:  8-bit (HIGH)
                   |__________function set of commands
    */
    // Write Operation - Instruction Register
    // PK1->R/W->0  PK2->RS->0
    PORTK_PK1 = 0;
    PORTK_PK2 = 0;

    // Latch Instruction
    PORTK_PK0 = 1;
    // PIT_Delay_us(PIT_CH3, 5);
    lcd_MicroDelay;
    PORTK_PK0 = 0;

    // 2nd Delay, 4.1ms+
    PIT_Sleep(PIT_CH2, 5);

    // Latch same Instruction again
    PORTK_PK0 = 1;
    // PIT_Delay_us(PIT_CH3, 5);
    lcd_MicroDelay;
    PORTK_PK0 = 0;

    // third Delay  100uS+
    PIT_Set1msDelay(PIT_CH3);

    // Latch same Instruction again
    PORTK_PK0 = 1;
    // PIT_Delay_us(PIT_CH3, 5);
    lcd_MicroDelay;
    PORTK_PK0 = 0;

    // Busy flag is active now**********************
    // lcd_Ins(0b00111000); //Function set
    //     //We can check for busy flag now**********************
    //     //5x8 dots, 2 lines
    lcd_Ins(0b00111000);
    lcd_Ins(0b00001111); // display controls
                /*||||_____Blink:   HIGH for on
                  |||______Cursor:  HIGH for on
                  ||_______Display: HIGH for on
                  |________Display Control commands
                                                  */
    lcd_Ins(0b00000001); // clear display, home position
    lcd_Ins(0b00000110); // mode controls
    /*             |||_____Shift:   LOW for no display shift
                   ||______Inc/Dec: HIGH for increment (to the left)
                   |_______Entry Mode commands
        */
}
void lcd_Ins(unsigned char command) // LCD_Inst
{
    while (lcd_Busy())
        ; // wait for the Busy Flag to be CLEARED
    PTH = command;
    PORTK_PK1 = 0; // Write Operation
    PORTK_PK2 = 0; // Instruction Register
    // Latch Instruction
    PORTK_PK0 = 1;
    lcd_MicroDelay;
    PORTK_PK0 = 0;
}
char lcd_Busy()
{
    char busyState;
    DDRH = 0x00;   // data bus as inputs for read
    PORTK_PK1 = 1; // Read Operation
    PORTK_PK2 = 0; // Instruction Register
    // Latch Instruction
    PORTK_PK0 = 1;
    lcd_MicroDelay;
    PORTK_PK0 = 0;

    // Busy Flag is the MSB and AC B0 - B6 of the Status Register
    busyState = PTH & PTH_PTH7_MASK;
    DDRH = 0xFF; // data bus returned to outputs for next write
    return busyState;
}
char lcd_GetAddr(void)
{
    char addr;
    while (lcd_Busy())
        ;

    DDRH = 0x00;   // data bus as inputs for read
    PORTK_PK1 = 1; // Read Operation
    PORTK_PK2 = 0; // Instruction Register

    // Latch Instruction
    PORTK_PK0 = 1;
    lcd_MicroDelay;
    PORTK_PK0 = 0;

    // Busy Flag is the MSB and AC B0 - B6 of the Status Register
    addr = PTH & 0x7F;
    DDRH = 0xFF; // data bus returned to outputs for next write
    return addr;
}
void lcd_Data(unsigned char val)
{
    while (lcd_Busy())
        ;
    PTH = val;                // Present Data
    PORTK &= ~PORTK_PK1_MASK; // Write
    PORTK |= PORTK_PK2_MASK;  // Data Register

    // Latch data
    PORTK |= PORTK_PK0_MASK;
    lcd_MicroDelay;
    PORTK &= ~PORTK_PK0_MASK;
}
void lcd_Addr(unsigned char addr)
{
    lcd_Ins(0x80 | addr);
}
void lcd_AddrXY(unsigned char ix, unsigned char iy)
{
    if (ix > 19 || ix < 0 || iy > 3 || iy < 0)
    {
        // do nothing
    }
    else
    {
        switch (iy)
        {
        case 0:
            lcd_Addr(LCD_ROW0 + ix);
            break;
        case 1:
            lcd_Addr(LCD_ROW1 + ix);
            break;
        case 2:
            lcd_Addr(LCD_ROW2 + ix);
            break;
        case 3:
            lcd_Addr(LCD_ROW3 + ix);
            break;

        default:
            break;
        }
    }
}
void lcd_String(char const *straddr)
{
    while (*straddr)
    {
        lcd_Data(*straddr++);
    }
}
void lcdSmartString(char const *straddr, unsigned int delay);
void lcd_StringXY(unsigned char ix, unsigned char iy, char const *straddr)
{
    lcd_AddrXY(ix, iy);
    lcd_String(straddr);
}
void lcd_DispControl(unsigned char curon, unsigned char blinkon)
{
    lcd_Ins(0b00001100 | curon<<1 | blinkon);
}
void lcd_Clear(void)
{
    lcd_Ins(0b00000001);
}
void lcd_Home(void)
{
    lcd_Ins(0b00000010);
}
void lcd_ShiftL(char);
void lcd_ShiftR(char);
void lcd_CGAddr(unsigned char addr)
{
    lcd_Ins(0b01000000 | addr);
}
void lcd_CGChar(unsigned char cgAddr, unsigned const char *cgData, int size)
{
    unsigned char line;    
    lcd_CGAddr(cgAddr);
    for (line = 0; line< size; line++)
    {
       lcd_Data(cgData[line]);
    }
    lcd_Ins(0b10000000);    //back to DDRAM, home location 0
}