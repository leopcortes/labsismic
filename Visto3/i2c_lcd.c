#include <msp430.h> 
#include <stdint.h>

#define LCD 0x27

#define CHAR  1
#define INSTR 0

#define BIT_RS BIT0
#define BIT_RW BIT1
#define BIT_EN BIT2
#define BIT_BL BIT3

void i2cConfig();
uint8_t I2C_WRITE(uint8_t addr, uint8_t * data, uint8_t nBytes);
uint8_t I2C_WR_BYTE(uint8_t addr, uint8_t byte);
void LCD_INIT();
void LCD_WR_NIBBLE(uint8_t nibble, uint8_t isChar);
void LCD_WR_BYTE(uint8_t byte, uint8_t isChar);
void lcd_print(uint8_t * str);

uint8_t line = 0x00;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	i2cConfig();
    LCD_INIT();

	//volatile uint8_t nack = I2C_WR_BYTE(0x27, 0);

	lcd_print("Hello World!");
    lcd_print("Ai=d,dddV   NNNN'\n'");
    lcd_print("Mn=d,dd  Mx=d,dd");

    while(1)
    {

    }
}

void i2cConfig()
{
    UCB0CTL1 = UCSWRST; // Resetar dispositivo
    UCB0CTL0 = UCSYNC | UCMODE_3 | UCMST; // Sincrono / Modo I2C / Mestre
    UCB0CTL1 |= UCSSEL__SMCLK; // SMCLK
    UCB0BRW = 100; // SCL @ SMCLK / 100 = 10 kHz

    P3SEL |=   BIT0 | BIT1; // Usar Modo I2C
    P3DIR &= ~(BIT0 | BIT1);
    P3REN |=   BIT0 | BIT1; // Habilitar resistores
    P3OUT |=   BIT0 | BIT1; // Pull-up

    UCB0CTL1 &= ~UCSWRST; // Zera o bit de RST
}

uint8_t I2C_WRITE(uint8_t addr, uint8_t * data, uint8_t nBytes)
{
    UCB0IFG = 0; // Zera flags

    UCB0I2CSA = addr; //sadd - Endereco do Escravo
    UCB0CTL1 |= UCTXSTT | UCTR; //Mestre transmissor, gera START e envia endereço

    while(!(UCB0IFG & UCTXIFG)); //Espera TXIFG (completar transm.)
    UCB0TXBUF = *data++; // Dado a ser escrito
    nBytes--;

    while(UCB0CTL1 & UCTXSTT);

    if(UCB0IFG & UCNACKIFG)
    {
        UCB0CTL1 |= UCTXSTP;
        while(UCB0CTL1 & UCTXSTP);
        return 1;
    }

    while(nBytes--)
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
}
void LCD_WR_NIBBLE(uint8_t nibble, uint8_t isChar)
{
    nibble <<= 4;

    I2C_WR_BYTE(LCD, nibble | BIT_BL | 0      | 0 | isChar);
    I2C_WR_BYTE(LCD, nibble | BIT_BL | BIT_EN | 0 | isChar);
    I2C_WR_BYTE(LCD, nibble | BIT_BL | 0      | 0 | isChar);
}

void LCD_WR_BYTE(uint8_t byte, uint8_t isChar)
{
    LCD_WR_NIBBLE(byte >> 4  , isChar);
    LCD_WR_NIBBLE(byte & 0x0F, isChar);
}

void lcd_print(uint8_t * str)
{
    while(*str)
    {
        if(*str == '\n')
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

void LCD_INIT()
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
}
