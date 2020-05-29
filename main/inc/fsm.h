/*
 * fsm.h
 *
 *  Created on: 27 may. 2020
 *      Author: lucasml
 */

#ifndef MAIN_INC_FSM_H_
#define MAIN_INC_FSM_H_


#include "stdint.h"
#include "stdio.h"
#include "driver/gpio.h"

typedef enum {
	AUTOMATICO,
	MANUAL
}modo_t;

typedef enum{
	ENCENDIDO,
	APAGADO
}estado_relay_t;

typedef struct{
	modo_t modo;
	estado_relay_t relay;
	gpio_num_t gpio_relay;
	uint16_t temperatura;
	char *nombre;
}control_t;


void fsminit(control_t *place,uint8_t gpio_relay,char *nombre_lugar);
void fsmcontrol(control_t *place);

#endif /* MAIN_INC_FSM_H_ */
