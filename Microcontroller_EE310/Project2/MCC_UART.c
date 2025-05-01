#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/uart/uart1.h"

#define _XTAL_FREQ 4000000

// === LCD Connections ===
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
void LCD_String_xy(char row, char pos, const char *);
void LCD_Clear(void);
void MSdelay(unsigned int);
void ADC1_Initialize(void);
uint16_t ADC1_Read(uint8_t channel);
void UART1_Write_Text(const char*);

// === Main ===
void main(void)
{
    SYSTEM_Initialize();
    ADC1_Initialize();

    LCD_Port = 0x00;
    LCD_Control = 0x00;
    TRISB = 0x00;
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;

    LCD_Init();
    LCD_Clear();

    LCD_String_xy(1, 0, "EDUARDO WILLIAMS");
    __delay_ms(300);
    LCD_Clear();

    TRISAbits.TRISA0 = 1; ANSELAbits.ANSELA0 = 1;
    TRISAbits.TRISA1 = 1; ANSELAbits.ANSELA1 = 1;
    TRISCbits.TRISC2 = 1; ANSELCbits.ANSELC2 = 0;
    TRISCbits.TRISC3 = 1; ANSELCbits.ANSELC3 = 0;
    TRISDbits.TRISD2 = 1; ANSELDbits.ANSELD2 = 0;
    TRISDbits.TRISD3 = 1; ANSELDbits.ANSELD3 = 0;

    uint16_t x_val, y_val;
    char displayLine[17];
    uint8_t last_dir_x = 0, last_dir_y = 0;
    uint8_t last_RC2 = 1, last_RC3 = 1, last_RD2 = 1, last_RD3 = 1;
    static uint16_t lcd_counter = 0;
    static uint16_t last_x = 0, last_y = 0;

    while (1)
    {
        // === Read ADC for Joystick ===
        x_val = ADC1_Read(0);  // RA0
        y_val = ADC1_Read(1);  // RA1

        // === Throttle LCD update ===
        lcd_counter++;
        if ((x_val != last_x || y_val != last_y) && lcd_counter > 1000)
        {
            sprintf(displayLine, "X:%4u Y:%4u", x_val, y_val);
            LCD_String_xy(1, 0, displayLine);
            last_x = x_val;
            last_y = y_val;
            lcd_counter = 0;
        }

        // === Direction Detection Based on Your Ranges ===
        if (x_val >= 3200 && x_val <= 3300 && y_val >= 1640 && y_val <= 1680 && last_dir_x != 1) {
            UART1_Write_Text("LEFT\r\n"); last_dir_x = 1; last_dir_y = 0;
        }
        else if (x_val >= 6 && x_val <= 60 && y_val >= 1400 && y_val <= 1800 && last_dir_x != 2) {
            UART1_Write_Text("RIGHT\r\n"); last_dir_x = 2; last_dir_y = 0;
        }
        else if (y_val >= 6 && y_val <= 60 && x_val >= 1400 && x_val <= 1800 && last_dir_y != 2) {
            UART1_Write_Text("UP\r\n"); last_dir_y = 2; last_dir_x = 0;
        }
        else if (y_val >= 3200 && y_val <= 3300 && x_val >= 1640 && x_val <= 1700 && last_dir_y != 1) {
            UART1_Write_Text("DOWN\r\n"); last_dir_y = 1; last_dir_x = 0;
        }
        else if (x_val >= 1630 && x_val <= 1650 && y_val >= 1630 && y_val <= 1650 &&
                 (last_dir_x != 0 || last_dir_y != 0)) {
            UART1_Write_Text("CENTER\r\n");
            last_dir_x = 0; last_dir_y = 0;
        }

        // === Button Debounced Check ===
        if (PORTCbits.RC2 == 0 && last_RC2 == 1) {
            __delay_ms(5);
            if (PORTCbits.RC2 == 0)
                UART1_Write_Text("UP (button)\r\n");
        } last_RC2 = PORTCbits.RC2;

        if (PORTCbits.RC3 == 0 && last_RC3 == 1) {
            __delay_ms(5);
            if (PORTCbits.RC3 == 0)
                UART1_Write_Text("DOWN (button)\r\n");
        } last_RC3 = PORTCbits.RC3;

        if (PORTDbits.RD2 == 0 && last_RD2 == 1) {
            __delay_ms(5);
            if (PORTDbits.RD2 == 0)
                UART1_Write_Text("LEFT (button)\r\n");
        } last_RD2 = PORTDbits.RD2;

        if (PORTDbits.RD3 == 0 && last_RD3 == 1) {
            __delay_ms(5);
            if (PORTDbits.RD3 == 0)
                UART1_Write_Text("RIGHT (button)\r\n");
        } last_RD3 = PORTDbits.RD3;
    }
}

// === ADC ===
void ADC1_Initialize(void)
{
    ADPCH = 0x00;
    ADCON0bits.ADON = 1;
    ADCON0bits.CONT = 0;
    ADCON0bits.FM = 1;
    ADCON0bits.CS = 1;
}

uint16_t ADC1_Read(uint8_t channel)
{
    ADPCH = channel;
    __delay_us(5);
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    return ((uint16_t)ADRESH << 8) | ADRESL;
}

// === UART ===
void UART1_Write_Text(const char* text)
{
    while (*text) {
        while (!UART1_IsTxReady());
        UART1_Write((uint8_t)*text);
        text++;
    }
}

// === LCD ===
void LCD_Init(void)
{
    MSdelay(20);
    LCD_Command(0x38); MSdelay(5);
    LCD_Command(0x0C); MSdelay(5);
    LCD_Command(0x06); MSdelay(5);
    LCD_Command(0x01); MSdelay(5);
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
    while (*msg) LCD_Char(*msg++);
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char loc = (row == 1) ? (0x80 | (pos & 0x0F)) : (0xC0 | (pos & 0x0F));
    LCD_Command(loc);
    LCD_String(msg);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    MSdelay(2);
}

void MSdelay(unsigned int val)
{
    for (unsigned int i = 0; i < val; i++)
        for (unsigned int j = 0; j < 165; j++);
}
