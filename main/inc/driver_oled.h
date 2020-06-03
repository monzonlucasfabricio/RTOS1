#ifndef MAIN_DRIVER_OLED_H_
#define MAIN_DRIVER_OLED_H_

#include <string.h>
#include <stdbool.h>
#include "font8x8_basic.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS   0x3C

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

/* SDA and SCL pins to be used */
#define SDA_PIN GPIO_NUM_15
#define SCL_PIN GPIO_NUM_2

/* SSD1306 settings */
/* SSD1306 width in pixels */
#define SSD1306_WIDTH            128

/* SSD1306 LCD height in pixels */
#define SSD1306_HEIGHT           64

/**
 * @brief  SSD1306 color enumeration
 *
 */
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SSD1306_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SSD1306_COLOR_t;


/**
 * @brief  Define I2C Pins for communication
 * @param  None
 */
#define SDA_PIN GPIO_NUM_15
#define SCL_PIN GPIO_NUM_2


/* I2C COMMUNICATION FUNCTIONS */
/**
 * @brief  Write command on the I2C port
 * @param  uint8_t type
 */
void SSD1306_WriteCommand(uint8_t byte);

/**
 * @brief  Write data on the I2C port
 * @param  uint8_t type
 */
void SSD1306_WriteData(uint8_t byte);

/**
 * @brief  Write one o more data on the I2C port
 * @param  uint8_t type for the data to send
 * @param  uint16_t type for the quantity of data to send
 */
void SSD1306_WriteNData(uint8_t *data, uint16_t count);


/* SSD1306 DISPLAY FUNCTIONS */
/**
 * @brief  Initializes SSD1306 Display
 * @param  None
 */
void SSD1306_DisplayInit(void);

/**
 * @brief  Initializes SSD1306 LCD
 * @param  uint16_t type for x location on the display
 * @param  uint16_t type for y location on the display
 * @param  color to be used for the screen fill.
 */
void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);

/**
 * @brief  Fill all the screen with a color selected
 * @param  Color depends on the screen to use
 */
void SSD1306_DisplayFill(SSD1306_COLOR_t Color);

/**
 * @brief  Clean all the screen
 * @param  None
 */
void SSD1306_DisplayClear(void);

/**
 * @brief  Writes character to internal RAM
 * @param  ch: character to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color used for writing
 * @retval character written
 */
char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color);

/**
 * @brief  Writes string to internal RAM
 * @param  *str: string to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color used for writing
 * @retval zero on success or character value when function failed
 */
char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color);

/**
 * @brief  Set x and y points
 * @param  x: for the x location on the screen, can take from x to the display WIDTH
 * @param  y: for the y location on the screen, can take from y to the display HEIGHT
 */
void SSD1306_GotoXY(uint16_t x, uint16_t y);

/**
 * @brief  Turn ON display
 * @param  None
 */
void SSD1306_ON(void);

/**
 * @brief  Turno OFF display
 * @param  None
 */
void SSD1306_OFF(void);

/**
 * @brief  Update the screen from internal RAM to Display
 * @note   This function must be called every time you do changes
 * @param  None
 */
void SSD1306_UpdateScreen(void);


#endif /* MAIN_DRIVER_OLED_H_ */
