/* Copyright 2021 @ Grayson Carr
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

#include QMK_KEYBOARD_H
#include "rgb_matrix_user.h"
#include "rgb_matrix_types_user.h"
#include "keymap_user.h"
#include <lib/lib8tion/lib8tion.h>

static keypos_t led_key[DRIVER_LED_TOTAL];

#ifdef INDICATOR_EFFECT_ENABLE
static keyevent_t      last_keypress;
static uint8_t         last_fn_layer;
static indicator_hit_t indicator_hit[INDICATOR_TYPE_COUNT];
#endif

void rgb_matrix_init_user(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led != NO_LED) {
                led_key[led] = (keypos_t){.row = row, .col = col};
            }
        }
    }
}

void process_rgb_matrix_user(uint16_t keycode, keyrecord_t *record) {
#ifdef INDICATOR_EFFECT_ENABLE
    if (record->event.pressed) {
        last_keypress = record->event;
    }
#endif
}

static void rgb_matrix_set_color_by_keycode(uint8_t led_min, uint8_t led_max, uint8_t layer, bool (*is_keycode)(uint16_t), uint8_t red, uint8_t green, uint8_t blue, indicator_type_t indicator_type, bool indicator_on) {
#ifdef INDICATOR_EFFECT_ENABLE
    indicator_hit_t hit = indicator_hit[indicator_type];  // TODO: Check firmware size using pointers instead of local cached hit
    if (hit.on != indicator_on) {
        hit.on = indicator_on;
        if (indicator_on) {
            uint8_t led = g_led_config.matrix_co[last_keypress.key.row][last_keypress.key.col];
            hit.point   = g_led_config.point[led];
            hit.time    = last_keypress.time & ~1;                         // QMK sets key event time LSB to 1, which needs to be cleared to prevent underflow of initial elapsed time calculation
            hit.time -= hit.tick * (256 / (rgb_matrix_config.speed | 1));  // Opposite of scale16by8
            hit.max_dist = 0;
        } else {
            hit.time = timer_read();
        }
        indicator_hit[indicator_type] = hit;
    }
    if (!indicator_on && hit.tick == 0) {
        return;  // Indicator off done state
    } else if (!indicator_on || hit.tick < 255) {
        uint16_t elapsed_time = timer_elapsed(hit.time);
        uint16_t tick         = scale16by8(elapsed_time, rgb_matrix_config.speed | 1) | 1;  // Set speed and tick LSB to 1 to make sure they are never 0
        if (indicator_on) {
            indicator_hit[indicator_type].tick = hit.tick = tick < 255 ? tick : 255;
        } else {
            indicator_hit[indicator_type].tick = hit.tick = tick < hit.max_dist ? hit.max_dist - tick : 0;  // TODO: Convert tick to 8 bit and use qsub8 or sub8
        }
    }
#endif  // INDICATOR_EFFECT_ENABLE

    for (uint8_t i = led_min; i < led_max; i++) {
        uint16_t keycode = keymap_key_to_keycode(layer, led_key[i]);
        if ((*is_keycode)(keycode)) {
#ifdef INDICATOR_EFFECT_ENABLE
            if (hit.tick < 255) {
                led_point_t led_point = g_led_config.point[i];
                int16_t     dx        = led_point.x - hit.point.x;
                int16_t     dy        = led_point.y - hit.point.y;
                uint8_t     dist      = sqrt16(dx * dx + dy * dy);
                if (dist > hit.tick) {
                    continue;
                } else if (indicator_on && dist > hit.max_dist) {
                    indicator_hit[indicator_type].max_dist = hit.max_dist = dist;
                }
            }  // hit.tick == 255 -> Indicator on done state
#endif         // INDICATOR_EFFECT_ENABLE
            rgb_matrix_set_color(i, red, green, blue);
        }
    }
}

#ifdef CAPS_LOCK_INDICATOR_COLOR
static bool is_caps_lock_indicator(uint16_t keycode) {
#    ifdef CAPS_LOCK_INDICATOR_LIGHT_ALPHAS
    return (KC_A <= keycode && keycode <= KC_Z) || keycode == KC_CAPS;
#    else   // CAPS_LOCK_INDICATOR_LIGHT_ALPHAS
    return keycode == KC_CAPS;
#    endif  // !CAPS_LOCK_INDICATOR_LIGHT_ALPHAS
}
#endif

#ifdef FN_LAYER_TRANSPARENT_KEYS_OFF
static bool is_transparent(uint16_t keycode) { return keycode == KC_TRNS; }
#endif

void rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t current_layer = get_highest_layer(layer_state);
    switch (current_layer) {
        case MAC_BASE:
        case WIN_BASE:
#ifdef CAPS_LOCK_INDICATOR_COLOR
            if (host_keyboard_led_state().caps_lock) {
                rgb_matrix_set_color_by_keycode(led_min, led_max, current_layer, is_caps_lock_indicator, CAPS_LOCK_INDICATOR_COLOR, CAPS_LOCK, true);
#    ifdef INDICATOR_EFFECT_ENABLE
            } else {
                rgb_matrix_set_color_by_keycode(led_min, led_max, current_layer, is_caps_lock_indicator, CAPS_LOCK_INDICATOR_COLOR, CAPS_LOCK, false);
#    endif  // INDICATOR_EFFECT_ENABLE
            }
#endif  // CAPS_LOCK_INDICATOR_COLOR
#ifdef FN_LAYER_TRANSPARENT_KEYS_OFF
#    ifdef INDICATOR_EFFECT_ENABLE
            rgb_matrix_set_color_by_keycode(led_min, led_max, last_fn_layer, is_transparent, RGB_OFF, FN_LAYER, false);
#    endif  // INDICATOR_EFFECT_ENABLE
            break;
        case MAC_FN:
        case WIN_FN:
            rgb_matrix_set_color_by_keycode(led_min, led_max, current_layer, is_transparent, RGB_OFF, FN_LAYER, true);
            if (last_fn_layer != current_layer) {
                last_fn_layer = current_layer;
            }
#endif  // FN_LAYER_TRANSPARENT_KEYS_OFF
            break;
    }
}
