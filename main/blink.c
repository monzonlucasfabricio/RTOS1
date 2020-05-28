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

/* Defino PIN DE RELAY */
#define GPIO_NUM_5 5

/* Defino tipo de sensor y gpio del sensor*/
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t dht_gpio = 33;

/* Defino pines de interrupcion y el bitmask para implementar la interrupcion en los pines */
#define GPIO_PULSADOR1     22
#define GPIO_PULSADOR2     23
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_PULSADOR1) | (1ULL<<GPIO_PULSADOR2))
#define ESP_INTR_FLAG_DEFAULT 0

/* Handlers de las Queue */
static xQueueHandle pulsador1_evt_queue = NULL;
static xQueueHandle pulsador2_evt_queue = NULL;
static xQueueHandle temperatura_evt_queue = NULL;

/* Prototipos */
void gpioInit(void);
static void pulsador1_isr(void* pvParameter);
static void pulsador2_isr(void* pvParameter);
//static void DisplayWrite(void* pvParameter);
static void medicion_temperatura(void* pvParameter);


/* Tareas de sincronizacion con interrupciones */
static void IRAM_ATTR gpio_isr_handler1(void* pvParameter);
static void IRAM_ATTR gpio_isr_handler2(void* pvParameter);


void app_main(void)
{
	/* Inicializacion del display y los gpios */
	DisplayInit();
	gpioInit();						//	GPIO's de Interrupcion
	gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT); //	GPIO para el relay
	gpio_set_pull_mode(GPIO_NUM_5, GPIO_PULLUP_ONLY); // 	GPIO para el relay en salida pullup
	Display_msj_bienvenida();		//	Mensaje de inicializacion de pantalla
	vTaskDelay(2000/portTICK_PERIOD_MS);
	displayclear();

	/* Nombre del lugar */
	SetCursor(0,0);
	OledPrint((void*)"----OFICINA1----");

	/* Creacion de las Queue */
	pulsador1_evt_queue = xQueueCreate(1, sizeof(uint32_t));
	pulsador2_evt_queue = xQueueCreate(1, sizeof(uint32_t));
	temperatura_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	/* Creacion de las tareas */
	//xTaskCreate(&DisplayWrite,"DisplayWrite",4096,NULL,10,NULL);
	xTaskCreate(&pulsador1_isr,"pulsador1_isr",configMINIMAL_STACK_SIZE*4,NULL,12,NULL);
	xTaskCreate(&pulsador2_isr,"pulsador2_isr",configMINIMAL_STACK_SIZE*4,NULL,12,NULL);
	xTaskCreate(&medicion_temperatura,"medicion_temperatura",configMINIMAL_STACK_SIZE*3,NULL,8,NULL);

	/* Instalar servicios de ISR en los gpios designados */
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(GPIO_PULSADOR1, gpio_isr_handler1, (void*) GPIO_PULSADOR1);
	gpio_isr_handler_add(GPIO_PULSADOR2, gpio_isr_handler2, (void*) GPIO_PULSADOR2);

	for(;;);
}



/* TAREAS */
/* --------------------------------------------------------------------------- */
/* Tarea del pulsador 1 */
static void pulsador1_isr(void* pvParameter){

	char luz_encendida[] = "Luz: Encendida";
	char luz_apagada[] = "Luz: Apagada";
	char borrarpalabra[] = "                ";

	uint32_t io_num;
	uint8_t count = 0;

	while(1){

		if(xQueueReceive(pulsador1_evt_queue, &io_num, portMAX_DELAY)) {

			/* Cambiar estado del relay */

			if(count == 0){
				SetCursor(0,7);
				OledPrint(borrarpalabra);
				OledPrint(luz_encendida);
				gpio_set_level(GPIO_NUM_5, 0);
			}
			if(count == 1){
				SetCursor(0,7);
				OledPrint(borrarpalabra);
				OledPrint(luz_apagada);
				gpio_set_level(GPIO_NUM_5, 1);
			}

			count++;

			if(count == 2){
				count = 0;
			}
		}
	}
}

/* Tarea del pulsador 2 */

static void pulsador2_isr(void* pvParameter){

	char modo_automatico[] = "Modo: Automatico" ;
	char modo_manual[] = "Modo: Manual";
	char borrarpalabra[] = "                ";

	uint32_t io_num;
	uint8_t count = 0;

	while(1){

		if(xQueueReceive(pulsador2_evt_queue, &io_num, portMAX_DELAY)) {

			if(count == 0){

				SetCursor(0,5);
				OledPrint(borrarpalabra);
				OledPrint(modo_automatico);
			}
			if(count == 1){
				SetCursor(0,5);
				OledPrint(borrarpalabra);
				OledPrint(modo_manual);
			}

			count++;

			if(count == 2){
				count = 0;
			}
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
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}



/* Tarea de Escribir en el display */
/*static void DisplayWrite(void* pvParameter){

	while(1){
		if(xQueueReceive(temperatura_evt_queue, &, portMAX_DELAY)){

		}
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

