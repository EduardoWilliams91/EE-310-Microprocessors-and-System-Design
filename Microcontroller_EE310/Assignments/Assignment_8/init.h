#ifndef INIT_H
#define INIT_H

#include <xc.h>

void initIO(void);
void initInterrupt(void);
void initSystem(void);

extern unsigned int count;
extern unsigned char relayState;
extern unsigned char mode;
extern char password[3];
extern char setPassword[3];
extern char passwordPos;
extern char entryPassword[3];
extern char entryLeftDigit;
extern char entryRightDigit;

#endif
