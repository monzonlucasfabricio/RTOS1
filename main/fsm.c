/*
 * fsm.c
 *
 *  Created on: 27 may. 2020
 *      Author: lucasml
 */

#include "driver_oled.h"
#include "fsm.h"

char modo_automatico[] = "Modo: Automatico";
char modo_manual[] = "Modo: Manual";
char luz_encendida[] = "Luz:ON";
char luz_apagada[] = "Luz:OFF";
char horario_trabajo[] = "H:Lab.";
char horario_fueradetrabajo[] = "H:NoLab.";
char borrarpalabra[] = "                ";


/**
 * @brief Initialization of the finite state machine
 * @param[in]	control_t *place -> Complete struct for the fsm
 * @param[in]	uint8_t gpio_relay -> GPIO pin number for the relay
 * @param[in]	char nombre_lugar -> Name of the device place
 */
void fsminit(control_t *place,uint8_t gpio_relay,char *nombre_lugar){

	place -> modo = AUTOMATIC;
	place -> relay = OFF;
	place -> gpio_relay = gpio_relay;
	place -> nombre = nombre_lugar;
	gpio_set_direction((place -> gpio_relay),GPIO_MODE_OUTPUT);
	gpio_set_pull_mode((place -> gpio_relay),GPIO_PULLUP_ONLY);
	place -> PIRsensor = DESACTIVATED;
	place -> timetable = OUTOFWORK;

	SSD1306_GotoXY (0, 0);
	SSD1306_Puts (nombre_lugar, &Font_7x10, 1);
	SSD1306_UpdateScreen();
}

/**
 * @brief Control for the finite state machine
 * @param[in]	control_t *place -> Complete struct for the fsm
 */
void fsmcontrol(control_t *place){

	switch(place -> modo){

	case AUTOMATIC:
	{
		SSD1306_GotoXY (0, 53);
		SSD1306_Puts ("                    ", &Font_7x10, 1);
		SSD1306_GotoXY (0, 53);
		SSD1306_Puts (modo_automatico, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
	break;

	case MANUAL:
	{
		SSD1306_GotoXY (0, 53);
		SSD1306_Puts ("                    ", &Font_7x10, 1);
		SSD1306_GotoXY (0, 53);
		SSD1306_Puts (modo_manual, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
	break;
	}


	switch(place->relay){
	case OFF:
	{
		gpio_set_level((place -> gpio_relay), 1);
		SSD1306_GotoXY (0, 43);
		SSD1306_Puts ("                    ", &Font_7x10, 1);
		SSD1306_GotoXY (0, 43);
		SSD1306_Puts (luz_apagada, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
	break;

	case ON:
	{
		gpio_set_level((place -> gpio_relay), 0);
		SSD1306_GotoXY (0, 43);
		SSD1306_Puts ("                    ", &Font_7x10, 1);
		SSD1306_GotoXY (0, 43);
		SSD1306_Puts (luz_encendida, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
	break;
	}


	switch(place -> timetable){
	case WORK:
	{
		SSD1306_GotoXY (0, 33);
		SSD1306_Puts ("                    ", &Font_7x10, 1);
		SSD1306_GotoXY (0, 33);
		SSD1306_Puts (horario_trabajo, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
	break;

	case OUTOFWORK:
	{
		SSD1306_GotoXY (0, 33);
		SSD1306_Puts ("                    ", &Font_7x10, 1);
		SSD1306_GotoXY (0, 33);
		SSD1306_Puts (horario_fueradetrabajo, &Font_7x10, 1);
		SSD1306_UpdateScreen();
	}
	break;
	}
}

