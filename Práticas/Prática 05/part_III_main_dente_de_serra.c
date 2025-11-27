/*
 * File:   main.c
 * Autor: Alan Rocha
 * Geração de onda dente de serra com PIC18F4520
 */

#include <xc.h>

// CONFIGs do PIC18F4520
#pragma config OSC = HS, FCMEN = OFF, IESO = OFF
#pragma config PWRT = OFF, BOREN = SBORDIS, BORV = 3
#pragma config WDT = OFF, WDTPS = 32768
#pragma config CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON
#pragma config STVREN = ON, LVP = OFF, XINST = OFF

unsigned char saw_value = 0;

void main(void) {
    TRISD = 0x00;                 // PORTD como saída
    TRISCbits.TRISC0 = 0;         // RC0 como saída

    PORTD = saw_value;
    PORTCbits.RC0 = 0;

    // Configuração do Timer2
    T2CONbits.T2CKPS = 0b00;      // Pré-escaler 1:1
    T2CONbits.T2OUTPS = 0b0000;   // Pós-escaler 1:1
    PR2 = 150;                    // Frequência da forma de onda 255	Mais lenta 150	Médio (padrão) 50	Mais rápida
    PIE1bits.TMR2IE = 1;          // Interrupção Timer2 ativada
    T2CONbits.TMR2ON = 1;         // Liga o Timer2

    // Habilita interrupções globais e periféricas
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

    while (1); // Loop infinito
}

void __interrupt(high_priority) ISR(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;

        // Envia valor atual
        PORTD = saw_value;

        // Gera pulso em RC0 (trigger para visualização)
        PORTCbits.RC0 = 1;
        __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
        PORTCbits.RC0 = 0;

        // Incrementa e reinicia se ultrapassar 255
        saw_value++;
    }
}
