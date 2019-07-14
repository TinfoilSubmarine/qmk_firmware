/* Copyright 2019 Michiel Visser
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "anne_pro_lighting.h"
#include "uart_tx_ringbuf.h"
#include "quantum.h"
#include "ch.h"
#include "hal.h"

#define LED_UART (&UARTD3)
/* UART transmission ringbuffer for the LED UART */
static uint8_t led_uart_tx_buffer[256] = {0};
static uart_tx_ringbuf_t led_uart_ringbuf = {
    .buf = led_uart_tx_buffer,
    .uart = LED_UART,
    .size = sizeof(led_uart_tx_buffer),
    .sending_elements = 0,
    .head = 0,
    .tail = 0,
};

/* Handler for finsihed LED UART transmissions */
static void led_uart_txend(UARTDriver *uart) {
    uart_tx_ringbuf_finish_transmission(&led_uart_ringbuf);
}

/* LED UART configuration */
static UARTConfig led_uart_cfg = {
    .txend1_cb = led_uart_txend,
    .txend2_cb = NULL,
    .rxend_cb = NULL,
    .rxchar_cb = NULL,
    .rxerr_cb = NULL,
    .speed = 38400,
    .cr1 = 0,
    .cr2 = USART_CR2_LINEN,
    .cr3 = 0
};

/* State of the leds on the keyboard */
static volatile bool leds_enabled = false;

void anne_pro_lighting_init(void) {
    /* Turn on lighting controller */
    writePinLow(C15);
    setPinOutput(C15);

    /* Initialize the lighting UART */
    uartStart(LED_UART, &led_uart_cfg);
    palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(7));
}

/* Buffer for the keystate packet */
static uint8_t keystate[12] = {9, 10, 7, 0};

/* Update the dynamic lighting packet based on a keypress */
void anne_pro_lighting_update_dynamic(keyrecord_t *record) {
    /* Make sure this is actually a keypress event */
    if (IS_NOEVENT(record->event)) return;
    /* Only update dynamic lighting modes when leds are enabled */
    if (leds_enabled) {
        /* Calculate the position of the key that was pressed */
        uint8_t row = record->event.key.row;
        uint8_t col = record->event.key.col;
        int position = row * MATRIX_COLS + col;
        int index = position / 8;
        int bit = position % 8;

        /* Update the keystate based on the location */
        if (record->event.pressed) {
            keystate[3 + index] |= (1 << bit);
        } else {
            keystate[3 + index] &= ~(1 << bit);
        }

        /* Send the keystate to the LED controller */
        uart_tx_ringbuf_write(&led_uart_ringbuf, 12, keystate);
        uart_tx_ringbuf_start_transmission(&led_uart_ringbuf);
    }
}

/* Update lighting, should be called every matrix scan */
void anne_pro_lighting_update(void) {
    if (!uart_tx_ringbuf_empty(&led_uart_ringbuf)) {
        uart_tx_ringbuf_start_transmission(&led_uart_ringbuf);
    }
}

/* Toggle the lighting on/off */
void anne_pro_lighting_toggle(void) {
    if (!leds_enabled) {
        anne_pro_lighting_on();
    } else {
        anne_pro_lighting_off();
    }
}

/* Turn the lighting on */
void anne_pro_lighting_on(void) {
    /* Wake up the LED controller */
    writePinHigh(C15);
    chThdSleepMilliseconds(50);
    /* Send turn light on command */
    uart_tx_ringbuf_write(&led_uart_ringbuf, 3, "\x09\x01\x01");
    uart_tx_ringbuf_start_transmission(&led_uart_ringbuf);
    leds_enabled = true;
    /* Wait for the message to be sent */
    chThdSleepMilliseconds(10);
}

/* Turn the lighting off */
void anne_pro_lighting_off(void) {
    /* Send turn light off command */
    uart_tx_ringbuf_write(&led_uart_ringbuf, 4, "\x09\x02\x01\x00");
    uart_tx_ringbuf_start_transmission(&led_uart_ringbuf);
    leds_enabled = false;
    /* Sleep the LED controller */
    writePinLow(C15);
}

/* Select the next effect rate */
void anne_pro_lighting_rate_next(void) {
    if (leds_enabled) {
        uart_tx_ringbuf_write(&led_uart_ringbuf, 6, "\x09\x04\x05\x00\x01\x00");
    }
}

/* Select the next brightness */
void anne_pro_lighting_brightness_next(void) {
    if (leds_enabled) {
        uart_tx_ringbuf_write(&led_uart_ringbuf, 6, "\x09\x04\x05\x00\x00\x01");
    }
}

/* Select the next lighting mode */
void anne_pro_lighting_mode_next(void) {
    if (leds_enabled) {
        uart_tx_ringbuf_write(&led_uart_ringbuf, 6, "\x09\x04\x05\x01\x00\x00");
    }
}

/* Set the lighting mode */
void anne_pro_lighting_mode(uint8_t mode) {
    if (leds_enabled) {
        uint8_t buf[] = {9, 2, 1, mode};
        uart_tx_ringbuf_write(&led_uart_ringbuf, 4, buf);
        uart_tx_ringbuf_start_transmission(&led_uart_ringbuf);
    }
}

/* Set the rate and brightness */
void anne_pro_lighting_rate_brightness(uint8_t rate, uint8_t brightness) {
    if (leds_enabled) {
        if (brightness > 10) brightness = 10;

        uint8_t buf[] = {9, 4, 2, rate, brightness, 0};
        uart_tx_ringbuf_write(&led_uart_ringbuf, 6, buf);
        uart_tx_ringbuf_start_transmission(&led_uart_ringbuf);
    }
}
