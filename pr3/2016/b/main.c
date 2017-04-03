#include <stdio.h>
#include "44b.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

#define N 4 //Tamaño del buffer tmrbuffer
#define M 128 //Tamaño del buffer readlineBuf que se pasa como parámetro a la rutina readline

/* Variables para la gestión de la ISR del teclado
 * 
 * Keybuffer: puntero que apuntará al buffer en el que la ISR del teclado debe
 *            almacenar las teclas pulsadas
 * keyCount: variable en el que la ISR del teclado almacenará el número de teclas pulsadas
 */
volatile static char *keyBuffer = NULL;
volatile static int keyCount = 0;

/* Variables para la gestion de la ISR del timer
 * 
 * tmrbuffer: puntero que apuntará al buffer que contendrá los dígitos que la ISR del
 *            timer debe mostrar en el display de 8 segmentos
 * tmrBuffSize: usado por printD8Led para indicar el tamaño del buffer a mostrar
 */
volatile static char *tmrBuffer = NULL;
volatile static int tmrBuffSize = 0;

//Variables globales para la gestión del juego
static char passwd[N];  //Buffer para guardar la clave inicial
static char guess[N];   //Buffer para guardar la segunda clave
char readlineBuf[M];    //Buffer para guardar la linea leída del puerto serie

//Configuración de la uart
struct ulconf uconf = {
	.ired = OFF,
	.par  = NONE,
	.stopb = 1,
	.wordlen = EIGHT,
	.echo = ON,
	.baud    = 115200,
};

enum state {
	INIT = 0,     //Init:       Inicio del juego
	SPWD = 1,     //Show Pwd:   Mostrar password
	DOGUESS = 2,  //Do guess:   Adivinar contraseña
	SGUESS = 3,   //Show guess: Mostrar el intento
	GOVER = 4     //Game Over:  Mostrar el resultado
};
enum state gstate; //estado/fase del juego 

//COMPLETAR: Declaración adelantada de las ISRs de timer y teclado (las marca como ISRs)
// Código de la parte1
void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));


// Función que va guardando las teclas pulsadas
static void push_buffer(char *buffer, int key)
{
	int i;
	for (i=0; i < N-1; i++)
		buffer[i] = buffer[i+1];
	buffer[N-1] = (char) key;
}

void timer_ISR(void)
{
	static int pos = 0; //contador para llevar la cuenta del dígito del buffer que toca mostrar


	D8Led_digit(tmrBuffer[pos]);

	//COMPLETAR: Visualizar el dígito en la posición pos del buffer tmrBuffer en el display

	// Si es el último dígito:
	//      Poner pos a cero,
	//      Parar timer
	//      Dar tmrBuffer valor NULL

	// Si no, se apunta al siguiente dígito a visualizar (pos)

	// COMPLETAR: Finalizar correctamente la ISR

	if(pos == tmrBuffSize-1){

		pos = 0;
		tmr_stop(TIMER0);
		tmrBuffer = NULL;

	} else {
		pos++;
	}

	ic_cleanflag(INT_TIMER0);
}

void printD8Led(char *buffer, int size)
{
	//Esta rutina prepara el buffer que debe usar timer_ISR (tmrBuffer)
	tmrBuffer = buffer;
	tmrBuffSize = size;
	
	//COMPLETAR: Arrancar el TIMER0 
	tmr_start(TIMER0);

	//COMPLETAR: Esperar a que timer_ISR termine (tmrBuffer)
	while(tmrBuffer!=NULL);
}

void keyboard_ISR(void)
{
	int key;

	/* Eliminar rebotes de presión */
	Delay(200);

	/* Escaneo de tecla */
	// COMPLETAR
	key = kb_scan();

	if (key != -1) {
		//COMPLETAR:
		//Si la tecla pulsada es F deshabilitar interrupciones por teclado y
		//poner keyBuffer a NULL

		//D8Led_digit(key);

		if(key == 15){
			keyBuffer = NULL;
			ic_disable(INT_EINT1);
		}
		// Si la tecla no es F guardamos la tecla pulsada en el buffer apuntado
		// por keybuffer mediante la llamada a la rutina push_buffer
		else{
			push_buffer(keyBuffer, key);
		}
		// Actualizamos la cuenta del número de teclas pulsadas
		keyCount++;
		/* Esperar a que la tecla se suelte, consultando el registro de datos rPDATG */		
		while (!(rPDATG & 0x02));
	}

	/* Eliminar rebotes de depresiÃ³n */
	Delay(200);

	//COMPLETAR: Finalizar correctamente la ISR
	ic_cleanflag(INT_EINT1);
}

