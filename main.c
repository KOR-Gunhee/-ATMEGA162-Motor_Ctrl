/*
 * Motor_Ctrl.c
 *
 * Created: 2020-03-13 오후 2:09:43
 * Author : ghhan
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>

#define MY_F_CPU 16000000UL
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((MY_F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define MAX_LENGTH 8

#define STOP	(0x1u<<0)
#define START	~STOP
#define Forword	(0x1u<<1)
#define Reverse	~Forword



uint32_t	data_Head=0;
uint32_t	data_Tail=0;
uint8_t		data_Buff[MAX_LENGTH]={0,};

uint8_t		motor_status=0;
uint8_t		action_status=0;
uint8_t		turn_status=0;

uint8_t		action_val=0;
uint8_t		turn_val=0;
uint8_t		pwm_val=0;

void usartInit()
{
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);
	UBRR0L = (uint8_t) UBRR_VALUE;
	
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01); //Charecter size : 8비트
	UCSR0C &= ~(1<<USBS0); //stop  bit : 1비트
	UCSR0C &= ~((1<<UPM01)|(1<<UPM00)); // no parity mode
	
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);	
}

void transmitByte(unsigned char ch)
{
	while(!(UCSR0A&(1<<UDRE0))){};
	
	UDR0 = ch;                                            /* send data */
}

uint8_t receiveByte(void) {
	while(!(UCSR0A&(1<<RXC0))){};
	return UDR0;                                /* return register value */
}

int printString(char *str) {
	//uint8_t i = 0;
	while (*str!= 0x00) {
		transmitByte(*str);
		str++;
	}
	return 0;
}

void readString(char str[], uint8_t maxLength) {
	char response;
	uint8_t i;
	i = 0;
	while (i < (maxLength - 1)) {                   /* prevent over-runs */
		response = receiveByte();
		transmitByte(response);                                    /* echo */
		if (response == '\r') {                     /* enter marks the end */
			break;
		}
		else {
			str[i] = response;                       /* add in a letter */
			i++;
		}
	}
	str[i] = 0;                          /* terminal NULL character */
}

void pwminit()
{
	DDRB = 0xFF;
	TCCR0 = 0b01101010;
}

void boardinit()
{
	DDRA = 0x03;
	PORTA = 0x02;
	OCR0 = 0x80;

	motor_status = PORTA;

	action_val=0;
	turn_val=1;
	pwm_val=2;
}

void Display_Menu()
{
	printString("\n\r");
	printString("----------SELECT MODE----------\n\r");
	printString("-1 : START/STOP               -\n\r");
	printString("-2 : FORWARD/REVERSE          -\r\n");
	printString("-3 : PWM 15%                  -\r\n");
	printString("-4 : PWM 30%                  -\n\r");
	printString("-5 : PWM 50%                  -\n\r");
	printString("-6 : PWM 70%                  -\n\r");
	printString("-7 : PWM 100%                 -\n\r");
	printString("-------------------------------\n\r");
}

void Display_Setting()
{
	printString("----------SETTING LIST---------\n\r");
	if(action_val==0){printString ("-START                        -\n\r");}
	else if(action_val==1){printString("-STOP                         -\n\r");}
	if (turn_val==0){printString("-REVERSE                      -\n\r");}
	else if(turn_val==1){printString("-FORWARD                      -\n\r");}
	if (pwm_val==0){printString("-PWM 17%                      -\n\r");}
	else if (pwm_val==1){printString("-PWM 30%                      -\n\r");}
	else if (pwm_val==2){printString("-PWM 50%                      -\n\r");}
	else if (pwm_val==3){printString("-PWM 70%                      -\n\r");}
	else if (pwm_val==4){printString("-PWM 100%                     -\n\r");}
	printString("-------------------------------\n\r");
}

