#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void LCD_Init();
void LCD_Command(char);
void LCD_Char(char);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void LCD_Clear();
void MSdelay(unsigned int);
char getKeypadKey();
void displayMode();
void playBuzzerTune();

#endif
