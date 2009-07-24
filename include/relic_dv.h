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
 * @defgroup dv Temporary double precision digit vector handling.
 */

/**
 * @file
 *
 * Interface of the function which manipulate temporary double precision digit
 * vectors.
 *
 * @version $Id$
 * @ingroup dv
 */

#ifndef RELIC_DV_H
#define RELIC_DV_H

#include <alloca.h>

#include "relic_conf.h"
#include "relic_types.h"
#include "relic_util.h"

/**
 * Size in bits of the biggest digit vector.
 */
#define TEMP MAX(BN_PRECI, FB_POLYN)

/**
 * Size in digits of a temporary vector.
 *
 * A temporary vector has enough size to store a multiplication result in any
 * finite field.
 */
#define DV_DIGS	(2 * (int)((TEMP)/(DIGIT) + (TEMP % DIGIT > 0)) + 2)

/**
 * Represents a temporary double precision digit vector.
 */
typedef dig_t *dv_t;

/**
 * Calls a function to allocate a temporary double precision digit vector.
 *
 * @param[out] A			- the double precision result.
 */
#if ALLOC == DYNAMIC
#define dv_new(A)			dv_new_dynam(&(A), DV_DIGS)
#elif ALLOC == STATIC
#define dv_new(A)			dv_new_statc(&(A), DV_DIGS)
#elif ALLOC == STACK
#define dv_new(A)															\
	A = (dig_t *)alloca(DV_DIGS * sizeof(dig_t) + ALIGN);					\
	A = (dig_t *)((dig_t)A + (ALIGN - ((dig_t)A & 0x0F)));					\

#endif

/**
 * Assigns zero to a temporary double precision digit vector.
 *
 * @param[out] a			- the temporary digit vector to assign.
 * @param[in] digits		- the number of words to initialize with zero.
 */
void dv_zero(dv_t a, int digits);

/**
 * Calls a function to clean and free a temporary double-precision digit vector.
 *
 * @param[out] A			- the temporary digit vector to clean and free.
 */
#if ALLOC == DYNAMIC
#define dv_free(A)		dv_free_dynam(&(A))
#elif ALLOC == STATIC
#define dv_free(A)		dv_free_statc(&(A))
#elif ALLOC == STACK
#define dv_free(A)		(void)A
#endif

/**
 * Allocates and initializes a temporary double precision digit vector.
 *
 * @param[out] a			- the new temporary digit vector.
 * @param[in] digits		- the required precision in digits.
 * @throw ERR_NO_MEMORY		- if there is no available memory.
 * @throw ERR_PRECISION		- if the required precision cannot be represented
 * 							by the library.
 */
#if ALLOC == DYNAMIC
void dv_new_dynam(dv_t *a, int digits);
#elif ALLOC == STATIC
void dv_new_statc(dv_t *a, int digits);
#endif

/**
 * Cleans and frees a temporary double precision digit vector.
 *
 * @param[out] a			- the temporary digit vector to clean and free.
 */
#if ALLOC == DYNAMIC
void dv_free_dynam(dv_t *a);
#elif ALLOC == STATIC
void dv_free_statc(dv_t *a);
#endif

#endif /* !RELIC_DV_H */