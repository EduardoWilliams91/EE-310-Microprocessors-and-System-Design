;---------------------
; Config Bits
;---------------------
CONFIG  FEXTOSC = LP

;---------------------
; Initialization
;---------------------
#include "./AssemblyConfig.inc"
#include <xc.inc> 
PROCESSOR 18F46K42
PSECT absdata,abs,ovrld   

;---------------------
; Program Inputs
;---------------------
; place your inputs here 

;---------------------
; Define EEPROM Temp Registers
;---------------------
EE_ADDRL EQU 0x30
EE_DATA  EQU 0x31
EE_ADDRH EQU 0x32 
REG35    EQU 0x35  ; Destination register for EEPROM read

;---------------------
; Bit Definitions
;---------------------
#define RD 0
#define WR 1
#define WREN 2
#define GIE 7

;---------------------
; MACRO DEFINITIONS
;---------------------

; --- EEPROM WRITE MACRO ---
EE_WRT MACRO
    BANKSEL NVMCON1
    CLRF NVMCON1             ; Disable any previous settings

    MOVF EE_ADDRL, W        
    MOVWF NVMADRL            ; Load EEPROM address

    MOVF EE_DATA, W         
    MOVWF NVMDAT             ; Load data to write

    BSF NVMCON1, WREN        ; Enable EEPROM write

    BCF INTCON0, GIE         ; Disable global interrupts

    MOVLW 0x55
    MOVWF NVMCON2            ; Unlock sequence part 1

    MOVLW 0xAA
    MOVWF NVMCON2            ; Unlock sequence part 2

    BSF NVMCON1, WR          ; Begin EEPROM write

    BSF INTCON0, GIE         ; Re-enable global interrupts
ENDM

; --- EEPROM READ TO REG35 MACRO ---
EE_READ_TO_REG35 MACRO
    BANKSEL NVMCON1
    CLRF    NVMCON1          ; Clear control bits

    MOVF    EE_ADDRL, W
    MOVWF   NVMADRL          ; Set EEPROM address

    BSF     NVMCON1, RD      ; Start EEPROM read
    NOP                      ; Required delay after RD
    NOP

    MOVF    NVMDAT, W
    MOVWF   REG35            ; Store result in REG35
ENDM

;---------------------
; MAIN PROGRAM 
;---------------------
        ORG     0x20H                   
        GOTO    _MAIN
    
_MAIN:
        ; --- Write to EEPROM ---
        MOVLW   0x12              ; EEPROM address
        MOVWF   EE_ADDRL
        MOVLW   'd'               ; Value to write
        MOVWF   EE_DATA
        EE_WRT                    ; Call EEPROM write macro

        ; --- Read from EEPROM into REG35 ---
        MOVLW   0x12              ; Address to read from
        MOVWF   EE_ADDRL
        EE_READ_TO_REG35          ; Read EEPROM into REG35

        ; Infinite loop (or continue with your logic)
WAIT:   GOTO WAIT

        END
