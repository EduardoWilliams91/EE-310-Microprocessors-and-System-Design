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

#define _XTAL_FREQ 4000000
#define Vref 3.3

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
uint16_t digital = 0;
float voltage = 0.0;
char data[17];

void main(void)
{
    // Set Ports First
    LCD_Port = 0x00;
    LCD_Control = 0x00;
    TRISB = 0x00;
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;

    __delay_ms(50); // Startup stabilization after unplug/replug

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
        ADCON0bits.GO = 1;              // Start ADC conversion
        while (ADCON0bits.GO);           // Wait until done

        digital = ((uint16_t)ADRESH << 8) | ADRESL; // Get 10-bit result
        voltage = ((float)digital * Vref) / 4096.0f; // Calculate voltage

        sprintf(data, "V = %.2f V", voltage);

        LCD_String_xy(1, 0, "Voltage Reading:");
        LCD_String_xy(2, 0, "                "); // Clear line first
        LCD_String_xy(2, 0, data);

        __delay_ms(500); // Update every 0.5 sec
    }
}

// === ADC Initialization ===
void ADC_Init(void)
{
    TRISAbits.TRISA0 = 1;       // RA0 as input
    ANSELAbits.ANSELA0 = 1;     // RA0 analog

    ADPCH = 0b000000;           // Select AN0

    ADCON0bits.CS = 1;          // Use internal clock
    ADCLK = 0x3F;               // Slow ADC clock divider for stability

    ADCON0bits.FM = 1;          // Right justified
    ADCON0bits.ON = 1;          // Turn ADC ON

    ADRESH = 0x00;              // Clear results
    ADRESL = 0x00;
}

// === LCD Functions ===
void LCD_Init()
{
    MSdelay(20); // Wait for LCD to power up properly

    LCD_Command(0x38);    // Function set: 8-bit, 2 lines, 5x8 dots
    MSdelay(5);

    LCD_Command(0x0C);    // Display ON, Cursor OFF
    MSdelay(5);

    LCD_Command(0x06);    // Entry mode: Increment cursor
    MSdelay(5);

    LCD_Command(0x01);    // Clear display
    MSdelay(5);
}

void LCD_Command(char cmd)
{
    ldata = cmd;
    RS = 0;
    EN = 1; NOP(); EN = 0;
    MSdelay(2);
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
    while (*msg)
        LCD_Char(*msg++);
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
    MSdelay(2);
}

// === Blocking Delay ===
void MSdelay(unsigned int val)
{
    for (unsigned int i = 0; i < val; i++)
        for (unsigned int j = 0; j < 165; j++);
}
