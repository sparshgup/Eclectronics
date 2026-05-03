#pragma once

#include <stdint.h>

void led_init(void);

// Show classification result. class_id 0=red, 1=green, 2=blue.
// Any other value turns the LED off.
void led_show_class(uint8_t class_id);

void led_off(void);
