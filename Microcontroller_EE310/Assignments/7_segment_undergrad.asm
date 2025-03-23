#include "./AssemblyConfig.inc"  ; Include additional configuration settings
#include <xc.inc>                ; Include XC compiler header file

PROCESSOR 18F46K42              ; Define the target microcontroller

;---------------------
; Program Constants
;---------------------    
    REG10   equ     10h       ; Define REG10 as a register at address 0x10 to store the digit

;---------------------
; Main Program
;---------------------
    PSECT absdata,abs,ovrld    ; Define an absolute section in memory

    ORG          0            ; Reset vector, starting program execution
    GOTO        _initialization ; Jump to initialization routine
    ORG          0020H        ; Set next program instruction location

_initialization: 
    RCALL _setupPortD         ; Configure PORTD for 7-segment display
    RCALL _setupPortA         ; Configure PORTA for switch input

_waitForPress:
    ; Wait until at least one button is pressed.
    BTFSC PORTA, 0            ; If RA0 (active low) is not pressed, skip
    BTFSC PORTA, 1            ; and check RA1 – if neither is pressed, stay here
    GOTO _waitForPress

_mainLoop:
    CALL _displayDigit        ; Update the 7-segment display with REG10
    CALL _delay2Seconds       ; Wait 2 seconds

    ;-------------------------------
    ; New Code: Check for both buttons pressed
    ; (active low: 0 means pressed)
    ;-------------------------------
    BTFSC PORTA, 0          ; If RA0 is pressed, skip next instruction
    GOTO _check_single_buttons  ; If RA0 is not pressed, go to individual checks
    BTFSC PORTA, 1          ; If RA1 is pressed, skip next instruction
    GOTO _check_single_buttons  ; If RA1 is not pressed, go to individual checks
    BRA _reset_loop         ; Both buttons are pressed -> branch to reset loop

_check_single_buttons:
    ;-------------------------------
    ; Individual Button Checks
    ;-------------------------------
    ; Check for RA0 press (increment) – note: active low (0 = pressed)
    BTFSS PORTA, 0            ; If RA0 is not pressed (bit set), skip next instruction
    BRA _increment_loop       ; Otherwise, jump into increment loop

    ; Check for RA1 press (decrement)
    BTFSS PORTA, 1            ; If RA1 is not pressed, skip next instruction
    BRA _decrement_loop       ; Otherwise, jump into decrement loop

    BRA _mainLoop             ; Continue main loop if no button is pressed

;-------------------------------
; Reset Loop (Both Buttons Pressed)
;-------------------------------
_reset_loop:
    CLRF REG10                ; Reset REG10 (for example, set display to 0)
    CALL _displayDigit        ; Update the 7-segment display
    ; Wait here until both buttons are released
_wait_reset_release:
    BTFSS PORTA, 0            ; Check RA0: if not pressed (bit = 1) it will skip
    BRA _wait_reset_release   ; If still pressed (0), keep waiting
    BTFSS PORTA, 1            ; Check RA1: if not pressed, skip
    BRA _wait_reset_release   ; If still pressed, keep waiting
    BRA _mainLoop             ; Both buttons are released; return to main loop

;-------------------------------
; Increment Loop (RA0 pressed)
;-------------------------------
_increment_loop:
    INCF REG10, F             ; Increment REG10
    ; Check if we reached 16 (0x10); if so, wrap around to 0.
    MOVF REG10, W
    XORLW 16                ; Compare with 16
    BTFSC STATUS, 2         ; If equal (zero flag set)
         CLRF REG10         ; then clear REG10 (wrap to 0)
    CALL _displayDigit        ; Update display
    CALL _delay2Seconds       ; Delay 2 seconds

    ; Continue incrementing as long as RA0 remains pressed.
    BTFSS PORTA, 0            ; If RA0 becomes released (bit = 1), skip branch
         BRA _increment_loop ; If still pressed (bit remains 0), repeat loop
_exit_increment_loop:
    BRA _mainLoop             ; Return to main loop when button is released

;-------------------------------
; Decrement Loop (RA1 pressed)
;-------------------------------
_decrement_loop:
    ; Check if REG10 is zero. Since buttons are active low,
    ; if REG10 is 0 then we want to wrap around to 15.
    MOVF REG10, W
    XORLW 0                 ; Compare with 0
    BTFSC STATUS, 2         ; If REG10 is 0 (zero flag set)
         GOTO _set15       ; then branch to set REG10 to 15
    ; Otherwise, decrement normally.
    DECF REG10, F
    GOTO _after_decrement

