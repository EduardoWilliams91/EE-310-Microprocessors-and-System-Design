#include "./AssemblyConfig.inc"
#include <xc.inc>

PROCESSOR 18F46K42

;--------------------------------------------------------------------------
; Register Definitions (located in the ACCESS bank)
;--------------------------------------------------------------------------
REF_TEMP     EQU 0x20  ; Reference temperature register
MEAS_TEMP    EQU 0x21  ; Measured temperature register
CONT_REG     EQU 0x22  ; Control register (stores system state)

; Temporary registers for conversion (choose registers not in use)
TEMP_VAL     EQU 0x23  ; Temporary storage for conversion
QUOTIENT     EQU 0x24  ; Holds quotient during division
REMAINDER    EQU 0x25  ; Holds remainder after division

; Storage for decimal equivalent values (placed in ACCESS)
; For refTemp: ones, tens, hundreds
REF_TEMP_1   EQU 0x40  ; Lower byte (ones)
REF_TEMP_2   EQU 0x41  ; Middle byte (tens)
REF_TEMP_3   EQU 0x42  ; Upper byte (hundreds)

; For measuredTemp: ones, tens, hundreds
MEAS_TEMP_1  EQU 0x50  ; Lower byte (ones)
MEAS_TEMP_2  EQU 0x51  ; Middle byte (tens)
MEAS_TEMP_3  EQU 0x52  ; Upper byte (hundreds)

;--------------------------------------------------------------------------
; Program Constraints:
;   refTemp can be between +10°C and +50°C
;   measuredTemp can be between -10°C and +60°C (can be negative)
;   Only PORTD can be used for output
;
; Program Constants
REG10 EQU 0x10  ; in HEX
REG11 EQU 0x11  ; in HEX
REG01 EQU 0x01  ; in HEX

; Definitions
#define measuredTempInput  -95  ; Input value for measured temperature
#define refTempInput       -65  ; Input value for reference temperature

;--------------------------------------------------------------------------
; Main Program
;--------------------------------------------------------------------------
    PSECT absdata,abs,ovrld     ; Uninitialized data section for PIC18
    ORG 0x20                    ; Start of program memory from register 0x20
    GOTO MAIN                   ; Jump to main program

MAIN:
    ;----------------------------------------------------------------------
    ; Clear STATUS Register at Startup
    ;----------------------------------------------------------------------
    CLRF   STATUS, 0      ; Clear STATUS register (Z, C, DC flags)

    ;----------------------------------------------------------------------
    ; Clear PORTD outputs and the control register
    ;----------------------------------------------------------------------
    CLRF   PORTD, 1       ; Clear PORTD (all outputs OFF)
    CLRF   CONT_REG, 0    ; Clear control register (in ACCESS bank)

    ;----------------------------------------------------------------------
    ; Store temperature inputs into registers
    ;----------------------------------------------------------------------
    MOVLW  measuredTempInput
    MOVWF  MEAS_TEMP, 0   ; Store measured temperature input

    MOVLW  refTempInput
    MOVWF  REF_TEMP, 0    ; Store reference temperature input

    ;----------------------------------------------------------------------
    ; Convert refTemp to its absolute value before decimal conversion
    ;----------------------------------------------------------------------
    MOVFF  REF_TEMP, TEMP_VAL
    BTFSS  TEMP_VAL, 7     ; If sign bit = 1 (negative), convert to absolute value
    GOTO   REF_CONV_SKIP_NEG
    COMF   TEMP_VAL, F     ; Two's complement to get absolute value
    INCF   TEMP_VAL, F

REF_CONV_SKIP_NEG:
    CALL   EXTRACT_DECIMAL
    MOVWF  REF_TEMP_1      ; Store ones digit
    MOVF   QUOTIENT, W
    MOVWF  TEMP_VAL
    CALL   EXTRACT_DECIMAL
    MOVWF  REF_TEMP_2      ; Store tens digit
    MOVF   QUOTIENT, W
    MOVWF  TEMP_VAL
    CALL   EXTRACT_DECIMAL
    MOVWF  REF_TEMP_3      ; Store hundreds digit

    ;----------------------------------------------------------------------
    ; Convert measuredTemp to its absolute value before decimal conversion
    ;----------------------------------------------------------------------
    MOVFF  MEAS_TEMP, TEMP_VAL
    BTFSS  TEMP_VAL, 7     ; If sign bit = 1 (negative), convert to absolute value
    GOTO   MEAS_CONV_SKIP_NEG
    COMF   TEMP_VAL, F     ; Two's complement to get absolute value
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
    ; Loop indefinitely (Replace with further logic if needed)
    ;----------------------------------------------------------------------
END_PROGRAM:
    GOTO   END_PROGRAM      ; Infinite loop

;--------------------------------------------------------------------------
; EXTRACT_DECIMAL: Convert HEX to Decimal by repeatedly subtracting 10
;   Input:  TEMP_VAL (value to convert)
;   Output: W = remainder (0..9), QUOTIENT = integer quotient
;--------------------------------------------------------------------------
EXTRACT_DECIMAL:
    CLRF   QUOTIENT         ; Clear quotient
    CLRF   REMAINDER        ; Clear remainder

DIV_LOOP:
    MOVF   TEMP_VAL, W      ; W = TEMP_VAL
    SUBLW  10               ; W = 10 - TEMP_VAL
    BTFSC  STATUS, 0        ; If carry=1 => TEMP_VAL < 10 (done)
    GOTO   DONE_DIV
    ; Subtract 10 from TEMP_VAL
    MOVLW  10
    SUBWF  TEMP_VAL, F      ; TEMP_VAL = TEMP_VAL - 10
    INCF   QUOTIENT, F      ; QUOTIENT++
    GOTO   DIV_LOOP

DONE_DIV:
    MOVF   TEMP_VAL, W      ; Store remainder in W (return value)
    RETURN

    END
