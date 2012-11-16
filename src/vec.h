/*
 *  VectorStream 1.7
 *  Vector streaming library.
 *  Copyright (C) 2002-2008 Ichiroh Kanaya
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __VEC_H
#define __VEC_H

#include <stddef.h>
#include <stdio.h>

#define VEC_MAXIMUM_LINE_LENGTH 1024

#ifdef __cplusplus
extern "C" {
	namespace pid {
#endif

	/* Error handler */
	typedef int (*vec_error_handler_t)(int error_type, const char *error_message);
	extern vec_error_handler_t vec_set_error_handler(vec_error_handler_t new_error_handler);

	/* Writing header to output stream */
	extern int vec_put_header_to_file(FILE *fout);

	/* Writing hint to output stream */
	extern int vec_put_hint_to_file(const char *hint, int parameter, FILE *fout);

	/* Writing general message (comment) to output stream */
	/* WARNING: '\n' in the comment will be replaced with ' '. */
	extern int vec_put_message_to_file(const char *message, FILE *fout);

	extern int vec_scan_messages_from_file_and_put_to_file(FILE *fin, FILE *fout);

	/* Writing nil */
	extern int vec_put_nil_to_file(FILE *fout);

	/* Writing and reading float vector */
	extern int vec_put_float_vector_to_file(size_t n, const float *v, size_t s, FILE *fout);
	extern int vec_new_float_vector_from_file(size_t *n, float **v, FILE *fin);

	extern void vec_delete_float_vector(float *v);

	/* DO NOT USE UNTIL YOU UNDERSTAND WHAT THESE ARE. */
	extern int vec_put_float_vector_to_file_binary(size_t n, const float *v, FILE *fout);
	extern int vec_new_float_vector_from_file_binary(size_t *n, float **v, FILE *fin);
	
	/* Writing and reading double vector */
	extern int vec_put_double_vector_to_file(size_t n, const double *v, size_t s, FILE *fout);
	extern int vec_new_double_vector_from_file(size_t *n, double **v, FILE *fin);

	extern void vec_delete_double_vector(double *v);

	/* DO NOT USE UNTIL YOU UNDERSTAND WHAT THESE ARE. */
	extern int vec_put_double_vector_to_file_binary(size_t n, const double *v, FILE *fout);
	extern int vec_new_double_vector_from_file_binary(size_t *n, double **v, FILE *fin);
	
	/* Slicing */
	extern int vec_slice_double_vector(double *a, const double *v, size_t offset, size_t length, size_t stride);
	
	/* Adding */
	extern int vec_add_double_multi_vector_to_multi_vector(double *a, size_t s, size_t n1, const double *v1, size_t n2, const double *v2);
	extern int vec_add_double_single_vector_to_multi_vector(double *a, size_t s, size_t n1, const double *v1, size_t n2, const double *v2);

	/* Multiplying */
	extern int vec_multiply_double_multi_matrix_to_multi_vector(double *a, size_t s, size_t nm, const double *m, size_t nv, const double *v, int transpose);

	extern int vec_multiply_double_single_matrix_to_multi_vector(double *a, size_t s, size_t nm, const double *m, size_t nv, const double *v, int transpose);
  
#ifdef __cplusplus
	}
}
#endif

#endif
