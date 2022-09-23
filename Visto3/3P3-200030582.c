/* 
    Visto 3 - Lab Sis Mic
    Leonardo Pereira Cortes - 200030582
*/

#include <msp430.h>
#include <stdint.h>

// Constantes
#define SERVO_DEG 180 // Maior angulo possivel
#define MIN_DUTY 500 // Menor duty cycle
#define MAX_DUTY 2500 // Maior duty cycle

#define LCD 0x27 // Endereco do escravo ??? - nao reconhece 0x27, 0x3F nem 0x20 - 0x27

#define CHAR  1
#define INSTR 0

#define BIT_RS BIT0
#define BIT_RW BIT1
#define BIT_EN BIT2
#define BIT_BL BIT3

// Funcoes de configuracao
void configPins();
void config_TA0();
void config_TA2();
void config_ADC();
void config_I2C();

// Funcoes LCD
uint8_t I2C_WRITE(uint8_t addr, uint8_t * data, uint8_t nBytes);
uint8_t I2C_WR_BYTE(uint8_t addr, uint8_t byte);
void LCD_INIT();
void LCD_WR_NIBBLE(uint8_t nibble, uint8_t isChar);
void LCD_WR_BYTE(uint8_t byte, uint8_t isChar);
void lcd_print(uint8_t * str);

// Funcoes Servo Motor
void prog_servo(int valor); // prog servo para exibir dados

// Variaveis globais
unsigned int servo_arr[SERVO_DEG+1]; // Array contendo angulos do servo
volatile uint16_t adcResult[2];
volatile uint16_t adc_SW;
uint8_t n = 0;
uint8_t n_antigo;
uint8_t line = 0x00;

/* ================= Main ================= */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Funcao para converter valores do servo em angulos
    unsigned int i;
    const int servo_valor = ((MAX_DUTY - MIN_DUTY) / SERVO_DEG); // ~11
    unsigned int servo_atual = MIN_DUTY;

    for(i=0; i<SERVO_DEG; i++) // Mapear os possiveis valores (angulos) para o servo motor
    {
        servo_atual += servo_valor; //atual + 11
        servo_arr[i] = servo_atual; //insere atual na posicao do angulo
    }

    // Configs Iniciais
    configPins();
    config_TA0();
    config_TA2();
    config_ADC();
    //config_I2C();
    //LCD_INIT();

    //lcd_print("Ai=d,dddV   NNNN'\n'");
    //lcd_print("Mn=d,dd  Mx=d,dd");

    __enable_interrupt(); // INT

    while(1)
    {
        n_antigo = n;
        if(adc_SW == 0) // Se o botao SW for pressionado
        {
            if(n_antigo == 0){ // Troca de canais
                n = 1;
            }else{
                n = 0;
            }
        }else{
            n = n_antigo;
        }
        prog_servo(adcResult[n]); // Programacao do servo com base no valor lido do canal selecionado
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
    adcResult[0] = (ADC12MEM0 + ADC12MEM2 + ADC12MEM4 + ADC12MEM6)/4; // 4 canais para o Eixo X
    adcResult[1] = (ADC12MEM1 + ADC12MEM3 + ADC12MEM5 + ADC12MEM7)/4; // 4 canais para o Eixo Y
    adc_SW = ADC12MEM8; // Botao SW

    ADC12CTL0 |= ADC12ENC;
}

/* ================= Funcoes ================= */

void configPins() // Funcao para configurar os pinos
{
    // TA2.2 Servo Motor (Pin 2.5)
    P2DIR |= BIT5; // Servo motor = output (P2.5)
    P2SEL |= BIT5; // Conecta pino no timer
}

/* Funcoes para o I2C *//*
void config_I2C() // Funcao para configurar pinos I2C - UCSB0 (P3.0 -> SDA e P3.1 -> SCL)
{
    UCB0CTL1 |= UCSWRST; // Resetar dispositivo
    UCB0CTL0 = UCSYNC | UCMODE_3 | UCMST; // Sincrono / Modo I2C / Mestre
    UCB0CTL1 = UCSSEL__SMCLK; // SMCLK
    UCB0BRW = 100; // SCL @ SMCLK / 100 = 10 kHz

    P3SEL |=  (BIT0 | BIT1); // Usar Modo I2C
    P3DIR &= ~(BIT0 | BIT1);
    P3REN |=  (BIT0 | BIT1); // Habilitar resistores
    P3OUT |=  (BIT0 | BIT1); // Pull-up

    UCB0CTL1 &= ~UCSWRST; // Zera o bit de RST
}

uint8_t I2C_WRITE(uint8_t addr, uint8_t * data, uint8_t nBytes)
{
    UCB0IFG = 0; // Zera flags

    UCB0I2CSA = addr; //sadd - Endereco do Escravo
    UCB0CTL1 |= UCTXSTT | UCTR; //Mestre transmissor, gera START e envia endereco

    while(!(UCB0IFG & UCTXIFG)); //Espera TXIFG (completar transm.)
    UCB0TXBUF = *data++; // Dado a ser escrito
    nBytes--;

    while(UCB0CTL1 & UCTXSTT); // Espera ACK/NACK

    if(UCB0IFG & UCNACKIFG) // Se for NACK manda stop
    {
        UCB0CTL1 |= UCTXSTP;
        while(UCB0CTL1 & UCTXSTP);
        return 1;
    }

    while(nBytes--) // Envio dos dados
    {
        while(!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = *data++;
    }

    while(!(UCB0IFG & UCTXIFG)); // Espera Transmitir

    UCB0CTL1 |= UCTXSTP; // Gera STOP

    while(UCB0CTL1 & UCTXSTP); // Espera STOP

    return 0;
}

uint8_t I2C_WR_BYTE(uint8_t addr, uint8_t byte)
{
    return I2C_WRITE(addr, &byte, 1);
}*/

