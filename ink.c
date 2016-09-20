/*****************************************************
This program was produced by the
CodeWizardAVR V2.03.9 Standard
Automatic Program Generator
© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 06.03.2010
Author  : Admin
Company : Microsoft
Comments: 


Chip type               : ATtiny2313
AVR Core Clock frequency: 8,000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 32
*****************************************************/

#include <tiny2313.h>
#include <delay.h>

// 1 Wire Bus functions
#asm
   .equ __w1_port=0x12 ;PORTD
   .equ __w1_bit=0
#endasm
#include <1wire.h>
#include <ds18b20.h>        //бібліотека для роботи з ds18b20

#define OT_TEMP 0
#define OT_TEMP_SET 1
#define OT_TIME 2
#define OT_LAST_NOT_USE 3


//#define MOTOR_TIME_OUT 10*10   //час в тіках. один тік = 100 мс
//#define MOTOR_TIME_OUT 10*10   //10 сек
//#define MOTOR_TIME_OUT 108000   //3 год   
#define MOTOR_TIME_OUT 180   //3 год в хвилинах
//#define MOTOR_TIME_OUT 1

#define TEMP_RANGE 002 //діапазон допустимої відхиленої температури для підігріва 
#define TEMP_RANGE_SEC 004 //діапазон допустимої відхиленої температури для основного 

#define MIN_TEMP_SET 300 //30.0 градусів
#define MAX_TEMP_SET 400 //40.0 градусів

int temp = 0, iStartMotor = 0, iMotorActions = 0;
char i = 0;
short temp2 = 0;
int milisec100 = 0;//одиниця часу в 100 мілісекунд
int minutes = MOTOR_TIME_OUT;
short OutDigit = 0;


unsigned char g_eType;


eeprom unsigned short g_iTempSet;
eeprom unsigned int g_iTimepMotor; 

