/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdbool.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "ssd1366.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver_oled.h"
#include "dht.h"
#include "fsm.h"


/* Finite state machine variable creation and name for the place for the device */
static control_t machine;
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
static void pulsador1_isr(void* pvParameter);
static void pulsador2_isr(void* pvParameter);
//static void fsmupdate(void* pvParameter);
static void medicion_temperatura(void* pvParameter);


/* Synchronization tasks prototypes */
static void IRAM_ATTR gpio_isr_handler1(void* pvParameter);
static void IRAM_ATTR gpio_isr_handler2(void* pvParameter);


void app_main(void)
{
	/* Display, GPIO, Finite state machine initialization and welcome message */
	DisplayInit();
	gpioInit();								//	Interrupt pin initialization
	Display_msj_bienvenida();				//	Welcome message display on screen
	vTaskDelay(2000/portTICK_PERIOD_MS);
	Displayclear();
	fsminit(&machine,GPIO_NUM_5,nombre);
	fsmcontrol(&machine);

	/* Queue creations */
	pulsador1_evt_queue = xQueueCreate(2, sizeof(uint32_t));
	pulsador2_evt_queue = xQueueCreate(2, sizeof(uint32_t));
	temperature_evt_queue = xQueueCreate(2, sizeof(uint32_t));

	/* Task creations */
	//xTaskCreate(&fsmupdate,"fsmupdate",configMINIMAL_STACK_SIZE*4,(void*)&machine,10,NULL);
	xTaskCreate(&pulsador1_isr,"pulsador1_isr",configMINIMAL_STACK_SIZE*4,(void*)&machine,3,NULL);
	xTaskCreate(&pulsador2_isr,"pulsador2_isr",configMINIMAL_STACK_SIZE*4,(void*)&machine,1,NULL);
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
static void pulsador1_isr(void* pvParameter){

	uint32_t io_num;
	static uint8_t count = 0;
	control_t *machine = pvParameter;

	while(1){
		if(xQueueReceive(pulsador1_evt_queue, &io_num, portMAX_DELAY)) {
			count++;
			if(machine -> relay == ON && count != 2 && count != 0){
					machine -> relay = OFF;
					count = 0;
			}
			else if(machine -> relay == OFF && count != 2 && count != 0){
					machine -> relay = ON;
					count = 0;
			}
			fsmcontrol(machine);
		}
	}
}

/* Tarea del pulsador 2 */

static void pulsador2_isr(void* pvParameter){

	uint32_t io_num;
	static uint8_t count = 0;
	control_t *machine = pvParameter;

	while(1){
		if(xQueueReceive(pulsador2_evt_queue, &io_num, portMAX_DELAY)) {
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
static void medicion_temperatura(void* pvParameter){

	int16_t temperature = 0;
	int16_t humidity = 0;

	char temperatura[] = "Temp:";
	char humedad[] = "Hum:";

	char valortemp[1]={0};
	char valorhum[1]={0};

	while (1){
		if (dht_read_data(sensor_type, dht_gpio, &humidity, &temperature) == ESP_OK){

			SetCursor(0,2);
			OledPrint(temperatura);
			SetCursor(5,2);
			OledPrint(humedad);

			itoa((temperature*0.1),valortemp,10);
			SetCursor(3,2);
			OledPrint(valortemp);
			itoa((humidity/10),valorhum,10);
			SetCursor(7,2);
			OledPrint(valorhum);
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}



/* Tarea de Escribir en el display */
/*static void fsmupdate(void* pvParameter){

	control_t *machine = pvParameter;

	while(1){
		if(xQueueReceive(temperature_evt_queue, &io_num, portMAX_DELAY) == pdTRUE){

		}
		fsmcontrol(machine);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
*/


/* Funcion de interrupcion 1 */
static void IRAM_ATTR gpio_isr_handler1(void* pvParameter)
{
    uint32_t gpio_num1 = (uint32_t) pvParameter;
    xQueueSendFromISR(pulsador1_evt_queue, &gpio_num1, NULL);
    vTaskDelay(1/portTICK_PERIOD_MS);
}

/* Funcion de interrupcion 2 */
static void IRAM_ATTR gpio_isr_handler2(void* pvParameter)
{
    uint32_t gpio_num2 = (uint32_t) pvParameter;
    xQueueSendFromISR(pulsador2_evt_queue, &gpio_num2, NULL);
    vTaskDelay(1/portTICK_PERIOD_MS);
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

