;---------------------
; Title: Keypad-Controlled Countdown and Increment Display Using 7-Segment
;---------------------
; Program Details:
; This program implements a simple user interface using a 4x1 keypad and a single
;7-segment display controlled by a PIC18F46K42 microcontroller. Upon powering up, 
;the system waits for the user to press key "2", which starts a countdown from 
;15 to 0, decrementing every 2 seconds. During the countdown:
;
;Pressing key "1" will pause the countdown and enter an increment mode, increasing 
;the count every 2 seconds. If the count exceeds 15, it wraps back to 0.
;
;Releasing key "1" resumes countdown.
;
;Pressing key "3" resets the system to zero and waits for "2" again.

; Inputs: Enters increment mode during countdown, key 1, Starts or resumes decrement, key 2
    ;Resets the counter to 0, key 3
; Outputs: Displays the current value (0–15 in hex)
    
; Date:  3/23/2025
; File Dependencies / Libraries: None 
; Compiler: xc8, 3.0
; Author: Eduardo Williams 
; Versions:
;       V1.0: First Version 
; Useful links: 
;       Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
;       PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
;       GITHUB: https://github.com/EduardoWilliams91/EE-310-Microprocessors-and-System-Design/tree/main/Microcontroller_EE310
    
    
    
#include "./AssemblyConfig.inc"
#include <xc.inc>

PROCESSOR 18F46K42

;---------------------
; Variable Definitions
;---------------------
REG10       EQU 0x10      ; Digit storage
what_button EQU 0x11      ; Stores keypad value
DELAY_OUTER EQU 0x20      ; Delay outer loop
DELAY_INNER EQU 0x21      ; Delay inner loop

;---------------------
; Program Start
;---------------------
    PSECT absdata,abs,ovrld

    ORG 0
    GOTO _setup

    ORG 0x0020

;---------------------
; Setup Routine
;---------------------
_setup:
    CLRF REG10
    RCALL _setupPortD
    RCALL _setupPortB
    CLRF PORTB
    GOTO mainLoop

;---------------------
; Reset and Go Back to Wait
;---------------------
_reset_and_restart:
    CLRF REG10
    CALL _displayDigit
    GOTO mainLoop

;---------------------
; Main Program Loop
;---------------------
mainLoop:
waitForKey2:
    ; Always display "0" while waiting
    MOVLW 0b11011110      ; 7-segment code for "0"
    MOVWF LATD

    RCALL _check_keypad

    MOVF what_button, W
    XORLW 0b11101100      ; Check if key "2" (now starts countdown)
    BTFSS STATUS, 2
    GOTO waitForKey2      ; Keep waiting

    MOVLW 15
    MOVWF REG10
    GOTO _decrement_loop

;---------------------
; Decrement Countdown Loop
;---------------------
_decrement_loop:
    CALL _displayDigit

wait_for_hold:
    RCALL _check_keypad

    ; Check if "1" is pressed → enter increment loop
    MOVF what_button, W
    XORLW 0b00000110     ; "1"
    BTFSC STATUS, 2
    RCALL _increment_loop

    ; Check if "3" is pressed → reset and go to mainLoop
    MOVF what_button, W
    XORLW 0b01101110     ; "3"
    BTFSC STATUS, 2
    GOTO _reset_and_restart

    ; Check if "2" is held → resume countdown
    MOVF what_button, W
    XORLW 0b11101100     ; "2"
    BTFSS STATUS, 2
    GOTO wait_for_hold

    ; Continue countdown
    CALL _delay2Seconds
    DECF REG10, F

    ; If reached 0, go back to wait state
    MOVF REG10, W
    XORLW 0
    BTFSC STATUS, 2
    GOTO mainLoop

    GOTO _decrement_loop

;---------------------
; Increment Loop for Key "1"
;---------------------
_increment_loop:
    ; Stay here while key "1" is held
inc_loop_check:
    CALL _displayDigit
    CALL _delay2Seconds

    ; Always increment
    INCF REG10, F
    MOVF REG10, W
    XORLW 16
    BTFSS STATUS, 2
    GOTO inc_check_release
    CLRF REG10             ; Wrap around to 0

inc_check_release:
    RCALL _check_keypad
    MOVF what_button, W
    XORLW 0b00000110     ; is key "1" still held?
    BTFSS STATUS, 2
    RETURN               ; Exit loop if released

    GOTO _increment_loop

;---------------------
; Display Current REG10 on 7-segment
;---------------------
_displayDigit:
    CALL _getDigitPattern
    MOVWF LATD
    RETURN

;---------------------
; Get 7-segment Pattern for Value in REG10
;---------------------
_getDigitPattern:
    LFSR 0, digitTable
    MOVF FSR0H, W
    MOVWF TBLPTRH
    MOVF FSR0L, W
    MOVWF TBLPTRL

    BANKSEL REG10
    MOVF REG10, W
    ADDWF TBLPTRL, F

    TBLRD*
    MOVF TABLAT, W
    RETURN

;---------------------
; Setup PORTD
;---------------------
_setupPortD:
    BANKSEL PORTD
    CLRF PORTD
    BANKSEL LATD
    CLRF LATD
    BANKSEL ANSELD
    CLRF ANSELD
    BANKSEL TRISD
    MOVLW 0x00
    MOVWF TRISD
    RETURN

;---------------------
; Setup PORTB
;---------------------
_setupPortB:
    BANKSEL PORTB
    CLRF PORTB
    BANKSEL LATB
    CLRF LATB
    BANKSEL ANSELB
    CLRF ANSELB
    BANKSEL TRISB
    MOVLW 0b11111000     ; RB0–RB2 = output (columns), RB3–RB7 = input (rows)
    MOVWF TRISB
    RETURN

;---------------------
; Keypad Scanning
;---------------------
_check_keypad:
    CLRF what_button

    ; Clear columns
    BCF LATB, 0
    BCF LATB, 1
    BCF LATB, 2

    ; --- Column 1 ---
    BSF LATB, 0
    NOP
    BTFSC PORTB, 3
    MOVLW 0b00000110     ; "1"
    BCF LATB, 0

    ; --- Column 2 ---
    BSF LATB, 1
    BTFSC PORTB, 3
    MOVLW 0b11101100     ; "2"
    BCF LATB, 1

    ; --- Column 3 ---
    BSF LATB, 2
    BTFSC PORTB, 3
    MOVLW 0b01101110     ; "3"
    BCF LATB, 2

    MOVWF what_button
    RETURN

;---------------------
; Delay for 2 seconds
;---------------------
_delay2Seconds:
    MOVLW  200
    MOVWF DELAY_OUTER
outer_loop:
    MOVLW 255
    MOVWF DELAY_INNER
inner_loop:
    NOP
    NOP
    DECFSZ DELAY_INNER, F
    GOTO inner_loop
    DECFSZ DELAY_OUTER, F
    GOTO outer_loop
    RETURN

;---------------------
; 7-segment digit lookup table
;---------------------
  ORG 0x300
digitTable:
    DB  0b11011110 ; 0
    DB  0b00000110 ; 1
    DB  0b11101100 ; 2
    DB  0b01101110 ; 3
    DB  0b00110110 ; 4
    DB  0b01111010 ; 5
    DB  0b11111010 ; 6
    DB  0b00001110 ; 7
    DB  0b11111110 ; 8
    DB  0b00111110 ; 9
    DB  0b10111110 ; A
    DB  0b11110010 ; B
    DB  0b11011000 ; C
    DB  0b11100110 ; D
    DB  0b11111000 ; E
    DB  0b10111000 ; F

    END
