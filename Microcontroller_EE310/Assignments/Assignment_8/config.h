#ifndef CONFIG_H
#define CONFIG_H

// === CONFIG BITS ===
#pragma config FEXTOSC = LP
#pragma config RSTOSC = EXTOSC
#pragma config CLKOUTEN = OFF, PR1WAY = ON, CSWEN = ON, FCMEN = ON
#pragma config MCLRE = EXTMCLR, PWRTS = PWRT_OFF, MVECEN = ON
#pragma config IVT1WAY = ON, LPBOREN = OFF, BOREN = SBORDIS
#pragma config BORV = VBOR_2P45, ZCD = OFF, PPS1WAY = ON
#pragma config STVREN = ON, DEBUG = OFF, XINST = OFF
#pragma config WDTCPS = WDTCPS_31, WDTE = OFF
#pragma config WDTCWS = WDTCWS_7, WDTCCS = SC
#pragma config BBSIZE = BBSIZE_512, BBEN = OFF, SAFEN = OFF, WRTAPP = OFF
#pragma config WRTB = OFF, WRTC = OFF, WRTD = OFF, WRTSAF = OFF, LVP = ON
#pragma config CP = OFF

#define _XTAL_FREQ 4000000

#endif
