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
 * Implementation of the low-level prime field modular reduction functions.
 *
 * @version $Id$
 * @ingroup fp
 */

#include "relic_fp.h"
#include "relic_fp_low.h"

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

/**
 * Accumulates a double precision digit in a triple register variable.
 *
 * @param[in,out] R2		- most significant word of the triple register.
 * @param[in,out] R1		- middle word of the triple register.
 * @param[in,out] R0		- lowest significant word of the triple register.
 * @param[in] A				- the first digit to multiply.
 * @param[in] B				- the second digit to multiply.
 */
#define COMBA_STEP(R2, R1, R0, A, B)										\
	dbl_t r = (dbl_t)(A) * (dbl_t)(B);										\
	dig_t _r = (R1);														\
	(R0) += (dig_t)(r);														\
	(R1) += (R0) < (dig_t)(r);												\
	(R2) += (R1) < _r;														\
	(R1) += (dig_t)(r >> (dbl_t)BN_DIGIT);								\
	(R2) += (R1) < (dig_t)(r >> (dbl_t)BN_DIGIT);						\

/**
 * Accumulates a single precision digit in a triple register variable.
 *
 * @param[in,out] R2		- most significant word of the triple register.
 * @param[in,out] R1		- middle word of the triple register.
 * @param[in,out] R0		- lowest significant word of the triple register.
 * @param[in] A				- the first digit to accumulate.
 */
#define COMBA_ADD(R2, R1, R0, A)											\
	dig_t __r = (R1);														\
	(R0) += (A);															\
	(R1) += (R0) < (A);														\
	(R2) += (R1) < __r;														\

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

dig_t fp_rdcn_low(dig_t *c, dig_t *a, dig_t *m, dig_t u) {
	int i, j;
	dig_t r0, r1, r2;
	dig_t *tmp, *tmpm, *tmpc;

	tmpc = c;

	r0 = r1 = r2 = 0;
	for (i = 0; i < FP_DIGS; i++, tmpc++, a++) {
		tmp = c;
		tmpm = m + i;
		for (j = 0; j < i; j++, tmp++, tmpm--) {
			COMBA_STEP(r2, r1, r0, *tmp, *tmpm);
		}
		COMBA_ADD(r2, r1, r0, *a);
		*tmpc = (dig_t)(r0 * u);
		COMBA_STEP(r2, r1, r0, *tmpc, *m);
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	for (i = FP_DIGS; i < 2 * FP_DIGS - 1; i++, a++) {
		tmp = c + (i - FP_DIGS + 1);
		tmpm = m + FP_DIGS - 1;
		for (j = i - FP_DIGS + 1; j < FP_DIGS; j++, tmp++, tmpm--) {
			COMBA_STEP(r2, r1, r0, *tmp, *tmpm);
		}
		COMBA_ADD(r2, r1, r0, *a);
		c[i - FP_DIGS] = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	COMBA_ADD(r2, r1, r0, *a);
	c[FP_DIGS - 1] = r0;

	return r1;
}