#include <xc.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#pragma config WDTE = OFF
#define _XTAL_FREQ 4000000  // 4 MHz internal clock

// === Segment patterns for 0–9 and E ===
#define SEGMENT_OFF   0b00000000
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
#define SEGMENT_E     0b11111000  // Letter E for Error

// === Get segment pattern for number or error ===
unsigned char getSegment(char key) {
    switch (key) {
        case 0: return SEGMENT_0;
        case 1: return SEGMENT_1;
        case 2: return SEGMENT_2;
        case 3: return SEGMENT_3;
        case 4: return SEGMENT_4;
        case 5: return SEGMENT_5;
        case 6: return SEGMENT_6;
        case 7: return SEGMENT_7;
        case 8: return SEGMENT_8;
        case 9: return SEGMENT_9;
        case 'E': return SEGMENT_E;
        default: return SEGMENT_OFF;
    }
}

// === Wait for and return one key (with '*' reset support) ===
char getKeypadKey() {
    while (1) {
        LATA |= 0x0F;

        for (char row = 0; row < 4; row++) {
            LATAbits.LATA0 = (row != 0);
            LATAbits.LATA1 = (row != 1);
            LATAbits.LATA2 = (row != 2);
            LATAbits.LATA3 = (row != 3);

            __delay_ms(1);

            if (!PORTCbits.RC4) {
                while (!PORTCbits.RC4);
                __delay_ms(5);
                if (row == 0) return 1;
                if (row == 1) return 4;
                if (row == 2) return 7;
                if (row == 3) return '*';
            }
            if (!PORTCbits.RC5) {
                while (!PORTCbits.RC5);
                __delay_ms(5);
                if (row == 0) return 2;
                if (row == 1) return 5;
                if (row == 2) return 8;
                if (row == 3) return 0;
            }
            if (!PORTCbits.RC6) {
                while (!PORTCbits.RC6);
                __delay_ms(5);
                if (row == 0) return 3;
                if (row == 1) return 6;
                if (row == 2) return 9;
                if (row == 3) return '#';
            }
            if (!PORTCbits.RC7) {
                while (!PORTCbits.RC7);
                __delay_ms(5);
                if (row == 0) return 'A';
                if (row == 1) return 'B';
                if (row == 2) return 'C';
                if (row == 3) return 'D';
            }
        }
    }
}

void displayDigits(char high, char low) {
    LATB = getSegment(high);
    LATD = getSegment(low);
}

void clearDisplays() {
    LATB = SEGMENT_OFF;
    LATD = SEGMENT_OFF;
    LATDbits.LATD0 = 0;  // Also clear dot in case it was on
}

void main(void) {
    // === Setup ===
    TRISB = 0x00; LATB = SEGMENT_OFF; ANSELB = 0x00;
    TRISD = 0x00; LATD = SEGMENT_OFF; ANSELD = 0x00;

    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;

    TRISCbits.TRISC4 = 1;
    TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC7 = 1;

    ANSELA = 0x00;
    ANSELC = 0x00;

    WPUCbits.WPUC4 = 1;
    WPUCbits.WPUC5 = 1;
    WPUCbits.WPUC6 = 1;
    WPUCbits.WPUC7 = 1;

    // === Startup Sequence: Blink 0 on both displays for 5 seconds ===
    for (int i = 0; i < 5; i++) {
        LATB = SEGMENT_0;
        LATD = SEGMENT_0;
        __delay_ms(500);
        clearDisplays();
        __delay_ms(500);
    }

    // === Variables ===
    char x_high = 0, x_low = 0, y_high = 0, y_low = 0;
    unsigned char x_input_REG = 0;
    unsigned char y_input_REG = 0;
    char operation = 'A';
    unsigned char Operation_Reg = 0; // Register 25 (conceptual)

    while (1) {
        // First input
        while (1) {
            x_high = getKeypadKey();
            if (x_high == '*') { clearDisplays(); continue; }
            if (x_high >= 0 && x_high <= 9) break;
        }
        LATB = getSegment(x_high);

        while (1) {
            x_low = getKeypadKey();
            if (x_low == '*') { clearDisplays(); goto restart; }
            if (x_low >= 0 && x_low <= 9) break;
        }
        LATD = getSegment(x_low);
        x_input_REG = x_high * 10 + x_low;

        // Wait for 'A', 'B', 'C', or 'D'
        while (1) {
            operation = getKeypadKey();
            if (operation == '*') { clearDisplays(); goto restart; }
            if (operation == 'A' || operation == 'B' || operation == 'C' || operation == 'D') {
                Operation_Reg = operation;
                break;
            }
        }

        clearDisplays();

        // Second input
        while (1) {
            y_high = getKeypadKey();
            if (y_high == '*') { clearDisplays(); goto restart; }
            if (y_high >= 0 && y_high <= 9) break;
        }
        LATB = getSegment(y_high);

        while (1) {
            y_low = getKeypadKey();
            if (y_low == '*') { clearDisplays(); goto restart; }
            if (y_low >= 0 && y_low <= 9) break;
        }
        LATD = getSegment(y_low);
        y_input_REG = y_high * 10 + y_low;

        // Wait for '#'
        while (1) {
            char confirm = getKeypadKey();
            if (confirm == '*') { clearDisplays(); goto restart; }
            if (confirm == '#') break;
        }

        // Perform calculation
        int result = 0;
        if (Operation_Reg == 'A') {
            result = x_input_REG + y_input_REG;
        } else if (Operation_Reg == 'B') {
            result = x_input_REG - y_input_REG;
        } else if (Operation_Reg == 'C') {
            result = x_input_REG * y_input_REG;
        } else if (Operation_Reg == 'D') {
            if (y_input_REG == 0) {
                LATB = SEGMENT_E;
                LATD = SEGMENT_E;
                goto waitClear;
            }
            result = x_input_REG / y_input_REG;
        }

        if (result > 99 || result < -99) {
            LATB = SEGMENT_E;
            LATD = SEGMENT_E;
        } else {
            unsigned char absResult = (result < 0) ? -result : result;
            LATB = getSegment(absResult / 10);
            LATD = getSegment(absResult % 10);
            if (result < 0) LATDbits.LATD0 = 1;  // Turn on dot (RD0) for negative sign
        }

        // Wait for '*' to clear
        waitClear:
        while (getKeypadKey() != '*');
        clearDisplays();

        restart: ;
    }
}
