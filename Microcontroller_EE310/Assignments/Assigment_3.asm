;---------------------
; Title: Temperature-Based Heating and Cooling Control System (PIC18F46K42 - Assembly)
;---------------------
; Program Details:
; This assembly program for the PIC18F46K42 microcontroller reads a measured temperature 
;and a reference temperature, converts them to their decimal equivalents, and then 
;determines whether to turn on a heating or cooling system based on a predefined temperature range. 

; Inputs: measuredTempInput, refTempInput
; Outputs: PORTD.1, PORTD.2
; Date: March 5, 2025
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

;--------------------------------------------------------------------------
; Register Definitions (located in the ACCESS bank)
;--------------------------------------------------------------------------
REF_TEMP     EQU 0x22  ; Reference temperature register
MEAS_TEMP    EQU 0x21  ; Measured temperature register
CONT_REG     EQU 0x20  ; Control register (stores system state)

; Temporary registers for conversion (choose registers not in use)
TEMP_VAL     EQU 0x23  ; Temporary storage for conversion
QUOTIENT     EQU 0x24  ; Holds quotient during division
REMAINDER    EQU 0x25  ; Holds remainder after division

; Storage for decimal equivalent values (placed in ACCESS)
    
; For refTemp: ones, tens, hundreds
REF_TEMP_1   EQU 0x72  
REF_TEMP_2   EQU 0x71  
REF_TEMP_3   EQU 0x70  

; For measuredTemp: ones, tens, hundreds
MEAS_TEMP_1  EQU 0x62  
MEAS_TEMP_2  EQU 0x61  
MEAS_TEMP_3  EQU 0x60  

;--------------------------------------------------------------------------
; Program Constraints:
;   refTemp can be between +10°C and +50°C
;   measuredTemp can be between -10°C and +60°C (can be negative)
;   Only PORTD can be used for output
;
; Program Constants
REG10 EQU 0x10  
REG11 EQU 0x11  
REG01 EQU 0x01  

; Definitions

#define refTempInput       15  ; Input value for reference temperature
#define measuredTempInput  -5  ; Input value for measured temperature
;--------------------------------------------------------------------------
; Main Program
;--------------------------------------------------------------------------
    PSECT absdata,abs,ovrld  
    ORG 0x20                 
    GOTO MAIN               

MAIN:
    ;----------------------------------------------------------------------
    ; Clear STATUS Register at Startup
    ;----------------------------------------------------------------------
    CLRF   STATUS, 0      

    ;----------------------------------------------------------------------
    ; Clear PORTD outputs and the control register
    ;----------------------------------------------------------------------
    CLRF   PORTD, 1       
    CLRF   CONT_REG, 0    

    ;----------------------------------------------------------------------
    ; Store temperature inputs into registers
    ;----------------------------------------------------------------------
    MOVLW  measuredTempInput
    MOVWF  MEAS_TEMP, 0   

    MOVLW  refTempInput
    MOVWF  REF_TEMP, 0    

    ;----------------------------------------------------------------------
    ; Convert refTemp to absolute value before decimal conversion
    ;----------------------------------------------------------------------
    MOVFF  REF_TEMP, TEMP_VAL
    BTFSS  TEMP_VAL, 7     
    GOTO   REF_CONV_SKIP_NEG
    COMF   TEMP_VAL, F     
    INCF   TEMP_VAL, F     

REF_CONV_SKIP_NEG:
    CALL   EXTRACT_DECIMAL
    MOVWF  REF_TEMP_1      
    MOVF   QUOTIENT, W
    MOVWF  TEMP_VAL
    CALL   EXTRACT_DECIMAL
    MOVWF  REF_TEMP_2      
    MOVF   QUOTIENT, W
    MOVWF  TEMP_VAL
    CALL   EXTRACT_DECIMAL
    MOVWF  REF_TEMP_3      

    ;----------------------------------------------------------------------
    ; Convert measuredTemp to absolute value before decimal conversion
    ;----------------------------------------------------------------------
    MOVFF  MEAS_TEMP, TEMP_VAL
    BTFSS  TEMP_VAL, 7     
    GOTO   MEAS_CONV_SKIP_NEG
    COMF   TEMP_VAL, F     
    INCF   TEMP_VAL, F     

