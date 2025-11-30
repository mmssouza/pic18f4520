#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000

// Definições dos pinos do LCD (modo 4 bits)
#define lcd_rs     PORTDbits.RD0
#define lcd_enable PORTDbits.RD1
#define lcd_db4    PORTDbits.RD4
#define lcd_db5    PORTDbits.RD5
#define lcd_db6    PORTDbits.RD6
#define lcd_db7    PORTDbits.RD7

//================ ROTINAS LCD 4 BITS ===================

void envia_nibble_lcd(char nibble) {
    lcd_db4 = (nibble >> 4) & 0x01;
    lcd_db5 = (nibble >> 5) & 0x01;
    lcd_db6 = (nibble >> 6) & 0x01;
    lcd_db7 = (nibble >> 7) & 0x01;
    
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

inline void escreve_lcd(char c) {
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
    envia_nibble_lcd(0x20);  // Modo 4 bits

    envia_byte_lcd(0, 0x28); // 4 bits, 2 linhas, 5x8
    envia_byte_lcd(0, 0x0C); // Display on, cursor off
    envia_byte_lcd(0, 0x06); // Incrementa cursor
    limpa_lcd();
}

void caracter_inicio(char linha, char coluna) {
    char posicao;
    posicao = (linha == 1) ? 0x80 : 0xC0;
    posicao += coluna;
    envia_byte_lcd(0, posicao);
}

//=============== ROTINAS VOLTÍMETRO ====================

unsigned int Valor_AD(void) {
    ADCON0bits.GO_DONE = 1;
    while (ADCON0bits.GO_DONE);
    return ((ADRESH << 8) | ADRESL);
}

float Tensao(void) {
    return (float)Valor_AD() * 5.0 / 1023.0;
}

// Para printf funcionar com LCD
void putch(char data) {
    escreve_lcd(data);
}

//==================== MAIN ======================

void main(void) {
    char buffer[16];

    // Configurações iniciais
    OSCCON = 0b01100000; // 4 MHz interno, pode ser ajustado conforme o cristal externo
    TRISD = 0x00;
    PORTD = 0x00;

    TRISA = 0x01;   // RA0 como entrada (AN0)
    PORTA = 0x00;

    // ADC
    ADCON0 = 0x01;  // Habilita ADC, canal AN0
    ADCON1 = 0x0E;  // AN0 analógico, o restante digital
    ADCON2 = 0x89;  // Tac = Fosc/8, resultado à direita, aquisição = 4 TAD

    inicializa_lcd();

    while (1) {
        float v = Tensao();
        limpa_lcd();
        caracter_inicio(1, 1);
        printf("Tensao:");

        caracter_inicio(2, 1);
        sprintf(buffer, "%.2f V", v);
        printf(buffer);

        __delay_ms(300);
    }
}
