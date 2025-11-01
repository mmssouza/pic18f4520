; ======================================================================
; Universidade Federal do Ceará (Campus) Sobral
; Prof.: Me. Alan Rocha
; Disciplina: Microprocessadores
; Semestre: 2025.2
; ======================================================================

;===============================================================================
; PIC18F4520 @ 20 MHz ? Contador 00..99 em ASM (pic-as), multiplexando 2 dígitos
; RB0 = SW0 (Resetar para 00)
; RB1 = SW1 (Contar quando pressionado; solto = Pausado)
; PORTD = segmentos a..g,dp (para displays comuns catodo)
; RB2 = ENABLE_UNI  (habilita dígito das unidades)  -> ativo em '1'
; RB3 = ENABLE_DEZ  (habilita dígito das dezenas)   -> ativo em '1'
;===============================================================================

;===============================================================================
; PIC18F4520 - Contador 00..99 em ASM (pic-as) @ 20 MHz
; RB0: SW0 (reset) / RB1: SW1 (conta enquanto pressionado)
; PORTD: segmentos a..g,dp (comum catodo)
; RB2: enable unidades  / RB3: enable dezenas (multiplexacao)
;===============================================================================

        PROCESSOR   18F4520
        #include    <xc.inc>

;------------------------- CONFIG BITS -----------------------------------------
        CONFIG  OSC = HS, FCMEN = OFF, IESO = OFF
        CONFIG  PWRT = OFF, BOREN = SBORDIS, BORV = 3
        CONFIG  WDT = OFF, WDTPS = 32768
        CONFIG  CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON
        CONFIG  STVREN = ON, LVP = OFF, XINST = OFF
        CONFIG  CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF
        CONFIG  CPB = OFF, CPD = OFF
        CONFIG  WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF
        CONFIG  WRTC = OFF, WRTB = OFF, WRTD = OFF
        CONFIG  EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF
        CONFIG  EBTRB = OFF

;------------------------- ALIASES ---------------------------------------------
SW0         EQU     0       ; RB0
SW1         EQU     1       ; RB1
EN_UNI      EQU     2       ; RB2
EN_DEZ      EQU     3       ; RB3
PARADO      EQU     0       ; bit 0 de FLAGS

;------------------------- VARIAVEIS -------------------------------------------
        PSECT   udata_acs
FLAGS:      RES     1
CONT_BCD:   RES     1       ; BCD 0x00..0x99
TEMP1:      RES     1
TEMP2:      RES     1
TICK:       RES     1       ; 100 ticks de 10 ms = 1 s

;------------------------- RESET VECTOR ----------------------------------------
        PSECT   resetVect, class=CODE, delta=2
        GOTO    START

;------------------------- CODIGO ----------------------------------------------
        PSECT   code, class=CODE, delta=2

; Delay aproximado de 1 ms em 20 MHz (Fcy=5 MHz -> 0,2 us por ciclo)
DELAY_1ms:
        MOVLW   .100
        MOVWF   TEMP1
D1ms_L:
        NOP
        NOP
        NOP
        NOP
        NOP
        DECFSZ  TEMP1, F
        BRA     D1ms_L
        RETURN

; 10 ms chamando 1 ms 10 vezes
DELAY_10ms:
        MOVLW   .10
        MOVWF   TEMP2
D10_L:
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        DECFSZ  TEMP2, F
        BRA     D10_L
        RETURN

; Converte digito (0..9 em W) para mascara de segmentos (W)
GET_SEG:
        ADDWF   PCL, F      ; salta dentro da tabela de RETLW logo abaixo
        RETLW   0x3F        ; 0
        RETLW   0x06        ; 1
        RETLW   0x5B        ; 2
        RETLW   0x4F        ; 3
        RETLW   0x66        ; 4
        RETLW   0x6D        ; 5
        RETLW   0x7D        ; 6
        RETLW   0x07        ; 7
        RETLW   0x7F        ; 8
        RETLW   0x6F        ; 9

