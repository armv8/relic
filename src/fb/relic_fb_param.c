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
 * Implementation of the binary field modulus manipulation.
 *
 * @version $Id$
 * @ingroup fb
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "relic_core.h"
#include "relic_conf.h"
#include "relic_dv.h"
#include "relic_fb.h"
#include "relic_fb_low.h"
#include "relic_error.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fb_param_set(int param) {
	switch (param) {
		case NIST_163:
			fb_poly_set_penta(7, 6, 3);
			break;
		case NIST_233:
			fb_poly_set_trino(74);
			break;
		case NIST_283:
			fb_poly_set_penta(12, 7, 5);
			break;
		case NIST_409:
			fb_poly_set_trino(87);
			break;
		case NIST_571:
			fb_poly_set_penta(10, 5, 2);
			break;
		case FAST_271:
			//fb_poly_set_trino(201);
			fb_poly_set_penta(207, 175, 111);
			break;
		case FAST_1223:
			fb_poly_set_trino(255);
			break;
		default:
			THROW(ERR_INVALID);
			break;
	}
}