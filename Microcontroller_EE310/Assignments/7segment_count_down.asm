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
    CLRF     REG10              ; Clear REG10 (start at 0)

_loop:
    CALL     _displayDigit      ; Call subroutine to display the digit
    CALL     _delay2Seconds     ; Call delay subroutine (2 seconds)
    
    INCF     REG10, F           ; Increment REG10
    MOVF     REG10, W           ; Move REG10 to W
    SUBLW    16                 ; Compare REG10 with 16 (for full range 0-F)
    BTFSS    STATUS, 2          ; Skip next instruction if REG10 == 16
    BRA      _loop              ; If not 16, repeat loop

    CLRF     REG10              ; Reset REG10 to 0 when reaching 16
    BRA      _loop              ; Restart counting from 0

;---------------------
; 7-Segment Display Subroutine Using Table Pointers
;---------------------
_displayDigit:
    CALL     _getDigitPattern   ; Get the corresponding 7-segment pattern
    MOVWF    LATD              ; Output the pattern to PORTD (7-segment display)
    RETURN                     ; Return from subroutine

;---------------------
; Lookup Table Using Table Pointers
;---------------------
_getDigitPattern:
    BANKSEL  TBLPTRH
    MOVLW    HIGH digitTable   ; Load high byte of the table address
    MOVWF    TBLPTRH          
    MOVLW    LOW digitTable    ; Load low byte of the table address
    MOVWF    TBLPTRL           ; Store into Table Pointer Low

    BANKSEL  REG10
    MOVF     REG10, W          ; Move digit index into W
    ADDWF    TBLPTRL, F        ; Adjust table pointer to the correct entry

    TBLRD*                    ; Read the pattern from the lookup table
    MOVF     TABLAT, W        ; Move the read value into W register
    RETURN                    ; Return with the pattern in W

    ORG 0x100                 ; Define start of lookup table in program memory
digitTable:
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
    DB  0b10111110  ; A
    DB  0b11110010  ; B
    DB  0b11011000  ; C
    DB  0b11100110  ; D
    DB  0b11111000  ; E
    DB  0b10111000  ; F

;---------------------
; 2-Second Delay Subroutine
;---------------------
_delay2Seconds:
    MOVLW    255           ; Outer loop count (tune as needed)
    MOVWF    20h          ; Store in a temp register

_delayOuter:
    MOVLW    200          ; Inner loop count
    MOVWF    21h          ; Store in another temp register

_delayInner:
    NOP                   ; No operation (adjust timing)
    NOP
    NOP
    DECFSZ   21h, F       ; Decrease inner loop counter
    BRA      _delayInner  ; Repeat inner loop

    DECFSZ   20h, F       ; Decrease outer loop counter
    BRA      _delayOuter  ; Repeat outer loop

    RETURN

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
