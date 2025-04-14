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
#include <string.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#define _XTAL_FREQ 4000000


#define RS LATD0
#define EN LATD1
#define ldata LATB
#define LCD_Port TRISB
#define LCD_Control TRISD

// === Function Prototypes ===
void LCD_Init();
void LCD_Command(char);
void LCD_Char(char);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void LCD_Clear();
void MSdelay(unsigned int);
char getKeypadKey();
void displayMode();
void playBuzzerTune();

// === Global Variables ===
unsigned int count = 0;
unsigned char relayState = 0;
unsigned char mode = 1;

char password[3] = "";
char setPassword[3] = "33";
char passwordPos = 0;

char entryPassword[3] = "";
char entryLeftDigit = '0';
char entryRightDigit = '0';

void main(void)
{
    LCD_Init();
    LCD_Clear();

    TRISA = 0xF0;
    TRISC = 0xF0;
    ANSELA = 0x00;
    ANSELC = 0x00;

    TRISAbits.TRISA0 = 0; TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA2 = 0; TRISAbits.TRISA3 = 0;

    TRISCbits.TRISC4 = 1; TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC6 = 1; TRISCbits.TRISC7 = 1;
    WPUCbits.WPUC4 = 1; WPUCbits.WPUC5 = 1;
    WPUCbits.WPUC6 = 1; WPUCbits.WPUC7 = 1;

    TRISD5 = 1; ANSELD5 = 0;
    TRISD6 = 1; ANSELD6 = 0;
    TRISA4 = 0; TRISA5 = 1;
    ANSELA4 = 0; ANSELA5 = 0;

    TRISDbits.TRISD7 = 0; LATDbits.LATD7 = 0; // Buzzer
    TRISCbits.TRISC2 = 1; ANSELCbits.ANSELC2 = 0; WPUCbits.WPUC2 = 1; // RC2 button input

    // Interrupt Setup
    PIE0bits.IOCIE = 1;
    IOCCNbits.IOCCN2 = 1;  // Falling edge
    IOCCFbits.IOCCF2 = 0;
    INTCON0bits.IPEN = 0;
    INTCON0bits.GIE = 1;

    LATAbits.LATA0 = 1;
    LATAbits.LATA1 = 1;
    LATAbits.LATA2 = 1;
    LATAbits.LATA3 = 1;

    char key;
    char prevRD5 = 0, prevRD6 = 0, prevRA5 = 0;

    LCD_String_xy(1, 0, "Relay");
    LCD_String_xy(2, 0, "Button: OFF");

    while(1)
    {
        key = getKeypadKey();

        if (key == 'A') {
            mode++;
            if (mode > 4) mode = 1;
            LCD_Clear();
            displayMode();
            MSdelay(300);
        }

        if (mode == 1)
        {
            if (PORTAbits.RA5 == 0 && prevRA5 == 0)
            {
                relayState = 1;
                LATAbits.LATA4 = 1;
                LCD_String_xy(2, 0, "Button: ON ");
                prevRA5 = 1;
            }
            else if (PORTAbits.RA5 == 1 && prevRA5 == 1)
            {
                relayState = 0;
                LATAbits.LATA4 = 0;
                LCD_String_xy(2, 0, "Button: OFF");
                prevRA5 = 0;
            }
        }
        else if (mode == 2)
        {
            if (PORTDbits.RD5 == 1 && prevRD5 == 0)
            {
                count++;
                char buf[16];
                sprintf(buf, "Count: %u   ", count);
                LCD_String_xy(2, 0, buf);
                prevRD5 = 1;
            }
            else if (PORTDbits.RD5 == 0) prevRD5 = 0;

            if (PORTDbits.RD6 == 1 && prevRD6 == 0)
            {
                if (count > 0) count--;
                char buf[16];
                sprintf(buf, "Count: %u   ", count);
                LCD_String_xy(2, 0, buf);
                prevRD6 = 1;
            }
            else if (PORTDbits.RD6 == 0) prevRD6 = 0;
        }
        else if (mode == 3)
        {
            if (key >= '0' && key <= '9' && passwordPos < 2)
            {
                password[passwordPos++] = key;
                password[passwordPos] = '\0';
                LCD_String_xy(1, 0, "Set Password:");
                LCD_String_xy(1, 14, password);
                MSdelay(200);
            }
            else if (key == '#')
            {
                if (passwordPos == 2)
                {
                    strcpy(setPassword, password);
                    LCD_String_xy(2, 0, "Password SAVED ");
                    MSdelay(1000);
                    LCD_String_xy(2, 0, "                ");
                }
            }
            else if (key == 'C')
            {
                password[0] = '\0';
                passwordPos = 0;
                LCD_String_xy(1, 0, "Set Password:");
                LCD_String_xy(1, 14, "  ");
                LCD_String_xy(2, 0, "Input cleared   ");
                MSdelay(1000);
                LCD_String_xy(2, 0, "                ");
            }
        }
        else if (mode == 4)
        {
            if (PORTDbits.RD5 == 1 && prevRD5 == 0)
            {
                if (++entryLeftDigit > '9') entryLeftDigit = '0';
                prevRD5 = 1;
            }
            else if (PORTDbits.RD5 == 0) prevRD5 = 0;

            if (PORTDbits.RD6 == 1 && prevRD6 == 0)
            {
                if (++entryRightDigit > '9') entryRightDigit = '0';
                prevRD6 = 1;
            }
            else if (PORTDbits.RD6 == 0) prevRD6 = 0;

            char disp[16];
            sprintf(disp, "Enter: %c%c", entryLeftDigit, entryRightDigit);
            LCD_String_xy(1, 0, disp);

            if (key == '#')
            {
                entryPassword[0] = entryLeftDigit;
                entryPassword[1] = entryRightDigit;
                entryPassword[2] = '\0';

                if (strcmp(entryPassword, setPassword) == 0)
                {
                    LCD_String_xy(2, 0, "Access Granted ");
                    LATAbits.LATA4 = 1;
                    MSdelay(5000);
                    LATAbits.LATA4 = 0;
                }
                else
                {
                    LCD_String_xy(2, 0, "Wrong Password ");
                    playBuzzerTune();
                }
                MSdelay(1000);
                LCD_String_xy(2, 0, "                ");
            }
        }

        MSdelay(10);
    }
}

