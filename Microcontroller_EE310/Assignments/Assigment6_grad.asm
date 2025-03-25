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
    
    
;---------------------
; Title: Keypad-Controlled Countdown and Increment Display Using 7-Segment
;---------------------

; Program behavior summary provided above.

#include "./AssemblyConfig.inc"
#include <xc.inc>

PROCESSOR 18F46K42

;---------------------
; Variable Definitions
;---------------------
REG10       EQU 0x10      ; Holds current count value (0–15)
what_button EQU 0x11      ; Stores the detected keypad value
DELAY_OUTER EQU 0x20      ; Outer loop counter for delay
DELAY_INNER EQU 0x21      ; Inner loop counter for delay

;---------------------
; Program Start
;---------------------
PSECT absdata,abs,ovrld

ORG 0
GOTO _setup               ; Jump to setup routine

ORG 0x0020                ; Start of program code

;---------------------
; Setup Routine
;---------------------
_setup:
    CLRF REG10            ; Clear counter to 0
    RCALL _setupPortD     ; Initialize PORTD for 7-segment output
    RCALL _setupPortB     ; Initialize PORTB for keypad I/O
    CLRF PORTB            ; Clear PORTB
    GOTO mainLoop         ; Start main loop

;---------------------
; Reset to zero and wait again for key "2"
;---------------------
_reset_and_restart:
    CLRF REG10            ; Reset count
    CALL _displayDigit    ; Show "0" on 7-segment
    GOTO mainLoop         ; Wait for key press again

;---------------------
; Main Loop - Wait for key "2" to start
;---------------------
mainLoop:
waitForKey2:
    MOVLW 0b11011110      ; Display "0" on 7-segment
    MOVWF LATD

    RCALL _check_keypad   ; Scan keypad
    MOVF what_button, W
    XORLW 0b11101100      ; Compare with code for key "2"
    BTFSS STATUS, 2
    GOTO waitForKey2      ; If not key "2", keep waiting

    MOVLW 15              ; Load initial countdown value
    MOVWF REG10
    GOTO _decrement_loop  ; Begin countdown

;---------------------
; Decrement Mode (while key "2" is held)
; - Counts down REG10 every 2 seconds
; - Can switch to Increment Mode if key "1" is held
; - Can reset if key "3" is pressed
; - Pauses countdown if no key or wrong key is held
;---------------------
_decrement_loop:
    CALL _displayDigit         ; Display current value (REG10) on 7-segment

wait_for_hold:
    RCALL _check_keypad        ; Check for any key press

    ; --- Switch to Increment Mode if key "1" is pressed ---
    MOVF what_button, W        ; Load detected key
    XORLW 0b00000110           ; Compare with "1" pattern
    BTFSC STATUS, 2            ; If equal (Z bit set), skip next
    RCALL _increment_loop      ; If key "1" held, jump to increment logic

    ; --- Reset everything if key "3" is pressed ---
    MOVF what_button, W
    XORLW 0b01101110           ; Compare with "3" pattern
    BTFSC STATUS, 2
    GOTO _reset_and_restart    ; If match, reset system

    ; --- Pause: wait until key "2" is held ---
    MOVF what_button, W
    XORLW 0b11101100           ; Compare with "2" pattern
    BTFSS STATUS, 2            ; If not equal, skip next
    GOTO wait_for_hold         ; Wait here until "2" is held

    ; --- Continue countdown ---
    CALL _delay2Seconds        ; Wait before next decrement
    DECF REG10, F              ; Decrement value in REG10

    ; --- If count reaches 0, restart main loop ---
    MOVF REG10, W
    XORLW 0                    ; Compare with 0
    BTFSC STATUS, 2            ; If REG10 == 0
    GOTO mainLoop              ; Jump to main loop

    GOTO _decrement_loop       ; Continue countdown loop


;---------------------
; Increment Mode (while key "1" is held)
; - Continuously increments the value in REG10
; - Displays the value on the 7-segment display
; - Increments every 2 seconds while key "1" is held
; - Wraps from 15 (0x0F) back to 0
;---------------------
_increment_loop:
inc_loop_check:
    CALL _displayDigit         ; Show current digit (value in REG10) on display
    CALL _delay2Seconds        ; Wait 2 seconds before next increment

    INCF REG10, F              ; Increment digit value in REG10
    MOVF REG10, W              ; Move updated value to WREG for comparison
    XORLW 16                   ; Compare with 16 (0x10)
    BTFSS STATUS, 2            ; Skip next if REG10 != 16
    GOTO inc_check_release     ; If not equal to 16, skip reset

    CLRF REG10                 ; If REG10 == 16, reset to 0 (wrap around)

inc_check_release:
    RCALL _check_keypad        ; Re-check keypad input (non-destructive call)
    MOVF what_button, W        ; Load detected key pattern
    XORLW 0b00000110           ; Compare with "1" key pattern
    BTFSS STATUS, 2            ; Skip next if still holding key "1"
    RETURN                     ; If released, exit increment loop

    GOTO _increment_loop       ; If still holding key "1", loop back and repeat

;---------------------
; Display REG10 value on 7-segment
;---------------------
_displayDigit:
    CALL _getDigitPattern  ; Get 7-segment pattern for value
    MOVWF LATD             ; Output to 7-segment
    RETURN

