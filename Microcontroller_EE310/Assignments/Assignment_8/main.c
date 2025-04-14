#include "config.h"
#include "init.h"
#include "functions.h"
#include <xc.h>
#include <stdio.h>
#include <string.h>

void main(void)
{
    LCD_Init();
    LCD_Clear();
    initSystem();

    char key;
    char prevRD5 = 0, prevRD6 = 0, prevRA5 = 0;

    LCD_String_xy(1, 0, "Relay");
    LCD_String_xy(2, 0, "Button: OFF");

    while (1)
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
        if (PORTCbits.RC2 == 0)
        {
            IOCCFbits.IOCCF2 = 0;

            LATAbits.LATA4 = 0;
            LATDbits.LATD7 = 1;

            LCD_Clear();
            LCD_String_xy(1, 0, "!!! EMERGENCY !!!");
            LCD_String_xy(2, 0, "Buzzer 10 Sec");

            for (int i = 0; i < 1000; i++) __delay_ms(10);

            LATDbits.LATD7 = 0;
            LCD_Clear();
            displayMode();
        }
        else
        {
            IOCCFbits.IOCCF2 = 0;
        }
    }
}
