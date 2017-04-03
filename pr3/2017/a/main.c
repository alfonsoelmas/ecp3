#include <stdio.h>
#include "44b.h"
#include "button.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

struct RLstat {
	int moving;
	int speed;
	int iter;
	int direction;
	int position;
};

static struct RLstat RL = {
	.moving = 0,
	.speed = 5,
	.iter = 0,
	.direction = 0,
	.position = 0,

};

void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void button_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));



void timer_ISR(void)
{

	if (RL.moving) {

			D8Led_init();
			if(RL.direction == 1)
			{
				RL.position=RL.position++;
				if(RL.position == 6)
					RL.position = 0;
			}
			else
			{
				RL.position=RL.position--;
				if(RL.position == -1)
					RL.position = 5;
			}
			D8Led_segment(RL.position);
		}
	D8Led_segment(RL.position);
}

void keyboard_ISR(void)
{
	int key;


	Delay(200);


	key = kb_scan();

	if(key == 0){
		tmr_set_prescaler(0, 255);
		tmr_set_divider(TIMER0, D1_8);
		tmr_set_count(TIMER0, 62500, 0);
		tmr_update(TIMER0);

	}else if(key == 1){
		tmr_set_prescaler(0, 255);
		tmr_set_divider(TIMER0, D1_8);
		tmr_set_count(TIMER0, 31250, 0);
		tmr_update(TIMER0);

	}else if(key == 2){
		tmr_set_divider(TIMER0, D1_8);
		tmr_set_count(TIMER0, 15625, 0);
		tmr_update(TIMER0);
	}else if(key == 3){
		tmr_set_divider(TIMER0, D1_4);
		tmr_set_count(TIMER0, 15625, 0);
		tmr_update(TIMER0);

	} else {

	}

	//Esperar a que se deje de presionar la tecla leyendo el bit 1 del registro de datos del puerto G.
	digital val;
	portG_read(1, val)

	/* Eliminar rebotes */
	Delay(200);

	//Borrado de flag...
	ic_cleanflag(INT_EINT1);
}

void button_ISR(void)
{
	unsigned int whichint = rEXTINTPND;
	unsigned int buttons = (whichint >> 2) & 0x3;

	//COMPLETAR: BUT1 cambia el estado de LED1 y cambia dirección de
	//movimiento del led rotante
	if (buttons & BUT1) {
			led1_switch();
			if (RL.direction == 0)
				RL.direction = 1;
			else
				RL.direction = 0;
		}

	//COMPLETAR: BUT2 cambia el estado de LED2
	//activa movimiento de led rotante si parado
	//o lo para si está en movimiento (actuar sobre rutinas del TIMER)
	if (buttons & BUT2) {
			led2_switch();
			if (RL.moving == 0){
				RL.moving = 1;
			}
			else
				RL.moving = 0;
			}

	// eliminamos rebotes
	Delay(2000);

	//COMPLETAR: debemos borrar las peticiones de interrupción correspondientes a los pulsadores en rEXTINTPND
	rEXTINTPND |= (0x3 << 2);
}

int setup(void)
{
	leds_init();
	D8Led_init();
	D8Led_segment(RL.position);

	/* Port G:*/
	//COMPLETAR: utilizando el interfaz para el puerto G definido en gpio.h
	//		configurar los pines 6 y 7 del puerto G para poder generar interrupciones
	//		configurar que las interrupciones generadas por los pines 6 y 7 sean por flanco de bajada
	//		activar las correspondientes resistencias de pull-up.
	portG_conf(6,EINT); //Cuando tratemos interrupciones serán EINT (entrada x interrupción)
	portG_conf(7,EINT); //"" lo mismo que el comentario anterior
	portG_eint_trig(6, FALLING);
	portG_eint_trig(7, FALLING);
	portG_conf_pup(6, ENABLE);
	portG_conf_pup(7, ENABLE);


	/*Configuración del timer0 para que salte cada segundo*/
	tmr_set_prescaler(0, 255);
	tmr_set_divider(TIMER0, D1_8);
	tmr_set_count(TIMER0, 62500, 31250);
	tmr_update(TIMER0);
	tmr_set_mode(TIMER0, RELOAD);

	tmr_stop(TIMER0);

	if (RL.moving)
		tmr_start(TIMER0);


	//Realizamos el registro de interrupciones
	pISR_TIMER0   = (int)(*timer_ISR);			//Registramos la función del timer
	pISR_EINT4567   = (int)(*button_ISR);			//Registramos la función del button
	pISR_EINT1    = (int)(*keyboard_ISR);	//""					 del teclado

	//Iniciamos el controlador de interrupciones
	ic_init();

	//Modo vectorizado la línea IRQ...
	ic_conf_irq(ENABLE, VEC);
	ic_conf_fiq(DISABLE);

	ic_conf_line(INT_TIMER0, IRQ); //Usaremos estas señales
	ic_conf_line(INT_EINT4567, IRQ);
	ic_conf_line(INT_EINT1, IRQ);

	ic_enable(INT_GLOBAL);
	ic_enable(INT_TIMER0);
	ic_enable(INT_EINT4567);
	ic_enable(INT_EINT1);


	Delay(0);
	return 0;
}

int main(void)
{
	setup();

	while (1) {

	}
}