int read_kbd(char *buffer)
{
	//Esta rutina prepara el buffer en el que keyboard_ISR almacenará las teclas 
	//pulsadas (keyBuffer) y pone a 0 el contador de teclas pulsadas
	keyBuffer = buffer;
	keyCount = 0;

	//COMPLETAR: Habilitar interrupciones por teclado
	ic_enable(INT_EINT1);

	//COMPLETAR: Esperar a que keyboard_ISR indique que se ha terminado de
	//introducir la clave (keyBuffer)
	while(keyBuffer != NULL);

	//COMPLETAR: Devolver número de teclas pulsadas
	return keyCount;
}

int readline(char *buffer, int size)
{
	int count = 0; //cuenta del número de bytes leidos
	char c;        //variable para almacenar el carácter leído

	if (size == 0)
		return 0;

	// COMPLETAR: Leer caracteres de la uart0 y copiarlos al buffer
	// hasta que llenemos el buffer (size) o el carácter leído sea
	// un retorno de carro '\r'
	// Los caracteres se leen de uno en uno, utilizando la interfaz
	// del módulo uart, definida en el fichero uart.h
	c = RdURXH0();
	while(count < size && c != '\r'){
		buffer[count] = c;

		count++;
		c = RdURXH0();
	}

	return count;
}


static int show_result()
{
	int error = 0;
	int i = 0;
	char buffer[2] = {0};

	// COMPLETAR: poner error a 1 si las contraseñas son distintas
	while(i<N && error == 0){
		if(passwd[i] != guess[i])
			error = 1;
		i++;
	}

	// COMPLETAR
	// MODIFICAR el código de la parte1 para que además de mostrar A o E en el
	// display de 8 segmentos se envíe por el puerto serie uart0 la cadena "\nCorrecto\n"
	// o "\nError\n" utilizando el interfaz del puerto serie definido en uart.h
	if(error == 0){
		buffer[0] = 10;
		buffer[1] = 10;
		uart_send_str(UART0, "\nCorrecto\n");
	} else {
		buffer[0] = 14;
		buffer[1] = 14;
		uart_send_str(UART0, "\nError\n");
	}
	printD8Led(buffer, 2);

	// COMPLETAR: esperar a que la ISR del timer indique que se ha terminado
	while(tmrBuffer != NULL);

	// COMPLETAR: Devolver el valor de error para indicar si se ha acertado o no
	return error;
}

int setup(void)
{

	D8Led_init();

	/* COMPLETAR: Configuración del timer0 para interrumpir cada segundo */
	tmr_set_prescaler(0, 255);
	tmr_set_divider(TIMER0, D1_4);
	tmr_set_count(TIMER0, 62500, 31250);
	tmr_update(TIMER0);
	tmr_set_mode(TIMER0, RELOAD);

	tmr_stop(TIMER0);



	/********************************************************************/

	// COMPLETAR: Registramos las ISRs

	pISR_TIMER0   = (int)(*timer_ISR);		//COMPLETAR: registrar la RTI del timer
	pISR_EINT1    = (int)(*keyboard_ISR);	//COMPLETAR: registrar la RTI del teclado

	/* Configuración del controlador de interrupciones*/

	ic_init();

	 /* Habilitamos la línea IRQ, en modo vectorizado y registramos una ISR para
	 * la línea IRQ
	 * Configuramos el timer 0 en modo IRQ y habilitamos esta línea
	 * Configuramos la línea EINT1 en modo IRQ y la habilitamos
	 */
	ic_conf_irq(ENABLE, VEC);
	ic_conf_fiq(DISABLE);
	ic_conf_line(INT_TIMER0, IRQ);
	ic_conf_line(INT_EINT1, IRQ);
	ic_enable(INT_TIMER0);
	ic_enable(INT_EINT1);

	/***************************************************/

	/***************************************************/
	//COMPLETAR: Configuración de la uart0
	
		// Hay que:
		// 1. inicializar el módulo
		uart_init();
		// 2. Configurar el modo linea de la uart0 usando la variable global uconf
		uart_lconf(UART0, uconf);
		// 3. Configurar el modo de recepción (POLL o INT) de la uart0
		uart_conf_rxmode(UART0, INT);
		// 4. Configurar el modo de transmisión (POLL o INT) de la uart0
		uart_conf_txmode(UART0, INT);

	/***************************************************/

	Delay(0);

	/* Inicio del juego */
	gstate = INIT;
	D8Led_digit(12);

	return 0;
}

