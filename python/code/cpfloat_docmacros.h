/* SPDX-FileCopyrightText: 2020 Massimiliano Fasi and Mantas Mikaitis */
/* SPDX-License-Identifier: LGPL-2.1-or-later                         */

#ifndef _CPFLOAT_DOCMACROS_
#define _CPFLOAT_DOCMACROS_

#define doc_cpfloat_validate_optstruct(FPTYPE, PMIN, PMAX, EMAX) \
/** \
 @brief Validate fields of @ref optstruct struct for `FPTYPE` storage format. \
 \
 @details This function checks whether the parameters stored in @p fpopts are \
 valid when `FPTYPE` is used as storage format. \
 \
 @param[in] fpopts Parameters describing the target format, the rounding mode, \
 and the probability of soft errors striking the rounded values. \
 \
 @return The function returns @b 0 if all the parameters are valid, and a \
 positive number if at least one of them is not. A negative number should be \
 understood as a warning, and indicates that a CPFloat function @em will \
 return @p 0 if @p fpopts is used as fourth argument, but might not perform as \
 intended.<p/>\
 \
 Possible return values are: \
 \li @b -4 The rounding mode specified in @p fpopts->round does not correspond \
 to a valid choice, thus no rounding will be performed. \
 \li @b -2 The required number of digits in @p fpopts->precision is between \
 PMIN and PMAX inclusive, which might cause double rounding if round-to-\
 nearest is used. \
 \li @b -1 The string in @p fpopts->format is not valid. This is not an error \
 as this value is not used by the C functions, but only by the MEX interface. \
 \li @b  0 All the parameters in @p fpopts are valid. \
 \li @b  2 The required number of digits in @p fpopts->precision is larger \
 than PMAX, the number of significant digits in a variable of type `FPTYPE`. \
 \li @b  3 The required maximum exponent in @p fpopts->emax is larger than \
 EMAX, the largest possible exponent for a variable of type `FPTYPE`. \
 \li @b  5 The value of @p fpopts->flip indicates that soft errors should be \
 introduced, but @p fpopts->p is not a real number between 0 and 1 and thus \
 does not represent a valid probability.<p/>\
 \
 Errors take precedence over warnings, thus a nonpositive return value \
 implies no errors. In case of multiple issues, the return value is that of \
 the first error (or warning, if no error is present) encountered in the order \
 given in the list above. \
 */

#define doc_cpfloat(FPTYPE, PMAX, EMAX) \
/** \
 @brief Round `FPTYPE` array to lower precision. \
 \
 @details If the function executes without errors, then the array @p X \
 contains the @p numelem entries of the array @p A rounded to a \
 lower-precision target format. The parameters of the target format and the \
 rounding mode to be used are encoded in @p fpopts. If required, the function \
 flips one bit in some of the entries of @p X.<p/>\
 \
 If OpenMP support is specified at compile time, several OpenMP threads are \
 used if @p numelem is large enough. This parameter is machine-dependent. <p/>\
 \
 @param[out] X Array of rounded values. \
 @param[in] A Input array. \
 @param[in] numelem Number of elements in @p X and @p A. \
 @param[in] fpopts Parameters describing the target format, the rounding mode, \
 and the probability of soft errors striking the rounded values. \
 \
 @return The function returns @b 1 if @p fpopts->precision is larger than \
 PMAX, @b 2 if @p fpopts->emax is larger than EMAX, and @b 0 otherwise. \
 */

#endif  /* #ifndef _CPFLOAT_DOCMACROS_ */

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
