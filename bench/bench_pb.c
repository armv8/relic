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
 * Benchmarks for the binary elliptic curve module.
 *
 * @version $Id$
 * @ingroup bench
 */

#include <stdio.h>

#include "relic.h"
#include "relic_bench.h"

void arith(void) {
	eb_t p, q;
	fb4_t r;

	fb4_new(r);
	eb_new(p);
	eb_new(q);

	BENCH_BEGIN("pb_map") {
		eb_rand(p);
		eb_rand(q);
		BENCH_ADD(pb_map(r, p, q));
	}
	BENCH_END;

#if PB_MAP == ETATS || !defined(STRIP)
	BENCH_BEGIN("pb_map_etats") {
		eb_rand(p);
		eb_rand(q);
		BENCH_ADD(pb_map_etats(r, p, q));
	}
	BENCH_END;
#endif

#if PB_MAP == ETATN || !defined(STRIP)
	BENCH_BEGIN("pb_map_etatn") {
		eb_rand(p);
		eb_rand(q);
		BENCH_ADD(pb_map_etatn(r, p, q));
	}
	BENCH_END;
#endif

	eb_free(p);
	eb_free(q);
	fb4_free(r);
}

int main(void) {
	core_init();
	conf_print();

#if defined(EB_SUPER) && FB_POLYN == 271
	eb_param_set(ETAT_S271);
	util_print("Curve ETAT-S271:\n");
#elif defined(EB_SUPER) && FB_POLYN == 1223
	eb_param_set(ETAT_S1223);
	util_print("Curve ETAT-S1223:\n");
#endif

	util_print("\n--- Arithmetic:\n\n");
	arith();

	core_clean();
	return 0;
}