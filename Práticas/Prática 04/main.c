// ======================================================================
// Universidade Federal do Ceará (Campus) Sobral
// Prof.: Me. Alan Rocha
// Disciplina: Microprocessadores
// Semestre: 2025.2
// ======================================================================

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// ----------------- DEFINES (ficam logo após os includes) --------------------
#define _XTAL_FREQ 20000000UL          // 20 MHz (HS)
#define motor_pin   LATDbits.LATD0     // para escrever no pino
#define motor_read  PORTDbits.RD0      // para ler o pino

// ----------------- CONFIG BITS (PIC18F4520, headers clássicos) --------------
#pragma config OSC   = HS
#pragma config FCMEN = OFF
#pragma config IESO  = OFF
#pragma config PWRT  = OFF
#pragma config BOREN = ON
#pragma config BORV  = 3
#pragma config WDT   = OFF
#pragma config WDTPS = 32768
#pragma config CCP2MX = PORTC
#pragma config PBADEN = OFF
#pragma config LPT1OSC = OFF
#pragma config MCLRE  = ON
#pragma config STVREN = ON
#pragma config LVP    = OFF
#pragma config XINST  = OFF
#pragma config DEBUG  = OFF

// ----------------- VARIÁVEIS -------------------------------------------------
volatile unsigned char duty = 0;  // 0..255 (usado pelo seu PWM por TMR2)

// ----------------- ADC -------------------------------------------------------
static unsigned int ADC_Read(void) {
    // Garante canal AN0, ADC ligado, resultado à direita
    ADCON0bits.CHS = 0b0000;     // AN0
    __delay_us(5);               // tempo de aquisição
    ADCON0bits.GO_DONE = 1;
    while (ADCON0bits.GO_DONE);  // espera converter
    // Combina ADRESH:ADRESL (10 bits válidos, right-justified)
    return (((unsigned int)ADRESH) << 8) | ADRESL;
}

// ----------------- MAIN ------------------------------------------------------
void main(void) {
    // I/O
    CMCON = 0x07;                // comparadores OFF
    TRISAbits.TRISA0 = 1;        // AN0 entrada
    TRISDbits.TRISD0 = 0;        // RD0 saída (motor)
    LATD = 0x00;
    LATA = 0x00;

    // ADC: AN0 analógico, Vref=VDD/VSS, right-justified, Tad adequado
    ADCON1 = 0x0E;               // AN0 analógico; restante digital
    ADCON2 = 0b10010110;         // ADFM=1; ACQT=6*Tad; ADCS=Fosc/64 (ok p/ 20 MHz)
    ADCON0 = 0b00000001;         // CHS=AN0, ADON=1

    // Timer2: você está usando PWM ?via interrupção? alterando PR2
    // Postscaler=1:16, TMR2ON=1, Prescaler=1:16 (ajuste se quiser outra base de tempo)
    T2CON = 0b01111110;          // TOUTPS=1111 (1:16), TMR2ON=1, T2CKPS=10 (1:16)
    PR2   = 255;                 // período base (será reprogramado na ISR)
    TMR2  = 0;

    // Interrupções (uma só, sem prioridade)
    RCONbits.IPEN = 0;           // sem prioridades ? uma ISR global
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE  = 1;

    duty = 127;                  // duty inicial ~50%

    while (1) {
        __delay_us(50);
        // Lê AN0 e escala 10 bits -> 8 bits (0..1023 -> 0..255)
        unsigned int adc = ADC_Read();
        duty = (unsigned char)(adc >> 2);

        // Limites de duty (anti-stall)
        if (duty > 250) duty = 250;
        else if (duty < 20) duty = 20;
    }
}

// ----------------- ISR ÚNICA (sem prioridade) -------------------------------
void __interrupt() isr(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;

        if (motor_read == 0) {
            PR2  = duty;   // tempo em nível alto
            TMR2 = 0;
            motor_pin = 1;
        } else {
            PR2  = 255 - duty; // tempo em nível baixo
            TMR2 = 0;
            motor_pin = 0;
        }
    }
}
