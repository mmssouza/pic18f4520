/*---------------------------------------------------------------
 * UNIVERSIDADE FEDERAL DO CEARÁ ? UFC
 * CAMPUS DE SOBRAL ? CURSO DE ENGENHARIA ELÉTRICA E COMPUTAÇÃO
 * DISCIPLINA: SBL0082 ? Microprocessadores (2025.2)
 *
 * PRÁTICA 07 ? SEMÁFORO DIGITAL COM LCD E TEMPORIZAÇÃO
 * Microcontrolador: PIC18F4520
 * Display: LCD Alfanumérico 16x2 (modo 4 bits)
 * LEDs: Verde, Amarelo (Laranja) e Vermelho
 *
 * AUTOR: Prof.: Me. Alan Rocha
 * DATA: 30/11/2025
 * VERSÃO: 1.0
 *--------------------------------------------------------------*/


#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000

// === Pinos LCD (modo 4 bits em RD0?RD7) ===
#define lcd_rs     PORTDbits.RD0
#define lcd_enable PORTDbits.RD1
#define lcd_db4    PORTDbits.RD4
#define lcd_db5    PORTDbits.RD5
#define lcd_db6    PORTDbits.RD6
#define lcd_db7    PORTDbits.RD7

// === Prototipagem das funções ===
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(char linha, char coluna);
void lcd_puts(const char *str);
void lcd_putch(char c);
void lcd_cmd(unsigned char cmd);
void lcd_write(unsigned char data);
void envia_nibble(unsigned char nibble);
void contador_segundos(unsigned char tempo);

// === Inicialização LCD ===
void envia_nibble(unsigned char nibble) {
    lcd_db4 = (nibble >> 4) & 1;
    lcd_db5 = (nibble >> 5) & 1;
    lcd_db6 = (nibble >> 6) & 1;
    lcd_db7 = (nibble >> 7) & 1;

    lcd_enable = 1;
    __delay_us(1);
    lcd_enable = 0;
    __delay_us(100);
}

void lcd_cmd(unsigned char cmd) {
    lcd_rs = 0;
    envia_nibble(cmd);
    envia_nibble(cmd << 4);
    __delay_ms(2);
}

void lcd_write(unsigned char data) {
    lcd_rs = 1;
    envia_nibble(data);
    envia_nibble(data << 4);
    __delay_ms(2);
}

void lcd_putch(char c) {
    lcd_write(c);
}

void lcd_puts(const char *str) {
    while (*str) {
        lcd_putch(*str++);
    }
}

void lcd_set_cursor(char linha, char coluna) {
    char pos = (linha == 1) ? 0x80 : 0xC0;
    pos += coluna;
    lcd_cmd(pos);
}

void lcd_clear(void) {
    lcd_cmd(0x01);
    __delay_ms(2);
}

void lcd_init(void) {
    TRISD = 0x00;
    PORTD = 0x00;
    lcd_enable = 0;
    lcd_rs = 0;

    __delay_ms(20);
    envia_nibble(0x30);
    __delay_ms(5);
    envia_nibble(0x30);
    __delay_us(150);
    envia_nibble(0x30);
    envia_nibble(0x20); // 4 bits

    lcd_cmd(0x28); // 4 bits, 2 linhas, 5x8
    lcd_cmd(0x0C); // Display on, cursor off
    lcd_cmd(0x06); // Incrementa cursor
    lcd_clear();
}

// === Função de contagem no LCD ===
void contador_segundos(unsigned char tempo) {
    char buffer[16];
    for (int i = tempo; i >= 0; i--) {
        lcd_set_cursor(2, 0);
        sprintf(buffer, "Tempo: %2ds   ", i);
        lcd_puts(buffer);
        __delay_ms(1000);
    }
}

// === Função principal ===
void main(void) {
    // Inicializações
    lcd_init();
    TRISC = 0xF8; // RC0?RC2 como saída (LEDs)
    PORTC = 0x00;

    while (1) {
        // Mensagem de início
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("Pratica 07:");
        lcd_set_cursor(2, 0);
        lcd_puts("Contagem inicial");
        __delay_ms(1500);

        lcd_clear();
        contador_segundos(10);

        // Verde aceso por 10s
        PORTCbits.RC0 = 1; // LED verde
        PORTCbits.RC1 = 0;
        PORTCbits.RC2 = 0;
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("SINAL VERDE");
        contador_segundos(10);

        // Amarelo por 3s
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 1; // LED amarelo
        PORTCbits.RC2 = 0;
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("ATENCAO...");
        contador_segundos(3);

        // Vermelho por 10s
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 0;
        PORTCbits.RC2 = 1; // LED vermelho
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("PARE");
        contador_segundos(10);
    }
}