/* Funcoes para o LCD *//*
void LCD_WR_NIBBLE(uint8_t nibble, uint8_t isChar)
{
    nibble <<= 4;

    I2C_WR_BYTE(LCD, nibble | BIT_BL | 0 | 0 | isChar); //                __
    I2C_WR_BYTE(LCD, nibble | BIT_BL | BIT_EN | 0 | isChar); // Enable __|  |__
    I2C_WR_BYTE(LCD, nibble | BIT_BL | 0 | 0 | isChar);
}

void LCD_WR_BYTE(uint8_t byte, uint8_t isChar)
{
    LCD_WR_NIBBLE(byte >> 4  , isChar);
    LCD_WR_NIBBLE(byte & 0x0F, isChar);
}

void lcd_print(uint8_t * str) // Funcao para escrever no LCD
{
    while(*str)
    {
        if(*str == '\n') // Quebra de linha
        {
            line ^= BIT6;
            LCD_WR_BYTE(BIT7 | line, INSTR);
        }
        else
        {
            LCD_WR_BYTE(*str, CHAR);
        }
        str++;
    }
}

void LCD_INIT() // Funcao para inicializar o LCD
{
    LCD_WR_NIBBLE(0x3, INSTR);
    LCD_WR_NIBBLE(0x3, INSTR);
    LCD_WR_NIBBLE(0x3, INSTR);
    LCD_WR_NIBBLE(0x2, INSTR);

    LCD_WR_BYTE(0x06, INSTR);
    LCD_WR_BYTE(0x0F, INSTR);
    LCD_WR_BYTE(0x14, INSTR);
    LCD_WR_BYTE(0x28, INSTR);

    LCD_WR_BYTE(0x01, INSTR);
}*/

/* Funcoes para o Joystick (Conversor AD) */
void config_TA0() // Funcao para configurar o Timer A0
{
    TA0CTL = MC__UP | TASSEL__SMCLK;
    TA0CCR0 = 100 - 1;
    TA0CCTL0 = CCIE;
}

void config_ADC() // Funcao para configurar o ADC
{
    ADC12CTL0 &= ~ADC12ENC;

    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;

    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_0 | ADC12SHP | ADC12DIV_0 | ADC12SSEL_0 | ADC12CONSEQ_1;

    ADC12CTL2 = ADC12TCOFF | ADC12RES_2 | ADC12SR;

    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_1; // Eixo X (P6.1)
    ADC12MCTL1 = ADC12SREF_0 | ADC12INCH_2; // Eixo Y (P6.2)
    ADC12MCTL2 = ADC12SREF_0 | ADC12INCH_1; // Eixo X (P6.1)
    ADC12MCTL3 = ADC12SREF_0 | ADC12INCH_2; // Eixo Y (P6.2)
    ADC12MCTL4 = ADC12SREF_0 | ADC12INCH_1; // Eixo X (P6.1)
    ADC12MCTL5 = ADC12SREF_0 | ADC12INCH_2; // Eixo Y (P6.2)
    ADC12MCTL6 = ADC12SREF_0 | ADC12INCH_1; // Eixo X (P6.1)
    ADC12MCTL7 = ADC12SREF_0 | ADC12INCH_2; // Eixo Y (P6.2)

    ADC12MCTL8 = ADC12SREF_0 | ADC12INCH_3 | ADC12EOS; // SW (P6.3)

    ADC12IE = ADC12IE0;

    ADC12CTL0 |= ADC12ENC;
}

/* Funcoes para o Servo Motor */

void config_TA2() // Funcao para configurar o Timer A2 (TA2.2)
{
    TA2CTL = TASSEL__SMCLK | // Usando timer SM
             MC__UP        | // Modo de subida
             TACLR;          // Timer clear

    TA2CCR0 = 20000 - 1; // Periodo de 20ms

    TA2CCR2 = 1; // Duty cycle 1%

    TA2CCTL2 = OUTMOD_7; // Reset-set
}

void prog_servo(int valor) // Funcao para movimentar o servo motor
{
    float voltagem; // Variavel aux
    voltagem = ((valor * 3.3) / 4085); // Conversao do digital para analogico

    if(voltagem >= 3){ // v > 3V = 33 graus
        TA2CCR2 = servo_arr[33];
    }
    else if(voltagem < 3 && voltagem >= 2.5){ // 3V < v < 2.5V = 28 graus
        TA2CCR2 = servo_arr[28];
    }
    else if(voltagem < 2.5 && voltagem >= 2){ // 2.5V < v < 2V = 23 graus
        TA2CCR2 = servo_arr[23];
    }
    else if(voltagem < 2 && voltagem >= 1.5){ // 2V < v < 1.5V = 18 graus
        TA2CCR2 = servo_arr[18];
    }
    else if(voltagem < 1.5 && voltagem >= 1){ // 1.5V < v < 1V = 13 graus
        TA2CCR2 = servo_arr[13];
    }
    else if(voltagem < 1 && voltagem >= 0.5){ // 1V < v < 0.5V = 7 graus
        TA2CCR2 = servo_arr[7];
    }
    else if(voltagem < 0.5 && voltagem >= 0){ // 0.5V < v < 0V = 0 graus
        TA2CCR2 = servo_arr[0];
    }
}
