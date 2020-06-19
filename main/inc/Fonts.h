/*
 * font8x8_basic.h
 *
 *  Created on: 2017/05/03
 *      Author: yanbe
 */

#ifndef MAIN_FONT8X8_BASIC_H_
#define MAIN_FONT8X8_BASIC_H_

#include <stdint.h>
#include <string.h>

typedef struct {
	uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef_t;

typedef struct {
	uint16_t Length;      /*!< String length in units of pixels */
	uint16_t Height;      /*!< String height in units of pixels */
} FONTS_SIZE_t;


extern FontDef_t Font_7x10;

extern FontDef_t Font_11x18;

extern FontDef_t Font_16x26;

char* FONTS_GetStringSize(char* str, FONTS_SIZE_t* SizeStruct, FontDef_t* Font);

#endif /* MAIN_FONT8X8_BASIC_H_ */


