// ======================================================================
// Universidade Federal do Ceará (Campus) Sobral
// Prof.: Me. Alan Rocha
// Disciplina: Microprocessadores
// Semestre: 2025.2
// ======================================================================

// ======================================================================
// "MICROS" na linha 1 e "PROF: ALAN" na linha 2
// PIC18F4520 + LCD 16x2 (8 bits, barramento inteiro em PORTD)
// RS = RB0, EN = RB1, RW = GND
// Cristal externo ~4 MHz (OSC = XT)
// ======================================================================

#include <xc.h>

// ----------------------------------------------------------------------
// CONFIG BITS (equivalentes aos do Assembly)
// Ajuste se necessário para seu compilador/versão do XC8
// ----------------------------------------------------------------------
#pragma config OSC = XT        // Oscilador externo XT (~4 MHz)
#pragma config FCMEN = OFF     // Fail-Safe Clock Monitor off
#pragma config IESO = OFF      // Sem troca INT/EXT automática

#pragma config PWRT = OFF      // Power-up Timer off
#pragma config BOREN = SBORDIS // Brown-out Reset hardware only
#pragma config BORV = 3        // Nível de Brown-out

#pragma config WDT = OFF       // Watchdog Timer off
#pragma config WDTPS = 32768   // Pós-escalonador do WDT (irrelevante com WDT off)

#pragma config CCP2MX = PORTC  // CCP2 em RC1
#pragma config PBADEN = OFF    // PORTB<4:0> como digital após reset
#pragma config LPT1OSC = OFF
#pragma config MCLRE = ON      // /MCLR habilitado (reset externo ativo em RE3/MCLR)

#pragma config STVREN = ON     // Reset em stack overflow/underflow
#pragma config LVP = OFF       // Desativa programação em baixa tensão (libera RB5)
#pragma config XINST = OFF     // Modo estendido off
#pragma config DEBUG = OFF     // Debugger off

// ----------------------------------------------------------------------
// Frequência de clock para as rotinas de atraso (__delay_ms/__delay_us)
// O cristal externo é ~4 MHz (XT).
// ----------------------------------------------------------------------
#define _XTAL_FREQ 4000000UL

// ----------------------------------------------------------------------
// Mapeamento de pinos LCD no hardware:
//
// LCD_RS -> RB0
// LCD_EN -> RB1
// LCD_RW -> GND (fixo no hardware, não controlamos por software)
// LCD_D0..D7 -> RD0..RD7
//
// Importante: PORTD e PORTB serão saídas.
// ----------------------------------------------------------------------
#define LCD_RS_LAT LATBbits.LATB0
#define LCD_EN_LAT LATBbits.LATB1

// Escrevemos diretamente em LATD para mandar comandos/dados (8 bits)
#define LCD_DATA_LAT LATD

// ----------------------------------------------------------------------
// Protótipos das funções de controle do LCD
// ----------------------------------------------------------------------
void lcd_pulseEnable(void);
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char c);
void lcd_init(void);
void lcd_setLine1(void);
void lcd_setLine2(void);
void lcd_print(const char *msg);

// ----------------------------------------------------------------------
// Função: lcd_pulseEnable
// Dá um pulso no pino EN do LCD. O LCD lê no flanco de descida.
// ----------------------------------------------------------------------
void lcd_pulseEnable(void) {
    LCD_EN_LAT = 1;        // EN = 1
    __delay_us(50);        // pequeno atraso de setup
    LCD_EN_LAT = 0;        // EN = 0
    __delay_us(50);        // pequeno atraso de hold
}

// ----------------------------------------------------------------------
// Função: lcd_command
// Envia um comando para o LCD.
// cmd é colocado no barramento D0..D7 -> PORTD
// RS = 0 (comando), RW = GND (fixo), pulso em EN
// Alguns comandos (ex: 0x01 clear display) exigem atraso maior.
// ----------------------------------------------------------------------
void lcd_command(unsigned char cmd) {
    LCD_RS_LAT = 0;        // RS = 0 -> comando
    LCD_DATA_LAT = cmd;    // coloca comando no barramento
    lcd_pulseEnable();     // pulso no EN

    // Espera pro LCD processar
    // Clear display (0x01) e Return Home (0x02) demoram mais (~1-2ms+)
    if(cmd == 0x01 || cmd == 0x02) {
        __delay_ms(2);
    } else {
        __delay_us(100);
    }
}

