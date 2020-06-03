/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "driver_oled.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdbool.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "dht.h"
#include "fsm.h"

/* Struct for dht11 sensor */
typedef struct {
	int16_t temperatura;
	int16_t humedad;
}hum_temp;

/* Finite state machine variable creation and name for the place for the device */
control_t machine;
char nombre[] = "----OFICINA2----";

/* Relay pin */
#define GPIO_NUM_5 5

/* Temperature sensor type and sensor gpio */
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t dht_gpio = 33;

/* Interrupt pins and bitmask to initialize them */
#define GPIO_PULSADOR1     22
#define GPIO_PULSADOR2     23
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_PULSADOR1) | (1ULL<<GPIO_PULSADOR2))
#define ESP_INTR_FLAG_DEFAULT 0

/* Queue Handlers */
static xQueueHandle pulsador1_evt_queue = NULL;
static xQueueHandle pulsador2_evt_queue = NULL;
static xQueueHandle temperature_evt_queue = NULL;

/* Tasks prototypes */
void gpioInit(void);
void pulsador1_isr(void* pvParameter);
void pulsador2_isr(void* pvParameter);
void DisplayWrite(void* pvParameter);
void medicion_temperatura(void* pvParameter);

/* Synchronization tasks prototypes */
void IRAM_ATTR gpio_isr_handler1(void* pvParameter);
void IRAM_ATTR gpio_isr_handler2(void* pvParameter);

void app_main(void)
{
	/* Display, GPIO, Finite state machine initialization and welcome message */
	SSD1306_DisplayInit();
	SSD1306_DisplayClear();
	gpioInit();								//	Interrupt pin initialization

	SSD1306_GotoXY (0, 32);
	SSD1306_Puts ("----STARTING----", &Font_7x10, 1);
	SSD1306_UpdateScreen();

	vTaskDelay(2000/portTICK_PERIOD_MS);
	SSD1306_DisplayClear();
	fsminit(&machine,GPIO_NUM_5,nombre);
	fsmcontrol(&machine);

	/* Queue creations */
	pulsador1_evt_queue = xQueueCreate(1, sizeof(uint32_t));
	pulsador2_evt_queue = xQueueCreate(1, sizeof(uint32_t));
	temperature_evt_queue = xQueueCreate(1, sizeof(hum_temp));

	/* Task creations */
	xTaskCreate(&DisplayWrite,"DisplayWrite",configMINIMAL_STACK_SIZE*4,NULL,1,NULL);
	xTaskCreate(&pulsador1_isr,"pulsador1_isr",configMINIMAL_STACK_SIZE*4,(void*)&machine,3,NULL);
	xTaskCreate(&pulsador2_isr,"pulsador2_isr",configMINIMAL_STACK_SIZE*4,(void*)&machine,3,NULL);
	xTaskCreate(&medicion_temperatura,"medicion_temperatura",configMINIMAL_STACK_SIZE*3,NULL,2,NULL);

	/* ISR service install for the interrupts */
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(GPIO_PULSADOR1, gpio_isr_handler1, (void*) GPIO_PULSADOR1);
	gpio_isr_handler_add(GPIO_PULSADOR2, gpio_isr_handler2, (void*) GPIO_PULSADOR2);

	/* never reach here */
	for(;;);
}


/* TASKS */
/* --------------------------------------------------------------------------- */
/* Tarea del pulsador 1 */
void pulsador1_isr(void* pvParameter){

	uint16_t io_num;
	control_t *machine = pvParameter;
	static uint16_t count = 0;

	while(1){

		if(xQueueReceive(pulsador1_evt_queue, &io_num, portMAX_DELAY)){
			count++;
			if(machine -> relay == ON && count == 1){
					machine -> relay = OFF;
					count = 0;
			}
			if(machine -> relay == OFF && count == 1){
					machine -> relay = ON;
					count = 0;
			}
			fsmcontrol(machine);
		}
	}
}

/* Tarea del pulsador 2 */

void pulsador2_isr(void* pvParameter){

	uint32_t io_num;
	static uint8_t count = 0;
	control_t *machine = pvParameter;

	while(1){
		if(xQueueReceive(pulsador2_evt_queue, &io_num, portMAX_DELAY)){
			count++;
			if(count == 2 && machine -> modo == AUTOMATIC){
				if(machine -> timetable == WORK){
					machine -> modo = MANUAL;
					count = 0;
				}
				else if(machine -> timetable == OUTOFWORK){
					machine -> modo = AUTOMATIC;
					count = 0;
				}
			}
			else if(count == 2 && machine -> modo == MANUAL){
				if(machine -> timetable == WORK){
					machine -> modo = AUTOMATIC;
					count = 0;
				}
				else if(machine -> timetable == OUTOFWORK){
					machine -> modo = AUTOMATIC;
					count = 0;
				}
			}
			fsmcontrol(machine);
		}
	}
}

/* Tarea de medicion de temperatura */
void medicion_temperatura(void* pvParameter){

	hum_temp enviado;

	enviado.temperatura = 0;
	enviado.humedad = 0;

	while (1){

		if (dht_read_data(sensor_type, dht_gpio, &enviado.humedad, &enviado.temperatura) == ESP_OK){

			xQueueSend(temperature_evt_queue, &enviado,portMAX_DELAY);
		}
		vTaskDelay(5000/portTICK_PERIOD_MS);
	}
}



/* @brief Write temperature and humidity on the display
 *
 *
 */
void DisplayWrite(void* pvParameter){

	char temperatura[] = "Temp:";
	char humedad[] = "Hum:";

	char valortemp[1]={0};
	char valorhum[1]={0};

	hum_temp tempyhum;

	while(1){

		if(xQueueReceive(temperature_evt_queue, &tempyhum, portMAX_DELAY) == pdTRUE){

			SSD1306_GotoXY (0, 10);
			SSD1306_Puts (temperatura, &Font_7x10, 1);
			SSD1306_GotoXY (70, 10);
			SSD1306_Puts (humedad, &Font_7x10, 1);
			itoa((tempyhum.temperatura*0.1),valortemp,10);
			SSD1306_GotoXY (35, 10);
			SSD1306_Puts (valortemp, &Font_7x10, 1);
			itoa((tempyhum.humedad*0.1),valorhum,10);
			SSD1306_GotoXY (95, 10);
			SSD1306_Puts (valorhum, &Font_7x10, 1);

			SSD1306_UpdateScreen();
		}
	}
}


/* Funcion de interrupcion 1 */
void IRAM_ATTR gpio_isr_handler1(void* pvParameter)
{
	uint32_t gpio_num1 = (uint32_t) pvParameter;
	xQueueSendFromISR(pulsador1_evt_queue, &gpio_num1, NULL);
}

/* Funcion de interrupcion 2 */
void IRAM_ATTR gpio_isr_handler2(void* pvParameter)
{
    uint32_t gpio_num2 = (uint32_t) pvParameter;
    xQueueSendFromISR(pulsador2_evt_queue, &gpio_num2, NULL);
}


/* Configuro la estructura gpio_config_t para los
 * pines de ISR */
void gpioInit(void){
gpio_config_t io_conf;									/* Defino una variable de configuracion*/
io_conf.intr_type = GPIO_INTR_POSEDGE;					/* Defino el flanco */
io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;				/* Defino la mascara con los pines van a tener la misma configuracion */
io_conf.mode = GPIO_MODE_INPUT;							/* Defino direccion */
io_conf.pull_up_en = 1;									/* Defino el pin como pull-up */
gpio_config(&io_conf);									/* Le doy el puntero a la funcion gpio_config(const gpio_config_t *pGPIOConfig) */
}

