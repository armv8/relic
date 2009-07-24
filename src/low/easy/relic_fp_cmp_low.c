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
 * Implementation of the low-level prime field comparison functions.
 *
 * @version $Id$
 * @ingroup fp
 */

#include "relic_core.h"
#include "relic_fp.h"
#include "relic_fp_low.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

int fp_cmp1_low(dig_t a, dig_t b) {
	if (a > b)
		return CMP_GT;
	if (a < b)
		return CMP_LT;
	return CMP_EQ;
}

int fp_cmpn_low(dig_t *a, dig_t *b) {
	int i;

	a += (FP_DIGS - 1);
	b += (FP_DIGS - 1);
	for (i = 0; i < FP_DIGS; i++, --a, --b) {
		if (*a != *b) {
			return (*a > *b ? CMP_GT : CMP_LT);
		}
	}
	return CMP_EQ;
}