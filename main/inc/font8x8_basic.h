/*
 * font8x8_basic.h
 *
 *  Created on: 2017/05/03
 *      Author: yanbe
 */

#ifndef MAIN_FONT8X8_BASIC_H_
#define MAIN_FONT8X8_BASIC_H_

#include <stdint.h>

/*
   Constant: font8x8_basic_tr
   Contains an 90 digree transposed 8x8 font map for unicode points 
   U+0000 - U+007F (basic latin)
   
   To make it easy to use with SSD1306's GDDRAM mapping and API,
   this constant is an 90 degree transposed.
   The original version written by Marcel Sondaar is availble at:
   https://github.com/dhepper/font8x8/blob/master/font8x8_basic.h 

   Conversion is done via following procedure:
   
	for (int code = 0; code < 128; code++) {
		uint8_t trans[8];
		for (int w = 0; w < 8; w++) {
			trans[w] = 0x00;
			for (int b = 0; b < 8; b++) {
				trans[w] |= ((font8x8_basic[code][b] & (1 << w)) >> w) << b;
			}
		}
	
		for (int w = 0; w < 8; w++) {
			if (w == 0) { printf("    { "); }
			printf("0x%.2X", trans[w]);
			if (w < 7) { printf(", "); }
			if (w == 7) { printf(" },   // U+00%.2X (%c)\n", code, code); }
		}
	}
*/

uint8_t font8x8_basic_tr[128][8];


#endif /* MAIN_FONT8X8_BASIC_H_ */


