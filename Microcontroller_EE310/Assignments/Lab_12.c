// === CONFIG BITS ===
#pragma config FEXTOSC = LP
#pragma config RSTOSC = EXTOSC
#pragma config CLKOUTEN = OFF, PR1WAY = ON, CSWEN = ON, FCMEN = ON
#pragma config MCLRE = EXTMCLR, PWRTS = PWRT_OFF, MVECEN = ON
#pragma config IVT1WAY = ON, LPBOREN = OFF, BOREN = SBORDIS
#pragma config BORV = VBOR_2P45, ZCD = OFF, PPS1WAY = ON
#pragma config STVREN = ON, DEBUG = OFF, XINST = OFF
#pragma config WDTCPS = WDTCPS_31, WDTE = OFF
#pragma config WDTCWS = WDTCWS_7, WDTCCS = SC
#pragma config BBSIZE = BBSIZE_512, BBEN = OFF, SAFEN = OFF, WRTAPP = OFF
#pragma config WRTB = OFF, WRTC = OFF, WRTD = OFF, WRTSAF = OFF, LVP = ON
#pragma config CP = OFF

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XTAL_FREQ 4000000
#define Vref 5

// === LCD Pin Macros ===
#define RS LATD0
#define EN LATD1
#define ldata LATB
#define LCD_Port TRISB
#define LCD_Control TRISD

// === Function Prototypes ===
void LCD_Init(void);
void LCD_Command(char);
void LCD_Char(char);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void LCD_Clear(void);
void MSdelay(unsigned int);
void ADC_Init(void);

int digital;
float voltage;
char data[10];

void main(void)
{
    // --- I/O Setup
    LCD_Port = 0x00;
    LCD_Control = 0x00;
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;

    LCD_Init();
    LCD_Clear();
    ADC_Init();

    while (1)
    {
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO);  // Wait for conversion

        digital = ((ADRESH << 8) | ADRESL);
        voltage = (digital * Vref) / 4096.0;
        sprintf(data, "%.2f V", voltage);

        LCD_Clear();
        LCD_String_xy(1, 0, "Voltage:");
        LCD_String_xy(2, 0, data);

        __delay_ms(500);
    }
}

// === LCD FUNCTIONS ===
void LCD_Init()
{
    MSdelay(15);
    LCD_Command(0x38); // 8-bit mode, 2 lines
    LCD_Command(0x0C); // Display ON, Cursor OFF
    LCD_Command(0x06); // Entry mode
    LCD_Command(0x01); // Clear display
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

// === Delay Function ===
void MSdelay(unsigned int val)
{
    for (unsigned int i = 0; i < val; i++)
        for (unsigned int j = 0; j < 165; j++);
}

// === ADC INIT ===
void ADC_Init(void)
{
    ADCON0bits.ADON = 0;
    ADCON0bits.FM = 1;
    ADCON0bits.CS = 1;
    ADCON0bits.ON = 1;

    TRISAbits.TRISA0 = 1;
    ANSELAbits.ANSELA0 = 1;

    ADPCH = 0b000000; // RA0
    ADCLK = 0x00;
    ADRESH = 0;
    ADRESL = 0;
    ADPREL = 0x00;
    ADPREH = 0x00;
    ADACQL = 0x00;
    ADACQH = 0x00;
}
