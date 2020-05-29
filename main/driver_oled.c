/*
 * driver_oled.c
 *
 *  Created on: 21 may. 2020
 *      Author: lucasml
 *
 *      The Arduino library is explained in this website
 *      https://robologs.net/2018/03/11/como-conectar-una-pantalla-oled-ssd1306-con-arduino-sin-libreria/
 *      If you want to learn about how it works or improve this library please check the website
 *      This library was written for the ESP32-Wroom-32 module and ESP-IDF API.
 *
 */

#include "driver_oled.h"


/**
 * @brief Display Init --> Bytes sending to memory on the display chip
 * @param[in] No params
 */
void DisplayInit(void) {

	i2c_config_t i2c_config = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = SDA_PIN,
			.scl_io_num = SCL_PIN,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = 1000000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_MUX_RATIO, true);
	i2c_master_write_byte(cmd, 0x3F, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_OFFSET, true);
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_START_LINE, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_PIN_MAP, true);
	i2c_master_write_byte(cmd, 0x12, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST , true);
	i2c_master_write_byte(cmd, 0x7F, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_RAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);
	i2c_master_write_byte(cmd, 0x80, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);

	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

/**
 * @brief Set x and y position on the display oled
 * Xmax is 8
 * Ymax is 8
 * @param[in] x point on display
 * @param[in] y point on display
 */
void SetCursor(uint8_t x, uint8_t y){

	uint8_t Xmax = 8;
	uint8_t Ymax = 8;

	if(x <= Xmax && y <= Ymax){
		i2c_cmd_handle_t cmd;
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, 0x00, true);
		i2c_master_write_byte(cmd, 0x10 | x, true);
		i2c_master_write_byte(cmd, 0xB0 | y, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
	else{
		printf("Verify X and Y points");
	}
}

/**
 * @brief Write on the display in the set position by SetCursor()
 * @param[in] String for the screen
 */
void OledPrint(char *text){

	uint8_t text_len = strlen(text);

	for (uint8_t i = 0; i < text_len; i++) {
		i2c_cmd_handle_t cmd;
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}


/**
 * @brief Clear the display screen
 * @param[in] no params
 */
void Displayclear(void){

	char text[] = "                ";

	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t i = 0;

	for(i = 0; i<=8 ; i++){
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, 0x10 , true);
		i2c_master_write_byte(cmd, 0xB0 | i , true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		for (uint8_t j = 0; j < text_len; j++) {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[j]], 8, true);
			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}
}


/**
 * @brief Welcome message
 * @param[in] no params
 */
void Display_msj_bienvenida(void) {

	char text[] = "----STARTING----";

	Displayclear();
	SetCursor(0,4);
	OledPrint(text);
}


/**
 * @brief Send a byte
 * @param[in] uint8_t byte
 */
void OledPrintByte(uint8_t byte){

	uint8_t bytetosend = byte;

	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, &bytetosend, sizeof(uint8_t), true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}


