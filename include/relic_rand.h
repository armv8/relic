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
 * @defgroup rand Pseudo-random number generator.
 */

/**
 * @file
 *
 * Interface of the pseudo-random number generator.
 *
 * @version $Id: relic_rand.h 3 2009-04-08 01:05:51Z dfaranha $
 * @ingroup rand
 */

#ifndef RELIC_RAND_H
#define RELIC_RAND_H

/**
 * Initializes the pseudo-random number generator.
 */
void rand_init(void);

/**
 * Finishes the pseudo-random number generator.
 */
void rand_clean(void);

/**
 * Sets the initial state of the pseudo-random number generator.
 * 
 * @param[in] buf			- the buffer that represents the initial state.
 * @param[in] size			- the number of bytes.
 */
void rand_seed(unsigned char *buf, int size);

/**
 * Writes size pseudo-random bytes in the passed buffer.
 * 
 * @param[out] buf			- the buffer to write.
 * @param[in] size			- the number of bytes to write.
 * @throw ERR_NO_READ		- it the pseudo-random number generator can't
 * 							generate the specified number of bytes.
 */
void rand_bytes(unsigned char *buf, int size);

#endif /* !RELIC_RAND_H */