_set15:
    MOVLW 15
    MOVWF REG10             ; Set REG10 to 15

_after_decrement:
    CALL _displayDigit      ; Update display
    CALL _delay2Seconds     ; Delay 2 seconds

    ; Continue decrementing as long as RA1 remains pressed.
    BTFSS PORTA, 1          ; If RA1 is released (bit = 1), skip branch
         BRA _decrement_loop ; If still pressed (bit remains 0), repeat loop
_exit_decrement_loop:
    BRA _mainLoop             ; Return to main loop when button is released

;-------------------------------
; 7-Segment Display Subroutine Using Table Pointers
;-------------------------------
_displayDigit:
    CALL _getDigitPattern   ; Get the corresponding 7-segment pattern
    MOVWF LATD              ; Output the pattern to PORTD (7-segment display)
    RETURN                  ; Return from subroutine

;-------------------------------
; Lookup Table Using Table Pointers
;-------------------------------
_getDigitPattern:
    BANKSEL TBLPTRH
    MOVLW HIGH digitTable   ; Load high byte of the table address
    MOVWF TBLPTRH
    MOVLW LOW digitTable    ; Load low byte of the table address
    MOVWF TBLPTRL           ; Store into Table Pointer Low

    BANKSEL REG10
    MOVF REG10, W           ; Move digit index into W
    ADDWF TBLPTRL, F        ; Adjust table pointer to the correct entry

    TBLRD*                  ; Read the pattern from the lookup table
    MOVF TABLAT, W          ; Move the read value into W register
    RETURN                  ; Return with the pattern in W

    ORG 0x100               ; Define start of lookup table in program memory
digitTable:
    DB  0b11011110         ; 0
    DB  0b00000110         ; 1
    DB  0b11101100         ; 2
    DB  0b01101110         ; 3
    DB  0b00110110         ; 4
    DB  0b01111010         ; 5
    DB  0b11111010         ; 6
    DB  0b00001110         ; 7
    DB  0b11111110         ; 8
    DB  0b00111110         ; 9
    DB  0b10111110         ; A
    DB  0b11110010         ; B
    DB  0b11011000         ; C
    DB  0b11100110         ; D
    DB  0b11111000         ; E
    DB  0b10111000         ; F

;-------------------------------
; 2-Second Delay Subroutine
;-------------------------------
_delay2Seconds:
    MOVLW 255               ; Outer loop count (tune as needed)
    MOVWF 20h               ; Store in a temp register

_delayOuter:
    MOVLW 200               ; Inner loop count
    MOVWF 21h               ; Store in another temp register

_delayInner:
    NOP                     ; No operation (adjust timing)
    NOP
    NOP
    DECFSZ 21h, F           ; Decrease inner loop counter
    BRA _delayInner         ; Repeat inner loop

    DECFSZ 20h, F           ; Decrease outer loop counter
    BRA _delayOuter         ; Repeat outer loop

    RETURN

;-------------------------------
; Port D Setup
;-------------------------------
_setupPortD:
    BANKSEL PORTD
    CLRF PORTD              ; Clear PORTD to ensure it's in a known state
    BANKSEL LATD
    CLRF LATD               ; Clear the data latch to prevent unintended outputs
    BANKSEL ANSELD
    CLRF ANSELD             ; Set all PORTD pins as digital (disable analog functions)
    BANKSEL TRISD
    MOVLW 0b00000000        ; Configure all RD0-RD7 pins as outputs
    MOVWF TRISD
    RETURN

;-------------------------------
; Port A Setup
;-------------------------------
_setupPortA:
    BANKSEL PORTA
    CLRF PORTA
    BANKSEL LATA
    CLRF LATA
    BANKSEL ANSELA
    CLRF ANSELA
    BANKSEL TRISA
    BSF TRISA, 0            ; Set RA0 as input
    BSF TRISA, 1            ; Set RA1 as input
    BANKSEL WPUA
    BSF WPUA, 0             ; Enable weak pull-up on RA0 (optional for stable input)
    BSF WPUA, 1             ; Enable weak pull-up on RA1 (optional for stable input)
    RETURN
     
    END                     ; End of program
