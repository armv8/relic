/*
 * Copyright 2007 Project RELIC
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file.
 *
 * RELIC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RELIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RELIC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * Elementary types.
 *
 * @version $Id: relic_types.h 38 2009-06-03 18:15:41Z dfaranha $
 * @ingroup relic
 */

#ifndef RELIC_TYPES_H
#define RELIC_TYPES_H

#include <stdint.h>

#include "relic_conf.h"

#if ARITH == GMP
#include <gmp.h>
#endif

/**
 * Represents a digit from a multiple precision integer.
 *
 * Each digit is represented as an unsigned long to use the biggest native
 * type that potentially supports native instructions.
 */
#if ARITH == GMP
typedef mp_limb_t dig_t;
#elif WORD == 8
typedef uint8_t dig_t;
#elif WORD == 16
typedef uint16_t dig_t;
#elif WORD == 32
typedef uint32_t dig_t;
#elif WORD == 64
typedef uint64_t dig_t;
#endif

/**
 * Represents a signed digit.
 */
#if WORD == 8
typedef int8_t sig_t;
#elif WORD == 16
typedef int16_t sig_t;
#elif WORD == 32
typedef int32_t sig_t;
#elif WORD == 64
typedef int64_t sig_t;
#endif

/**
 * Represents a double-precision integer from a multiple precision integer.
 *
 * This is useful to store a result from a multiplication of two digits.
 */
#if WORD == 8
typedef uint16_t dbl_t;
#elif WORD == 16
typedef uint32_t dbl_t;
#elif WORD == 32
typedef uint64_t dbl_t;
#elif WORD == 64
typedef __uint128_t dbl_t;
#endif

/**
 * Size in bits of a digit.
 */
#define DIGIT	(8 * sizeof(dig_t))

/**
 * Logarithm of the digit size in bits in base two.
 */
#if WORD == 8
#define DIGIT_LOG		3
#elif WORD == 16
#define DIGIT_LOG		4
#elif WORD == 32
#define DIGIT_LOG		5
#elif WORD == 64
#define DIGIT_LOG		6
#endif

/**
 * Specification for aligned variables.
 */
#ifdef ALIGN
#define align 	__attribute__ ((aligned (ALIGN)))
#else
#define align 	/* empty*/
#endif

#endif /* !RELIC_TYPES_H */