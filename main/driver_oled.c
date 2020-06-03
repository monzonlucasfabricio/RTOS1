
#include "driver_oled.h"

/**
 * @brief SSD1306 data buffer
 * @note  This buffer will take all the data to write on the display RAM
 */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/**
 * @brief Private SSD1306 Structure
 */
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;


static SSD1306_t SSD1306;

/* I2C COMMUNICATION FUNCTIONS */
void SSD1306_WriteCommand(uint8_t byte){

	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
	i2c_master_write(cmd, &byte,sizeof(uint8_t), true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

}

void SSD1306_WriteNData(uint8_t *data, uint16_t count){

	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, data, count, true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

}

void SSD1306_WriteData(uint8_t byte){

	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, &byte, sizeof(uint8_t), true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

}

/* SSD1306 DISPLAY FUNCTIONS */

void SSD1306_UpdateScreen(void){
	uint8_t m;
	for (m = 0; m < 8; m++) {
		SSD1306_WriteCommand(0xB0 + m);											//!< Increment the PAGE from 0 to 7
		SSD1306_WriteCommand(0x00);												//!< Set Lower Column Start Address for Page Addressing Mode
		SSD1306_WriteCommand(0x10);												//!< Set Higher Column Start Address for Page Addressing Mode
		SSD1306_WriteNData(&SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);  //!< Write the data set on the buffer before
	}
}

void SSD1306_DisplayFill(SSD1306_COLOR_t Color){

	memset(SSD1306_Buffer, (Color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer)); //!< Write all the buffer with zeros or ones
}

void SSD1306_GotoXY(uint16_t x, uint16_t y) {

	/* Set write pointers */
	SSD1306.CurrentX = x;						//!< Set x location on the structure to copy the data
	SSD1306.CurrentY = y;						//!< Set y location on the structure to copy the data

}

char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color){

	uint32_t i, b, j;

	/* Check available space in LCD */
	if (
			SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) ||
			SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}

	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t) color);
			} else {
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}

	/* Increase pointer */
	SSD1306.CurrentX += Font->FontWidth;

	/* Return character written */
	return ch;
}

char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SSD1306_Putc(*str, Font, color) != *str) {
			/* Return error */
			return *str;
		}

		/* Increase string pointer */
		str++;
	}

	/* Everything OK, zero should be returned */
	return *str;
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) {

	/* Check if pixels are inverted */
	if (SSD1306.Inverted) {
		color = (SSD1306_COLOR_t)!color;						//!< Invert color if necessary
	}

	/* Set color */
	if (color == SSD1306_COLOR_WHITE) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);  //!< Assign a 1 to the position an then shift the result of y%8 times
	} else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}

	/* Example for x = 10, y = 2;
	 *
	 * SSD1306_Buffer[10 + (y/8)*128] = SSD1306_Buffer[42]
	 * 2 % 8 = 2
	 *
	 *
	 *
	 */

}

void SSD1306_DisplayInit(void) {

	//! I2C config for ESP32
	i2c_config_t i2c_config = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = SDA_PIN,
			.scl_io_num = SCL_PIN,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = 1000000
	};

	//! Driver Install
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0xAE, true); //!< Turn OFF display
	i2c_master_write_byte(cmd, 0x20, true); //!< Set memory addressing mode
	i2c_master_write_byte(cmd, 0x10, true); //!< Page addressing mode
	i2c_master_write_byte(cmd, 0xB0, true); //!< Set page Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0xC8, true); //!< Set COM output scan direction
	i2c_master_write_byte(cmd, 0x00, true); //!< Set Low column address
	i2c_master_write_byte(cmd, 0x10, true); //!< Set High column address
	i2c_master_write_byte(cmd, 0x40, true); //!< Set Start line address
	i2c_master_write_byte(cmd, 0x81, true); //!< Set contrast control register
	i2c_master_write_byte(cmd, 0xFF, true); //!< Set contrast
	i2c_master_write_byte(cmd, 0xA1, true); //!< Set segment re-map 0-127
	i2c_master_write_byte(cmd, 0xA6, true); //!< Set normal display
	i2c_master_write_byte(cmd, 0xA8, true); //!< Set multiplex ratio(1 to 64)
	i2c_master_write_byte(cmd, 0x3F, true); //!<
	i2c_master_write_byte(cmd, 0xA4, true);	//!< 0xA4, Output follows RAM content- 0xA5, Output ignores RAM content
	i2c_master_write_byte(cmd, 0xD3, true); //!< Set display offset
	i2c_master_write_byte(cmd, 0x00, true); //!< No offset
	i2c_master_write_byte(cmd, 0xD5, true); //!< Set display clock divide ratio/oscillator frequency
	i2c_master_write_byte(cmd, 0xF0, true); //!< Set divide ratio
	i2c_master_write_byte(cmd, 0xD9, true); //!< Set pre-charge period
	i2c_master_write_byte(cmd, 0x22, true); //!<
	i2c_master_write_byte(cmd, 0xDA, true); //!< Set COM pins hardware configuration
	i2c_master_write_byte(cmd, 0x12, true); //!<
	i2c_master_write_byte(cmd, 0xDB, true); //!< Set vcomh
	i2c_master_write_byte(cmd, 0x20, true); //!< 0x20,0.77xVCC
	i2c_master_write_byte(cmd, 0x8D, true); //!< Set dc-dc enable
	i2c_master_write_byte(cmd, 0x14, true); //!<
	i2c_master_write_byte(cmd, 0xAF, true); //!< Turn ON display
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	SSD1306_DisplayFill(SSD1306_COLOR_BLACK);

	/* Set default values */
	SSD1306.CurrentX = 0;					//!< Default X location
	SSD1306.CurrentY = 0;					//!< Default Y location

	/* Initialized OK */
	SSD1306.Initialized = 1;				//!< Successful initialization
}

void SSD1306_ON(void) {

	SSD1306_WriteCommand(0x8D);				//!< Set charge pump
	SSD1306_WriteCommand(0x14);				//!<
	SSD1306_WriteCommand(0xAF);				//!< Display ON
}

void SSD1306_OFF(void) {

	SSD1306_WriteCommand(0x8D);				//!< Set charge pump
	SSD1306_WriteCommand(0x10);				//!<
	SSD1306_WriteCommand(0xAE);				//!< Display OFF

}

void SSD1306_DisplayClear(void)
{
	SSD1306_DisplayFill (0);			//!< Fill display with 0x00
    SSD1306_UpdateScreen();				//!< Update after RAM was written
}

