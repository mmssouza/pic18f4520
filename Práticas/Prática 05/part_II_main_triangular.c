/*
 * File:   main.c
 * Autor: Alan Rocha
 * Geração de onda triangular no PIC18F4520
 */

#include <xc.h>

// CONFIGS ? mesmas configurações do código anterior
#pragma config OSC = HS, FCMEN = OFF, IESO = OFF
#pragma config PWRT = OFF, BOREN = SBORDIS, BORV = 3
#pragma config WDT = OFF, WDTPS = 32768
#pragma config CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON
#pragma config STVREN = ON, LVP = OFF, XINST = OFF

#define TRIANGULAR_WAVE

#ifdef TRIANGULAR_WAVE
const unsigned char tri_wave[100] = {
    // Sobe de 0 a 255 em passos de ~5
    0, 5, 10, 15, 20, 25, 30, 35, 40, 45,
    50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
    100, 105, 110, 115, 120, 125, 130, 135, 140, 145,
    150, 155, 160, 165, 170, 175, 180, 185, 190, 195,
    200, 205, 210, 215, 220, 225, 230, 235, 240, 245,
    // Desce de 255 até 0
    250, 245, 240, 235, 230, 225, 220, 215, 210, 205,
    200, 195, 190, 185, 180, 175, 170, 165, 160, 155,
    150, 145, 140, 135, 130, 125, 120, 115, 110, 105,
    100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
    50, 45, 40, 35, 30, 25, 20, 15, 10, 5
};
unsigned char *ptr = &tri_wave[0];
#endif

unsigned char i;

void main(void) {
    TRISD = 0x00;                 // PORTD como saída
    TRISCbits.TRISC0 = 0;         // RC0 como saída

    #ifdef TRIANGULAR_WAVE
        PORTD = *ptr;
        ptr++;
        PORTCbits.RC0 = 0;
    #else
        PORTD = 0;
        i = 0;
    #endif

    // Configuração do Timer2
    T2CONbits.T2CKPS = 0b00;
    T2CONbits.T2OUTPS = 0b0000;
    PR2 = 150;
    PIE1bits.TMR2IE = 1;          // Habilita interrupção do Timer2
    T2CONbits.TMR2ON = 1;

    // Habilita interrupções globais e de periféricos
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

    while (1); // Loop infinito
}

void __interrupt(high_priority) ISR(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;

        #ifdef TRIANGULAR_WAVE
            PORTD = *ptr;
            PORTCbits.RC0 = 1;
            __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
            PORTCbits.RC0 = 0;

            ptr++;
            if (ptr > &tri_wave[99]) {
                ptr = &tri_wave[0]; // reinicia a forma de onda
            }
        #else
            PORTD = i++;
            PORTCbits.RC0 = 1;
            __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
            PORTCbits.RC0 = 0;
        #endif
    }
}
