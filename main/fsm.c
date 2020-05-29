/*
 * fsm.c
 *
 *  Created on: 27 may. 2020
 *      Author: lucasml
 */

#include "fsm.h"
#include "driver_oled.h"

char modo_automatico[] = "Modo: Automatico";
char modo_manual[] = "Modo: Manual";
char luz_encendida[] = "Luz: Encendida";
char luz_apagada[] = "Luz: Apagada";
char borrarpalabra[] = "                ";


/**
 * @brief Initialization of the finite state machine
 * @param[in]	control_t *place -> Complete struct for the fsm
 * @param[in]	uint8_t gpio_relay -> GPIO pin number for the relay
 * @param[in]	char nombre_lugar -> Name of the device place
 */
void fsminit(control_t *place,uint8_t gpio_relay,char *nombre_lugar){

	place -> modo = AUTOMATICO;
	place -> relay = APAGADO;
	place -> gpio_relay = gpio_relay;
	place -> nombre = nombre_lugar;
	gpio_set_direction((place -> gpio_relay),GPIO_MODE_OUTPUT);
	gpio_set_pull_mode((place -> gpio_relay),GPIO_PULLUP_ONLY);

	SetCursor(0,0);
	OledPrint(nombre_lugar);
}

/**
 * @brief Control for the finite state machine
 * @param[in]	control_t *place -> Complete struct for the fsm
 */
void fsmcontrol(control_t *place){

	switch(place -> modo){

	case AUTOMATICO:
	{
		SetCursor(0,5);
		OledPrint(borrarpalabra);
		OledPrint(modo_automatico);
		switch(place->relay){
		case APAGADO:
		{
			gpio_set_level((place -> gpio_relay), 1);
			SetCursor(0,7);
			OledPrint(borrarpalabra);
			OledPrint(luz_apagada);
		}
		break;

		case ENCENDIDO:
		{
			gpio_set_level((place -> gpio_relay), 0);
			SetCursor(0,7);
			OledPrint(borrarpalabra);
			OledPrint(luz_encendida);
		}
		break;
		}
	}
	break;

	case MANUAL:
	{
		SetCursor(0,5);
		OledPrint(borrarpalabra);
		OledPrint(modo_manual);
		switch(place->relay){
		case APAGADO:
		{
			gpio_set_level((place -> gpio_relay), 1);
			SetCursor(0,7);
			OledPrint(borrarpalabra);
			OledPrint(luz_apagada);
		}
		break;

		case ENCENDIDO:
		{
			gpio_set_level((place -> gpio_relay), 0);
			SetCursor(0,7);
			OledPrint(borrarpalabra);
			OledPrint(luz_encendida);
		}
		break;
		}
	}
	break;

	}
}
