/*
 * ejercicio2_V0.c
 *
 * Created: 5/10/2019 15:24:44
 * Author : Diego Neudeck
 ejercicio 2 de trabajo integrador de digitales 2, contador de 99 segundo
 a traves de dos display con 3 bottones de entrada.
 */
 //puerto A, salida para el multiplexor, puerto B entradas de los botones, puerto C para multiplexacion y puerto D para prender el led
 
 //definiciones:------------------------------------------
 //-------------------------------------------------------
 #define TOP1 15624				//1seg modo CTC timer 1
 #define VCP0  99				//10ms modo normal timer 0
 #define F_CPU	16000000UL		//	Define Fcpu = 16 MHz p/cálculo de retardos
 #define leer_pin 1000
 #define unidad 0				//cambio el valor en el que se prende el display
 #define desena 0	
 #define but_deley 60			//tiempo de delay en los pulsadores			
//interrupcion de archivos de cabecera---------------------
//---------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>		//	Contiene macros para generar retardos.
#define is_high(p,b)	(p & _BV(b)) == _BV(b)	//	is_high(p,b) p/testear si el bit b de p es 1.
#define is_low(p,b)		(p & _BV(b)) == 0		//	is_low(p,b) p/testear si el bit b de p es 0.

//variables globales
//-------------------------------------------------------------
int DISPLAY[10]={0b0000,0b0001,0b0010,0b0011,0b0100,0b0101,0b0110,0b0111,0b1000,0b1001};
int des=desena;
int uni=unidad;
int ban1=0;				//para multiplexacion de los display
int ban2=0;				//para saber cuando aprieto P1
int ban3=0;				//para activar el led
int cont_led=0;
int led_pren=0;
int contactivo=0;

//DEclaracion de funciones-----------------------------------------
//-------------------------------------------------------------
void configPUERTOS();
void configTIMER0();
void configTIMER1();

//Rutinas de interrupcion-------------------------------------
//-------------------------------------------------------------
ISR (TIMER0_OVF_vect)
{
	
	if(ban1==0)
	{
		PORTC = 0x01;				//corresponde al displey unidad
		PORTA = DISPLAY[uni]; 		//Escribo en unidad el numero en que esta uni
		ban1 = 1;
	}
	else
	{
		PORTC = 0x02;
		PORTA = DISPLAY[des];			//escribo en el display unidad el valor de des
		ban1 = 0;						
	}
	
}

ISR (TIMER1_COMPA_vect)
{
	if(ban2==1)
	{
		led_pren=1;					//para no variar los numeros mientras esta decrementando
		contactivo=1;
		uni=uni-1;
		if(uni==-1)
		{
			des=des-1;
			uni=9;
			if(des==-1)
			{
				des=0;
				uni=0;
				ban2=0;
				ban3=1;
			}
		}
	}
	if(ban3==1)
	{			
		if(cont_led<4)
		{
			cont_led=cont_led+1;
			PORTD = 0x00;
			led_pren=1;
		}
		else 
		{
			cont_led=0;
			PORTD = 0x01;
			ban3=0;
			des=desena;
			uni=unidad;
			led_pren=0;
		}
	}
}
int main(void)
{
	configPUERTOS();			
	configTIMER0();
	configTIMER1();
	TIFR0= 0x00;							//borro los flags
	TIFR1= 0x00;
	sei();
    while (1) 
    {	
//inicia la secuencia y la pone en pausa. P1
		_delay_ms(but_deley);
		if (is_low(PINB,PB0))	//	testeo bits cero que es del pulsador P1
		{
			if(contactivo==0)				//si el contador esta activo pongo en pausa
				ban2=1;						//con la bandera contactivo.
			else
			{
				ban2=0;
				contactivo=0;
			}
		}
//-----------------------------------------------------------------------------------------
//pulsador P2, incrementa y resetea si esta activo el contador-----------------------------
		_delay_ms(but_deley);
		if(is_low(PINB,PB1))		//testeo bits 1 que es el incremento (P2)
		{	
			if(led_pren==0)			//con led_prende se que no inicializo el contador
			{
			uni=uni+1;
			if(uni==10)
			{
				uni=0;
				des=des+1;
				if(des==10)
				{
					des=0;
				}
			}
			}
			else
			{
				contactivo=0;			//asi al apretar P1 empienzo el conteo nuevamente
				uni=unidad;
				des=desena;
				ban2=0;					//asi apago el contador
				led_pren=0;				//asi puedo apagar el led	
			}
			
		}
//--------------------------------------------------------------------------------------------
//pulsador P3 decrementa antes de iniciar el decremento
		_delay_ms(but_deley);
		if(is_low(PINB,PB2))		//testeo el bits 2 que es P3
		{
			if(led_pren==0)
			{
			uni=uni-1;
			if(uni==-1)
			{
				uni=9;
				des=des-1;
				if(des==-1)
				{
					des=9;
				}
			}
			}
		}
	}
}

//funciones:
//---------------------------------------------------------
//definicion de puertos y timers---------------------------

void configPUERTOS()
{
	DDRA = 0x0F;				//configuro como salida los 4 bits menos significativos 
	PORTA = 0x00;				//inicializo el puerto A en cero
	DDRC = 0x03;				//inicializo el puerto c como salidas de multiplexacion de display
	
	PORTC = 0x02;				//inicializo en uno.
	DDRD = 0x01;				//inicializo el puerto D como salida para el led
	PORTD = 0x01;				//inicializo en uno el puerto D para tener apagado el led.
	DDRB = 0x00;				//inicializo a b como entrada

	PORTB = 0x07;				//activo las resistencias internas del puerto B
	
}

void configTIMER0()
{
	TCCR0A = 0x00;				//configuro en modo normal
	TCCR0B = 0x04;				//con preescaler de 256
	TIMSK0 = 0x01;				//habilito por desbordamiento 
	TCNT0 = VCP0;				//precargo el registro 
}

void configTIMER1()
{
	TCCR1A = 0x00;
	TCCR1B = 0x0D;				//configuro en modo ctc y prescaler de 1024
	OCR1A = TOP1;				//cargo el valor de igualacion
	TIMSK1 = 0x02;				//habilito la interrupcion por igualacion (modo ctc)
	
}
