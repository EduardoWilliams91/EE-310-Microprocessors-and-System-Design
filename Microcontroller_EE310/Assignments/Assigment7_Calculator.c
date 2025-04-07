//---------------------
// Title: 4x4 Keypad-Based 2-Digit Calculator with 7-Segment Display on PIC18F47K42
//---------------------
// Program Details:
// This program is a calculator implemented on a PIC18F47K42 microcontroller 
// using C with the XC8 compiler. It allows the user to perform basic arithmetic 
// operations—addition, subtraction, multiplication, and division—via a
// 4x4 matrix keypad, with results displayed on two 7-segment displays.
// At startup, the displays blink “0” for five seconds as a visual indicator 
// that the system is ready. The user enters a 2-digit number 
// (split into two key presses), selects an operation 
// (A for addition, B for subtraction, C for multiplication, or D for division), 
// and then inputs a second 2-digit number. 
// The result is calculated and shown on the displays unless it exceeds the 
// range −99 to 99 or involves division by zero, in which case the system shows 
// an error by displaying 'E' on both digits. If the result is negative, the dot 
// on the second 7-segment display (RD0) is turned on. The operation continues 
// until the user presses *, which resets the displays and restarts the input process
// ---------------------------------------------------------------------------- 
//
// Inputs:
// User  enter 2-digit numbers
// Stored in x_high, x_low, y_high, and y_low
// Combined to form x_input_REG and y_input_REG
// 'A'	Addition	x_input_REG + y_input_REG	 
// 'B'	Subtraction	x_input_REG - y_input_REG
// 'C'	Multiplication	x_input_REG * y_input_REG
// 'D'	Division	x_input_REG / y_input_REG
// Store operation value in Operation_Reg
// '#'	Confirms the operation and triggers display
// '*'	Resets the system at any stage and clears the displays
// ------------------------------------------------------------------------------
// Outputs: 
// PORTB → first digit (most significant digit)
// PORTD → second digit (least significant digit)
// -----------------------------------------------------------------------------    
// Date:  4/5/2025
// File Dependencies / Libraries: None 
// Compiler: xc8, 3.0
// Author: Eduardo Williams 
// Versions:
//       V1.0: First Version 
// Useful links: 
//       Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
//       PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
//       GITHUB: https://github.com/EduardoWilliams91/EE-310-Microprocessors-and-System-Design/tree/main/Microcontroller_EE310



#include <xc.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#pragma config WDTE = OFF
#define _XTAL_FREQ 4000000  // 4 MHz internal clock

// === Segment patterns for 0–9 and E ===
// Segment bit order: DP-G-F-E-D-C-B-A (from MSB to LSB)
// Each bit enables a segment on a common cathode 7-segment display
#define SEGMENT_OFF   0b00000000  // All segments off
#define SEGMENT_0     0b11011110  // Segments: A B C D E F (G off)
#define SEGMENT_1     0b00000110  // Segments: B C
#define SEGMENT_2     0b11101100  // Segments: A B D E G
#define SEGMENT_3     0b01101110  // Segments: A B C D G
#define SEGMENT_4     0b00110110  // Segments: B C F G
#define SEGMENT_5     0b01111010  // Segments: A C D F G
#define SEGMENT_6     0b11111010  // Segments: A C D E F G
#define SEGMENT_7     0b00001110  // Segments: A B C
#define SEGMENT_8     0b11111110  // Segments: All except DP
#define SEGMENT_9     0b00111110  // Segments: A B C D F G
#define SEGMENT_E     0b11111000  // Segments: A D E F G

// Function to convert a key input into the corresponding 7-segment display pattern
unsigned char getSegment(char key) {
    // Use a switch-case to map the input key to the corresponding segment pattern
    switch (key) {
        case 0: return SEGMENT_0; // If key is 0, return the pattern for displaying '0'
        case 1: return SEGMENT_1; // If key is 1, return the pattern for displaying '1'
        case 2: return SEGMENT_2; // If key is 2, return the pattern for displaying '2'
        case 3: return SEGMENT_3; // If key is 3, return the pattern for displaying '3'
        case 4: return SEGMENT_4; // If key is 4, return the pattern for displaying '4'
        case 5: return SEGMENT_5; // If key is 5, return the pattern for displaying '5'
        case 6: return SEGMENT_6; // If key is 6, return the pattern for displaying '6'
        case 7: return SEGMENT_7; // If key is 7, return the pattern for displaying '7'
        case 8: return SEGMENT_8; // If key is 8, return the pattern for displaying '8'
        case 9: return SEGMENT_9; // If key is 9, return the pattern for displaying '9'
        case 'E': return SEGMENT_E; // If key is character 'E', return the pattern for 'E'
        default: return SEGMENT_OFF; // For any other input, turn off the display
    }
}


