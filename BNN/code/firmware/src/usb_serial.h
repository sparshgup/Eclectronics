#pragma once

// Drive the USB-CDC CLI. Call from main loop after stdio_init_all().
// Reads lines like "0110" or "i 0 1 1 0" and runs an inference, printing
// the result. Pressing the USER button also triggers a built-in test sweep.
void usb_serial_task(void);
