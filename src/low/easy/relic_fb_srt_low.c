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
 * Implementation of the low-level binary field square root.
 *
 * @version $Id$
 * @ingroup fb
 */

#include <stdlib.h>

#include "relic_fb.h"
#include "relic_fb_low.h"
#include "relic_util.h"

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

static const dig_t table_evens[16] = {
	0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};
static const dig_t table_odds[16] = {
	0, 4, 1, 5, 8, 12, 9, 13, 2, 6, 3, 7, 10, 14, 11, 15
};

static void fb_srtt_low(dig_t *c, dig_t *a, int fa) {
	int i, j, n, h, sh, rh, lh, sa, la, ra;
	dv_t t = NULL;
	dig_t d, d_e, d_o;

	dv_new(t);
	dv_zero(t, 2 * FB_DIGS);

	sh = 1 + (FB_BITS >> FB_DIG_LOG);
	h = (sh + 1) / 2;
	rh = (h << FB_DIG_LOG) - 1 - (FB_BITS - 1) / 2;
	lh = FB_DIGIT - rh;

	SPLIT(la, sa, (fa + 1) >> 1, FB_DIG_LOG);
	ra = FB_DIGIT - la;

	for (i = 0; i < sh; i++) {
		n = i >> 1;
		d = a[i];

		d_e = d_o = 0;
		for (j = 0; j < FB_DIGIT / 8; j++) {
			d_e |= table_evens[((d & 0x05) + ((d & 0x50) >> 3))] << (j << 2);
			d_o |= table_odds[((d & 0x0A) + ((d & 0xA0) >> 5))] << (j << 2);
			d >>= 8;
		}

		i++;
		if (i < sh) {
			d = a[i];

			for (j = 0; j < FB_DIGIT / 8; j++) {
				d_e |= table_evens[((d & 0x05) + ((d & 0x50) >> 3))] <<
						((FB_DIGIT / 2) + (j << 2));
				d_o |= table_odds[((d & 0x0A) + ((d & 0xA0) >> 5))] <<
						((FB_DIGIT / 2) + (j << 2));
				d >>= 8;
			}
		}

		t[n] ^= d_e;

		if (rh == 0) {
			t[h + n] = d_o;
		} else {
			t[h + n - 1] ^= (d_o << lh);
			t[h + n] ^= (d_o >> rh);
		}
		if (la == 0) {
			t[n + sa] ^= d_o;
		} else {
			t[n + sa] ^= (d_o << la);
			t[n + sa + 1] ^= (d_o >> ra);
		}
	}
	fb_copy(c, t);
	dv_free(t);
}

static void fb_srtp_low(dig_t *c, dig_t *a, int fa, int fb, int fc) {
	int i, j, n, h, sh, rh, lh, sa, la, ra, sb, lb, rb, sc, lc, rc;
	dv_t t = NULL;
	dig_t d, d_e, d_o;

	dv_new(t);
	dv_zero(t, 2 * FB_DIGS);

	sh = 1 + (FB_BITS >> FB_DIG_LOG);
	h = (sh + 1) >> 1;
	rh = (h << FB_DIG_LOG) - 1 - (FB_BITS - 1) / 2;
	lh = FB_DIGIT - rh;

	SPLIT(la, sa, (fa + 1) >> 1, FB_DIG_LOG);
	ra = FB_DIGIT - la;

	SPLIT(lb, sb, (fb + 1) >> 1, FB_DIG_LOG);
	rb = FB_DIGIT - lb;

	SPLIT(lc, sc, (fc + 1) >> 1, FB_DIG_LOG);
	rc = FB_DIGIT - lc;

	for (i = 0; i < sh; i++) {
		n = i >> 1;
		d = a[i];

		d_e = d_o = 0;
		for (j = 0; j < FB_DIGIT / 8; j++) {
			d_e |= table_evens[((d & 0x5) + ((d & 0x50) >> 3))] << (j << 2);
			d_o |= table_odds[((d & 0xA) + ((d & 0xA0) >> 5))] << (j << 2);
			d >>= 8;
		}
		i++;

		if (i < sh) {
			d = a[i];
			for (j = 0; j < FB_DIGIT / 8; j++) {
				d_e |= table_evens[((d & 0x5) + ((d & 0x50) >> 3))] <<
						(FB_DIGIT / 2 + (j << 2));
				d_o |= table_odds[((d & 0xA) + ((d & 0xA0) >> 5))] <<
						(FB_DIGIT / 2 + (j << 2));
				d >>= 8;
			}
		}

		t[n] ^= d_e;

		if (rh == 0) {
			t[h + n] = d_o;
		} else {
			t[h + n - 1] ^= (d_o << lh);
			t[h + n] ^= (d_o >> rh);
		}
		if (la == 0) {
			t[n + sa] ^= d_o;
		} else {
			t[n + sa] ^= (d_o << la);
			t[n + sa + 1] ^= (d_o >> ra);
		}
		if (lb == 0) {
			t[n + sb] ^= d_o;
		} else {
			t[n + sb] ^= (d_o << lb);
			t[n + sb + 1] ^= (d_o >> rb);
		}
		if (lc == 0) {
			t[n + sc] ^= d_o;
		} else {
			t[n + sc] ^= (d_o << lc);
			t[n + sc + 1] ^= (d_o >> rc);
		}
	}
	fb_copy(c, t);
	dv_free(t);
}

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fb_srtn_low(dig_t *c, dig_t *a) {
	int fa, fb, fc;

	fb_poly_get_quick(&fa, &fb, &fc);

	if (fb == 0) {
		fb_srtt_low(c, a, fa);
	} else {
		fb_srtp_low(c, a, fa, fb, fc);
	}
}