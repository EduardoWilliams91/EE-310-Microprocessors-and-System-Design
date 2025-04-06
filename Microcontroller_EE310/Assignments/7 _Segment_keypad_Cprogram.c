#include <xc.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#pragma config WDTE = OFF       // Disable Watchdog Timer
#define _XTAL_FREQ 4000000      // Clock frequency for delay

// === Segment patterns for common cathode ===
// Each pattern represents which segments (a–g) are turned on (1 = ON)
#define SEGMENT_OFF   0b00000000  // All segments off
#define SEGMENT_0     0b11011110
#define SEGMENT_1     0b00000110
#define SEGMENT_2     0b11101100
#define SEGMENT_3     0b01101110
#define SEGMENT_4     0b00110110
#define SEGMENT_5     0b01111010
#define SEGMENT_6     0b11111010
#define SEGMENT_7     0b00001110
#define SEGMENT_8     0b11111110
#define SEGMENT_9     0b00111110
#define SEGMENT_A     0b10111110
#define SEGMENT_B     0b11110010
#define SEGMENT_C     0b11011000
#define SEGMENT_D     0b11100110
#define SEGMENT_STAR  0b01101000  // Symbol: *
#define SEGMENT_HASH  0b10010110  // Symbol: #

void main(void) {
    // === 7-Segment Display Connection ===
    // 7-segment display is now connected to PORTB (RB0–RB7)
    TRISB = 0x00;         // Set PORTB as output
    LATB = SEGMENT_OFF;   // Turn off all segments initially
    ANSELB = 0x00;        // Set PORTB as digital I/O

    // === Keypad Connection ===
    // 4x4 Keypad matrix:
    // - Rows connected to RA0–RA3 (outputs)
    // - Columns connected to RC4–RC7 (inputs with pull-ups)
    TRISAbits.TRISA0 = 0;  // Row 0
    TRISAbits.TRISA1 = 0;  // Row 1
    TRISAbits.TRISA2 = 0;  // Row 2
    TRISAbits.TRISA3 = 0;  // Row 3

    TRISCbits.TRISC4 = 1;  // Column 0
    TRISCbits.TRISC5 = 1;  // Column 1
    TRISCbits.TRISC6 = 1;  // Column 2
    TRISCbits.TRISC7 = 1;  // Column 3

    ANSELA = 0x00;  // Set PORTA to digital I/O
    ANSELC = 0x00;  // Set PORTC to digital I/O

    // Enable internal pull-ups on column inputs (RC4–RC7)
    WPUCbits.WPUC4 = 1;
    WPUCbits.WPUC5 = 1;
    WPUCbits.WPUC6 = 1;
    WPUCbits.WPUC7 = 1;

    while (1) {
        int keyPressed = -1;  // -1 indicates no key is pressed

        // === Set all rows high ===
        LATA |= 0x0F;

        // === Scan Row 0 (RA0 = 0) ===
        LATAbits.LATA0 = 0;
        __delay_ms(1);
        if (!PORTCbits.RC4) keyPressed = 1;
        else if (!PORTCbits.RC5) keyPressed = 2;
        else if (!PORTCbits.RC6) keyPressed = 3;
        else if (!PORTCbits.RC7) keyPressed = 'A';
        LATAbits.LATA0 = 1;

        // === Scan Row 1 (RA1 = 0) ===
        LATAbits.LATA1 = 0;
        __delay_ms(1);
        if (!PORTCbits.RC4) keyPressed = 4;
        else if (!PORTCbits.RC5) keyPressed = 5;
        else if (!PORTCbits.RC6) keyPressed = 6;
        else if (!PORTCbits.RC7) keyPressed = 'B';
        LATAbits.LATA1 = 1;

        // === Scan Row 2 (RA2 = 0) ===
        LATAbits.LATA2 = 0;
        __delay_ms(1);
        if (!PORTCbits.RC4) keyPressed = 7;
        else if (!PORTCbits.RC5) keyPressed = 8;
        else if (!PORTCbits.RC6) keyPressed = 9;
        else if (!PORTCbits.RC7) keyPressed = 'C';
        LATAbits.LATA2 = 1;

        // === Scan Row 3 (RA3 = 0) ===
        LATAbits.LATA3 = 0;
        __delay_ms(1);
        if (!PORTCbits.RC4) keyPressed = '*';
        else if (!PORTCbits.RC5) keyPressed = 0;
        else if (!PORTCbits.RC6) keyPressed = '#';
        else if (!PORTCbits.RC7) keyPressed = 'D';
        LATAbits.LATA3 = 1;

        // === Display character while key is pressed ===
        if (keyPressed != -1) {
            switch (keyPressed) {
                case 0:   LATB = SEGMENT_0; break;
                case 1:   LATB = SEGMENT_1; break;
                case 2:   LATB = SEGMENT_2; break;
                case 3:   LATB = SEGMENT_3; break;
                case 4:   LATB = SEGMENT_4; break;
                case 5:   LATB = SEGMENT_5; break;
                case 6:   LATB = SEGMENT_6; break;
                case 7:   LATB = SEGMENT_7; break;
                case 8:   LATB = SEGMENT_8; break;
                case 9:   LATB = SEGMENT_9; break;
                case 'A': LATB = SEGMENT_A; break;
                case 'B': LATB = SEGMENT_B; break;
                case 'C': LATB = SEGMENT_C; break;
                case 'D': LATB = SEGMENT_D; break;
                case '*': LATB = SEGMENT_STAR; break;
                case '#': LATB = SEGMENT_HASH; break;
            }
        } else {
            LATB = SEGMENT_OFF;  // No key pressed, turn off display
        }
    }
}
