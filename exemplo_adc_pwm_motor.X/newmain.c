
#define _XTAL_FREQ 20e6
#define motor_pin PORTDbits.RD0                       

unsigned char duty = 0x00;                    

//void OpenADC( unsigned int ReadADC config, unsigned char config2, unsigned char portconfig)


unsigned int ADC_Read(){
    unsigned int aux = 0;               
    ADCON0bits.GO_DONE = 1;                
    while(ADCON0bits.GO_DONE);
    //aux = (unsigned int) ADRESH;
    //aux <<= 8;
    //aux += (unsigned int) ADRESL;
    aux = ADRES; 
    return aux; //Return result
}


void main(){
    //OSCCON = 0b01100000;
    GIE = 0x01;                        
    PEIE = 0x01;                       
    T2CON = 0b01111111;  // 1111  1101
    TMR2IE = 1;
    ADCON1 = 0x8E;
    ADCON0 = 0x80; 
    ADCON2 = 0b10010110;
    ADCON0bits.ADON = 1; 
    TRISA = 0x01; 
    TRISD = 0x00;
    PORTA = 0x00;
    PORTD = 0x00; 
    duty = 127;   
    TMR2ON = 1;
            
    while(1){
       __delay_us(50);
       duty = (unsigned char) (ADC_Read() >> 2);                  //Variável adc recebe valor adc do AN0
       
       if (duty > 250)
       {   
           duty = 250;
       } else if (duty  <  20)
       {
           duty = 20;
       }
    }

}

void __interrupt(low_priority) tmr2(void) {
//void low_priority interrupt tmr2(void){
    if(TMR2IF){                                                            
        TMR2IF = 0x00;                    
        if (motor_pin == 0){                             //Saída servo1 em high? Sim...
            PR2 = duty;
            TMR2 = 0;//TMR0 recebe valor atual do duty
            motor_pin = 1;                      //Saída do servo1 em low
        } else {                                  
            PR2 = 255 - duty;
            TMR2 = 0;//TMR0 recebe valor máximo menos valor do duty
            motor_pin = 0;                      //Saída do servo1 em high
        } 
    } 
}