;---------------------
; Get 7-segment pattern from lookup table
; Input: REG10 holds digit value (0x00–0x0F)
; Output: WREG holds 7-segment pattern for digit
;---------------------
_getDigitPattern:
    LFSR 0, digitTable     ; Load address of digitTable into FSR0 (File Select Register 0)
    MOVF FSR0H, W          ; Move high byte of FSR0 to WREG
    MOVWF TBLPTRH          ; Set high byte of table pointer to point to lookup table
    MOVF FSR0L, W          ; Move low byte of FSR0 to WREG
    MOVWF TBLPTRL          ; Set low byte of table pointer

    BANKSEL REG10          ; Ensure we are in the correct bank to access REG10
    MOVF REG10, W          ; Move digit value (0–15) from REG10 into WREG
    ADDWF TBLPTRL, F       ; Add digit value to table pointer to get correct entry address
                           ; (Assumes digitTable is at a page boundary, so TBLPTRH doesn't change)

    TBLRD*                 ; Table read: load byte at TBLPTR into TABLAT
    MOVF TABLAT, W         ; Move digit pattern from TABLAT into WREG (output)
    RETURN                 ; Return with 7-segment pattern in WREG

;---------------------
; 7-Segment Digit Lookup Table (0–F in HEX)
; Each byte represents segment bits: abcdefg. Bit 7 may be unused or used for DP.
;---------------------
ORG 0x300                 ; Place table at program memory address 0x300
digitTable:
    DB  0b11011110 ; 0 => Segments a b c d e f (g off)
    DB  0b00000110 ; 1 => Segments c f
    DB  0b11101100 ; 2 => Segments a b d e g
    DB  0b01101110 ; 3 => Segments a c d e g
    DB  0b00110110 ; 4 => Segments b c f g
    DB  0b01111010 ; 5 => Segments a c d f g
    DB  0b11111010 ; 6 => Segments a c d e f g
    DB  0b00001110 ; 7 => Segments a c f
    DB  0b11111110 ; 8 => All segments on (except DP)
    DB  0b00111110 ; 9 => Segments a b c d f g
    DB  0b10111110 ; A => Segments a b c e f g
    DB  0b11110010 ; B => Segments c d e f g
    DB  0b11011000 ; C => Segments a d e f
    DB  0b11100110 ; D => Segments b c d e g
    DB  0b11111000 ; E => Segments a d e f g
    DB  0b10111000 ; F => Segments a e f g

;---------------------
; Setup PORTD for 7-segment output
;---------------------
_setupPortD:
    BANKSEL PORTD          ; Select the bank containing PORTD
    CLRF PORTD             ; Clear PORTD output latch (just in case)

    BANKSEL LATD           ; Select bank for LATD (output latch)
    CLRF LATD              ; Clear LATD to ensure all outputs start LOW

    BANKSEL ANSELD         ; Select bank for ANSELD (analog select register)
    CLRF ANSELD            ; Set all PORTD pins to digital mode

    BANKSEL TRISD          ; Select bank for TRISD (data direction register)
    MOVLW 0x00             ; WREG = 0x00 (all bits 0)
    MOVWF TRISD            ; Set all PORTD pins as outputs (0 = output)
    RETURN                 ; Done with PORTD setup

;---------------------
; Setup PORTB for keypad I/O
;---------------------
_setupPortB:
    BANKSEL PORTB          ; Select the bank containing PORTB
    CLRF PORTB             ; Clear PORTB (optional reset)

    BANKSEL LATB           ; Select bank for LATB (output latch)
    CLRF LATB              ; Clear LATB to ensure known output state

    BANKSEL ANSELB         ; Select bank for ANSELB (analog select register)
    CLRF ANSELB            ; Set all PORTB pins to digital mode

    BANKSEL TRISB          ; Select bank for TRISB (data direction register)
    MOVLW 0b11111000       ; Set RB0–RB2 as outputs (columns), RB3–RB7 as inputs (rows)
                           ; Binary: 11111000 = 0xF8
                           ; Bit value 1 = input (rows), 0 = output (columns)
    MOVWF TRISB            ; Apply direction settings to PORTB
    RETURN                 ; Done with PORTB setup


;---------------------
; Scan keypad for key press (simplified 4x1 keypad)
; Only checks RB3 (one row), assumes 3 columns on RB0–RB2
; Stores corresponding 7-segment pattern into 'what_button'
;---------------------
_check_keypad:
    CLRF what_button        ; Clear the key value (default to "no key pressed")

    ; Clear all columns before starting scan
    BCF LATB, 0             ; Set Column 1 (RB0) LOW
    BCF LATB, 1             ; Set Column 2 (RB1) LOW
    BCF LATB, 2             ; Set Column 3 (RB2) LOW

    ; --- Scan Column 1 ---
    BSF LATB, 0             ; Set Column 1 HIGH (RB0 = 1)
    NOP                     ; Small delay to allow signal to stabilize
    BTFSC PORTB, 3          ; Check if Row 1 (RB3) is HIGH (key pressed)
    MOVLW 0b00000110        ; If so, load 7-segment code for "1" into WREG
    BCF LATB, 0             ; Set Column 1 back to LOW

    ; --- Scan Column 2 ---
    BSF LATB, 1             ; Set Column 2 HIGH (RB1 = 1)
    BTFSC PORTB, 3          ; Check if Row 1 (RB3) is HIGH
    MOVLW 0b11101100        ; If so, load 7-segment code for "2"
    BCF LATB, 1             ; Set Column 2 LOW

    ; --- Scan Column 3 ---
    BSF LATB, 2             ; Set Column 3 HIGH (RB2 = 1)
    BTFSC PORTB, 3          ; Check if Row 1 (RB3) is HIGH
    MOVLW 0b01101110        ; If so, load 7-segment code for "3"
    BCF LATB, 2             ; Set Column 3 LOW

    MOVWF what_button       ; Store result in 'what_button' register
                            ; If no key was pressed, WREG still holds 0
    RETURN                  ; Done scanning


;---------------------
; Delay for ~2 seconds
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



END
