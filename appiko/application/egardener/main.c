#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "boards.h"
#include "hal_clocks.h"
#include "hal_nop_delay.h"
#include "hal_gpio.h"
#include "common_util.h"
#include "tinyprintf.h"
#include "uart_printf.h"
#include "hal_uart.h"

#include "ms_timer.h"
#include "nrf_util.h"
#include "simple_adc.h"

#define GPIO_MOTOR_PIN   26

#define PUMP_ON_SIGNAL	0
#define PUMP_OFF_SIGNAL	1

#define TIMER_CHECK_VALUES MS_TIMER2
#define TIMER_AUTO_WATER MS_TIMER0
#define TIMER_STOP_WATER MS_TIMER1
	
#define PIN_WATER_LOW	ANALOG_PIN_4
#define PIN_MOISTURE ANALOG_PIN_2
#define PREFERRED_BAUD_RATE HAL_UART_BAUD_115200 //HAL_UART_BAUD_9600

const int AUTO_WATER_FREQUENCY = 10000; // 2#define PIN_WATER_LOW	ANALOG_PIN_40 secs    
const int WATER_STOP_FREQUENCY = 5000;    
const int CHECK_VALUES_FREQUENCY = 10000; // 10 sec    

const float MOISTURE_THRESHOLD = 3000;
const int WATER_THRESHOLD = 0;


typedef enum
{
    MOISTURE_STATE_LOW,
    MOISTURE_STATE_NORMAL
} enum_moisture_state;

typedef enum
{
    WATER_LEVEL_LOW,
    WATER_LEVEL_ENOUGH,
} enum_water_level;

typedef enum
{
    PUMP_OFF,
    PUMP_ON
} enum_pump_state;

enum_moisture_state moisture_state = MOISTURE_STATE_NORMAL;
enum_water_level water_level = WATER_LEVEL_LOW ;
enum_pump_state pump_state = PUMP_OFF ;

void (*ptr_timer_water_garden) (void);
void (*ptr_timer_stop_water_garden) (void);
void (*ptr_check_values) (void);

void check_water_level(void);
void check_moisture_level(void);
void turn_pump_on(void);
void turn_pump_off(void);
void check_values(void);

#define DEBUGMODE

void debug_print(char *fmt, ...) 
	{
		#ifdef DEBUGMODE 
		{ 
			va_list va;
			va_start(va, fmt);
			tfp_printf("debug mode on");
			tfp_debug_printf(fmt, va);
			va_end(va);
			
		} 
		#else 
			{
				
			}
		#endif
	}
 

void water_garden()
{
	check_values();

	debug_print("\n Pump State: %d, Moisture: %d, Water: %d " ,pump_state,moisture_state, water_level);

	if ((pump_state == PUMP_OFF) && (moisture_state == MOISTURE_STATE_LOW) && (water_level == WATER_LEVEL_ENOUGH)) {
		
		turn_pump_on();

		// start timer to stop water garden
		ms_timer_start(TIMER_STOP_WATER, MS_REPEATED_CALL, RTC_TICKS_MS(WATER_STOP_FREQUENCY), 				   ptr_timer_stop_water_garden);

	} else {
		debug_print("\n  Pump State: %d, Moisture: %d, Water: %d " ,pump_state,moisture_state, water_level);		
		
	}
}

void stop_water_garden() {
	static int stop_water_count = 0;
	debug_print("\n In stop water garden: %d " ,(++ stop_water_count));		
	
	check_values();

	if (moisture_state == MOISTURE_STATE_NORMAL) {
		turn_pump_off();
	} 
	if (water_level == WATER_LEVEL_LOW) {
				turn_pump_off();
	}

	// if pump is off, there is no reason for timer 2 to be on
	if (pump_state == PUMP_OFF && ms_timer_get_on_status(TIMER_STOP_WATER)) {
		// stop timer 2	
		debug_print("\n Stopping Timer to stop water");		
		ms_timer_stop(TIMER_STOP_WATER);
	} 
}

