//Tavoite 3 pistettä
//Tekijät: Aleksi Sipola ja Mikael Ryynänen
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int led_state = 0;
int led_wait = 0;
// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

void dispatcher_task(void *, void *, void*);
void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
#define STACKSIZE 500
#define PRIORITY 5
K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(red_thvoidread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// FIFO data type
struct data_t {
	char msg[20];
};


int main(void)
{
        led_state = 0;
	if (init_led() != 0){
                return -1;
        }

        int ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}

	// Wait for everything to initialize and threads to start
	k_msleep(100);
	printk("Started serial read example\n");

	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	// Dispatcher data to fifo
	struct data_t dispatcher_data;

	while (true) {
		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {
			//printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\r') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			// Character is newline, copy dispatcher data and put to FIFO buffer
			} else {
				printk("Received msg: %s\n", uart_msg);
				// Copy UART message to dispatcher data
				strcpy(dispatcher_data.msg, uart_msg);
				// Put dispatcher data to FIFO buffer
				k_fifo_put(&dispatcher_fifo, &dispatcher_data);
				// Clear UART message buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);
			}
		}
		k_msleep(10);
	}
	return 0;
}

// Initialize leds
int init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
        int gren = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0 && gren < 0) {
		return -1;
	}
        
	// set led off
	gpio_pin_set_dt(&red,0);
        gpio_pin_set_dt(&green,0);

	printk("Led initialized ok\n");
        return 0;
}

int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

void dispatcher_task(void *, void *, void*) {

	// Store received data
	struct data_t *received;

	while (true) {

		received = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		// printk("Dispatcher received: %s\n", received->msg);
                bool parser = false;
                int time = 0;
		// Example msg: R,1000
		// Parse message: color
		//char color = received->msg[0];
		// time
                char color;
                char *token = strtok(received->msg, ",");

                while (token != NULL) {
                        // If the token is a number, convert it to integer
                        if (parser == false) {
                                color = token[0];
                                parser = true;
                                token = strtok(NULL, ",");
                                continue;
                        }
                        if (parser == true) {
                                time = atoi(token);
                                parser = false;
                                token = strtok(NULL, ",");
                                switch (color)
                                {
                                        case 'R':
                                                led_state = 1;
                                                break;
                                        case 'Y':
                                                led_state = 2;
                                                break;
                                        case 'G':
                                                led_state = 3;
                                                break;
                                        
                                        default:
                                                break;
                                }
                                k_sleep(K_MSEC(time));
                                continue;
                        }
                }
		//printk("Parsed msg: %c, %d\n", color, time);
                k_yield();
	}
}

// Task to handle red led
void red_led_task(void *, void *, void*) {
	while (true) {

                if(led_state == 1){
                        gpio_pin_set_dt(&red,1);
                        gpio_pin_set_dt(&green,0);
                }
                k_yield();
	}
}
void green_led_task(void *, void *, void*) {
	
        while (true) {
                if(led_state == 3){
                        gpio_pin_set_dt(&green,1);
                        gpio_pin_set_dt(&red,0);

                }
        k_yield();
	}
}
void yellow_led_task(void *, void *, void*) {
	
        while (true) {
                if(led_state == 2){
                        gpio_pin_set_dt(&green,1);
                        gpio_pin_set_dt(&red,1);

                }
               k_yield();
        }
}
