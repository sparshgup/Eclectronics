#include "usb_serial.h"
#include "board.h"
#include "led.h"
#include "spi_data.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const char *CLASS_NAMES[] = { "setosa", "versicolor", "virginica" };

// Iris demo vectors: a few representative binarized samples covering each
// class. Used by the button-triggered self-test.
static const uint8_t DEMO_INPUTS[] = { 0x0, 0x6, 0xF };
static const char *DEMO_LABELS[]   = { "setosa-ish", "versicolor-ish", "virginica-ish" };

static void run_inference(uint8_t nibble) {
    uint8_t cls = bnn_infer(nibble);
    const char *name = (cls < 3) ? CLASS_NAMES[cls] : "?";
    printf("input=0b%c%c%c%c -> class %u (%s)\n",
           (nibble & 0x8) ? '1' : '0',
           (nibble & 0x4) ? '1' : '0',
           (nibble & 0x2) ? '1' : '0',
           (nibble & 0x1) ? '1' : '0',
           cls, name);
    led_show_class(cls);
}

// Parse a line like "0110", "0 1 1 0", or "i 0 1 1 0" into a 4-bit nibble.
// Returns -1 on parse error.
static int parse_nibble(const char *line) {
    int bits[4];
    int n = 0;
    for (const char *p = line; *p && n < 4; p++) {
        if (*p == '0' || *p == '1') {
            bits[n++] = *p - '0';
        } else if (!isspace((unsigned char)*p) && *p != 'i' && *p != ',') {
            return -1;
        }
    }
    if (n != 4) return -1;
    return (bits[0] << 3) | (bits[1] << 2) | (bits[2] << 1) | bits[3];
}

static void run_demo_sweep(void) {
    printf("\n-- demo sweep --\n");
    for (size_t i = 0; i < sizeof(DEMO_INPUTS); i++) {
        printf("[%s] ", DEMO_LABELS[i]);
        run_inference(DEMO_INPUTS[i]);
        sleep_ms(400);
    }
    led_off();
}

static void print_help(void) {
    printf(
        "BNN inference accelerator — commands:\n"
        "  4 bits, e.g. 0110          run inference on that input\n"
        "  d                          demo sweep (also via USER button)\n"
        "  h                          this help\n"
    );
}

void usb_serial_task(void) {
    static char  line[64];
    static size_t len = 0;
    static bool  prev_btn = true;   // active low, idle high
    static bool  banner = false;

    if (!banner) {
        print_help();
        banner = true;
    }

    // ---- USER button: edge-triggered sweep ----
    bool btn = gpio_get(PIN_USER_BTN);
    if (prev_btn && !btn) {
        sleep_ms(20);  // simple debounce
        if (!gpio_get(PIN_USER_BTN)) run_demo_sweep();
    }
    prev_btn = btn;

    // ---- Read non-blocking from USB CDC ----
    int c = getchar_timeout_us(0);
    while (c != PICO_ERROR_TIMEOUT) {
        if (c == '\r' || c == '\n') {
            if (len > 0) {
                line[len] = '\0';
                if (line[0] == 'h' || line[0] == '?') {
                    print_help();
                } else if (line[0] == 'd') {
                    run_demo_sweep();
                } else {
                    int n = parse_nibble(line);
                    if (n >= 0) run_inference((uint8_t)n);
                    else printf("parse error: expected 4 binary digits, got '%s'\n", line);
                }
                len = 0;
            }
        } else if (len < sizeof(line) - 1) {
            line[len++] = (char)c;
        }
        c = getchar_timeout_us(0);
    }
}
