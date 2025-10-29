; TODO INSERT CONFIG CODE HERE USING CONFIG BITS GENERATOR
; PIC18F4520 Configuration Bit Settings

; Assembly source line config statements

#include "p18f4520.inc"

; CONFIG1H
  CONFIG  OSC = HS              ; Oscillator Selection bits (HS oscillator)
  CONFIG  FCMEN = OFF           ; Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
  CONFIG  IESO = OFF            ; Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

; CONFIG2L
  CONFIG  PWRT = OFF            ; Power-up Timer Enable bit (PWRT disabled)
  CONFIG  BOREN = SBORDIS       ; Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
  CONFIG  BORV = 3              ; Brown Out Reset Voltage bits (Minimum setting)

; CONFIG2H
  CONFIG  WDT = OFF             ; Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
  CONFIG  WDTPS = 32768         ; Watchdog Timer Postscale Select bits (1:32768)

; CONFIG3H
  CONFIG  CCP2MX = PORTC        ; CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
  CONFIG  PBADEN = ON           ; PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
  CONFIG  LPT1OSC = OFF         ; Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
  CONFIG  MCLRE = ON            ; MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

; CONFIG4L
  CONFIG  STVREN = ON           ; Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
  CONFIG  LVP = OFF             ; Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
  CONFIG  XINST = OFF           ; Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

; CONFIG5L
  CONFIG  CP0 = OFF             ; Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
  CONFIG  CP1 = OFF             ; Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
  CONFIG  CP2 = OFF             ; Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
  CONFIG  CP3 = OFF             ; Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

; CONFIG5H
  CONFIG  CPB = OFF             ; Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
  CONFIG  CPD = OFF             ; Data EEPROM Code Protection bit (Data EEPROM not code-protected)

; CONFIG6L
  CONFIG  WRT0 = OFF            ; Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
  CONFIG  WRT1 = OFF            ; Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
  CONFIG  WRT2 = OFF            ; Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
  CONFIG  WRT3 = OFF            ; Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

; CONFIG6H
  CONFIG  WRTC = OFF            ; Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
  CONFIG  WRTB = OFF            ; Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
  CONFIG  WRTD = OFF            ; Data EEPROM Write Protection bit (Data EEPROM not write-protected)

; CONFIG7L
  CONFIG  EBTR0 = OFF           ; Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR1 = OFF           ; Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR2 = OFF           ; Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR3 = OFF           ; Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

; CONFIG7H
  CONFIG  EBTRB = OFF           ; Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

VARIAVEIS UDATA_ACS 0
FLAGS  RES 1  ; Variável flags no endereço 0x00
CONTAGEM RES 1  ; Variável contagem no endereço 0x01
TEMP1 RES 1    ; Variável TEMP1 (contador de atraso) no endereço 0x02
TEMP2 RES 1    ; Variável TEMP2 (contador de atraso) no endereço 0x03
 
PARADO equ 0     ; Flag PARADO -> Bit 0 da variavel FLAGS     
SW0 equ 0        ; SW0 -> Push button RC0 
SW1 equ 1        ; SW1 -> Push button RC1   
  
RES_VECT  CODE    0x0000   ; processor reset vector
    GOTO    START          ; go to beginning of program

MAIN_PROG CODE ; let linker place main program
 
START
    clrf CONTAGEM    ; Zera CONTAGEM
    bsf FLAGS,PARADO ; Flag PARADO <- TRUE
    clrf TRISD       ; TRISD <- 00000000 (pinos dos displays como saída)
    bsf TRISC,SW0    ; RCO entrada (SW0)
    bsf TRISC,SW1    ; RC1 entrada (SW1) 
    clrf PORTD       ; Para mostrar "00" nos displays
LOOP
    bsf FLAGS,PARADO
    btfss PORTC,SW0  ; Verifica estado de SW0  
    bra EVENTO_SW0   ; SW0 pressionada: desvia para EVENTO_SW0
    btfss PORTC,SW1  ; SW0 não pressionada: verifica estado de SW1
    bcf FLAGS,PARADO ; SW1 nao pressionada:flag PARADO em 1
    bra ATUALIZA_CONTAGEM ; Desvia para gerenciar a atualização da CONTAGEM
   
EVENTO_SW0          ; Ação ao evento de SW0 pressionado
    clrf CONTAGEM   ; zera variável CONTAGEM
    bsf FLAGS,PARADO ; Leva Flag PARADO para FALSE (para contagem)
    clrf PORTD       ; Mostra "00" nos displays    
    
ATUALIZA_CONTAGEM   ; Evento de atualização do contador CONTAGEM    
    btfsc FLAGS,PARADO ; Testa estado da flag PARADO
    bra ATRASO         ; Se setada pula atualização da contagem
    incf CONTAGEM,w    ; Atualiza contagem incrementando contador
    daw                ; Converte para BCD      
    movwf CONTAGEM       
    movwf PORTD        ; Escreve resultado na Porta D
ATRASO                 ; Aqui realiza o atraso de tempo      
    rcall ATRASO_1s    ; Realiza atraso de 1 segundo
    GOTO LOOP          ; loop forever
   
;*******************************************************************************
; Rotina ATRASO_1s
; Gera atraso correspondente a 1 segundo
; supõe o microcontrolador operando a um clock de 4MHz    
;*******************************************************************************
    
ATRASO_1s
    MOVLW .200 
    MOVWF TEMP2
L1
    RCALL ATRASO_1ms
    RCALL ATRASO_1ms
    RCALL ATRASO_1ms
    RCALL ATRASO_1ms
    RCALL ATRASO_1ms
    DECFSZ TEMP2  ; Chama 5x200 vezes a rotina de atraso de 1ms
    BRA L1
    RETURN
    
;*******************************************************************************
; Rotina ATRASO
; Gera um atraso correspondente a 1 ms
; supõe o microcontrolador operando a um clock de 4MHz    
;*******************************************************************************   
ATRASO_1ms
    MOVLW .100  
    MOVWF TEMP1
LOOP2
    NOP               
    NOP
    NOP
    NOP
    NOP
    DECFSZ TEMP1 ; Executa 100 vezes 5 NOPs
    BRA LOOP2
    RETURN     
    END