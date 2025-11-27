/*
 * Projeto: Geração de onda degrau (staircase) com PIC18F4520
 * Autor: Alan Rocha
 * Hardware: DAC R-2R ligado ao PORTD
 */

#include <xc.h>

// ===== CONFIGURAÇÕES DO PIC18F4520 =====
#pragma config OSC = HS, FCMEN = OFF, IESO = OFF
#pragma config PWRT = OFF, BOREN = SBORDIS, BORV = 3
#pragma config WDT = OFF, WDTPS = 32768
#pragma config CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON
#pragma config STVREN = ON, LVP = OFF, XINST = OFF

// ===== VARIÁVEIS GLOBAIS =====
unsigned char stair_levels[] = {0, 32, 64, 96, 128, 160, 192, 224}; // 8 níveis de degrau
unsigned char step_index = 0;
unsigned char repeat_count = 0;
#define REPETICOES_POR_DEGRAU 10  // Número de interrupções antes de trocar o degrau

// ===== FUNÇÃO PRINCIPAL =====
void main(void) {
    // PORTD e RC0 como saídas
    TRISD = 0x00;                 // PORTD todo como saída (conectado ao DAC)
    TRISCbits.TRISC0 = 0;        // RC0 como saída (pulso de trigger)

    PORTD = stair_levels[step_index];  // Primeiro nível de saída
    PORTCbits.RC0 = 0;

    // Configuração do Timer2
    T2CONbits.T2CKPS = 0b00;      // Pré-escaler 1:1
    T2CONbits.T2OUTPS = 0b0000;   // Pós-escaler 1:1
    PR2 = 100;                    // Frequência de atualização da onda
    PIE1bits.TMR2IE = 1;          // Habilita interrupção do Timer2
    T2CONbits.TMR2ON = 1;         // Liga o Timer2

    // Habilita interrupções globais e periféricas
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

    while (1); // Loop principal (não faz nada, tudo ocorre na ISR)
}

// ===== INTERRUPÇÃO DE ALTA PRIORIDADE =====
void __interrupt(high_priority) ISR(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;  // Limpa flag do Timer2

        // Gera um pulso curto em RC0 (pode ser usado como trigger no osciloscópio)
        PORTCbits.RC0 = 1;
        __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
        PORTCbits.RC0 = 0;

        // Mantém o nível por algumas interrupções antes de mudar
        repeat_count++;
        if (repeat_count >= REPETICOES_POR_DEGRAU) {
            repeat_count = 0;
            step_index++;

            // Reinicia ao final do vetor de níveis
            if (step_index >= sizeof(stair_levels)) {
                step_index = 0;
            }

            PORTD = stair_levels[step_index];  // Atualiza o novo nível
        }
    }
}