; Mantem os dois digitos acesos por ~10 ms (multiplexacao)
REFRESH_2DIG_10ms:
        ; unidades: nibble baixo
        MOVF    CONT_BCD, W
        ANDLW   0x0F
        CALL    GET_SEG
        ; XORLW   0xFF      ; INVERTER PARA ANODO (descomentar se preciso)
        MOVWF   PORTD
        BCF     LATB, EN_DEZ
        BSF     LATB, EN_UNI
        RCALL   DELAY_1ms

        ; dezenas: nibble alto -> usar SWAPF no registrador de arquivo
        SWAPF   CONT_BCD, W
        ANDLW   0x0F
        CALL    GET_SEG
        ; XORLW   0xFF      ; INVERTER PARA ANODO (descomentar se preciso)
        MOVWF   PORTD
        BCF     LATB, EN_UNI
        BSF     LATB, EN_DEZ
        RCALL   DELAY_1ms

        ; desabilita os dois para evitar rastro
        BCF     LATB, EN_UNI
        BCF     LATB, EN_DEZ

        ; completa ~10 ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RCALL   DELAY_1ms
        RETURN

; Incrementa CONT_BCD com correcao BCD (00..99, depois volta a 00)
INC_BCD:
        INCF    CONT_BCD, F              ; +1
        ; se unidades = 10, zera unidades e incrementa dezenas
        MOVF    CONT_BCD, W
        ANDLW   0x0F
        XORLW   0x0A
        BTFSC   STATUS, Z
        GOTO    AJUSTA_DEZ

        RETURN

AJUSTA_DEZ:
        ; zera unidades
        MOVF    CONT_BCD, W
        ANDLW   0xF0
        MOVWF   CONT_BCD
        ; ++ dezenas
        MOVF    CONT_BCD, W
        ADDLW   0x10
        MOVWF   CONT_BCD
        ; se dezenas = 10 (A0h), volta para 00
        MOVF    CONT_BCD, W
        ANDLW   0xF0
        XORLW   0xA0
        BTFSC   STATUS, Z
        CLRF    CONT_BCD
        RETURN

;-------------------------------------------------------------------------------
; PROGRAMA PRINCIPAL
;-------------------------------------------------------------------------------
START:
        ; deixa RB0..RB4 digitais se alguem trocar config PBADEN por ON
        MOVLW   0x0F
        MOVWF   ADCON1

        ; direcao das portas
        CLRF    TRISD                  ; PORTD saida (segmentos)
        BSF     TRISB, SW0             ; RB0 entrada (SW0)
        BSF     TRISB, SW1             ; RB1 entrada (SW1)
        BCF     TRISB, EN_UNI          ; RB2 saida (enable unidades)
        BCF     TRISB, EN_DEZ          ; RB3 saida (enable dezenas)

        ; estados iniciais
        CLRF    LATD
        BCF     LATB, EN_UNI
        BCF     LATB, EN_DEZ
        CLRF    CONT_BCD               ; inicia em 00
        BSF     FLAGS, PARADO          ; comeca pausado
        CLRF    TICK

MAIN_LOOP:
        ; leitura dos botoes
        BTFSS   PORTB, SW0             ; pressionado = nivel baixo (assume pull-up)
        GOTO    EVT_RESET

        ; SW1 pressionado -> contar; solto -> pausar
        BTFSS   PORTB, SW1
        BCF     FLAGS, PARADO          ; contando
        BTFSC   PORTB, SW1
        BSF     FLAGS, PARADO          ; pausado

        ; mantem displays e gera base de tempo de 10 ms
        RCALL   REFRESH_2DIG_10ms
        INCF    TICK, F
        MOVF    TICK, W
        XORLW   .100                   ; 100 * 10 ms = 1 s
        BTFSS   STATUS, Z
        GOTO    MAIN_LOOP

        CLRF    TICK
        BTFSC   FLAGS, PARADO
        GOTO    MAIN_LOOP

        RCALL   INC_BCD
        GOTO    MAIN_LOOP

EVT_RESET:
        CLRF    CONT_BCD               ; volta para 00
        ; pequeno debounce mantendo display
        MOVLW   .5
        MOVWF   TEMP2
ER_DB:
        RCALL   REFRESH_2DIG_10ms
        DECFSZ  TEMP2, F
        BRA     ER_DB
        GOTO    MAIN_LOOP

        END     resetVect
