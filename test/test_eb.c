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
 * Tests for the binary elliptic curve arithmetic module.
 *
 * @version $Id$
 * @ingroup test
 */

#include <stdio.h>

#include "relic.h"
#include "relic_test.h"

void eb_new_impl(eb_t a) {
	eb_new(a);
}

int memory(void) {
	err_t e;
	int code = STS_ERR;
	eb_t a = NULL;

	TRY {
		TEST_BEGIN("memory can be allocated") {
			/* We need another function call for the stack case, so that
			 * the frame for this function does not provoke an overflow. */
			eb_new_impl(a);
			eb_free(a);
		} TEST_END;
	} CATCH(e) {
		switch (e) {
			case ERR_NO_MEMORY:
				util_print("FATAL ERROR!\n");
				ERROR(end);
				break;
		}
	}
	code = STS_OK;
  end:
	return code;
}

int util(void) {
	int code = STS_ERR;
	eb_t a = NULL, b = NULL, c = NULL;

	TRY {
		eb_new(a);
		eb_new(b);
		eb_new(c);

		TEST_BEGIN("comparison is consistent") {
			eb_rand(a);
			eb_rand(b);
			TEST_ASSERT(eb_cmp(a, b) != CMP_EQ, end);
		}
		TEST_END;

		TEST_BEGIN("copy and comparison are consistent") {
			eb_rand(a);
			eb_rand(b);
			eb_rand(c);
			if (eb_cmp(a, c) != CMP_EQ) {
				eb_copy(c, a);
				TEST_ASSERT(eb_cmp(c, a) == CMP_EQ, end);
			}
			if (eb_cmp(b, c) != CMP_EQ) {
				eb_copy(c, b);
				TEST_ASSERT(eb_cmp(b, c) == CMP_EQ, end);
			}
		}
		TEST_END;

		TEST_BEGIN("negation is consistent") {
			eb_rand(a);
			eb_neg(b, a);
			TEST_ASSERT(eb_cmp(a, b) != CMP_EQ, end);
			eb_neg(b, b);
			TEST_ASSERT(eb_cmp(a, b) == CMP_EQ, end);
		}
		TEST_END;

		TEST_BEGIN("assignment to random and comparison are consistent") {
			eb_rand(a);
			eb_set_infty(c);
			TEST_ASSERT(eb_cmp(a, c) != CMP_EQ, end);
			TEST_ASSERT(eb_cmp(c, a) != CMP_EQ, end);
		}
		TEST_END;

		TEST_BEGIN("assignment to infinity and infinity test are consistent") {
			eb_set_infty(a);
			TEST_ASSERT(eb_is_infty(a), end);
		}
		TEST_END;
	}
	CATCH_ANY {
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(a);
	eb_free(b);
	eb_free(c);
	return code;
}

int addition(void) {
	int code = STS_ERR;

	eb_t a = NULL, b = NULL, c = NULL, d = NULL, e = NULL;

	TRY {
		eb_new(a);
		eb_new(b);
		eb_new(c);
		eb_new(d);
		eb_new(e);

		TEST_BEGIN("point addition is commutative") {
			eb_rand(a);
			eb_rand(b);
			eb_add(d, a, b);
			eb_add(e, b, a);
			TEST_ASSERT(eb_cmp(d, e) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point addition is associative") {
			eb_rand(a);
			eb_rand(b);
			eb_rand(c);
			eb_add(d, a, b);
			eb_add(d, d, c);
			eb_add(e, b, c);
			eb_add(e, e, a);
			eb_norm(d, d);
			eb_norm(e, e);
			TEST_ASSERT(eb_cmp(d, e) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point addition has identity") {
			eb_rand(a);
			eb_set_infty(d);
			eb_add(e, a, d);
			TEST_ASSERT(eb_cmp(e, a) == CMP_EQ, end);
			eb_add(e, d, a);
			TEST_ASSERT(eb_cmp(e, a) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point addition has inverse") {
			eb_rand(a);
			eb_neg(d, a);
			eb_add(e, a, d);
			TEST_ASSERT(eb_is_infty(e), end);
		} TEST_END;

#if EB_ADD == BASIC || !defined(STRIP)
		TEST_BEGIN("point addition in affine coordinates is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_add(d, a, b);
			eb_norm(d, d);
			eb_add_basic(e, a, b);
			TEST_ASSERT(eb_cmp(e, d) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_ADD == PROJC || !defined(STRIP)
		TEST_BEGIN("point addition in projective coordinates is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_add_projc(a, a, b);
			eb_rand(b);
			eb_rand(c);
			eb_add_projc(b, b, c);
			/* a and b in projective coordinates. */
			eb_add_projc(d, a, b);
			eb_norm(d, d);
			eb_norm(a, a);
			eb_norm(b, b);
			eb_add(e, a, b);
			eb_norm(e, e);
			TEST_ASSERT(eb_cmp(e, d) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point addition in mixed coordinates (z2 = 1) is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_add_projc(a, a, b);
			eb_rand(b);
			/* a and b in projective coordinates. */
			eb_add_projc(d, a, b);
			eb_norm(d, d);
			/* a in affine coordinates. */
			eb_norm(a, a);
			eb_add(e, a, b);
			eb_norm(e, e);
			TEST_ASSERT(eb_cmp(e, d) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point addition in mixed coordinates (z1,z2 = 1) is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_norm(a, a);
			eb_norm(b, b);
			/* a and b in affine coordinates. */
			eb_add(d, a, b);
			eb_norm(d, d);
			eb_add_projc(e, a, b);
			eb_norm(e, e);
			TEST_ASSERT(eb_cmp(e, d) == CMP_EQ, end);
		} TEST_END;
#endif
	}
	CATCH_ANY {
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(a);
	eb_free(b);
	eb_free(c);
	eb_free(d);
	eb_free(e);
	return code;
}

int subtraction(void) {
	int code = STS_ERR;
	eb_t a = NULL, b = NULL, c = NULL, d = NULL;

	TRY {
		eb_new(a);
		eb_new(b);
		eb_new(c);
		eb_new(d);

		TEST_BEGIN("point subtraction is anti-commutative") {
			eb_rand(a);
			eb_rand(b);
			eb_sub(c, a, b);
			eb_sub(d, b, a);
			eb_norm(c, c);
			eb_norm(d, d);
			eb_neg(d, d);
			TEST_ASSERT(eb_cmp(c, d) == CMP_EQ, end);
		}
		TEST_END;

		TEST_BEGIN("point subtraction has identity") {
			eb_rand(a);
			eb_set_infty(c);
			eb_sub(d, a, c);
			eb_norm(d, d);
			TEST_ASSERT(eb_cmp(d, a) == CMP_EQ, end);
		}
		TEST_END;

		TEST_BEGIN("point subtraction has inverse") {
			eb_rand(a);
			eb_sub(c, a, a);
			eb_norm(c, c);
			TEST_ASSERT(eb_is_infty(c), end);
		}
		TEST_END;

#if EB_ADD == BASIC || !defined(STRIP)
		TEST_BEGIN("point subtraction in affine coordinates is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_sub(c, a, b);
			eb_norm(c, c);
			eb_sub_basic(d, a, b);
			TEST_ASSERT(eb_cmp(c, d) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_ADD == PROJC || !defined(STRIP)
		TEST_BEGIN("point subtraction in projective coordinates is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_add_projc(a, a, b);
			eb_rand(b);
			eb_rand(c);
			eb_add_projc(b, b, c);
			/* a and b in projective coordinates. */
			eb_sub_projc(c, a, b);
			eb_norm(c, c);
			eb_norm(a, a);
			eb_norm(b, b);
			eb_sub(d, a, b);
			eb_norm(d, d);
			TEST_ASSERT(eb_cmp(c, d) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point subtraction in mixed coordinates (z2 = 1) is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_add_projc(a, a, b);
			eb_rand(b);
			/* a and b in projective coordinates. */
			eb_sub_projc(c, a, b);
			eb_norm(c, c);
			/* a in affine coordinates. */
			eb_norm(a, a);
			eb_sub(d, a, b);
			eb_norm(d, d);
			TEST_ASSERT(eb_cmp(c, d) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point subtraction in mixed coordinates (z1,z2 = 1) is correct") {
			eb_rand(a);
			eb_rand(b);
			eb_norm(a, a);
			eb_norm(b, b);
			/* a and b in affine coordinates. */
			eb_sub(c, a, b);
			eb_norm(c, c);
			eb_sub_projc(d, a, b);
			eb_norm(d, d);
			TEST_ASSERT(eb_cmp(c, d) == CMP_EQ, end);
		} TEST_END;
#endif
	}
	CATCH_ANY {
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(a);
	eb_free(b);
	eb_free(c);
	eb_free(d);
	return code;
}

int doubling(void) {
	int code = STS_ERR;
	eb_t a = NULL, b = NULL, c = NULL;

	TRY {
		eb_new(a);
		eb_new(b);
		eb_new(c);

		TEST_BEGIN("point doubling is correct") {
			eb_rand(a);
			eb_add(b, a, a);
			eb_norm(b, b);
			eb_dbl(c, a);
			eb_norm(c, c);
			TEST_ASSERT(eb_cmp(b, c) == CMP_EQ, end);
		} TEST_END;

#if EB_ADD == BASIC || !defined(STRIP)
		TEST_BEGIN("point doubling in affine coordinates is correct") {
			eb_rand(a);
			eb_dbl(b, a);
			eb_norm(b, b);
			eb_dbl_basic(c, a);
			TEST_ASSERT(eb_cmp(b, c) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_ADD == PROJC || !defined(STRIP)
		TEST_BEGIN("point doubling in projective coordinates is correct") {
			eb_rand(a);
			eb_dbl_projc(a, a);
			/* a in projective coordinates. */
			eb_dbl_projc(b, a);
			eb_norm(b, b);
			eb_norm(a, a);
			eb_dbl(c, a);
			eb_norm(c, c);
			TEST_ASSERT(eb_cmp(b, c) == CMP_EQ, end);
		} TEST_END;

		TEST_BEGIN("point doubling in mixed coordinates (z1 = 1) is correct") {
			eb_rand(a);
			eb_dbl_projc(b, a);
			eb_norm(b, b);
			eb_dbl(c, a);
			eb_norm(c, c);
			TEST_ASSERT(eb_cmp(b, c) == CMP_EQ, end);
		} TEST_END;
#endif

	}
	CATCH_ANY {
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(a);
	eb_free(b);
	eb_free(c);
	return code;
}

int multiplication(void) {
	int code = STS_ERR;
	eb_t p = NULL, q = NULL, r = NULL;
	bn_t n = NULL, k = NULL;

	TRY {
		eb_new(q);
		eb_new(r);
		bn_new(k);

		p = eb_curve_get_gen();
		n = eb_curve_get_ord();

		TEST_BEGIN("generator has the right order") {
			eb_mul(r, p, n);
			TEST_ASSERT(eb_is_infty(r) == 1, end);
		} TEST_END;

		TEST_BEGIN("generator multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_gen(r, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;

#if EB_MUL == BASIC || !defined(STRIP)
		TEST_BEGIN("binary point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_basic(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_MUL == CONST || !defined(STRIP)
		if (!eb_curve_is_super()) {
			TEST_BEGIN("constant-time point multiplication is correct") {
				bn_rand(k, BN_POS, bn_bits(n));
				bn_mod_basic(k, k, n);
				eb_mul(q, p, k);
				eb_mul_const(r, p, k);
				TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
			}
			TEST_END;
		}
#endif

#if EB_MUL == WTNAF || !defined(STRIP)
		TEST_BEGIN("wtnaf point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_wtnaf(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		}
		TEST_END;
#endif

	}
	CATCH_ANY {
		util_print("FATAL ERROR!\n");
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(q);
	eb_free(r);
	bn_free(k);
	return code;
}

int fixed(void) {
	int code = STS_ERR;
	eb_t p = NULL, q = NULL, r = NULL;
	eb_t t[FB_BITS] = { NULL };
	bn_t n = NULL, k = NULL;

	TRY {
		p = eb_curve_get_gen();
		eb_new(q);
		eb_new(r);
		bn_new(k);

		n = eb_curve_get_ord();

		for (int i = 0; i < EB_TABLE; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre(t, p);
			eb_mul_fix(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE; i++) {
			eb_free(t[i]);
		}

#if EB_FIX == BASIC || !defined(STRIP)
		for (int i = 0; i < EB_TABLE_BASIC; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("binary fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre_basic(t, p);
			eb_mul_fix_basic(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE_BASIC; i++) {
			eb_free(t[i]);
		}
#endif

#if EB_FIX == YAOWI || !defined(STRIP)
		for (int i = 0; i < EB_TABLE_YAOWI; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("yao windowing fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre_yaowi(t, p);
			eb_mul_fix_yaowi(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE_YAOWI; i++) {
			eb_free(t[i]);
		}
#endif

#if EB_FIX == NAFWI || !defined(STRIP)
		for (int i = 0; i < EB_TABLE_NAFWI; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("naf windowing fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre_nafwi(t, p);
			eb_mul_fix_nafwi(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE_NAFWI; i++) {
			eb_free(t[i]);
		}
#endif

#if EB_FIX == COMBS || !defined(STRIP)
		for (int i = 0; i < EB_TABLE_COMBS; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("single-table comb fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre_combs(t, p);
			eb_mul_fix_combs(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE_COMBS; i++) {
			eb_free(t[i]);
		}
#endif

#if EB_FIX == COMBD || !defined(STRIP)
		for (int i = 0; i < EB_TABLE_COMBD; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("double-table comb fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre_combd(t, p);
			eb_mul_fix_combd(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE_COMBD; i++) {
			eb_free(t[i]);
		}
#endif

#if EB_FIX == WTNAF || !defined(STRIP)
		for (int i = 0; i < EB_TABLE_WTNAF; i++) {
			eb_new(t[i]);
		}
		TEST_BEGIN("wtnaf fixed point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			eb_mul(q, p, k);
			eb_mul_pre_wtnaf(t, p);
			eb_mul_fix_wtnaf(q, t, k);
			eb_mul(r, p, k);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
		for (int i = 0; i < EB_TABLE_WTNAF; i++) {
			eb_free(t[i]);
		}
#endif
	}
	CATCH_ANY {
		util_print("FATAL ERROR!\n");
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(q);
	eb_free(r);
	bn_free(k);
	return code;
}

int simultaneous(void) {
	int code = STS_ERR;
	eb_t p = NULL, q = NULL, r = NULL, s = NULL;
	bn_t n = NULL, k = NULL, l = NULL;

	TRY {
		p = eb_curve_get_gen();
		eb_new(q);
		eb_new(r);
		eb_new(s);
		bn_new(k);
		bn_new(l);

		n = eb_curve_get_ord();

		TEST_BEGIN("simultaneous point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			bn_rand(l, BN_POS, bn_bits(n));
			bn_mod_basic(l, l, n);
			eb_mul(q, p, k);
			eb_mul(s, q, l);
			eb_mul_sim(r, p, k, q, l);
			eb_add(q, q, s);
			eb_norm(q, q);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;

#if EB_SIM == BASIC || !defined(STRIP)
		TEST_BEGIN("basic simultaneous point multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			bn_rand(l, BN_POS, bn_bits(n));
			bn_mod_basic(l, l, n);
			eb_mul_sim(r, p, k, q, l);
			eb_mul_sim_basic(q, p, k, q, l);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_SIM == TRICK || !defined(STRIP)
		TEST_BEGIN("shamir's trick for simultaneous multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			bn_rand(l, BN_POS, bn_bits(n));
			bn_mod_basic(l, l, n);
			eb_mul_sim(r, p, k, q, l);
			eb_mul_sim_trick(q, p, k, q, l);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_SIM == INTER || !defined(STRIP)
		TEST_BEGIN("interleaving for simultaneous multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			bn_rand(l, BN_POS, bn_bits(n));
			bn_mod_basic(l, l, n);
			eb_mul_sim(r, p, k, q, l);
			eb_mul_sim_inter(q, p, k, q, l);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
#endif

#if EB_SIM == JOINT || !defined(STRIP)
		TEST_BEGIN("jsf for simultaneous multiplication is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			bn_rand(l, BN_POS, bn_bits(n));
			bn_mod_basic(l, l, n);
			eb_mul_sim(r, p, k, q, l);
			eb_mul_sim_joint(q, p, k, q, l);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
#endif

		TEST_BEGIN("simultaneous multiplication with generator is correct") {
			bn_rand(k, BN_POS, bn_bits(n));
			bn_mod_basic(k, k, n);
			bn_rand(l, BN_POS, bn_bits(n));
			bn_mod_basic(l, l, n);
			eb_mul_sim_gen(r, k, q, l);
			eb_mul_sim(q, eb_curve_get_gen(), k, q, l);
			TEST_ASSERT(eb_cmp(q, r) == CMP_EQ, end);
		} TEST_END;
	}
	CATCH_ANY {
		util_print("FATAL ERROR!\n");
		ERROR(end);
	}
	code = STS_OK;
  end:
	eb_free(q);
	eb_free(r);
	bn_free(k);
	return code;
}

int test(void) {
	if (util() != STS_OK) {
		return STS_ERR;
	}

	if (addition() != STS_OK) {
		return STS_ERR;
	}

	if (subtraction() != STS_OK) {
		return STS_ERR;
	}

	if (doubling() != STS_OK) {
		return STS_ERR;
	}

	if (multiplication() != STS_OK) {
		return STS_ERR;
	}

	if (fixed() != STS_OK) {
		return STS_ERR;
	}

	if (simultaneous() != STS_OK) {
		return STS_ERR;
	}

	return STS_OK;
}

int main(void) {
	core_init();

	if (memory() != STS_OK) {
		core_clean();
		return 1;
	}
#if defined(EB_STAND) && defined(EB_ORDIN) && FB_POLYN == 163
	eb_param_set(NIST_B163);
	util_print("Curve NIST-B163:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif
#if defined(EB_STAND) && defined(EB_KBLTZ) && FB_POLYN == 163
	eb_param_set(NIST_K163);
	util_print("Curve NIST-K163:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

#if defined(EB_STAND) && defined(EB_ORDIN) && FB_POLYN == 233
	eb_param_set(NIST_B233);
	util_print("\nCurve NIST-B233:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif
#if defined(EB_STAND) && defined(EB_KBLTZ) && FB_POLYN == 233
	eb_param_set(NIST_K233);
	util_print("\nCurve NIST-K233:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

#if defined(EB_STAND) && defined(EB_ORDIN) && FB_POLYN == 283
	eb_param_set(NIST_B283);
	util_print("\nCurve NIST-B283:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif
#if defined(EB_STAND) && defined(EB_KBLTZ) && FB_POLYN == 283
	eb_param_set(NIST_K283);
	util_print("\nCurve NIST-K283:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

#if defined(EB_STAND) && defined(EB_ORDIN) && FB_POLYN == 409
	eb_param_set(NIST_B409);
	util_print("\nCurve NIST-B409:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif
#if defined(EB_STAND) && defined(EB_KBLTZ) && FB_POLYN == 409
	eb_param_set(NIST_K409);
	util_print("\nCurve NIST-K409:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

#if defined(EB_STAND) && defined(EB_ORDIN) && FB_POLYN == 571
	eb_param_set(NIST_B571);
	util_print("\nCurve NIST-B571:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif
#if defined(EB_STAND) && defined(EB_KBLTZ) && FB_POLYN == 571
	eb_param_set(NIST_K571);
	util_print("\nCurve NIST-K571:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

#if defined(EB_SUPER) && FB_POLYN == 271
	eb_param_set(ETAT_S271);
	util_print("\nCurve ETAT-S271:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

#if defined(EB_SUPER) && FB_POLYN == 1223
	eb_param_set(ETAT_S1223);
	util_print("\nCurve ETAT-S1223:\n");

	if (test() != STS_OK) {
		core_clean();
		return 1;
	}
#endif

	core_clean();
	return 0;
}