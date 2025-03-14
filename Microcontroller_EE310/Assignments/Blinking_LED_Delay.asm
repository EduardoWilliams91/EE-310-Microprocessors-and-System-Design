;---------------------
; Title: "PIC18F46K42: Switch-Controlled LED Toggle with Assembly Delay Loop"
;---------------------
; Program Details:
;This assembly program for the PIC18F46K42 toggles two LEDs (RD0 & RB0) when a 
;switch (RD1) is pressed. It initializes PORTD and PORTB, configures the switch as 
;an input, and LEDs as outputs. A software delay loop controls the toggle speed. 
;The program runs in an infinite loop, waiting for user input to trigger LED toggling

; Inputs:
;LED0 (RD0) Toggles ON/OFF 
; Outputs: 
;Inner_loop   
;Outer_loop  
; Date: March 13, 2025
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
; Program Inputs
;---------------------
Inner_loop  equ 10 ;255  ; in decimal
Outer_loop  equ 10 ;131

;---------------------
; Program Constants
;---------------------
REG10   equ     10h   ; Register for inner loop
REG11   equ     11h   ; Register for outer loop

;---------------------
; Definitions
;---------------------
#define SWITCH    PORTD,1  
#define LED0      PORTD,0
#define LED1      PORTB,0
   
;---------------------
; Main Program
;---------------------
    PSECT absdata,abs,ovrld        ; Do not change
    
    ORG          0                ; Reset vector
    GOTO        _initialization

    ORG          0020H            ; Begin assembly at 0020H

_initialization: 
    RCALL _setupPortD
    RCALL _setupPortB
    
_main:
    BTG      LED0    ; Toggle LED0
    BTG      LED1    ; Toggle LED1
    CALL     loopDelay 
    BRA      _main   ; Repeat forever
    
;---------------------
; Delay Subroutine
;---------------------
loopDelay: 
    MOVLW    Inner_loop
    MOVWF    REG10
    MOVLW    Outer_loop
    MOVWF    REG11
_loop1:
    DECFSZ   REG10,1  ; Decrease inner loop counter
    GOTO     _loop1
    MOVLW    Inner_loop  ; Reset inner loop counter
    MOVWF    REG10
    DECFSZ   REG11,1  ; Decrease outer loop counter
    GOTO     _loop1
    RETURN

;---------------------
; Port D Setup
;---------------------
_setupPortD:
    BANKSEL  PORTD
    CLRF     PORTD    ; Clear PORTD
    BANKSEL  LATD
    CLRF     LATD     ; Clear Data Latch
    BANKSEL  ANSELD
    CLRF     ANSELD   ; Set digital I/O
    BANKSEL  TRISD
    MOVLW    0b11111110  ; Set RD[7:1] as inputs, RD0 as output
    MOVWF    TRISD
    RETURN

;---------------------
; Port B Setup
;---------------------
_setupPortB:
    BANKSEL  PORTB
    CLRF     PORTB    ; Clear PORTB
    BANKSEL  LATB
    CLRF     LATB     ; Clear Data Latch
    BANKSEL  ANSELB
    CLRF     ANSELB   ; Set digital I/O
    BANKSEL  TRISB
    MOVLW    0b11111110  ; Set RB[7:1] as inputs, RB0 as output
    MOVWF    TRISB
    RETURN    

    END
