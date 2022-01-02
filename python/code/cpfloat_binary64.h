/* SPDX-FileCopyrightText: 2020 Massimiliano Fasi and Mantas Mikaitis */
/* SPDX-License-Identifier: LGPL-2.1-or-later                         */

/**
 * @file cpfloat_binary64.h
 * @brief CPFloat functions for `double` arrays.
 */

#ifndef _CPFLOAT_BINARY64_
#define _CPFLOAT_BINARY64_

#include "cpfloat_definitions.h"
#include "cpfloat_docmacros.h"

/* Validation of floating-point parameters. */
doc_cpfloat_validate_optstruct(double, 26, 53, 1023)
static inline int cpfloat_validate_optstruct(optstruct *fpopts);

/* Rounding functions. */
doc_cpfloat(double, 53, 1023)
static inline int cpfloat(double *X, double *A, size_t numelem,
                          optstruct *fpopts);
doc_cpfloat(double, 53, 1023)
static inline int cpf_fpround(double *X, double *A,
                                  size_t numelem, optstruct *fpopts);

/** @cond */
#define FUNSUFFIX
#define FPTYPE double
#define INTTYPE uint64_t
#define INTSUFFIX ULL
#define DEFPREC 53
#define DEFEMAX 1023
#define DEFEMIN -1022
#define NLEADBITS 12
#define NBITS 64
#define FULLMASK 0xFFFFFFFFFFFFFFFFULL
#define ABSMASK 0x7FFFFFFFFFFFFFFFULL
#define SIGNMASK 0x8000000000000000ULL
#define EXPMASK 0x7FF0000000000000ULL
#define FRACMASK 0x000FFFFFFFFFFFFFULL

#ifdef PCG_VARIANTS_H_INCLUDED
#define INITRAND(seed) pcg64_srandom_r(seed, time(NULL), (intptr_t)seed);
#define ADVANCERAND(seed, thread, nloc) pcg64_advance_r(seed, thread *nloc - 1);
#define GENRAND(seed) pcg64_random_r(seed)
#else /* #ifdef PCG_VARIANTS_H_INCLUDED */
//#warning "The default C random number generator is being used."
//#warning "Please compile with -include <path-to-pcg_variants.h>"
//#warning "and link with -L <path-to-libpcg_random.a> -lpcg_random."
#define INITRAND(seed) *seed = time(NULL);
#define GEN_SINGLE_RAND(seed)                                                  \
  ((INTTYPE)rand()) +                                     \
   ((INTTYPE)rand() << 31)
#endif /* #ifdef PCG_VARIANTS_H_INCLUDED */

//#include "cpfloat_threshold_binary64.h"
#include "cpfloat_template.h"
/** @endcond */

#endif  /* #ifndef _CPFLOAT_BINARY64_ */

/*
 * CPFloat - Custom Precision Floating-point numbers.
 *
 * Copyright 2020 Massimiliano Fasi and Mantas Mikaitis
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
