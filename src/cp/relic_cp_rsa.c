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
 * Implementation of the RSA protocol.
 *
 * @version $Id$
 * @ingroup cp
 */

#include <string.h>

#include "relic_core.h"
#include "relic_conf.h"
#include "relic_error.h"
#include "relic_rand.h"
#include "relic_bn.h"
#include "relic_util.h"
#include "relic_cp.h"
#include "relic_hf.h"

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

#define RSA_EXP 			"65537"

#if CP_RSAPD == PKCS1
#define RSA_PAD_LENGTH		(11)
#elif CP_RSAPD == PKCS2
#define RSA_PAD_LENGTH		(2 * 20 + 2)
#else
#define RSA_PAD_LENGTH		(0)
#endif

#define RSA_TYPE_PUB		(02)

#define RSA_TYPE_PRV		(01)

#define RSA_PAD_PRV			(0xFF)

#define HASH_LEN			(20)

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

#if CP_RSA == BASIC || !defined(STRIP)

int cp_rsa_gen_basic(rsa_pub_t * pub, rsa_prv_t * prv, int bits) {
	bn_t t = NULL, r = NULL;
	int result = STS_ERR;

	TRY {
		bn_new(t);
		bn_new(r);

		bn_gen_prime(prv->p, bits / 2);
		bn_gen_prime(prv->q, bits / 2);

		bn_mul(pub->n, prv->p, prv->q);
		bn_copy(prv->n, pub->n);
		bn_sub_dig(prv->p, prv->p, 1);
		bn_sub_dig(prv->q, prv->q, 1);

		bn_mul(t, prv->p, prv->q);

		bn_read_str(pub->e, RSA_EXP, strlen(RSA_EXP), 10);

		bn_gcd_ext(r, prv->d, NULL, pub->e, t);
		if (bn_sign(prv->d) == BN_NEG) {
			bn_add(prv->d, prv->d, t);
		}

		if (bn_cmp_dig(r, 1) == CMP_EQ) {
			bn_add_dig(prv->p, prv->p, 1);
			bn_add_dig(prv->q, prv->q, 1);

			result = STS_OK;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(t);
		bn_free(r);
	}

	return result;
}

#endif

#if CP_RSA == QUICK || !defined(STRIP)

int cp_rsa_gen_quick(rsa_pub_t * pub, rsa_prv_t * prv, int bits) {
	bn_t t = NULL, r = NULL;
	int result = STS_ERR;

	TRY {
		bn_new(t);
		bn_new(r);

		bn_gen_prime(prv->p, bits / 2);
		bn_gen_prime(prv->q, bits / 2);

		/* n = pq. */
		bn_mul(pub->n, prv->p, prv->q);
		bn_copy(prv->n, pub->n);
		bn_sub_dig(prv->p, prv->p, 1);
		bn_sub_dig(prv->q, prv->q, 1);

		/* phi(n) = (p - 1)(q - 1). */
		bn_mul(t, prv->p, prv->q);

		bn_read_str(pub->e, RSA_EXP, strlen(RSA_EXP), 10);

		/* d = e^(-1) mod phi(n). */
		bn_gcd_ext(r, prv->d, NULL, pub->e, t);
		if (bn_sign(prv->d) == BN_NEG) {
			bn_add(prv->d, prv->d, t);
		}

		if (bn_cmp_dig(r, 1) == CMP_EQ) {
			/* dP = d mod (p - 1). */
			bn_mod_basic(prv->dp, prv->d, prv->p);
			/* dQ = d mod (q - 1). */
			bn_mod_basic(prv->dq, prv->d, prv->q);

			bn_add_dig(prv->p, prv->p, 1);
			bn_add_dig(prv->q, prv->q, 1);

			/* qInv = q^(-1) mod p. */
			bn_gcd_ext(r, prv->qi, NULL, prv->q, prv->p);
			if (bn_sign(prv->qi) == BN_NEG) {
				bn_add(prv->qi, prv->qi, prv->p);
			}

			result = STS_OK;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(t);
		bn_free(r);
	}

	return STS_OK;
}

#endif

int cp_rsa_enc(unsigned char *out, int *out_len, unsigned char *in, int in_len,
		rsa_pub_t * pub) {
	bn_t m = NULL, eb = NULL;
	int sign = BN_POS, size, result = STS_OK;

	bn_size_bin(&size, pub->n);

	if (in_len > (size - RSA_PAD_LENGTH)) {
		return STS_ERR;
	}

	TRY {
		bn_new(m);
		bn_new(eb);

		bn_zero(m);
		bn_zero(eb);

#if CP_RSAPD == PKCS1
		{
			unsigned char pad;

			/* EB = 00 | 02 | PS | 00 | D. */
			bn_add_dig(eb, eb, 0);
			bn_lsh(eb, eb, 8);
			bn_add_dig(eb, eb, RSA_TYPE_PUB);

			for (int i = 0; i < size - 3 - in_len; i++) {
				bn_lsh(eb, eb, 8);
				do {
					rand_bytes(&pad, 1);
				} while (pad == 0);
				bn_add_dig(eb, eb, pad);
			}
			bn_lsh(eb, eb, 8);
			bn_add_dig(eb, eb, 0);
			bn_lsh(eb, eb, in_len * 8);
		}
#endif
#if CP_RSAPD == PKCS2
		{
			unsigned char hash[HASH_LEN + 4];

			hf_map(hash, "", 0);

			/* EB = HASH | PS | 01 | M. */
			bn_read_bin(eb, hash, HASH_LEN, BN_POS);
			bn_print(eb);
			bn_lsh(eb, eb, size - HASH_LEN - 1 - in_len);
			bn_add_dig(eb, eb, 1);
			bn_lsh(eb, eb, in_len);
		}
#endif

		bn_read_bin(m, in, in_len, BN_POS);
		bn_add(eb, eb, m);

#if CP_RSAPD == PKCS2
		{
			unsigned char seed[HASH_LEN + 4];
			unsigned char hash[BN_DIGS * sizeof(dig_t) + 4];
			int h_len = BN_DIGS * sizeof(dig_t);

			rand_bytes(seed, HASH_LEN);

			size = CEIL(size - HASH_LEN - 1, HASH_LEN) - 1;

			for (int i = 0; i < size; i++) {
				seed[HASH_LEN] = (i >> 24) & 0xFF;
				seed[HASH_LEN + 1] = (i >> 16) & 0xFF;
				seed[HASH_LEN + 2] = (i >> 8) & 0xFF;
				seed[HASH_LEN + 3] = i & 0xFF;
				hf_map(hash + i * HASH_LEN, seed, HASH_LEN + 4);
			}
			bn_zero(m);
			bn_read_bin(m, hash, size * HASH_LEN, BN_POS);

			for (int i = 0; i < m->used; i++) {
				eb->dp[i] ^= m->dp[i];
			}

			bn_write_bin(hash, &h_len, &sign, m);
			hash[h_len] = hash[h_len + 1] = hash[h_len + 2] = hash[h_len + 3] = 0;
			hf_map(hash, hash, HASH_LEN);

			bn_copy(m, eb);
			bn_read_bin(eb, hash, HASH_LEN, BN_POS);
			bn_size_bin(&size, m);
			bn_lsh(eb, eb, size);
			bn_add(eb, eb, m);
			bn_print(eb);
		}
#endif

#if BN_MOD == MONTY
		bn_mod_monty_conv(eb, eb, pub->n);
		bn_mxp(eb, eb, pub->e, pub->n);
		bn_mod_monty_back(eb, eb, pub->n);
#else
		bn_mxp(eb, eb, pub->e, pub->n);
#endif

		bn_size_bin(&size, pub->n);

		if (size <= *out_len) {

			*out_len = size;
			memset(out, 0, *out_len);
			bn_write_bin(out, &size, &sign, eb);
		} else {
			result = STS_ERR;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(m);
	}

	if (sign == BN_NEG) {
		return STS_ERR;
	}
	return result;
}

#if CP_RSA == BASIC || !defined(STRIP)

int cp_rsa_dec_basic(unsigned char *out, int *out_len, unsigned char *in,
		int in_len, rsa_prv_t * prv) {
	bn_t m = NULL, eb = NULL;
	int sign, size, result = STS_OK;

	bn_size_bin(&size, prv->n);

	if (in_len < 0 || in_len != size) {
		return STS_ERR;
	}

#if CP_RSAPD == PKCS2
	if (in_len < 2 * HASH_LEN + 2) {
		return STS_ERR;
	}
#endif

	TRY {
		bn_new(m);
		bn_new(eb);

		bn_read_bin(eb, in, in_len, BN_POS);
#if BN_MOD == MONTY
		bn_mod_monty_conv(eb, eb, prv->n);
		bn_mxp(eb, eb, prv->d, prv->n);
		bn_mod_monty_back(eb, eb, prv->n);
#else
		bn_mxp(eb, eb, prv->d, prv->n);
#endif

#if CP_RSAPD == PKCS1
		{
			unsigned char pad;
			/* EB = 00 | 02 | PS | 00 | D. */
			size--;
			bn_rsh(m, eb, 8 * size);
			if (!bn_is_zero(m)) {
				result = STS_ERR;
			} else {
				size--;
				bn_rsh(m, eb, 8 * size);
				bn_mod_2b(m, m, 8);
				pad = (unsigned char)m->dp[0];
				if (pad == RSA_TYPE_PUB) {
					size--;
					bn_rsh(m, eb, 8 * size);
					bn_mod_2b(m, m, 8);
					pad = (unsigned char)m->dp[0];
					while (pad != 0) {
						size--;
						bn_rsh(m, eb, 8 * size);
						bn_mod_2b(m, m, 8);
						pad = (unsigned char)m->dp[0];
					}
				} else {
					result = STS_ERR;
				}
			}
		}
#endif

#if CP_RSAPD == PKCS2
		{
			unsigned char seed[HASH_LEN + 4];
			unsigned char hash[BN_DIGS * sizeof(dig_t) + 4];
			int h_len = BN_DIGS * sizeof(dig_t);

			bn_mod_2b(m, eb, 8 * (size - HASH_LEN - 1));

			bn_write_bin(hash, &h_len, &sign,m);

			hf_map(hash, hash, h_len);

			bn_rsh(m, eb, 8 * (size - HASH_LEN - 1));
			bn_mod_2b(m, 8 * HASH_LEN);

		}
#endif

		if (result == STS_OK) {
			bn_mod_2b(m, eb, size * 8);
			bn_size_bin(&size, m);

			if (size <= *out_len) {
				bn_write_bin(out, &size, &sign, m);

				*out_len = size;
			} else {
				result = STS_ERR;
			}
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(m);
		bn_free(eb);
	}

	if (sign == BN_NEG) {
		return STS_ERR;
	}

	return result;
}

#endif

#if CP_RSA == QUICK || !defined(STRIP)

int cp_rsa_dec_quick(unsigned char *out, int *out_len, unsigned char *in,
		int in_len, rsa_prv_t * prv) {
	bn_t m = NULL, eb = NULL;
	int sign, size, result = STS_OK;

	bn_size_bin(&size, prv->n);

	if (in_len < 0 || in_len > size) {
		return STS_ERR;
	}

	TRY {
		bn_new(m);
		bn_new(eb);

		bn_read_bin(eb, in, in_len, BN_POS);

		bn_copy(m, eb);

		/* m1 = c^dP mod p. */
#if BN_MOD == MONTY
		bn_mod_monty_conv(eb, eb, prv->p);
		bn_mxp(eb, eb, prv->dp, prv->p);
		bn_mod_monty_back(eb, eb, prv->p);
#else
		bn_mxp(eb, eb, prv->dp, prv->p);
#endif

		/* m2 = c^dQ mod q. */
#if BN_MOD == MONTY
		bn_mod_monty_conv(m, m, prv->q);
		bn_mxp(m, m, prv->dq, prv->q);
		bn_mod_monty_back(m, m, prv->q);
#else
		bn_mxp(m, m, prv->dq, prv->q);
#endif

		/* m1 = m1 - m2 mod p. */
		bn_sub(eb, eb, m);
		while (bn_sign(eb) == BN_NEG) {
			bn_add(eb, eb, prv->p);
		}
		bn_mod_basic(eb, eb, prv->p);
		/* m1 = qInv(m1 - m2) mod p. */
		bn_mul(eb, eb, prv->qi);
		bn_mod_basic(eb, eb, prv->p);
		/* m = m2 + m1 * q. */
		bn_mul(eb, eb, prv->q);
		bn_add(eb, eb, m);
		bn_mod_basic(eb, eb, prv->n);

#if CP_RSAPD == PKCS1
		{
			unsigned char pad;

			/* EB = 00 | 02 | PS | 00 | D. */
			size--;
			bn_rsh(m, eb, 8 * size);

			if (!bn_is_zero(m)) {
				result = STS_ERR;
			} else {
				size--;
				bn_rsh(m, eb, 8 * size);
				bn_mod_2b(m, m, 8);
				pad = (unsigned char)m->dp[0];
				if (pad == RSA_TYPE_PUB) {
					size--;
					bn_rsh(m, eb, 8 * size);
					bn_mod_2b(m, m, 8);
					pad = (unsigned char)m->dp[0];
					while (pad != 0) {
						size--;
						bn_rsh(m, eb, 8 * size);
						bn_mod_2b(m, m, 8);
						pad = (unsigned char)m->dp[0];
					}
				} else {
					result = STS_ERR;
				}
			}
		}
#endif

		if (result == STS_OK) {
			bn_mod_2b(m, eb, size * 8);

			bn_size_bin(&size, m);

			if (size <= *out_len) {

				bn_write_bin(out, &size, &sign, m);
				*out_len = size;
			} else {
				result = STS_ERR;
			}
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(m);
		bn_free(eb);
	}

	if (sign == BN_NEG) {
		return STS_ERR;
	}

	return result;
}

#endif

#if CP_RSA == BASIC || !defined(STRIP)

int cp_rsa_sign_basic(unsigned char *sig, int *sig_len, unsigned char *msg,
		int msg_len, rsa_prv_t * prv) {
	bn_t m = NULL, eb = NULL;
	int sign = BN_POS, size, result = STS_OK;
	unsigned char hash[HASH_LEN];

	bn_size_bin(&size, prv->n);

	TRY {
		bn_new(m);
		bn_new(eb);

		bn_zero(m);
		bn_zero(eb);

#if CP_RSAPD == PKCS1
		{
			/* EB = 00 | 01 | PS | 00 | D. */
			bn_add_dig(eb, eb, 0);
			bn_lsh(eb, eb, 8);
			bn_add_dig(eb, eb, RSA_TYPE_PRV);

			for (int i = 0; i < size - 3 - HASH_LEN; i++) {
				bn_lsh(eb, eb, 8);
				bn_add_dig(eb, eb, RSA_PAD_PRV);
			}
			bn_lsh(eb, eb, 8);
			bn_add_dig(eb, eb, 0);
			bn_lsh(eb, eb, HASH_LEN * 8);
		}
#endif
		hf_map(hash, msg, msg_len);
		bn_read_bin(m, hash, HASH_LEN, BN_POS);
		bn_add(eb, eb, m);

#if BN_MOD == MONTY
		bn_mod_monty_conv(eb, eb, prv->n);
		bn_mxp(eb, eb, prv->d, prv->n);
		bn_mod_monty_back(eb, eb, prv->n);
#else
		bn_mxp(eb, eb, prv->d, prv->n);
#endif

		bn_size_bin(&size, prv->n);

		if (size <= *sig_len) {
			*sig_len = size;
			memset(sig, 0, *sig_len);
			bn_write_bin(sig, &size, &sign, eb);
		} else {
			result = STS_ERR;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(m);
	}

	if (sign == BN_NEG) {
		return STS_ERR;
	}
	return result;
}

#endif

#if CP_RSA == QUICK || !defined(STRIP)

int cp_rsa_sign_quick(unsigned char *sig, int *sig_len, unsigned char *msg,
		int msg_len, rsa_prv_t * prv) {
	bn_t m = NULL, eb = NULL;
	int sign = BN_POS, size, result = STS_OK;
	unsigned char hash[HASH_LEN];

	bn_size_bin(&size, prv->n);

	TRY {
		bn_new(m);
		bn_new(eb);

		bn_zero(m);
		bn_zero(eb);

#if CP_RSAPD == PKCS1
		{
			/* EB = 00 | 01 | PS | 00 | D. */
			bn_add_dig(eb, eb, 0);
			bn_lsh(eb, eb, 8);
			bn_add_dig(eb, eb, RSA_TYPE_PRV);

			for (int i = 0; i < size - 3 - HASH_LEN; i++) {
				bn_lsh(eb, eb, 8);
				bn_add_dig(eb, eb, 0xFF);
			}
			bn_lsh(eb, eb, 8);
			bn_add_dig(eb, eb, 0);
			bn_lsh(eb, eb, HASH_LEN * 8);
		}
#endif
		hf_map(hash, msg, msg_len);
		bn_read_bin(m, hash, HASH_LEN, BN_POS);
		bn_add(eb, eb, m);

		bn_copy(m, eb);

		/* m1 = c^dP mod p. */
#if BN_MOD == MONTY
		bn_mod_monty_conv(eb, eb, prv->p);
		bn_mxp(eb, eb, prv->dp, prv->p);
		bn_mod_monty_back(eb, eb, prv->p);
#else
		bn_mxp(eb, eb, prv->dp, prv->p);
#endif

		/* m2 = c^dQ mod q. */
#if BN_MOD == MONTY
		bn_mod_monty_conv(m, m, prv->q);
		bn_mxp(m, m, prv->dq, prv->q);
		bn_mod_monty_back(m, m, prv->q);
#else
		bn_mxp(m, m, prv->dq, prv->q);
#endif

		/* m1 = m1 - m2 mod p. */
		bn_sub(eb, eb, m);
		while (bn_sign(eb) == BN_NEG) {
			bn_add(eb, eb, prv->p);
		}
		bn_mod_basic(eb, eb, prv->p);
		/* m1 = qInv(m1 - m2) mod p. */
		bn_mul(eb, eb, prv->qi);
		bn_mod_basic(eb, eb, prv->p);
		/* m = m2 + m1 * q. */
		bn_mul(eb, eb, prv->q);
		bn_add(eb, eb, m);
		bn_mod_basic(eb, eb, prv->n);

		bn_size_bin(&size, prv->n);

		if (size <= *sig_len) {
			*sig_len = size;
			memset(sig, 0, *sig_len);
			bn_write_bin(sig, &size, &sign, eb);
		} else {
			result = STS_ERR;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(m);
	}

	if (sign == BN_NEG) {
		return STS_ERR;
	}
	return result;
}

#endif

int cp_rsa_ver(unsigned char *sig, int sig_len, unsigned char *msg,
		int msg_len, rsa_pub_t * pub) {
	bn_t m = NULL, eb = NULL;
	int sign, size, result = 1;
	unsigned char hash1[20], hash2[20];

	bn_size_bin(&size, pub->n);

	TRY {
		bn_new(m);
		bn_new(eb);

		bn_read_bin(eb, sig, sig_len, BN_POS);

#if BN_MOD == MONTY
		bn_mod_monty_conv(eb, eb, pub->n);
		bn_mxp(eb, eb, pub->e, pub->n);
		bn_mod_monty_back(eb, eb, pub->n);
#else
		bn_mxp(eb, eb, pub->e, pub->n);
#endif

#if CP_RSAPD == PKCS1
		{
			unsigned char pad;
			/* EB = 00 | 01 | PS | 00 | D. */
			size--;
			bn_rsh(m, eb, 8 * size);
			if (!bn_is_zero(m)) {
				result = STS_ERR;
			} else {
				size--;
				bn_rsh(m, eb, 8 * size);
				bn_mod_2b(m, m, 8);
				pad = (unsigned char)m->dp[0];
				if (pad == RSA_TYPE_PRV) {
					size--;
					bn_rsh(m, eb, 8 * size);
					bn_mod_2b(m, m, 8);
					pad = (unsigned char)m->dp[0];
					while (pad != 0) {
						size--;
						bn_rsh(m, eb, 8 * size);
						bn_mod_2b(m, m, 8);
						pad = (unsigned char)m->dp[0];
					}
				} else {
					result = 0;
				}
			}
		}
#endif

		if (result == 1) {
			bn_mod_2b(m, eb, size * 8);
			bn_size_bin(&size, m);

			if (size <= 20) {
				bn_write_bin(hash1, &size, &sign, m);

				if (sign == BN_POS) {
					hf_map(hash2, msg, msg_len);
					if (memcmp(hash1, hash2, HASH_LEN) == 0) {
						result = 1;
					} else {
						result = 0;
					}
				} else {
					result = 0;
				}
			} else {
				result = 0;
			}
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(m);
		bn_free(eb);
	}

	return result;
}