// Function to detect and return the pressed key from a 4x4 keypad
char getKeypadKey() {
    while (1) {
        // Set all row pins (RA0 - RA3) to high (inactive)
        LATA |= 0x0F;  // 0x0F = 00001111

        // Loop through each row one by one
        for (char row = 0; row < 4; row++) {

            // Activate the current row by pulling it LOW and others HIGH
            LATAbits.LATA0 = (row != 0);  // Only set LOW if current row
            LATAbits.LATA1 = (row != 1);
            LATAbits.LATA2 = (row != 2);
            LATAbits.LATA3 = (row != 3);

            __delay_ms(1);  // Short delay to settle the output

            // Check if any column input (RC4 to RC7) is LOW (i.e., key press detected)

            // Check Column 1 (RC4)
            if (!PORTCbits.RC4) {
                // Wait until the key is released (debounce)
                while (!PORTCbits.RC4) __delay_ms(5);
                // Return corresponding key based on row
                if (row == 0) return '1';
                if (row == 1) return '4';
                if (row == 2) return '7';
                if (row == 3) return '*';
            }

            // Check Column 2 (RC5)
            if (!PORTCbits.RC5) {
                while (!PORTCbits.RC5) __delay_ms(5);
                if (row == 0) return '2';
                if (row == 1) return '5';
                if (row == 2) return '8';
                if (row == 3) return '0';
            }

            // Check Column 3 (RC6)
            if (!PORTCbits.RC6) {
                while (!PORTCbits.RC6) __delay_ms(5);
                if (row == 0) return '3';
                if (row == 1) return '6';
                if (row == 2) return '9';
                if (row == 3) return '#';
            }

            // Check Column 4 (RC7)
            if (!PORTCbits.RC7) {
                while (!PORTCbits.RC7) __delay_ms(5);
                if (row == 0) return 'A';
                if (row == 1) return 'B';
                if (row == 2) return 'C';
                if (row == 3) return 'D';
            }
        }
    }
}


// === Displays two digits on PORTB and PORTD ===
void displayDigits(char high, char low) {
    // Convert the 'high' character to its 7-segment pattern and output it to PORTB
    LATB = getSegment(high);   // Display the high digit on PORTB

    // Convert the 'low' character to its 7-segment pattern and output it to PORTD
    LATD = getSegment(low);    // Display the low digit on PORTD
}

// === Clears both 7-segment displays and the decimal/negative dot ===
void clearDisplays() {
    // Turn off all segments on the display connected to PORTB
    LATB = SEGMENT_OFF;

    // Turn off all segments on the display connected to PORTD
    LATD = SEGMENT_OFF;

    // Clear the negative indicator or decimal dot connected to LATD0
    LATDbits.LATD0 = 0;
}


