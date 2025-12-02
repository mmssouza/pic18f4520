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
 * AUTOR: Prof. Me. Alan Rocha
 * DATA: 02/12/2025
 * VERSÃO: 1.2
 *--------------------------------------------------------------*/

// ========== BITS DE CONFIGURAÇÃO (USE PARA CRISTAL EXTERNO DE 20 MHz) ==========
#pragma config OSC = HS        // Oscilador externo em modo HS (para 20 MHz)
#pragma config FCMEN = OFF     // Desabilita Fail-Safe Clock Monitor
#pragma config IESO = OFF      // Desabilita troca automática de oscilador

#pragma config PWRT = OFF      // Desabilita Power-up Timer
#pragma config BOREN = OFF     // Desabilita Brown-out Reset

#pragma config WDT = OFF       // Desabilita Watchdog Timer
#pragma config LVP = OFF       // Desabilita Programação em Baixa Tensão
#pragma config DEBUG = OFF     // Desabilita modo de depuração

// ================= INCLUSÕES E DEFINIÇÕES ====================
#include <xc.h>                // Biblioteca principal do compilador XC8
#include <stdio.h>             // Biblioteca para usar sprintf()
#define _XTAL_FREQ 20000000    // Define a frequência do cristal externo (20 MHz)

// ========== DEFINIÇÃO DOS PINOS DO LCD (modo 4 bits) ==========
#define lcd_rs     PORTDbits.RD0  // Pino RS do LCD (registrador de dados/comando)
#define lcd_enable PORTDbits.RD1  // Pino Enable do LCD
#define lcd_db4    PORTDbits.RD4  // Pino de dados D4
#define lcd_db5    PORTDbits.RD5  // Pino de dados D5
#define lcd_db6    PORTDbits.RD6  // Pino de dados D6
#define lcd_db7    PORTDbits.RD7  // Pino de dados D7

// ========= PROTOTIPAÇÃO DAS FUNÇÕES ==========
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(char linha, char coluna);
void lcd_puts(const char *str);
void lcd_putch(char c);
void lcd_cmd(unsigned char cmd);
void lcd_write(unsigned char data);
void envia_nibble(unsigned char nibble);
void contador_segundos(unsigned char tempo);

// ========== ENVIO DE NIBBLE PARA LCD ==========
void envia_nibble(unsigned char nibble) {
    lcd_db4 = (nibble >> 4) & 1;
    lcd_db5 = (nibble >> 5) & 1;
    lcd_db6 = (nibble >> 6) & 1;
    lcd_db7 = (nibble >> 7) & 1;

    lcd_enable = 1;         // Pulso de enable para aceitar o dado
    __delay_us(1);
    lcd_enable = 0;
    __delay_us(100);
}

// ========== ENVIO DE COMANDO AO LCD ==========
void lcd_cmd(unsigned char cmd) {
    lcd_rs = 0;             // RS = 0 indica comando
    envia_nibble(cmd);      // Envia os 4 bits altos
    envia_nibble(cmd << 4); // Envia os 4 bits baixos
    __delay_ms(2);
}

// ========== ENVIO DE DADO AO LCD ==========
void lcd_write(unsigned char data) {
    lcd_rs = 1;              // RS = 1 indica dado
    envia_nibble(data);
    envia_nibble(data << 4);
    __delay_ms(2);
}

// ========== IMPRIME UM CARACTERE ==========
void lcd_putch(char c) {
    lcd_write(c);
}

// ========== IMPRIME STRING ==========
void lcd_puts(const char *str) {
    while (*str) {
        lcd_putch(*str++);
    }
}

// ========== POSICIONA CURSOR ==========
void lcd_set_cursor(char linha, char coluna) {
    char pos = (linha == 1) ? 0x80 : 0xC0; // Linha 1 ou 2
    pos += coluna;
    lcd_cmd(pos);
}

// ========== LIMPA O DISPLAY ==========
void lcd_clear(void) {
    lcd_cmd(0x01);          // Comando para limpar
    __delay_ms(2);
}

// ========== INICIALIZA LCD ==========
void lcd_init(void) {
    TRISD = 0x00;           // Todos os pinos do PORTD como saída
    PORTD = 0x00;

    lcd_enable = 0;
    lcd_rs = 0;

    __delay_ms(20);         // Tempo de estabilização

    envia_nibble(0x30);     // Sequência padrão de inicialização
    __delay_ms(5);
    envia_nibble(0x30);
    __delay_us(150);
    envia_nibble(0x30);
    envia_nibble(0x20);     // Habilita modo 4 bits

    lcd_cmd(0x28);          // Interface de 4 bits, 2 linhas, fonte 5x8
    lcd_cmd(0x0C);          // Display ON, cursor OFF
    lcd_cmd(0x06);          // Incrementa cursor automaticamente
    lcd_clear();
}

// ========== CONTADOR DE SEGUNDOS COM EXIBIÇÃO ==========
void contador_segundos(unsigned char tempo) {
    char buffer[16];
    for (int i = tempo; i >= 0; i--) {
        lcd_set_cursor(2, 0);                // Linha 2, coluna 0
        sprintf(buffer, "Tempo: %2ds   ", i); // Cria string com o tempo restante
        lcd_puts(buffer);                   // Exibe no LCD
        __delay_ms(1000);                   // Espera 1 segundo
    }
}

// ========== FUNÇÃO PRINCIPAL ==========
void main(void) {
    lcd_init();              // Inicializa o LCD

    TRISC = 0xF8;            // RC0, RC1 e RC2 como saída (LEDs)
    PORTC = 0x00;            // Inicializa LEDs desligados

    while (1) {
        // Mensagem inicial
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("Pratica 07:");
        lcd_set_cursor(2, 0);
        lcd_puts("Contagem inicial");
        __delay_ms(1500);    // Espera 1,5s

        lcd_clear();
        contador_segundos(10);  // Contagem inicial de 10s

        // LED VERDE (passagem)
        PORTCbits.RC0 = 1;   // Liga verde
        PORTCbits.RC1 = 0;   // Apaga amarelo
        PORTCbits.RC2 = 0;   // Apaga vermelho
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("SINAL VERDE");
        contador_segundos(10); // Verde por 10s

        // LED AMARELO (atenção)
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 1;   // Liga amarelo
        PORTCbits.RC2 = 0;
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("ATENCAO...");
        contador_segundos(3);  // Amarelo por 3s

        // LED VERMELHO (pare)
        PORTCbits.RC0 = 0;
        PORTCbits.RC1 = 0;
        PORTCbits.RC2 = 1;   // Liga vermelho
        lcd_clear();
        lcd_set_cursor(1, 0);
        lcd_puts("PARE");
        contador_segundos(10); // Vermelho por 10s
    }
}
