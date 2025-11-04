// ======================================================================
// Universidade Federal do Ceará (Campus) Sobral
// Prof.: Me. Alan Rocha
// Disciplina: Microprocessadores
// Semestre: 2025.2
// ======================================================================

//==============================================================================
// PIC18F4520 ? Cronometro "estilo relogio" com base de tempo precisa de 1 s
// Timer0 em 16 bits + prescaler para gerar overflow a cada ~1,000 s
// RB0/INT0  : start/pausa (toggle)
// RB1/INT1  : reset (zera e pausa)
// PORTD     : [D7..D4]=DEZENAS_BCD, [D3..D0]=UNIDADES_BCD  (usa decodificadores)
//==============================================================================

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

//--------------------- CONFIG BITS (iguais ao trecho fornecido) ----------------
#pragma config OSC = HS, FCMEN = OFF, IESO = OFF
#pragma config PWRT = OFF, BOREN = SBORDIS, BORV = 3
#pragma config WDT = OFF, WDTPS = 32768
#pragma config CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON
#pragma config STVREN = ON, LVP = OFF, XINST = OFF
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF
#pragma config CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF
#pragma config WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF
#pragma config EBTRB = OFF

//--------------------- Frequencia do cristal -----------------------------------
// Ajuste para a sua placa. Se usa 20 MHz, deixe 20e6. Se usa 4 MHz, mude aqui
#define _XTAL_FREQ 20000000UL   // HS=20 MHz (Fcy=Fosc/4 = 5 MHz)

//--------------------- Estado do cronometro ------------------------------------
static volatile uint8_t dez = 0;     // dezenas de segundo (0..5)
static volatile uint8_t uni = 0;     // unidades de segundo (0..9)
static volatile bool contando = false;

//--------------------- Preload do Timer0 para 1 segundo ------------------------
// Timer0 16-bit, clock interno (Fosc/4), prescaler 1:256
// Tick Timer0 = (Fosc/4) * 256 = 0,2us * 256 = 51,2us em 20 MHz
// Ticks necessarios para 1 s  = 1 / 51,2us = 19531,25
// Valor de preload = 65536 - 19531 = 46005 = 0xB3CD  (erro ~0,0125%)
// Se trocar o cristal, recompute (ou ajuste os defines abaixo).
#define TMR0_PRELOAD_H  0xB3
#define TMR0_PRELOAD_L  0xCD

static inline void tmr0_reload(void) {
    TMR0H = TMR0_PRELOAD_H;
    TMR0L = TMR0_PRELOAD_L;
}

//--------------------- Inicializacao de hardware --------------------------------
static void hw_init(void)
{
    // PORTB digital (RB0/RB1 como botoes)
    ADCON1 = 0x0F;
    CMCON  = 0x07;      // comparadores off

    // PORTD como saida (BCD dos dois digitos)
    TRISD = 0x00;
    LATD  = 0x00;

    // RB0 e RB1 como entrada
    TRISBbits.TRISB0 = 1;   // INT0
    TRISBbits.TRISB1 = 1;   // INT1

    // Pull-ups internos em PORTB (pressionado = 0 quando ligado ao GND)
    INTCON2bits.RBPU = 0;

    // Interrupcoes externas: borda de descida
    INTCON2bits.INTEDG0 = 0;    // INT0 falling
    INTCON2bits.INTEDG1 = 0;    // INT1 falling
    INTCONbits.INT0IF   = 0;
    INTCON3bits.INT1IF  = 0;
    INTCONbits.INT0IE   = 1;    // habilita INT0
    INTCON3bits.INT1IE  = 1;    // habilita INT1

    // Configura Timer0: 16 bits, clock interno, prescaler 1:256
    T0CONbits.T08BIT = 0;   // 16-bit
    T0CONbits.T0CS   = 0;   // clock interno (Fosc/4)
    T0CONbits.PSA    = 0;   // usa prescaler
    T0CONbits.T0PS2  = 1;   // 1:256
    T0CONbits.T0PS1  = 1;
    T0CONbits.T0PS0  = 1;
    tmr0_reload();
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;  // habilita interrupcao do Timer0
    T0CONbits.TMR0ON  = 1;  // liga Timer0

    // Interrupcoes globais
    INTCONbits.PEIE = 1;
    INTCONbits.GIE  = 1;

    // Estado inicial
    dez = 0;
    uni = 0;
    contando = false;

    // Atualiza exibicao
    LATD = (uint8_t)((dez << 4) | (uni & 0x0F));
}

//--------------------- Atualiza exibicao BCD em PORTD --------------------------
static inline void display_update(void)
{
    LATD = (uint8_t)((dez << 4) | (uni & 0x0F));
}

//--------------------- Incremento estilo relogio (00..59) ----------------------
static inline void tick_1s(void)
{
    if (!contando) return;

    // ++segundos
    uni++;
    if (uni >= 10) {
        uni = 0;
        dez++;
        if (dez >= 6) {   // 60 s -> volta para 00
            dez = 0;
        }
    }
    display_update();
}

//--------------------- Programa principal --------------------------------------
int main(void)
{
    hw_init();

    for (;;)
    {
        // loop ocioso: tudo acontece nas interrupcoes
        // se quiser economizar energia em hardware real:
        // SLEEP();  (acordaria pelo INT0/INT1/TMR0)
        NOP();
    }
    // return 0;
}

//--------------------- Rotina de interrupcao -----------------------------------
void __interrupt() isr(void)
{
    // Timer0: base de tempo de 1 segundo
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        tmr0_reload();      // prepara a proxima janela de 1 s
        tick_1s();          // avanca o cronometro como um relogio (00..59)
    }

    // INT0 (RB0): alterna entre contar/pausar
    if (INTCONbits.INT0IF) {
        INTCONbits.INT0IF = 0;
        contando = !contando;
    }

    // INT1 (RB1): zera e pausa
    if (INTCON3bits.INT1IF) {
        INTCON3bits.INT1IF = 0;
        contando = false;
        dez = 0;
        uni = 0;
        display_update();
    }
}
