/* Projeto Final - Laboratorio de Sistemas Microprocessados
 * Leonardo Pereira Cortes - 200030582
 * Projeto Irrigador Automatico
 * Sensor de Umidade do Solo, Bomba Peristaltica com Rele, LEDs e UART
 * */

#include <msp430.h>
#include <stdint.h>

/* =============== Funcoes ================ */

// Funcoes de configuracao
void config_Pins();
void config_TA0();
void config_TA2();
void config_ADC();
void config_Uart(uint8_t interface);
void uartWrite(char * str);

//Funcoes de programacao
void prog_bomba(int valor);
void prog_leds(int valor);
void prog_uart(int valor);

/* ========== Variaveis Globais =========== */

volatile uint16_t adcResult;

/* ================= Main ================= */

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configuracoes Iniciais
    config_Pins();
    config_TA0();
    config_TA2();
    config_ADC();
    config_Uart(1);

    __enable_interrupt(); // INT

    while(1) // Loop de leitura, ativacao da bomba e do UART
    {
        prog_leds(adcResult);
        prog_bomba(adcResult);
        prog_uart(adcResult);
    }
}

// Rotinas de tratamento das int (ISR) do ADC
#pragma vector = TIMER0_A0_VECTOR
__interrupt void ADC_TRIGGER()
{
    ADC12CTL0 &= ~ADC12SC;
    ADC12CTL0 |= ADC12SC;
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC_RESULT()
{
    adcResult = ADC12MEM0;

    ADC12CTL0 |= ADC12ENC;
}

/* ================ Funcoes ================ */

void config_Pins() // Funcao para configurar os pinos
{
    P2DIR |= BIT5; // Bomba = output (P2.5)
    P2SEL |= BIT5; // Conecta pino no timer

    //LEDS
    P2DIR |=  BIT0; // LED verde
    P2OUT &= ~BIT0;

    P2DIR |=  BIT2; // LED vermelho
    P2OUT &= ~BIT2;
}


void prog_leds(int valor) // Funcao para acender os LEDs
{
    if(valor <= 3000 && valor > 2000) // Umido: liga o led verde e desliga o vermelho
    {
        P2OUT |=  BIT0;
        P2OUT &= ~BIT2;
    }
    else                              // Seco: liga o led vermelho e desliga o verde
    {
        P2OUT &= ~BIT0;
        P2OUT |=  BIT2;
    }
}

/* Funcoes para o Sensor de Umidade do Solo (Conversor AD) */

void config_TA0() // Funcao para configurar o Timer A0
{
    TA0CTL = MC__UP | TASSEL__SMCLK;
    TA0CCR0 = 100 - 1;
    TA0CCTL0 = CCIE;
}

void config_ADC() // Funcao para configurar o Conversor AD
{
    ADC12CTL0 &= ~ADC12ENC;

    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;

    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_0 | ADC12SHP | ADC12DIV_0 | ADC12SSEL_0 | ADC12CONSEQ_1;

    ADC12CTL2 = ADC12TCOFF | ADC12RES_2 | ADC12SR;

    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0; // Pino leitura do sensor de umidade (P6.0)

    ADC12IE = ADC12IE0;

    ADC12CTL0 |= ADC12ENC;
}

/* Funcoes para a Bomba (Modulo Rele/Bomba Peristaltica) */

void config_TA2() // Funcao para configurar o Timer A2 (TA2.2)
{
    TA2CTL = TASSEL__SMCLK | // Usando timer SM
             MC__UP        | // Modo de subida
             TACLR;          // Timer clear

    TA2CCR0 = 20000 - 1; // Periodo de 20ms

    TA2CCR2 = 10; // Duty cycle 10%

    TA2CCTL2 = OUTMOD_7; // Reset-set
}

void prog_bomba(int valor) // Funcao para movimentar a bomba
{
    // Umido ~ 2400/2600 (3000 < x < 2000)
    if(valor <= 3000 && valor > 2000) // Se estiver umido: bomba desligada
    {
        TA2CCR2 = 0;
    }
    else                              // Se estiver seco: ativa bomba
    {
        TA2CCR2 = 3000;
    }
}

/* Funcoes para o UART */

void config_Uart(uint8_t interface) // Funcao para configurar o UART
{
    if(interface == 1)
    {
        UCA1CTL1 = UCSWRST;

        UCA1CTL0 |= UCMODE_0; // Modo UART
        UCA1CTL1 |= UCSSEL__SMCLK;
        UCA1BRW  = 6; // D = 2^20 / 9600 / 16 = 6,83 (oversampling)
        UCA1MCTL = UCBRF_13 | UCOS16;

        P4SEL |= BIT4 | BIT5;

        UCA1CTL1 &= ~UCSWRST;
    }
}

void uartWrite(char * str)
{
    while(*str)
    {
        while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = *str++;
    }
}

void prog_uart(int valor) // Funcao para escrever no UART
{
    if(valor <= 3000 && valor > 1000)           // Umido
    {
        uartWrite("Solo Esta Irrigado.\n\r");
        __delay_cycles(1000000);                // Uma impressao a cada segundo
    }
    else                                        // Seco
    {
        uartWrite("Solo Esta Seco!!\n\r");
        __delay_cycles(1000000);                // Uma impressao a cada segundo
    }
}