// === EMERGENCY INTERRUPT HANDLER ===
void __interrupt(irq(default), base(0x0008)) ISR(void)
{
    if (IOCCFbits.IOCCF2)
    {
        // Only activate if RC2 is really LOW (button pressed)
        if (PORTCbits.RC2 == 0)
        {
            IOCCFbits.IOCCF2 = 0;

            LATAbits.LATA4 = 0;           // Turn off relay
            LATDbits.LATD7 = 1;           // Buzzer ON

            LCD_Clear();
            LCD_String_xy(1, 0, "!!! EMERGENCY !!!");
            LCD_String_xy(2, 0, "Buzzer 10 Sec");

            for (int i = 0; i < 1000; i++) __delay_ms(10); // 10 sec

            LATDbits.LATD7 = 0;           // Buzzer OFF
            LCD_Clear();
            displayMode();                // Redisplay mode
        }
        else
        {
            IOCCFbits.IOCCF2 = 0; // Clear only
        }
    }
}

// === Buzzer Tune (2-sec) ===
void playBuzzerTune()
{
    for (int i = 0; i < 4; i++)
    {
        LATDbits.LATD7 = 1;
        MSdelay(125);
        LATDbits.LATD7 = 0;
        MSdelay(125);
    }
}

// === Keypad Scanner ===
char getKeypadKey()
{
    const char keyMap[4][4] = {
        {'1','4','7','*'},
        {'2','5','8','0'},
        {'3','6','9','#'},
        {'A','B','C','D'}
    };
    unsigned char rowMask[4] = {0b1110, 0b1101, 0b1011, 0b0111};

    for (int row = 0; row < 4; row++)
    {
        LATA = (LATA & 0xF0) | rowMask[row];
        MSdelay(1);
        for (int col = 0; col < 4; col++)
        {
            if (((PORTC >> (col + 4)) & 0x01) == 0)
            {
                MSdelay(20);
                if (((PORTC >> (col + 4)) & 0x01) == 0)
                {
                    while (((PORTC >> (col + 4)) & 0x01) == 0);
                    return keyMap[row][col];
                }
            }
        }
    }

    return 0;
}

// === Display Mode Info ===
void displayMode()
{
    if (mode == 1)
    {
        LCD_String_xy(1, 0, "Relay");
        LCD_String_xy(2, 0, "Button: OFF");
    }
    else if (mode == 2)
    {
        LCD_String_xy(1, 0, "Photo Button U/D");
        char buf[16];
        sprintf(buf, "Count: %u", count);
        LCD_String_xy(2, 0, buf);
    }
    else if (mode == 3)
    {
        LCD_String_xy(1, 0, "Set Password:");
        LCD_String_xy(1, 14, setPassword);
        LCD_String_xy(2, 0, "#:Save C:Clr     ");
        password[0] = '\0';
        passwordPos = 0;
    }
    else if (mode == 4)
    {
        LCD_String_xy(1, 0, "Enter ");
        LCD_String_xy(2, 0, "#: Enter");
        entryLeftDigit = '0';
        entryRightDigit = '0';
    }
}

// === LCD Drivers ===
void LCD_Init()
{
    MSdelay(15);
    LCD_Port = 0x00;
    LCD_Control = 0x00;
    LCD_Command(0x01);
    LCD_Command(0x38);
    LCD_Command(0x0C);
    LCD_Command(0x06);
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

// === Delay (Blocking) ===
void MSdelay(unsigned int val)
{
    for (unsigned int i = 0; i < val; i++)
        for (unsigned int j = 0; j < 165; j++);
}
