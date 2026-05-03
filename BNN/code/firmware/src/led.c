#include "led.h"
#include "board.h"

#include "hardware/gpio.h"

// Common-anode RGB: drive low to light a channel.
static void set_rgb(int r, int g, int b) {
    gpio_put(PIN_LED_R, r ? 0 : 1);
    gpio_put(PIN_LED_G, g ? 0 : 1);
    gpio_put(PIN_LED_B, b ? 0 : 1);
}

void led_init(void) {
    gpio_init(PIN_LED_R); gpio_set_dir(PIN_LED_R, GPIO_OUT);
    gpio_init(PIN_LED_G); gpio_set_dir(PIN_LED_G, GPIO_OUT);
    gpio_init(PIN_LED_B); gpio_set_dir(PIN_LED_B, GPIO_OUT);
    led_off();
}

void led_show_class(uint8_t class_id) {
    switch (class_id) {
        case 0: set_rgb(1, 0, 0); break;  // setosa     — red
        case 1: set_rgb(0, 1, 0); break;  // versicolor — green
        case 2: set_rgb(0, 0, 1); break;  // virginica  — blue
        default: set_rgb(0, 0, 0); break;
    }
}

void led_off(void) {
    set_rgb(0, 0, 0);
}
