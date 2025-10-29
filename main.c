/*
 * File:   main.c
 * Author: marcelo
 *
 * Created on 16 de Maio de 2019, 08:53
 */
// CONFIG1H
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

#define SIN_TABLE   

#include <xc.h>

#ifdef SIN_TABLE
const char sin_tbl[] = { 127, 135, 143, 151, 159, 166, 174, 181, 188, 195, 
                         202, 208, 214, 220, 225, 230, 235, 239, 242, 246, 
                         248, 250, 252, 253, 254, 255, 254, 253, 252, 250, 
                         248, 246, 242, 239, 235, 230, 225, 220, 214, 208, 
                         202, 195, 188, 181, 174, 166, 159, 151, 143, 135, 
                         127, 119, 111, 103,  95,  88,  80,  73,  66,  59,  
                          52,  46,  40,  34,  29,  24,  19,  15,  12,   8,   
                           6,   4,   2,   1,   0,   0,   0,   1,   2,   4,   
                           6,   8,  12,  15,  19,  24,  29,  34,  40,  46,  
                          52,  59,  66,  73,  80,  88,  95, 103, 111, 119 };
unsigned char *ptr = &sin_tbl[0];
#endif

unsigned char i;

void main(void) {
  #ifdef SIN_TABLE
    PORTD = *ptr;
    ptr++;
    TRISC0 = 0;
    PORTCbits.RC0 = 0;
  #else
    PORTD = 0;
    //TRISC0 = 0;
    TRISCbits.TRISC0 = 0;
  #endif
    TRISD = 0;
    T2CONbits.T2CKPS = 0b00;
    T2CONbits.T2OUTPS = 0b0000;
    PR2 = 150;
    TMR2IE = 1;
    T2CONbits.TMR2ON = 1;
    GIEH = 1;
    GIEL = 1;
    i = 0;
    
    while (1);
}

void __interrupt(high_priority) ISR(void) {
    if (TMR2IF) {
        TMR2IF = 0;
        #ifdef SIN_TABLE
        PORTD = *ptr;
        PORTCbits.RC0 = 1;
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");      
        PORTCbits.RC0 = 0;                 
        ptr++;
        if (ptr > &sin_tbl[99])
        {
          ptr = &sin_tbl[0];
        }  
        #else
        PORTD = i;
        PORTCbits.RC0 = 1;
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");      
        PORTCbits.RC0 = 0;           
        i++;        
        #endif
    }
}