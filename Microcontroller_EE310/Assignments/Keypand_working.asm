;---------------------
; Title: Keypad
;---------------------
; Program Details:
; The purpose of this program is to check which number has been pressed 
; on the keypad and then turn on PORD accordingly, showing the binary
; representation. 
    
; Inputs: RB3. 4. 6. 7
; Outputs: RD0-7 & RB0,1,2
; Setup: The Curiosity Board

; Date: Feb 24, 2024
; File Dependencies / Libraries: It is required to include the
;   AssemblyConfig.inc in the Header Folder
; Compiler: xc8, 2.4
; Author: Farid Farahmand
; Versions:
;       V1.0: Original
; Useful links:
; Keypad: https://www.circuitbasics.com/how-to-set-up-a-keypad-on-an-arduino/ 

;---------------------
; Initialization
;---------------------
#include "./AssemblyConfig.inc"
#include <xc.inc>

;---------------------
; Program Inputs
;---------------------


;---------------------
; Definitions
;---------------------

;---------------------
; Program Constants
;---------------------
what_button EQU		20h
BYTE01	EQU		0xF2		;Data bytes
BYTE02	EQU		0x32
REG00	EQU		0x00		;Data Register addresses
REG01	EQU		0x01
REG02	EQU		0x02
REG10	EQU		0x10

;---------------------
; Program Organization
;---------------------
    PSECT absdata,abs,ovrld        ; Do not change

    ORG          0                ;Reset vector
    GOTO        _setup

    ORG          0020H           ; Begin assembly at 0020H
;---------------------
; Macros
;---------------------
 
 BYTE	MACRO	REGXX
    LFSR	1,REGXX	    ; 1 represents FSR1
    MOVFF	POSTDEC1,REG00
    MOVFF	INDF1,REG00
ENDM

 ;---------------------
; Setup & Main Program
;---------------------   
_setup:
    clrf what_button
    clrf WREG
    RCALL _setupPortD
    RCALL _setupPortB
    clrf PORTB


_main:
    RCALL _check_keypad
    MOVFF   what_button,PORTD
    
;    MOVLW	0xAA
;    MOVWF	REG10
;    BYTE	0x10
    
    GOTO    _main


;-------------------------------------
; Call Functions
;-------------------------------------
_setupPortD:
    BANKSEL	PORTD ;
    CLRF	PORTD ;Init PORTA
    BANKSEL	LATD ;Data Latch
    CLRF	LATD ;
    BANKSEL	ANSELD ;
    CLRF	ANSELD ;digital I/O
    BANKSEL	TRISD ;
    MOVLW	0b00000000 ;Set RD[7:1] as outputs
    MOVWF	TRISD ;and set RD0 as ouput
    RETURN

_setupPortB:
    BANKSEL	PORTB ;
    CLRF	PORTB ;Init PORTB
    BANKSEL	LATB ;Data Latch
    CLRF	LATB ;
    BANKSEL	ANSELB ;
    CLRF	ANSELB ;digital I/O
    BANKSEL	TRISB ;
    MOVLW	0b11111000 ;
    MOVWF	TRISB ;
    RETURN

_check_keypad:
    clrf what_button       ; Clear previous button value
    ; Set all columns LOW
    bcf LATB, 0
    bcf LATB, 1
    bcf LATB, 2

    ;--------------------------------
    ; Scan Column 1 (RB0 = HIGH)
    ;--------------------------------
    bsf LATB, 0            ; Set column 1 HIGH
    nop                    ; Small delay (optional, for signal settling)
    
    btfsc PORTB, 3         ; RB3 = Row 1 → Key "1"
    movlw 0b00000110       ; Key "1"
    btfsc PORTB, 4         ; RB4 = Row 2 → Key "4"
    movlw 0b00110110       ; Key "4"
    btfsc PORTB, 6         ; RB6 = Row 3 → Key "7"
    movlw 0b00001110       ; Key "7"
    btfsc PORTB, 7         ; RB7 = Row 4 → Key "*"
    movlw 0b00001110       ; Key "*"

    bcf LATB, 0            ; Pull column LOW

    ;--------------------------------
    ; Scan Column 2 (RB1 = HIGH)
    ;--------------------------------
    bsf LATB, 1

    btfsc PORTB, 3         ; Key "2"
    movlw 0b11101100       ; Key "2"
    btfsc PORTB, 4         ; Key "5"
    movlw 0b01111010       ; Key "5"
    btfsc PORTB, 6         ; Key "0"
    movlw 0b11011110       ; Key "0"
    btfsc PORTB, 7         ; Key "8"
    movlw 0b11111110       ; Key "8"

    bcf LATB, 1

    ;--------------------------------
    ; Scan Column 3 (RB2 = HIGH)
    ;--------------------------------
    bsf LATB, 2

    btfsc PORTB, 3         ; Key "3"
    movlw 0b01101110       ; Key "3"
    btfsc PORTB, 4         ; Key "6"
    movlw 0b11111010       ; Key "6"
    btfsc PORTB, 6         ; Key "#"
    movlw 0b00110000       ; Key "#"
    btfsc PORTB, 7         ; Key "9"
    movlw 0b00111110       ; Key "9"

    bcf LATB, 2

    ;--------------------------------
    ; Store final value
    ;--------------------------------
    movwf what_button
    return


    END