void check_moisture_level(void) {
	int16_t input_moisture_reading= -1 ;
	input_moisture_reading = simple_adc_get_value(SIMPLE_ADC_GAIN1_5, PIN_MOISTURE);

	if (input_moisture_reading > MOISTURE_THRESHOLD) {
		debug_print("\n moisture reading is low: %d", input_moisture_reading);
		moisture_state = MOISTURE_STATE_LOW;
	} else  {
		debug_print("\n moisture reading is normal: %d", input_moisture_reading);		
		moisture_state = MOISTURE_STATE_NORMAL;
	}
	
}

void check_water_level(void) {
	int16_t water_low_indicator = simple_adc_get_value(SIMPLE_ADC_GAIN1, PIN_WATER_LOW);

	debug_print("\n water level indicator reads : %d \n", water_low_indicator);
	
	if (water_low_indicator < WATER_THRESHOLD) {
		//debug_print("\n There is NOT sufficient water");
		water_level = WATER_LEVEL_LOW;
	} else  {
		//debug_print("\n There is sufficient water: ");		
		water_level = WATER_LEVEL_ENOUGH;		
	}
}

void turn_pump_on() {
	debug_print("\n Turning pump on");		

	hal_gpio_pin_write(GPIO_MOTOR_PIN, PUMP_ON_SIGNAL);
	pump_state = PUMP_ON;	
}

void turn_pump_off() {
	debug_print("\n Turning pump off");		

	hal_gpio_pin_write(GPIO_MOTOR_PIN, PUMP_OFF_SIGNAL);
	pump_state = PUMP_OFF;	
}

void init_components() {	
	hal_gpio_cfg_output(PIN_MOISTURE, HAL_GPIO_PULL_UP);

	hal_gpio_cfg_output(PIN_WATER_LOW, HAL_GPIO_PULL_DOWN);


	// init pump
	// initialize pin GPIO_MOTOR_PIN as output and pull it up (so keep a high)
	hal_gpio_cfg_output(GPIO_MOTOR_PIN, HAL_GPIO_PULL_UP);
}

void write_values() {
	char*  moisture_id = "M";
	char* water_id = "W";
	char*  moisture_value = "";
	char* water_value = "";

	const int MAX_STR_FOR_SERIAL_WRITE = 5;
	char final_string_for_serial_write[MAX_STR_FOR_SERIAL_WRITE]; 

	if (moisture_state == MOISTURE_STATE_NORMAL) {
		moisture_value = "1";	
	} else {
		moisture_value = "0";	
	}

	if (water_level == WATER_LEVEL_ENOUGH) {
		water_value = "1";	
	} else {
		water_value = "0";	
	}

	final_string_for_serial_write[0] = *moisture_id;
	final_string_for_serial_write[1] = *moisture_value;
	final_string_for_serial_write[2] = *water_id;
	final_string_for_serial_write[3] = *water_value;
	final_string_for_serial_write[4] = '\0';

	tfp_printf(final_string_for_serial_write );
}

void check_values() {
	check_water_level();
	check_moisture_level();
	write_values();
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
	ptr_timer_water_garden = & water_garden;
	ptr_timer_stop_water_garden =&stop_water_garden;
	ptr_check_values = &check_values;

	hfclk_xtal_init_blocking(); 
	lfclk_init(LFCLK_SRC_Xtal); 

    ms_timer_init(APP_IRQ_PRIORITY_MID);

	hal_uart_init(PREFERRED_BAUD_RATE, NULL);
	//uart_printf_init(UART_PRINTF_BAUD_9600);

	debug_print("Water Level Sensor - Final Demo !\n");
	init_components();

    ms_timer_start(TIMER_AUTO_WATER, MS_REPEATED_CALL, RTC_TICKS_MS(AUTO_WATER_FREQUENCY), 				   ptr_timer_water_garden);

	// set timer to check values periodically
    //ms_timer_start(TIMER_CHECK_VALUES, MS_REPEATED_CALL, RTC_TICKS_MS(CHECK_VALUES_FREQUENCY), 		//		   ptr_check_values);

	while (true)
	{
		//water_garden();
		__WFI();
		//hal_nop_delay_ms(50); // to take care of switch debouncing
	}
}