void main(void) {
    // === I/O Setup for Displays and Keypad ===

    // Set PORTB as output for the first 7-segment display (high digit)
    TRISB = 0x00;          // All PORTB pins as output
    LATB = SEGMENT_OFF;    // Turn off display initially
    ANSELB = 0x00;         // Disable analog function on PORTB (digital mode)

    // Set PORTD as output for the second 7-segment display (low digit)
    TRISD = 0x00;
    LATD = SEGMENT_OFF;
    ANSELD = 0x00;

    // Configure RA0–RA3 as output for keypad row control
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;

    // Configure RC4–RC7 as input for keypad column reading
    TRISCbits.TRISC4 = 1;
    TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC7 = 1;

    ANSELA = 0x00;  // Set PORTA to digital mode
    ANSELC = 0x00;  // Set PORTC to digital mode

    // Enable weak pull-up resistors for keypad column inputs
    WPUCbits.WPUC4 = 1;
    WPUCbits.WPUC5 = 1;
    WPUCbits.WPUC6 = 1;
    WPUCbits.WPUC7 = 1;

    // === Startup Animation: Blink "0" on both displays 5 times ===
    for (int i = 0; i < 5; i++) {
        LATB = SEGMENT_0;
        LATD = SEGMENT_0;
        __delay_ms(500);    // Display "00" for 500ms
        clearDisplays();     // Turn off both displays
        __delay_ms(500);    // Wait 500ms before next blink
    }

    // === Variable Declarations ===
    char x_high = 0, x_low = 0, y_high = 0, y_low = 0;
    unsigned char x_input_REG = 0;
    unsigned char y_input_REG = 0;
    char operation = 'A';
    unsigned char Operation_Reg = 0;  // Stores selected operation ('A', 'B', 'C', or 'D')

    while (1) {
        // === Get First Number: Tens Digit ===
        while (1) {
            x_high = getKeypadKey();           // Wait for valid key
            if (x_high == '*') {               // If clear button pressed
                clearDisplays(); 
                continue;
            }
            if (x_high >= 0 && x_high <= 9) break;  // Only accept digits 0–9
        }
        LATB = getSegment(x_high);  // Display tens digit on PORTB

        // === Get First Number: Units Digit ===
        while (1) {
            x_low = getKeypadKey();
            if (x_low == '*') { clearDisplays(); goto restart; }
            if (x_low >= 0 && x_low <= 9) break;
        }
        LATD = getSegment(x_low);  // Display units digit on PORTD

        // Combine tens and units to form full input (e.g., 4 and 2 → 42)
        x_input_REG = x_high * 10 + x_low;

        // === Get Operation Selection (A = +, B = −, C = ×, D = ÷) ===
        while (1) {
            operation = getKeypadKey();
            if (operation == '*') { clearDisplays(); goto restart; }
            if (operation == 'A' || operation == 'B' || operation == 'C' || operation == 'D') {
                Operation_Reg = operation;
                break;
            }
        }

        clearDisplays();  // Prepare for next input

        // === Get Second Number: Tens Digit ===
        while (1) {
            y_high = getKeypadKey();
            if (y_high == '*') { clearDisplays(); goto restart; }
            if (y_high >= 0 && y_high <= 9) break;
        }
        LATB = getSegment(y_high);

        // === Get Second Number: Units Digit ===
        while (1) {
            y_low = getKeypadKey();
            if (y_low == '*') { clearDisplays(); goto restart; }
            if (y_low >= 0 && y_low <= 9) break;
        }
        LATD = getSegment(y_low);
        y_input_REG = y_high * 10 + y_low;

        // === Confirm Operation with '#' ===
        while (1) {
            char confirm = getKeypadKey();
            if (confirm == '*') { clearDisplays(); goto restart; }
            if (confirm == '#') break;  // Proceed on confirmation
        }

        // === Perform Arithmetic Operation ===
        int result = 0;
        if (Operation_Reg == 'A') {
            result = x_input_REG + y_input_REG;       // Addition
        } else if (Operation_Reg == 'B') {
            result = x_input_REG - y_input_REG;       // Subtraction
        } else if (Operation_Reg == 'C') {
            result = x_input_REG * y_input_REG;       // Multiplication
        } else if (Operation_Reg == 'D') {
            if (y_input_REG == 0) {                   // Handle divide-by-zero
                LATB = SEGMENT_E;
                LATD = SEGMENT_E;
                goto waitClear;
            }
            result = x_input_REG / y_input_REG;       // Division
        }

        // === Display Result or Error ===
        if (result > 99 || result < -99) {
            // Show "EE" for out-of-range result
            LATB = SEGMENT_E;
            LATD = SEGMENT_E;
        } else {
            // Display result normally (absolute value)
            unsigned char absResult = (result < 0) ? -result : result;
            LATB = getSegment(absResult / 10);        // Tens digit
            LATD = getSegment(absResult % 10);        // Units digit

            if (result < 0) LATDbits.LATD0 = 1;       // Turn on dot for negative sign
        }

        // === Wait for Clear '*' to Restart ===
        waitClear:
        while (getKeypadKey() != '*');   // Block until user presses '*'
        clearDisplays();                 // Reset displays

        // === Restart from beginning ===
        restart: ;
    }
}
