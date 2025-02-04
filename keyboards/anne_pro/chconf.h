/* Copyright 2020 QMK
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

/*
 * This file was auto-generated by:
 *    `qmk chibios-confmigrate -i keyboards/anne_pro/chconf.h -r lib/chibios/os/rt/templates/chconf.h`
 */

#pragma once

#define CH_CFG_ST_RESOLUTION 16

#define CH_CFG_ST_FREQUENCY 1000

#define CH_DBG_SYSTEM_STATE_CHECK FALSE

#define CH_DBG_ENABLE_CHECKS FALSE

#define CH_DBG_ENABLE_ASSERTS FALSE

#define CH_DBG_TRACE_MASK CH_DBG_TRACE_MASK_DISABLED

#define CH_DBG_ENABLE_STACK_CHECK FALSE

#define CH_DBG_FILL_THREADS FALSE

#define CH_CFG_SYSTEM_EXTRA_FIELDS \
  /* Add threads custom fields here.*/

#define CH_CFG_SYSTEM_INIT_HOOK() {                                         \
  /* Add threads initialization code here.*/                                \
}

#include_next <chconf.h>

