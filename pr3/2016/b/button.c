/*-------------------------------------------------------------------
**
**  Fichero:
**    button.c  10/6/2014
**
**    Estructura de Computadores
**    Dpto. de Arquitectura de Computadores y Autom�tica
**    Facultad de Inform�tica. Universidad Complutense de Madrid
**
**  Prop�sito:
**    Contiene las implementaciones de las funciones
**    para la gesti�n de los pulsadores de la placa de prototipado
**
**  Notas de dise�o:
**
**-----------------------------------------------------------------*/

#include "44b.h"
#include "utils.h"
#include "button.h"
#include "leds.h"
#include "gpio.h"

unsigned int read_button(void)
{
	unsigned int buttons = 0;
	enum digital valB6;
	enum digital valB7;

	int a = portG_read(6, &valB6);
	int b = portG_read(7, &valB7);

	if(valB7==LOW)
	{
		if(valB6==LOW)
		{
			buttons = 3;
		}
		else
		{
			buttons = 2;
		}
	}
	else
	{
		if(valB6==LOW)
		{
			buttons = 1;
		}
		else buttons = 0;
	}
	// COMPLETAR: tomar la implementación de la primera parte

	return buttons;
}


