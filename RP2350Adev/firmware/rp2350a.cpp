#include "pico/stdlib.h"

#define USER_LED 25

int main() {
    gpio_init(USER_LED);
    gpio_set_dir(USER_LED, GPIO_OUT);

    while (true) {
        gpio_put(USER_LED, 1);
        sleep_ms(500);
        gpio_put(USER_LED, 0);
        sleep_ms(500);
    }
}