MEAS_CONV_SKIP_NEG:
    CALL   EXTRACT_DECIMAL
    MOVWF  MEAS_TEMP_1
    MOVF   QUOTIENT, W
    MOVWF  TEMP_VAL
    CALL   EXTRACT_DECIMAL
    MOVWF  MEAS_TEMP_2
    MOVF   QUOTIENT, W
    MOVWF  TEMP_VAL
    CALL   EXTRACT_DECIMAL
    MOVWF  MEAS_TEMP_3

    ;----------------------------------------------------------------------
    ; Temperature Range Checks
    ;----------------------------------------------------------------------
;    ; Check refTemp: >=10 ?
;    MOVLW  10
;    SUBWF  REF_TEMP, 0
;    BTFSS  STATUS, 0        
;    GOTO   NO_ACTION
;
;    ; Check refTemp: <=50 ?
;    MOVLW  50
;    SUBWF  REF_TEMP, 0
;    BTFSC  STATUS, 0        
;    GOTO   NO_ACTION
;
;    ; Check measuredTemp: >= -10 ?
;    MOVLW  0xF6            
;    SUBWF  MEAS_TEMP, 0    
;    BTFSC  STATUS, 0       
;    GOTO   NO_ACTION    
;
;    ; Check measuredTemp: <=60 ?
;    MOVLW  60
;    SUBWF  MEAS_TEMP, 0
;    BTFSC  STATUS, 0       
;    GOTO   NO_ACTION

    ;----------------------------------------------------------------------
    ; Compare measuredTemp with refTemp  
    ;----------------------------------------------------------------------  

    _CHECK_NEGATIVE:
    BTFSC   MEAS_TEMP, 7 ; Skip if bit 7 is 0 (measuredTemp is positive)
    NEGF    MEAS_TEMP    ; If bit 7 is 1, perform two's complement negation
    
_COMPARE:
    MOVF    MEAS_TEMP, W ; Load measuredTemp into WREG
    CPFSEQ  REF_TEMP	    ; Skip next if measuredTemp == refTemp
    GOTO    _CHECK_GREATER  ; Check if measuredTemp > refTemp
    GOTO    NO_ACTION	    ; if measuredTemp == refTemp
   
_CHECK_GREATER:
    CPFSLT  REF_TEMP	    ; Skip next if measuredTemp > refTemp
    GOTO    HEATING_ON	    ; if measuredTemp < refTemp
    GOTO    TURN_ON_COOLING	    ; if measuredTemp > refTemp
    
;------------------------------------------------------------------------------------
    
TURN_ON_COOLING:
    BSF    PORTD, 2       ; Turn ON Cooling
    BCF    PORTD, 1       ; Ensure Heating is OFF
    MOVLW  0x02
    MOVWF  CONT_REG       ; Set CONT_REG = 2
    GOTO   END_PROGRAM

HEATING_ON:
    BSF    PORTD, 1       ; Turn ON Heating
    BCF    PORTD, 2       ; Ensure Cooling is OFF
    MOVLW  0x01
    MOVWF  CONT_REG       ; Set CONT_REG = 1
    GOTO   END_PROGRAM

NO_ACTION:
    CLRF   CONT_REG       ; Clear CONT_REG to 0
    BCF    PORTD, 1       ; Ensure Heating is OFF
    BCF    PORTD, 2       ; Ensure Cooling is OFF
    GOTO   END_PROGRAM

END_PROGRAM:
    GOTO   END_PROGRAM  

;----------------------------------------------------------------------
; EXTRACT_DECIMAL: Convert HEX to Decimal by repeatedly subtracting 10
;----------------------------------------------------------------------
EXTRACT_DECIMAL:
    CLRF   QUOTIENT      
    CLRF   REMAINDER     

DIV_LOOP:
    MOVLW  10
    SUBWF  TEMP_VAL, W   
    BTFSS  STATUS, 0     
    GOTO   DONE_DIV
    
    MOVLW  10
    SUBWF  TEMP_VAL, F   
    INCF   QUOTIENT, F   
    GOTO   DIV_LOOP

DONE_DIV:
    MOVF   TEMP_VAL, W   
    MOVWF  REMAINDER     
    RETURN

    END
