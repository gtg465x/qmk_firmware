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

#pragma once

// clang-format off

typedef enum {
    CAPS_LOCK,
    FN_LAYER,
    INDICATOR_TYPE_COUNT
} indicator_type_t;

// clang-format on

#ifdef INDICATOR_EFFECT_ENABLE
typedef struct {  // Should this be PACKED?
    led_point_t point;
    uint16_t    time;
    uint16_t    tick;
} indicator_hit_t;
#endif
