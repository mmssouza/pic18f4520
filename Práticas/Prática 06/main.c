/*---------------------------------------------------------------
 * UNIVERSIDADE FEDERAL DO CEARÁ ? UFC
 * CAMPUS DE SOBRAL - CURSO DE ENGENHARIA ELÉTRICA E COMPUTAÇÃO
 * DISCIPLINA: SBL0082 ? Microprocessadores (2025.2)
 *
 * PRÁTICA 06 - VOLTÍMETRO DIGITAL COM LCD 16x2
 * Microcontrolador: PIC18F4520
 * Display: LCD Alfanumérico 16x2 (modo 4 bits)
 *
 * AUTOR: Prof.: Me. Alan Rocha
 * DATA: 02/12/2025
 * VERSÃO: 1.2
 *--------------------------------------------------------------*/

#include <xc.h>
#include <stdio.h>

// === Frequência usada para __delay_ms() e __delay_us() ===
#define _XTAL_FREQ 20000000  // Cristal externo de 20 MHz

// === Configurações dos FUSES (Exemplo com cristal HS) ===
// (Você também pode configurar isso pelo MPLAB-X em Window > PIC Memory Views > Configuration Bits)
#pragma config OSC = HS        // Oscilador externo High Speed (20 MHz)
#pragma config FCMEN = OFF     // Fail-Safe Clock Monitor OFF
#pragma config IESO = OFF      // Internal/External Switch Over OFF
#pragma config PWRT = OFF
#pragma config BOREN = SBORDIS
#pragma config WDT = OFF
#pragma config LVP = OFF

// === Pinos do LCD conectados ao PORTD ===
#define lcd_rs     PORTDbits.RD0
#define lcd_enable PORTDbits.RD1
#define lcd_db4    PORTDbits.RD4
#define lcd_db5    PORTDbits.RD5
#define lcd_db6    PORTDbits.RD6
#define lcd_db7    PORTDbits.RD7

// =========== ROTINAS LCD (Modo 4 Bits) ==============

void envia_nibble_lcd(char nibble) {
    lcd_db4 = (nibble >> 4) & 1;
    lcd_db5 = (nibble >> 5) & 1;
    lcd_db6 = (nibble >> 6) & 1;
    lcd_db7 = (nibble >> 7) & 1;

    lcd_enable = 1;
    __delay_us(1);
    lcd_enable = 0;
    __delay_us(100);
}

void envia_byte_lcd(char endereco, char dado) {
    lcd_rs = endereco;
    envia_nibble_lcd(dado);
    envia_nibble_lcd(dado << 4);
}

void escreve_lcd(char c) {
    envia_byte_lcd(1, c);
}

void limpa_lcd(void) {
    envia_byte_lcd(0, 0x01);
    __delay_ms(2);
}

void inicializa_lcd(void) {
    TRISD = 0x00;
    PORTD = 0x00;

    lcd_enable = 0;
    lcd_rs = 0;

    __delay_ms(20);
    envia_nibble_lcd(0x30);
    __delay_ms(5);
    envia_nibble_lcd(0x30);
    __delay_us(150);
    envia_nibble_lcd(0x30);
    envia_nibble_lcd(0x20);  // Ativa modo 4 bits

    envia_byte_lcd(0, 0x28); // Interface 4 bits, 2 linhas, caractere 5x8
    envia_byte_lcd(0, 0x0C); // Display ON, cursor OFF
    envia_byte_lcd(0, 0x06); // Incrementa cursor automaticamente
    limpa_lcd();
}

void posiciona_cursor(char linha, char coluna) {
    char posicao = (linha == 1) ? 0x80 : 0xC0;
    posicao += coluna;
    envia_byte_lcd(0, posicao);
}

// ========== ROTINAS DO VOLTÍMETRO =============

unsigned int le_ADC(void) {
    ADCON0bits.GO_DONE = 1;
    while (ADCON0bits.GO_DONE); // Espera fim da conversão
    return ((ADRESH << 8) | ADRESL); // Junta os 10 bits
}

float calcula_tensao(void) {
    return (float)le_ADC() * 5.0 / 1023.0; // Conversão para Volts
}

// Suporte ao printf()
void putch(char data) {
    escreve_lcd(data);
}

// =============== FUNÇÃO PRINCIPAL ===============

void main(void) {
    char buffer[16];

    // === Configuração de portas ===
    TRISA = 0b00000001;   // RA0 como entrada (AN0)
    PORTA = 0x00;

    TRISD = 0x00;         // LCD como saída
    PORTD = 0x00;

    // === Configuração do ADC ===
    ADCON0 = 0x01;  // Habilita ADC, canal AN0
    ADCON1 = 0x0E;  // AN0 analógico, demais digitais
    ADCON2 = 0x89;  // Tac = Fosc/8, resultado à direita, TAD ~2 ?s

    inicializa_lcd();

    while (1) {
        float tensao = calcula_tensao();

        limpa_lcd();
        posiciona_cursor(1, 0);
        printf("Tensao lida:");

        posiciona_cursor(2, 0);
        sprintf(buffer, "%.2f V      ", tensao); // Ex: "3.21 V"
        printf(buffer);

        __delay_ms(300); // Atualiza leitura a cada 300 ms
    }
}
