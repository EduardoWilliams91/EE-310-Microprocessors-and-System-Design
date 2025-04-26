// === CONFIG BITS ===
#pragma config FEXTOSC = LP     
#pragma config RSTOSC = EXTOSC  
#pragma config CLKOUTEN = OFF   
#pragma config PR1WAY = ON      
#pragma config CSWEN = ON       
#pragma config FCMEN = ON       
#pragma config MCLRE = EXTMCLR  
#pragma config PWRTS = PWRT_OFF 
#pragma config MVECEN = ON      
#pragma config IVT1WAY = ON     
#pragma config LPBOREN = OFF    
#pragma config BOREN = SBORDIS  
#pragma config BORV = VBOR_2P45 
#pragma config ZCD = OFF        
#pragma config PPS1WAY = ON     
#pragma config STVREN = ON      
#pragma config DEBUG = OFF      
#pragma config XINST = OFF      
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF       
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC      
#pragma config BBSIZE = BBSIZE_512
#pragma config BBEN = OFF       
#pragma config SAFEN = OFF      
#pragma config WRTAPP = OFF     
#pragma config WRTB = OFF       
#pragma config WRTC = OFF       
#pragma config WRTD = OFF       
#pragma config WRTSAF = OFF     
#pragma config LVP = ON         
#pragma config CP = OFF         

#include <xc.h> 
#include "PWM.h"
#include "configwords.h"
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#define _XTAL_FREQ 4000000      // Fosc frequency for _delay() functions
#define FCY (_XTAL_FREQ / 4)     // Instruction cycle frequency (1 MHz)

#define myLED PORTBbits.RB0

// Define the desired duty cycle percentage here
#define DESIRED_DUTY_CYCLE_PERCENT 10.0

// Global Variables
uint16_t checkdutyCycle;
char preScale;
_Bool pwmStatus;

void main(void)
{
    // Clock Initialization
    OSCSTATbits.HFOR = 1;          // Enable HFINTOSC
    OSCFRQ = 0x02;                 // Set internal oscillator to 4 MHz

    // I/O Configuration
    ANSELB = 0x00;                 // Set PORTB as digital
    TRISB = 0x00;                  // Set PORTB as output
    PORTB = 0x00;                  // Clear PORTB outputs

    // Timer2 and PWM Module Initialization
    TMR2_Initialize();
    TMR2_StartTimer();
    PWM_Output_D8_Enable();
    PWM2_Initialize();

    // --- Dynamic Duty Cycle Setup ---
    // Calculate duty register value dynamically based on desired percentage
    uint16_t calculatedDuty = (uint16_t)((DESIRED_DUTY_CYCLE_PERCENT / 100.0) * (4 * (T2PR + 1)));
    PWM2_LoadDutyValue(calculatedDuty);

    // Optional: Calculate actual duty cycle for validation
    checkdutyCycle = (uint16_t)((100UL * calculatedDuty) / (4 * (T2PR + 1)));
    preScale = ((T2CON >> 4) & (0x0F)); // Prescaler setting readback (for diagnostics)

    while (1)
    {
        pwmStatus = PWM2_OutputStatusGet();
        PORTBbits.RB2 = pwmStatus;

        if (PIR4bits.TMR2IF == 1)
        {
            PIR4bits.TMR2IF = 0;
            myLED ^= 1; // Toggle LED on Timer2 overflow
        }
    }
}