void Select_Mode(uint8_t data)
{
	printString("\t= ");
	
	switch (data)
	{
		case '1': 
		if ((motor_status&STOP)==0)
		{
			printString("STOP\n\r");
			action_val = 1;
			PORTA = (turn_status|STOP);
		}
		else
		{
			printString("START\n\r");
			action_val = 0;
			PORTA &= (turn_status|START);
		}		
		motor_status = PORTA;
		action_status = (motor_status&STOP);

		break;		 
		case '2':
		if ((motor_status&Forword)==0)
		{
			printString("FORWARD\n\r");
			turn_val = 1;
			PORTA = (action_status|Forword);
		}
		else
		{
			printString("REVERSE\n\r");
			turn_val = 0;
			PORTA &= (action_status|Reverse);
		}
		motor_status = PORTA;
		turn_status = (motor_status&Forword);
		break;		 
		case '3':		 
		printString("PWM 17%\n\r");
		pwm_val=0;
		OCR0 = 0x2D;
		break;		  
		case '4':		  
		printString("PWM 30%\n\r");
		pwm_val=1;
		OCR0 = 0x4D;
		break;
		case '5':
		printString("PWM 50%\n\r");
		pwm_val=2;
		OCR0 = 0x80;
		break;		  
		case '6':		  
		printString("PWM 70%\n\r");
		pwm_val=3;
		OCR0 = 0xB3;
		break;
		case '7':
		printString("PWM 100%\n\r");
		pwm_val=4;
		OCR0 = 0xFF;
		break;
		default : 
		printString("no data\n\r");
		Display_Menu();
		break;
	}
	Display_Setting();
	printString("SELECT MODE : ");
}

void Select_Test_Mode(uint8_t data)		//PWM TEST function
{
	printString("\t= ");
	
	switch (data)
	{
		case '1':
		if ((motor_status&STOP)==0)
		{
			printString("STOP\n\r");
			action_val = 1;
			PORTA = (turn_status|STOP);
		}
		else
		{
			printString("START\n\r");
			action_val = 0;
			PORTA &= (turn_status|START);
		}
		motor_status = PORTA;
		action_status = (motor_status&STOP);

		break;
		case '2':
		if ((motor_status&Forword)==0)
		{
			printString("FORWARD\n\r");
			turn_val = 1;
			PORTA = (action_status|Forword);
		}
		else
		{
			printString("REVERSE\n\r");
			turn_val = 0;
			PORTA &= (action_status|Reverse);
		}
		motor_status = PORTA;
		turn_status = (motor_status&Forword);
		break;
		case '3':
		printString("PWM 16%\n\r");
		pwm_val=0;
		OCR0 = 0x2A	;
		break;
		case '4':
		printString("PWM 17%\n\r");
		pwm_val=1;
		OCR0 = 0x2C;
		break;
		case '5':
		printString("PWM 18%\n\r");
		pwm_val=2;
		OCR0 = 0x2E;
		break;
		case '6':
		printString("PWM 19%\n\r");
		pwm_val=3;
		OCR0 = 0x30;
		break;
		case '7':
		printString("PWM 20%\n\r");
		pwm_val=4;
		OCR0 = 0x33;
		break;
		default :
		printString("no data\n\r");
		Display_Menu();
		break;
	}
	Display_Setting();
	printString("SELECT MODE : ");
}

void GetString(uint8_t data)
{
	if(data == '\r')
	{
		Select_Mode(data_Buff[data_Tail-1]);
		data_Tail=0;
	}
	else
	{
		transmitByte(data);
		if(data_Tail>=MAX_LENGTH){printString("\n\r");printString("overlord\n\r");data_Tail=0;}
		data_Buff[data_Tail++]=data;
	}
}
int main(void)
{
	/* Replace with your application code */
	usartInit();
	pwminit();
	boardinit();
	Display_Menu();
	Display_Setting();
	printString("SELECT MODE : ");
	
	while(1)
 		{
			GetString(receiveByte());
		}
}

