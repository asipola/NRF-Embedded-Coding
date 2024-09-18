//Tavoite 3 pistettä
//Tekijät: Aleksi Sipola ja Mikael Ryynänen
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

// Configure buttons
#define BUTTON_0 DT_ALIAS(sw0) // Configured for use
#define BUTTON_1 DT_ALIAS(sw1) 
#define BUTTON_2 DT_ALIAS(sw2) 
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)  
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static struct gpio_callback button_0_data;
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {1});
static struct gpio_callback button_1_data;
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {2});
static struct gpio_callback button_2_data;
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {3});
static struct gpio_callback button_3_data;
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {4});
static struct gpio_callback button_4_data;
bool pressed = false;
int led_state = 0;
// Button interrupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pressed == true)
        {
                pressed = false;
        }else 
        {
                pressed = true;
        }
}
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pressed == true)
        {
            led_state = 1;
            printk("Led is now red!");
        }
}
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pressed == true)
        {
            led_state = 2;
            printk("Led is now yellow!");
        }
}
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pressed == true)
        {
            led_state = 3;
            printk("Led is now green!");
        }
}
// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5
void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
// Main program
int main(void)
{
        led_state = 0;
	if (ini_led() != 0){
                return -1;
        }
	if (init_button() < 0) {
		return 0;
	}
        while(true){
                if (pressed == false){
                if(led_state>=3){
                        led_state=1;
                }
                else{
                        led_state++;
                }
                }k_sleep(K_SECONDS(1));
        
        }
        
	return 0;
}

// Initialize leds
int ini_led() {

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
void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pressed == true)
        {
        led_state = -1;
         gpio_pin_set_dt(&green,0);
         gpio_pin_set_dt(&red,0);   
        }
}
// Button initialization
int init_button() {

	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
              gpio_pin_configure_dt(&button_1, GPIO_INPUT);
              gpio_pin_configure_dt(&button_2, GPIO_INPUT);
              gpio_pin_configure_dt(&button_3, GPIO_INPUT);
              gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
              gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
              gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
              gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
              gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
        gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
        gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);
        gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);
        gpio_init_callback(&button_4_data, button_4_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	printk("Set up button 0 ok\n");
}

// Task to handle red led
void red_led_task(void *, void *, void*) {
	while (true) {
                if(led_state == 1){
                        gpio_pin_set_dt(&red,1);
                        gpio_pin_set_dt(&green,0);
                }
                k_sleep(K_SECONDS(1));
	}
}
void green_led_task(void *, void *, void*) {
	
        while (true) {
                if(led_state == 3){
                        gpio_pin_set_dt(&green,1);
                        gpio_pin_set_dt(&red,0);

                }
        k_sleep(K_SECONDS(1));
	}
}
void yellow_led_task(void *, void *, void*) {
	
        while (true) {
                if(led_state == 2){
                        gpio_pin_set_dt(&green,1);
                        gpio_pin_set_dt(&red,1);

                }
                k_sleep(K_SECONDS(1));
        }
}
