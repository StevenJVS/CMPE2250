//IIC Library Components
//Processor:  MC9S12XDP512
//Crystal:  16 MHz
//by Carlos Estay
//December 2019
//Adapted by Carlos Estay, 2020
//Edit 4/4/2023 update naming


#define WAIT_COUNT_MAX 100

#define IIC0_STOP 1
#define IIC0_NOSTOP 0

#define IIC0_WRITE 0  
#define IIC0_READ 1

#define IIC0_ACK 1
#define IIC0_NACK 0

#define IIC0_WAIT 1
#define IIC0_NOWAIT 0

#define DAC_A 0x20
#define DAC_B 0x21
#define DAC_ALL 0x2F


//Thermometer/Barometer Register Addresses and bits
#define MPL3115A2_ADD 0x60
#define DR_STATUS 0x00
#define DR_STATUS_PTDR_MASK 0x08
#define CTRL_REG1 0x26
#define PT_DATA_CFG 0x13



//RTC Register addresses and bits
#define RTC_ADD 0x68

//RTC Clock Registers
#define RTC_HSECONDS 0x00
#define RTC_SECONDS 0x01
#define RTC_SECONDS_ST 0x80
#define RTC_MINUTES 0x02
#define RTC_CHOUR 0x03
#define RTC_DAY 0x04
#define RTC_DATE 0x05
#define RTC_MONTH 0x06
#define RTC_YEAR 0x07

//RTC Alarm Registers
#define RTC_AL_MONTH 0x0A
#define RTC_AL_DATE 0x0B
#define RTC_AL_HOUR 0x0C
#define RTC_AL_HOUR_HT 0x40
#define RTC_AL_MIN 0x0D
#define RTC_AL_SEC 0x0E

typedef enum __RTC_Day
{
    Sun = 1,
    Mon = 2,
    Tue = 3,
    Wed = 4,
    Th = 5,
    Fri = 6,
    Sat = 7,
}RTC_Day;

typedef enum __RTC_Month
{
    Jan = 1,
    Feb = 2,
    Mar = 3,
    Apr = 4,
    May = 5,
    Jun = 6,
    Jul = 7,
    Aug = 8,
    Sep = 9,
    Oct = 10,
    Nov = 11,
    Dec = 12,
}RTC_Month;

typedef struct __RTC_Read
{
    unsigned int Year;
    enum RTC_Month Month;
    unsigned int MonthDay;
    enum RTC_Day Day;
    unsigned char Hours;
    unsigned char Minutes;
    unsigned char Seconds;
    unsigned char HuSeconds;
}RTC_Read;


//This assumes BUS Speed is 20MHZ
enum I2C_BusRate
{
    I2CBus100 = 0x21,
    I2CBus400 = 0xB,
};


void I2C0_Init(void);   //works with both legacy and genericized projects
void I2C0_InitBus(enum I2C_BusRate);   //BUS Speed as parameter (100K or 400K)
int I2C0_WaitForBus(void);
int I2C0_Addr(char Address, char IsRead, char WaitForBus);
int I2C0_WriteByte(char Val, char IssueStop);
int I2C0_ReadByte(char *buff, char IssueAck, char IssueStop);
void I2C0_RepeatStart(void);

int I2C0_WriteDAC(unsigned int value, char dac);
int I2C0_RegWrite(char addr, char reg, char data, char issueStop);
int I2C0_RegRead(char *buff, char addr, char reg);
