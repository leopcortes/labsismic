/*
 * Leonardo Pereira Cortes - 200030582
 * Visto 2 (Problema 2)
 * LabSisMic - 2022.1
 * 07/08/2021
 */

#include <msp430.h>

// Constantes
#define SERVO_DEG 180 // Maior angulo possivel
#define MIN_DUTY 500 // Menor duty cycle
#define MAX_DUTY 2500 // Maior duty cycle

// Funcoes
void configPins();
void config_TA0();
void config_TA1();
void config_TA2();
void calc_dist();
void leds();
void prog_servo();

// Variaveis globais
unsigned const int BREAKPOINT1 = 10; //10 cm
unsigned const int BREAKPOINT2 = 30; //30 cm
unsigned const int BREAKPOINT3 = 50; //50 cm
unsigned int ready;
unsigned int t1 = 0; //Captura de subida
unsigned int t2 = 0; //Captura de descida
unsigned int ruido = 0;
unsigned int servo_arr[SERVO_DEG+1]; //Array contendo angulos do servo
long dist_cm; // Distancia em cm
long dt; // Delta t (t2 - t1)

void main()
{
    WDTCTL = WDTPW | WDTHOLD; // Segura o watchdog timer

    // Variaveis auxiliares do servo motor
    unsigned int i;
    const int servo_valor = ((MAX_DUTY - MIN_DUTY) / SERVO_DEG); // ~11
    unsigned int servo_atual = MIN_DUTY;

    for(i=0; i<SERVO_DEG; i++) // Mapear os possiveis valores (angulos) para o servo motor
    {
        servo_atual += servo_valor; //atual + 11
        servo_arr[i] = servo_atual; //insere atual na posicao do angulo
    }

    // Config dos GPIOs e Timers
    configPins();
    config_TA0();
    config_TA1();
    config_TA2();

    __enable_interrupt(); // GIE = 1

    while (1) // Loop infinito
    {
        while(!ready); // Espera o timer fazer duas capturas
        ready = 0;

        calc_dist(); // Chama funcao para calcular distancia
        leds(dist_cm); // Chama funcao para acender os leds
        prog_servo(dist_cm); // Chama funcao para mover o servo motor
    }
}

// Rotina de Tratamento da INT (ISR)
#pragma vector = TIMER1_A1_VECTOR
__interrupt void ta1_ccifg()
{
    switch (TA1IV) // Timer A1 Interrupt Vector
    {
    case 0x02: // CCR1
        if (TA1CCTL1 & CCI) // Se for flanco de subida
        {
            t1 = TA1CCR1; // Captura valor de t1
        }
        else // Se for flanco de descida
        {
            t2 = TA1CCR1; // Captura valor de t2
            ready = 1; // Alerta main que dt pode ser computado
        }
        break;
    }
}

void configPins() // Funcao para configurar os pinos
{

    // DIR dos LEDs
    P1DIR |= BIT0; // P1.0 = output (vermelho)
    P4DIR |= BIT7; // P4.7 = output (verde)

    // Estado inicial dos LEDs (desligados)
    P1OUT &= ~BIT0;
    P4OUT &= ~BIT7;

    // TA0.4 Trigger (Pin 1.5)
    P1DIR |= BIT5; // Trigger = output (P1.5)
    P1SEL |= BIT5; // Conecta pino no timer

    // TA1.1 Echo (Pin 2.0)
    P2DIR &= ~BIT0; // Echo = input (P2.0)
    P2SEL |= BIT0;  // Conecta pino no timer

    // TA2.2 Servo Motor (Pin 2.5)
    P2DIR |= BIT5; // Servo motor = output (P2.5)
    P2SEL |= BIT5; // Conecta pino no timer
}

void config_TA0() // Funcao para configurar o Timer A0 (TA0.4)
{
    TA0CTL = TASSEL__SMCLK | // Usando timer SM
             MC__UP        | // Modo de subida
             TACLR;          // Timer clear

    TA0CCR0 = 50000 - 1; // 50 ms
    TA0CCR4 = 1000 - 1; // Pulso do trigger (1000 microsec)

    TA0CCTL4 = OUTMOD_7; // Reset-set
}

void config_TA1() // Funcao para configurar o Timer A1 (TA1.1)
{
    TA1CTL = TASSEL__SMCLK | // Usando timer SM
             MC_2          |
             TACLR;     // Timer clear

    TA1CCTL1 = CAP    | // Modo de captura
               SCS    | // Sincronizar
               CCIS_0 | // CCIxA
               CM_3   | // Flanco de subida e descida
               CCIE;    // INT local
}

void config_TA2() // Funcao para configurar o Timer A2 (TA2.2)
{
    TA2CTL = TASSEL__SMCLK | // Usando timer SM
             MC__UP        | // Modo de subida
             TACLR;          // Timer clear

    TA2CCR0 = 20000 - 1; // Periodo de 20ms

    TA2CCR2 = 1; // Duty cycle 1%

    TA2CCTL2 = OUTMOD_7; // Reset-set
}

void calc_dist() // Funcao para calcular a distancia e converter para cm
{
    dt = t2 - t1; // Calcula delta t

    if (dt < 0) // Se a distancia calculada for negativa, torna positiva
    {
        dt = -dt;
    }

    dist_cm = (dt / 29.5) / 2; // 340 m/s = 29 microsec/cm (div por 2 pois e distancia de ida e volta)
}

void leds(dist_cm) // Funcao para acender os leds
{
    if (dist_cm > BREAKPOINT3) // Acima de 50 cm: desliga os dois
    {
        P1OUT &= ~BIT0;
        P4OUT &= ~BIT7;
    }
    else if ((dist_cm <= BREAKPOINT3) && (dist_cm > BREAKPOINT2)) // De 30 ate 50 cm: liga o verde
    {
        P1OUT &= ~BIT0;
        P4OUT |= BIT7;
    }
    else if ((dist_cm <= BREAKPOINT2) && (dist_cm > BREAKPOINT1)) // De 10 ate 30 cm: liga o vermelho
    {
        P1OUT |= BIT0;
        P4OUT &= ~BIT7;
    }
    else // Abaixo de 10 cm: liga os dois
    {
        P1OUT |= BIT0;
        P4OUT |= BIT7;
    }
}

void prog_servo(dist_cm) // Funcao para movimentar o servo motor
{
    if (dist_cm >= 50)
    {
        TA2CCR2 = servo_arr[180]; // 180 graus = 50 cm
    }
    else if ((dist_cm < 50) && (dist_cm >= 41))
    {
        TA2CCR2 = servo_arr[148]; // 148 graus = 41 cm
    }
    else if ((dist_cm < 41) && (dist_cm >= 33))
    {
        TA2CCR2 = servo_arr[119]; // 119 graus = 33 cm
    }
    else if ((dist_cm < 33) && (dist_cm >= 25))
    {
        TA2CCR2 = servo_arr[90]; // 90 graus = 25 cm
    }
    else if ((dist_cm < 25) && (dist_cm >= 16))
    {
        TA2CCR2 = servo_arr[58]; // 58 graus = 16 cm
    }
    else if ((dist_cm < 16) && (dist_cm >= 8))
    {
        TA2CCR2 = servo_arr[29]; // 29 graus = 8 cm
    }
    else if (dist_cm < 8)
    {
        TA2CCR2 = servo_arr[0]; // 0 graus = 0 cm
    }
    else
    {
        TA2CCR2 = servo_arr[180];
    }
}
