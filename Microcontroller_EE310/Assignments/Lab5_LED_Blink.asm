;---------------------
; Title: Waveform Generator with Delay
;---------------------
; Program Details:
; The purpose of this program is to demonstrate how to call a delay function. 

; Inputs: Inner_loop ,Outer_loop 
; Outputs: PORTD
; Date: March 5, 2025
; File Dependencies / Libraries: None 
; Compiler: xc8, 2.4
; Author: Eduardo Williams 
; Versions:
;       V1.3: Changes the loop size
; Useful links: 
;       Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
;       PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
;       List of Instrcutions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 


;---------------------
; Initialization - make sure the path is correct
;---------------------
#include ".\AssemblyConfig.inc"
#include "C:\Users\Willi\MPLABXProjects\MyFirstAssembly_MPLAB.X\AssemblyConfig.inc"

#include <xc.inc>; matching words to registers

;********************************************************************
; Program: Blink LED with Delay using Nested Loops
; Description: This program toggles an LED on PORTD with a delay.
; The delay is controlled using two nested loops.
;********************************************************************

;---------------------
; Program Inputs
;---------------------
Inner_loop  equ 5         ; Inner loop delay count (decimal)
Outer_loop  equ 5         ; Outer loop delay count (decimal)

;---------------------
; Program Constants
;---------------------
REG10   equ     10h       ; Register for inner loop counter
REG11   equ     11h       ; Register for outer loop counter
REG01   equ     1h        ; Register to store toggle value

;---------------------
; Pin Definitions
;---------------------
#define SWITCH    LATD,2   ; Switch connected to LATD,2
#define LED0      PORTD,0  ; LED0 connected to PORTD,0
#define LED1      PORTD,1  ; LED1 connected to PORTD,1

;---------------------
; Main Program
;---------------------
    PSECT absdata,abs,ovrld        ; Define absolute section (Do not change)
    
    ORG          0                  ; Reset vector (starting address)
    GOTO        _start1              ; Jump to main program

    ORG          0020H               ; Start assembling code at 0020H

;---------------------------------------------------------------
; Initialization Section
;---------------------------------------------------------------
_start1:
    MOVLW       0b11111100           ; Load WREG with binary 11111100
    MOVWF       TRISD,0              ; Configure PORTD (RD0 & RD1 as outputs)
    MOVLW	0b11111101
    MOVWF       REG01,0              ; Initialize REG01 with the same value

;---------------------------------------------------------------
; Main Loop: Toggles LED with delay
;---------------------------------------------------------------
_onoff:
    MOVFF       REG01,PORTD          ; Move REG01 value to PORTD (toggle LED state)
    MOVLW       Inner_loop            ; Load WREG with Inner_loop constant (5)
    MOVWF       REG10                 ; Store in REG10 (inner loop counter)
    MOVLW       Outer_loop            ; Load WREG with Outer_loop constant (5)
    MOVWF       REG11                 ; Store in REG11 (outer loop counter)

;---------------------------------------------------------------
; Delay Implementation: Nested Loop
;---------------------------------------------------------------
_loop1:
    DECF        REG10,1               ; Decrement REG10 (inner loop counter)
    BNZ         _loop1                ; If REG10 is not zero, repeat inner loop
    DECF        REG11,1               ; Decrement REG11 (outer loop counter)
    BNZ         _loop1                ; If REG11 is not zero, repeat outer loop

;---------------------------------------------------------------
; Toggle Register Value and Repeat
;---------------------------------------------------------------
    COMF        REG01,1               ; Complement (negate) REG01 (toggle LED state)
    BRA         _onoff                ; Repeat the loop (infinite loop)

;---------------------------------------------------------------
; End of Program
;---------------------------------------------------------------
END
