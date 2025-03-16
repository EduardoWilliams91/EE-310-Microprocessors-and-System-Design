;---------------------
; Title: "7-Segment Display Driver for PIC18F46K42 Using Assembly"
;---------------------
; Program Details:
;This program is designed to display digits (0-9) on a 7-segment display using a 
;PIC18F46K42 microcontroller. It utilizes a lookup table stored in program memory 
;to retrieve the correct segment pattern for each digit. The digit is loaded into 
;a register, processed through a table lookup, and sent to PORTD to control the 
;7-segment display.

; Inputs:
 ;Digit (0-9) stored in REG10 (passed via MOVLW instruction).
; Outputs: 
;  7-Segment Display (Common Cathode or Anode) connected to PORTD, where each bit 
;  controls a segment to form the desired number
; Date: March 16, 2025
; File Dependencies / Libraries: None 
; Compiler: xc8, 3.0
; Author: Eduardo Williams 
; Versions:
;       V1.0: First Version 
; Useful links: 
;       Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
;       PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
;       GITHUB: https://github.com/EduardoWilliams91/EE-310-Microprocessors-and-System-Design/tree/main/Microcontroller_EE310
    
    
#include "./AssemblyConfig.inc"  ; Include additional configuration settings
#include <xc.inc>               ; Include XC compiler header file

PROCESSOR 18F46K42              ; Define the target microcontroller

;---------------------
; Program Constants
;---------------------
REG10   equ     10h             ; Define REG10 as a register at address 0x10 to store the digit

;---------------------
; Main Program
;---------------------
    PSECT absdata,abs,ovrld    ; Define an absolute section in memory

    ORG          0             ; Reset vector, starting program execution
    GOTO        _initialization ; Jump to initialization routine
    ORG          0020H         ; Set next program instruction location

_initialization: 
    RCALL _setupPortD          ; Call subroutine to configure PORTD

_main:
    MOVLW    9                 ; Load W register with the digit 9
    CALL     _displayDigit      ; Call the subroutine to display the digit
    BRA      _main              ; Infinite loop, repeat main program

;---------------------
; 7-Segment Display Subroutine Using Table Pointers
;---------------------
_displayDigit:
    MOVWF    REG10             ; Store digit from W into REG10
    CALL     _getDigitPattern   ; Get the corresponding 7-segment pattern
    MOVWF    LATD              ; Output the pattern to PORTD (7-segment display)
    RETURN                     ; Return from subroutine

;---------------------
; Lookup Table Using Table Pointers (Corrected)
;---------------------
_getDigitPattern:
    ; Ensure correct memory bank selection for accessing table
    BANKSEL  TBLPTRH
    MOVLW    HIGH digitTable   ; Load high byte of the table address
    MOVWF    TBLPTRH          
    MOVLW    LOW digitTable    ; Load low byte of the table address
    MOVWF    TBLPTRL          ; Store into Table Pointer Low

    ; Adjust table pointer to access the correct digit's pattern
    BANKSEL  REG10
    MOVF     REG10, W         ; Move digit index into W
    ADDWF    TBLPTRL, F       ; Adjust table pointer to the correct entry

    ; Read data from program memory
    TBLRD*                    ; Read the pattern from the lookup table
    MOVF     TABLAT, W        ; Move the read value into W register
    RETURN                    ; Return with the pattern in W

    ORG 0x100                 ; Define start of lookup table in program memory
digitTable:
    ; Binary values representing 7-segment patterns for digits 0-9
    DB  0b11011110  ; 0
    DB  0b00000110  ; 1
    DB  0b11101100  ; 2
    DB  0b01101110  ; 3
    DB  0b00110110  ; 4
    DB  0b01111010  ; 5
    DB  0b11111010  ; 6
    DB  0b00001110  ; 7
    DB  0b11111110  ; 8
    DB  0b00111110  ; 9

; This lookup table defines the binary bit patterns for a 7-segment display.
; Each entry corresponds to a specific digit (0-9). When retrieved using table
; pointers, the appropriate pattern is sent to PORTD to illuminate the correct segments.

;---------------------
; Port D Setup
;---------------------
_setupPortD:
    BANKSEL  PORTD
    CLRF     PORTD    ; Clear PORTD to ensure it's in a known state
    BANKSEL  LATD
    CLRF     LATD     ; Clear the data latch to prevent unintended outputs
    BANKSEL  ANSELD
    CLRF     ANSELD   ; Set all PORTD pins as digital (disable analog functions)
    BANKSEL  TRISD
    MOVLW    0b00000000  ; Configure all RD0-RD7 pins as outputs
    MOVWF    TRISD
    RETURN

    END               ; End of program
