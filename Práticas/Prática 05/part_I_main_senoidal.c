/*
 * File:   main.c
 * Author: Alan Rocha
 * Geração de onda senoidal
 * Data: 25/11/2025
 */

#include <xc.h>

// ----------------------------------------------------------------------
// CONFIG BITS para o PIC18F4520
// ----------------------------------------------------------------------

// CONFIG1H
#pragma config OSC = HS        // Oscilador HS externo
#pragma config FCMEN = OFF     // Fail-Safe Clock Monitor desabilitado
#pragma config IESO = OFF      // Troca automática INT/EXT desabilitada

// CONFIG2L
#pragma config PWRT = OFF      // Power-up Timer desabilitado
#pragma config BOREN = SBORDIS // Brown-out Reset somente por hardware
#pragma config BORV = 3        // Nível de tensão do BOR

// CONFIG2H
#pragma config WDT = OFF       // Watchdog Timer desabilitado
#pragma config WDTPS = 32768   // Pós-escalonador do WDT

// CONFIG3H
#pragma config CCP2MX = PORTC  // CCP2 em RC1
#pragma config PBADEN = OFF    // PORTB<4:0> como digital
#pragma config LPT1OSC = OFF   // Timer1 com operação padrão
#pragma config MCLRE = ON      // MCLR habilitado

// CONFIG4L
#pragma config STVREN = ON     // Reset por Stack full/underflow habilitado
#pragma config LVP = OFF       // ICSP com alimentação única desabilitado
#pragma config XINST = OFF     // Instruções estendidas desabilitadas

// ----------------------------------------------------------------------
// Tabela de seno (opcional)
// ----------------------------------------------------------------------
#define SIN_TABLE

#ifdef SIN_TABLE
const char sin_tbl[] = {
  127,135,143,151,159,166,174,181,188,195,
  202,208,214,220,225,230,235,239,242,246,
  248,250,252,253,254,255,254,253,252,250,
  248,246,242,239,235,230,225,220,214,208,
  202,195,188,181,174,166,159,151,143,135,
  127,119,111,103,95,88,80,73,66,59,
  52,46,40,34,29,24,19,15,12,8,
  6,4,2,1,0,0,0,1,2,4,
  6,8,12,15,19,24,29,34,40,46,
  52,59,66,73,80,88,95,103,111,119
};
unsigned char *ptr = &sin_tbl[0];
#endif

unsigned char i;

// ----------------------------------------------------------------------
// Função principal
// ----------------------------------------------------------------------
void main(void) {
    // Configuração dos registradores
    TRISD = 0x00;                // PORTD como saída
    TRISCbits.TRISC0 = 0;       // RC0 como saída digital

    #ifdef SIN_TABLE
        PORTD = *ptr;           // Saída do valor inicial da senoide
        ptr++;
        PORTCbits.RC0 = 0;
    #else
        PORTD = 0;
        i = 0;
    #endif

    // Configuração do Timer2
    T2CONbits.T2CKPS = 0b00;    // Pré-escaler 1:1
    T2CONbits.T2OUTPS = 0b0000; // Pós-escaler 1:1
    PR2 = 150;                  // Valor de comparação
    TMR2IE = 1;                 // Habilita interrupção do Timer2
    T2CONbits.TMR2ON = 1;       // Liga Timer2

    // Habilitação global de interrupções
    INTCONbits.GIE = 1;         // Interrupções de alta prioridade
    INTCONbits.PEIE = 1;        // Interrupções de periféricos

    while (1); // Loop infinito
}

// ----------------------------------------------------------------------
// Interrupção de Alta Prioridade
// ----------------------------------------------------------------------
void __interrupt(high_priority) ISR(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;    // Limpa flag de interrupção

        #ifdef SIN_TABLE
            PORTD = *ptr;
            PORTCbits.RC0 = 1;
            __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
            PORTCbits.RC0 = 0;

            ptr++;
            if (ptr > &sin_tbl[99]) {
                ptr = &sin_tbl[0];
            }
        #else
            PORTD = i;
            PORTCbits.RC0 = 1;
            __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
            PORTCbits.RC0 = 0;
            i++;
        #endif
    }
}