static char ascii2digit(char c)
{
	char d = -1;

	if ((c >= '0') && (c <= '9'))
		d = c - '0';
	else if ((c >= 'a') && (c <= 'f'))
		d = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'F'))
		d = c - 'A' + 10;

	return d;
}


int loop(void)
{
	int count; //número de teclas pulsadas
	int error;

	//Máquina de estados

	switch (gstate) {
	case INIT:
		do {
			//COMPLETAR:
			//Visualizar una C en el display
			D8Led_digit(12);
			Delay(500);
			//Llamar a la rutina read_kbd para guardar los 4 dígitos en el buffer passwd
			count = read_kbd(passwd);
			//Esta rutina devuelve el número de teclas pulsadas.
			//Dibujar una E en el display si el número de teclas pulsadas es menor que 4
			if(count < N){
				D8Led_digit(14);
				Delay(10000);
			}
		} while (count < N);

		//COMPLETAR: Pasar al estado siguiente
		gstate = SPWD;

		break;

	case SPWD:

		// COMPLETAR:
		// Visualizar en el display los 4 dígitos del buffer passwd, para
		// ello llamar a la rutina printD8Led
		printD8Led(passwd, N);
		//Delay(10000);
		//COMPLETAR: Pasar al estado siguiente
		gstate = DOGUESS;

		break;

	case DOGUESS:
		Delay(10000);
		do {
			//COMPLETAR:
			/*
			 * 1. Mandar por el puerto serie uart0 la cadena "Introduzca passwd: "
			 *    usando el interfaz definido en uart.h
			 */
			uart_send_str(UART0, "Introduzca passwd: ");

			/*
			 * 2. Mostrar una F en el display
			 */
			D8Led_digit(15);

			/*
			 * 3. Llamar a la rutina readline para leer una línea del puerto
			 *    serie en el buffer readlineBuf, almacenando en count el
			 *    valor devuelto (número de caracteres leídos)
			 */
			count = readline(readlineBuf, 50); //TODO size?

			/*
			 * 4. Si el último caracter leído es un '\r' decrementamos count
			 *    para no tenerlo en cuenta
			 */
			if(readlineBuf[count] == '\r')
				count--;

			/*
			 * 5. Si count es menor de 4 la clave no es válida, mostramos
			 *    una E (digito 14) en el display de 8 segmentos y esperamos
			 *    1 segundo con Delay.
			 */
			if(count < 4){
				D8Led_digit(14);
				Delay(10000);
			}

		} while (count < 4);

		/* COMPLETAR: debemos copiar los 4 últimos caracteres de readline en
		 * el buffer guess, haciendo la conversión de ascii-hexadecimal a valor
		 * decimal. Para ello podemos utilizar la función ascii2digit
		 * definida más arriba.
		 */
		guess[0] = ascii2digit(readlineBuf[count-3]);
		guess[1] = ascii2digit(readlineBuf[count-2]);
		guess[2] = ascii2digit(readlineBuf[count-1]);
		guess[3] = ascii2digit(readlineBuf[count]);

		//COMPLETAR: Pasar al estado siguiente
		//Código de la parte1
		gstate = SGUESS;
		break;

	case SGUESS:
		//COMPLETAR:
		//Visualizar en el display los 4 dígitos del buffer guess,
		//para ello llamar a la rutina printD8Led
		printD8Led(guess, N);
		Delay(10000);
		//COMPLETAR: Pasar al estado siguiente
		gstate = GOVER;

		break;

	case GOVER:
		//COMPLETAR:
		//Mostrar el mensaje de acierto o error con show_result()
		error = show_result();
		Delay(10000);
		//Si he acertado el estado siguiente es INIT sino DOGUESS
		if(error == 0)
			gstate = INIT;
		else
			gstate = DOGUESS;
		break;
	}

	return 0;
}

int main(void)
{
	setup();

	while (1) {
		loop();
	}
}
