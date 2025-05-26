/*
 * File:   main.c
 * Author: Turcotronics
 * EPROM Reader
 *
 * Created on 21 maggio 2025, 21.26
 */

// PIC16F877A Configuration Bit Settings
// 'C' source line config statements
// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = ON        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#define _XTAL_FREQ 16000000

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

#define ER_READSPI  's' //Read SPI EEPROM
#define ER_READI2C  'i' //Read I2C EEPROM
#define ER_READ     'r' //Read Parallel EPROM
#define ER_MINADDR  'm' //(addr) Min address
#define ER_MAXADDR  'M' //(addr) Max address
#define ER_LOCATION 'l' //(addr) Read location

char RxBuff[8];
char TxBuff[8]="EReader";
long MinAddr=0;
long MaxAddr=32767;//Default EPROM 27256
long i=0;

void __set_baud_rate(long baud_rate);
void getString(char *input, unsigned int length);
void putch(char data);
char getch();
void flushRx();
void cleanRx();

//________________________________________________
void main(void) {
    //I/O registers init
    PORTA=0;
    TRISA=0x00;
    PORTB=0;
    TRISB=0x00;
    PORTC=0;
    TRISC=0x90;
    PORTD=0;
    TRISD=0xFF;
    PORTE=0;
    TRISE=0x00;
    
    //Serial init
    __set_baud_rate(115200);
    TXSTA = 0b00100100; //Tx enable, async, high speed mode
    RCSTA = 0b10010000; //Rx enable, serial port enable
    
//    for(int i=0;i<8;i++)
//        putch(TxBuff[i]);
//    for(int i=0;i<8;i++)
//        putch(TxBuff[i]);
    
    //WDT init
    PS2=0;PS1=1;PS0=1;//8x18ms sleep wakeup after 144ms (wdt is 18ms))
    PSA=1;//Prescaler to the WDT
    
    while(1)
    {
        CLRWDT();
        getString(RxBuff, 8);
        cleanRx();
        if(RxBuff[0]==ER_READ)
        {
			flushRx();
			RE2=0;// EPROM /CE
			RC5=0;// EPROM /OE
            for(i=MinAddr; i<=MaxAddr; i++)
            {
				 //RE1=(__bit)((i>>15)&0x0001);
				 RE0=(__bit)((i>>14)&0x0001);
				 RC2=(__bit)((i>>13)&0x0001);
				 RC1=(__bit)((i>>12)&0x0001);
				 RC0=(__bit)((i>>11)&0x0001);
				 RB5=(__bit)((i>>10)&0x0001);
				 RB4=(__bit)((i>>9)&0x0001);
				 RB2=(__bit)((i>>8)&0x0001);
				 RB1=(__bit)((i>>7)&0x0001);
				 RB0=(__bit)((i>>6)&0x0001);
				 PORTA=i&0x3F;
				 __delay_us(2);
				 putch(PORTD);
				 CLRWDT();
            }
        }
        else if(RxBuff[0]==ER_LOCATION)
        {
			flushRx();
			RE2=0;// EPROM /CE
			RC5=0;// EPROM /OE
			i=atol(RxBuff+1);
            //RE1=(__bit)((i>>15)&0x0001);
            RE0=(__bit)((i>>14)&0x0001);
            RC2=(__bit)((i>>13)&0x0001);
            RC1=(__bit)((i>>12)&0x0001);
            RC0=(__bit)((i>>11)&0x0001);
            RB5=(__bit)((i>>10)&0x0001);
            RB4=(__bit)((i>>9)&0x0001);
            RB2=(__bit)((i>>8)&0x0001);
            RB1=(__bit)((i>>7)&0x0001);
            RB0=(__bit)((i>>6)&0x0001);
            PORTA=i&0x3F;
            __delay_us(2);
            putch(PORTD);
        }
        else if(RxBuff[0]==ER_READI2C)
        {
			flushRx();
			PORTA=0;
            for(i=MinAddr; i<=MaxAddr; i++)
            {
             //TODO read I2C on TxBuff[0]
             putch(TxBuff[0]);
             __delay_us(2);
             CLRWDT();
            }
        }
        else if(RxBuff[0]==ER_READSPI)
        {
			flushRx();
			PORTA=0;
            for(i=MinAddr; i<=MaxAddr; i++)
            {
             //TODO read SPI on TxBuff[0]
             putch(TxBuff[0]);
             __delay_us(2);
             CLRWDT();
            }
        }
        else if (RxBuff[0]==ER_MINADDR)
        {
            MinAddr=atol(RxBuff+1);
        }
        else if (RxBuff[0]==ER_MAXADDR)
        {
            MaxAddr=atol(RxBuff+1);
        }
    }
    
    return;
}

//________________________________________________
void __set_baud_rate(long baud_rate)
{
    SPBRG = (unsigned char)((_XTAL_FREQ / 16) / baud_rate);
}

//________________________________________________
void putch(char data)
{                            //required for printf
    while(!TRMT)
        CLRWDT();
    TXREG = data;
}

//________________________________________________
char getch()
{
    while(!RCIF)
        CLRWDT();
    return RCREG;
}

//________________________________________________
void flushRx()
{
    char Temp=0;
    while(RCIF)
    {
        Temp=RCREG;
        CLRWDT();
    }
}

//________________________________________________
void cleanRx()
{
    for(int i=0;i<8;i++)
    {
      if(RxBuff[i]<20)
          RxBuff[i]=0;
    }
}

//________________________________________________
void getString(char *input, unsigned int length)
{
    for(int i=0;i<length;i++)
    {                       
        input[i] = (char)getch();
        if(input[i]==13)
            break;
    }
}
