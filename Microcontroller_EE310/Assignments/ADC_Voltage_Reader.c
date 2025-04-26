// === CONFIG BITS ===
#pragma config FEXTOSC = LP     
#pragma config RSTOSC = EXTOSC  
#pragma config CLKOUTEN = OFF   
#pragma config PR1WAY = ON      
#pragma config CSWEN = ON       
#pragma config FCMEN = ON       
#pragma config MCLRE = EXTMCLR  
#pragma config PWRTS = PWRT_OFF 
#pragma config MVECEN = ON      
#pragma config IVT1WAY = ON     
#pragma config LPBOREN = OFF    
#pragma config BOREN = SBORDIS  
#pragma config BORV = VBOR_2P45 
#pragma config ZCD = OFF        
#pragma config PPS1WAY = ON     
#pragma config STVREN = ON      
#pragma config DEBUG = OFF      
#pragma config XINST = OFF      
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF       
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC      
#pragma config BBSIZE = BBSIZE_512
#pragma config BBEN = OFF       
#pragma config SAFEN = OFF      
#pragma config WRTAPP = OFF     
#pragma config WRTB = OFF       
#pragma config WRTC = OFF       
#pragma config WRTD = OFF       
#pragma config WRTSAF = OFF     
#pragma config LVP = ON         
#pragma config CP = OFF         

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#define _XTAL_FREQ 4000000
#define Vref 5.0

// LCD Connections
#define RS LATD0
#define EN LATD1
#define ldata LATB
#define LCD_Port TRISB
#define LCD_Control TRISD

// Function Prototypes
void LCD_Init();
void LCD_Command(char);
void LCD_Char(char);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void LCD_Clear();
void MSdelay(unsigned int);
void ADC_Init(void);

// Global Variables
int digital = 0;
float voltage = 0.0;
char data[10];

void main(void)
{
    LCD_Init();
    LCD_Clear();
    ADC_Init();

    // Show your name first
    LCD_String_xy(1, 0, "EDUARDO");
    LCD_String_xy(2, 0, "WILLIAMS");
    __delay_ms(3000); // Show for 3 seconds

    LCD_Clear(); // Now clear and start voltage display

    while (1)
    {
        ADCON0bits.GO = 1;              // Start ADC
        while (ADCON0bits.GO);           // Wait until done

        digital = ((ADRESH << 8) | ADRESL);   // 10-bit result
        voltage = (digital * Vref) / 4096.0;  // Calculate voltage

        sprintf(data, "V=%.2fV", voltage);    // Format string

        LCD_String_xy(1, 0, "Voltage Reading:");
        LCD_String_xy(2, 0, data);

        __delay_ms(500); // Update every 0.5s
    }
}

// === ADC Initialization ===
void ADC_Init(void)
{
    ADCON0bits.ADON = 0;        // Turn off ADC
    ADCON0bits.FM = 1;          // Right justified
    ADCON0bits.CS = 1;          // Use internal oscillator
    ADCON0bits.ON = 1;          // Turn on ADC

    TRISAbits.TRISA0 = 1;       // RA0 as input
    ANSELAbits.ANSELA0 = 1;     // RA0 as analog

    ADPCH = 0b000000;           // AN0 selected

    ADCLK = 0x00;               // Clock divider

    ADRESH = 0;                 // Clear result
    ADRESL = 0;

    ADPREL = 0x00;              // Precharge
    ADPREH = 0x00;

    ADACQL = 0x00;              // Acquisition time
    ADACQH = 0x00;
}

// === LCD Functions ===
void LCD_Init()
{
    MSdelay(20); // Give LCD controller time to power up properly

    LCD_Port = 0x00;      // Clear PORTB (data bus)
    LCD_Control = 0x00;   // Clear control lines

    TRISB = 0x00;         // Set PORTB (D0-D7) as output
    TRISDbits.TRISD0 = 0; // Set RD0 (RS) as output
    TRISDbits.TRISD1 = 0; // Set RD1 (EN) as output

    LCD_Command(0x38);    // 8-bit, 2 line, 5x8 font
    MSdelay(5);

    LCD_Command(0x0C);    // Display ON, Cursor OFF
    MSdelay(5);

    LCD_Command(0x06);    // Entry mode: increment cursor
    MSdelay(5);

    LCD_Command(0x01);    // Clear display
    MSdelay(5);
}

void LCD_Command(char cmd)
{
    ldata = cmd;
    RS = 0;
    EN = 1; NOP(); EN = 0;
    MSdelay(3);
}

void LCD_Char(char dat)
{
    ldata = dat;
    RS = 1;
    EN = 1; NOP(); EN = 0;
    MSdelay(1);
}

void LCD_String(const char *msg)
{
    while (*msg) LCD_Char(*msg++);
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char loc = (row == 1) ? (0x80 | (pos & 0x0F)) : (0xC0 | (pos & 0x0F));
    LCD_Command(loc);
    LCD_String(msg);
}

void LCD_Clear()
{
    LCD_Command(0x01);
}

// === Blocking Delay ===
void MSdelay(unsigned int val)
{
    for (unsigned int i = 0; i < val; i++)
        for (unsigned int j = 0; j < 165; j++);
}
