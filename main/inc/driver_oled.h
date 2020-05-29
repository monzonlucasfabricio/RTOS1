/*
 * driver_oled.h
 *
 *  Created on: 21 may. 2020
 *      Author: lucasml
 */

#ifndef MAIN_DRIVER_OLED_H_
#define MAIN_DRIVER_OLED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdbool.h>
#include "ssd1366.h"
#include "font8x8_basic.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

/* Defino pines SDA y SCL para el display OLED */
#define SDA_PIN GPIO_NUM_15
#define SCL_PIN GPIO_NUM_2

/* Prototipo de funciones globales */
void DisplayInit(void);
void displayclear(void);
void Display_msj_bienvenida(void);
void SetCursor(uint8_t x, uint8_t y);
void OledPrint(char *text);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_DRIVER_OLED_H_ */