int my_ds18b20_temperature()
{
    unsigned char i=0;                      
    unsigned char *p;
    
    w1_write(0xbe);
    p=(char *) &__ds18b20_scratch_pad;
    do
        *(p++)=w1_read();
    while (++i<9);       
    
    if(0 != w1_dow_crc8(&__ds18b20_scratch_pad,9))
    {
        w1_init();
        w1_write(0xcc);
        
        
        w1_write(0x44);
        delay_ms(1);

        w1_init();
        w1_write(0xcc);

        w1_write(0xbe);
        i=0;
        p=(char *) &__ds18b20_scratch_pad;
        do
            *(p++)=w1_read();
        while (++i<9);
        
        
        if(0 != !w1_dow_crc8(&__ds18b20_scratch_pad,9))
        {
            w1_init();
            return ((*((int *) &__ds18b20_scratch_pad.temp_lsb) & ((int)0xFFFF))*0.0625)*10;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

//цифри
const char numbers[] = 
{
    // 0 - для включення
    0b00101000,  //0
    0b11101110,  //1
    0b00110010,  //2
    0b10100010,  //3
    0b11100100,  //4
    0b10100001,  //5
    0b00100001,  //6
    0b11101010,  //7
    0b00100000,  //8
    0b10100000  //9
};

void TurnOnDisplay(char iNum, char iDig, char dot)
{
    char d = numbers[iDig]; 
              
    PORTB = 0xFF; //отключаем все сегменты
    
    PORTD.4 = 3 == iNum;
    PORTD.5 = 2 == iNum;
    PORTD.6 = 1 == iNum;
             
    PORTB = d;

    if(dot)
    {         
        PORTB.5 = 0; //крапка
    }
                    
    delay_us(150);
}

void DisplayTemp(int tempr)
{
    //остання цифра
    char a1 = tempr%10;
//    char s = sizeof(tempr); 
    TurnOnDisplay(1, a1, 0);
                     
    //середня
    tempr /= 10;
    a1 = tempr%10;
    TurnOnDisplay(2, a1, 1);

    //перша
    tempr /= 10;
    a1 = tempr%10;
    TurnOnDisplay(3, a1, 0);

    PORTD.4 = 0;
    PORTD.5 = 0;
    PORTD.6 = 0;    
}

void StartMotorActions()
{
    PORTD.3 = 1;//включаємо мотор
}

void StopMotorActions()
{
    PORTD.3 = 0;//виключаємо мотор
}

void StartLampActions()
{
    PORTD.2 = 1;//включаємо лампи
}

void StopLampActions()
{
    PORTD.2 = 0;//виключаємо лампи
}

void StartSubLampActions()
{
    PORTD.1 = 1;//включаємо підтримуючі лампи 
}

void StopSubLampActions()
{
    PORTD.1 = 0;//виключаємо підтримуючі лампи 
}


void main(void)
{
// Declare your local variables here
minutes = MOTOR_TIME_OUT;
g_eType = OT_TEMP;

if(g_iTempSet ==0xFFFF)
{
    g_iTempSet = MIN_TEMP_SET;     
} 

// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port A initialization
// Func2=In Func1=In Func0=In 
// State2=T State1=P State0=P 
PORTA=0x03;
DDRA=0x00;

// Port B initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=0 State0=0 
PORTB=0x00;
DDRB=0xFF;

// Port D initialization
// Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State6=1 State5=1 State4=1 State3=0 State2=0 State1=0 State0=0 
PORTD=0x00;
DDRD=0x7F;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 31,250 kHz
// Mode: CTC top=OCR0A
// OC0A output: Disconnected
// OC0B output: Disconnected
TCCR0A=0x02;
TCCR0B=0x01;
TCNT0=0x00;
OCR0A=0xFF;
OCR0B=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 7,813 kHz
// Mode: Normal top=FFFFh
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x05;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// Interrupt on any change on pins PCINT0-7: Off
GIMSK=0x00;
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x40;

// Universal Serial Interface initialization
// Mode: Disabled
// Clock source: Register & Counter=no clk.
// USI Counter Overflow Interrupt: Off
USICR=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;

if(g_iTimepMotor > 0)
{
    StartMotorActions();
    iMotorActions = g_iTimepMotor; 
}
    
TCNT1 = 0;
while (1)
      {                     
      
        temp = my_ds18b20_temperature();        

            

        if(temp <= g_iTempSet - TEMP_RANGE_SEC)
        {
            StartLampActions();
        }
        else
        {
            if(temp >= g_iTempSet)
            {
                StopLampActions();            
            }        
        }

        
        if(temp <= g_iTempSet - TEMP_RANGE)
        {
            StartSubLampActions();
        }
        else
        {

            if(temp >= g_iTempSet + TEMP_RANGE)
            {
                StopSubLampActions();
            }        
        }
        
        switch(g_eType)
        {
            case OT_TEMP:
                OutDigit = temp;
            break;    
            
            case OT_TEMP_SET:   
                OutDigit = temp2;    
            break;
            
            case OT_TIME:
                OutDigit = minutes;
            break;
        }                       
        
        i = 0;  
        do
        {
            DisplayTemp(OutDigit);  
            
            if(TCNT1 > 790)
            {  
                i++;
                milisec100++;  
                if(iMotorActions > 0)
                {
                    iMotorActions--;
                    if(0 == iMotorActions)
                    {
                        StopMotorActions();  
                        milisec100 = 0;
                    }
                }   
                
                if(milisec100 > 600)//пройшов час в одну хв
                {                   
                    minutes--;
                    milisec100 = 0;
                }
                              
                if(minutes <= 0)
                {
                    StartMotorActions();
                    iMotorActions = g_iTimepMotor; 
                    milisec100 = 0; 
                    minutes = MOTOR_TIME_OUT;
                }
                               
                if(g_eType == OT_TIME)
                {
                    if(0 == PINA.0)//рахуємо кількість тактів для повороту мотора
                    {
                        iStartMotor++;      
                        StartMotorActions();
                    }              
                    else
                    {
                        if(iStartMotor > 0)
                        {
                            g_iTimepMotor = iStartMotor;
                            iStartMotor = 0;           
                            minutes = MOTOR_TIME_OUT;
                            StopMotorActions(); 
                        }
                    }
                }
                else if(g_eType == OT_TEMP_SET)
                {         
                    if(0 == PINA.0)//налаштовуємо температуру
                    {
                        temp2++;//тимчасова змінна, щоб не використовувати орігінал, для збереження ресурсі памяті          
                        if(temp2 > MAX_TEMP_SET)
                        {
                            temp2 = MIN_TEMP_SET;    
                        }
                        TCNT1 = 0;
                        while(TCNT1 < 2000)//майже 0,25 сек
                        {
                            DisplayTemp(temp2);
                            OutDigit = temp2;
                        }
                    }              
                }
                
                TCNT1 = 0;
            }
        }
        while(i < 10);

        if(0 == PINA.1)
        {                   
            g_eType++;

            if(temp2 > 0 && g_eType - 1 == OT_TEMP_SET)//якщо виходимо з режиму редагування температури, то встановлюємо її і записуємо в памяті 
            {
                g_iTempSet = temp2;
            }                           

            if(g_eType == OT_TEMP_SET)//якщо якщо переходимо в режим редагування темератури, то заносимо її в тимчасову змінну 
            {
                temp2 = g_iTempSet;
            } 

            if(g_eType == OT_LAST_NOT_USE)
            {
                g_eType = 0;
            }        
            
            TCNT1 = 0;
            while(TCNT1 < 20000)//майже 2.5 сек
            {
                DisplayTemp(g_eType);
            }
        } 
      };
}