// ----------------------------------------------------------------------
// Função: lcd_data
// Envia um caractere ASCII para o LCD.
// RS = 1 (dado de texto), RW = GND fixo, pulso em EN
// ----------------------------------------------------------------------
void lcd_data(unsigned char c) {
    LCD_RS_LAT = 1;       // RS = 1 -> dado
    LCD_DATA_LAT = c;     // coloca caractere no barramento
    lcd_pulseEnable();    // pulso em EN
    __delay_us(100);      // pequeno atraso entre caracteres
}

// ----------------------------------------------------------------------
// Função: lcd_init
// Sequência clássica para LCD 16x2 HD44780 em modo 8 bits.
//
// 0x38 -> interface 8 bits, 2 linhas, fonte 5x8
// 0x0C -> display ON, cursor OFF, blink OFF
// 0x01 -> clear display
// 0x06 -> entry mode: cursor avança, sem shift de tela
// ----------------------------------------------------------------------
void lcd_init(void) {
    __delay_ms(20);   // atraso inicial após energizar o LCD
    __delay_ms(20);

    lcd_command(0x38); // Function Set: 8 bits, 2 linhas
    lcd_command(0x0C); // Display ON, cursor OFF, blink OFF
    lcd_command(0x01); // Clear display
    lcd_command(0x06); // Entry mode (cursor++)

    // Agora o LCD está pronto
}

// ----------------------------------------------------------------------
// Posição do cursor na linha 1 (endereço DDRAM 0x00 -> comando 0x80)
// ----------------------------------------------------------------------
void lcd_setLine1(void) {
    lcd_command(0x80); // forçar cursor coluna 0 linha 1
}

// ----------------------------------------------------------------------
// Posição do cursor na linha 2 (endereço DDRAM 0x40 -> comando 0xC0)
// ----------------------------------------------------------------------
void lcd_setLine2(void) {
    lcd_command(0xC0); // forçar cursor coluna 0 linha 2
}

// ----------------------------------------------------------------------
// Função: lcd_print
// Envia uma string terminada em '\0' para o LCD.
// Cada caractere é mandado via lcd_data().
// ----------------------------------------------------------------------
void lcd_print(const char *msg) {
    while(*msg != '\0') {
        lcd_data(*msg);
        msg++;
    }
}

// ----------------------------------------------------------------------
// Função principal (equivalente ao START em Assembly)
//
// Fluxo:
// 1. Desabilita interrupções globais
// 2. Configura portas digitais e direções
// 3. Inicializa o LCD
// 4. Escreve "MICROS" na linha 1
// 5. Escreve "PROF: ALAN" na linha 2
// 6. Loop infinito
// ----------------------------------------------------------------------
void main(void) {

    // Desabilitar interrupções globais durante init
    INTCON = 0x00;

    // Configurar canais analógicos como digitais.
    // ADCON1 = 0x0F força PORTA/PORTB como digital (desliga entradas analógicas).
    ADCON1 = 0x0F;

    // Zerar latches antes de configurar TRIS
    LATB = 0x00;
    LATD = 0x00;

    // PORTB como saída (RB0 = RS, RB1 = EN)
    TRISB = 0x00;  // 0 = saída em todos os bits de B

    // PORTD como saída (D0..D7 do LCD)
    TRISD = 0x00;  // 0 = saída em todos os bits de D

    // Inicializar LCD
    lcd_init();

    // Linha 1: "MICROS"
    lcd_setLine1();
    lcd_print("MICROS");

    // Linha 2: "PROF: ALAN"
    lcd_setLine2();
    lcd_print("PROF: ALAN");

    // Loop infinito (equivalente ao BRA MainLoop)
    while(1) {
        // Nada a fazer; mantém a mensagem na tela
        // Se quisesse piscar algo futuramente, faria aqui
    }
}
