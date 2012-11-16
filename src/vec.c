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

#include <ctype.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include "vec.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define FORMAT_STR_1 "%.16g"
#define FORMAT_STR_2 "%18.16g"

int default_error_handler(int error_type, const char *error_message) {
	if (error_type != 0) {
		fprintf(stderr, "Vector Stream: Error (%d): %s\n", error_type, error_message);
		exit(error_type);
		return error_type;
	}
	else {
		fprintf(stderr, "Vector Stream: Warning: %s\n", error_message);
		return 0;
	}
}

static vec_error_handler_t vec_error_handler = default_error_handler;

static int skip_whitespace(FILE *fin) {
	int c;

	do {
		if (feof(fin))
			return 1;
		c = fgetc(fin);
	}
	while (isspace(c));
	ungetc(c, fin);
	return 0;
}

static int skip_comment(FILE *fin) {
	int c;

	skip_whitespace(fin);
	c = fgetc(fin);
	if (c == '%') {
		/* skip to EOL */
		do {
			c = fgetc(fin);
		}
		while (c != '\n');
		/* check the next line (recursive call) */
		return skip_comment(fin);
	}
	else {
		ungetc(c, fin);
		return skip_whitespace(fin);
	}
}

static char *get_token(FILE *fin) {
	/* Warning: this function returns MUTABLE array. */
	static int buff_length = 128;
	static char *buff = 0;
	int i = 0;
	int c;

	if (!buff) {
	buff = (char *)calloc(buff_length, sizeof(char));
	}
	skip_whitespace(fin);
	while (!feof(fin) && !isspace(c = fgetc(fin))) {
		if (i > buff_length - 2) {
			char *tmp_buff;

			tmp_buff = (char *)calloc(buff_length * 2, sizeof(char));
			memcpy(tmp_buff, buff, buff_length * sizeof(char));
			free(buff);
			buff_length *= 2;
			buff = tmp_buff;
		}
		buff[i] = c;
		++i;
	}
	buff[i] = 0;
	return &buff[0];
}

vec_error_handler_t vec_set_error_handler(vec_error_handler_t new_error_handler) {
	vec_error_handler_t current_error_handler = vec_error_handler;
	vec_error_handler = new_error_handler;
	return current_error_handler;
}

int vec_put_header_to_file(FILE *fout) {
	if (fout) {
		fputs("%!VCTR\n"
			"%-format=1.1\n"
			"%%Created with " PACKAGE_STRING "\n",
			fout);
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_header_to_file: fout == NULL");
		return 1;
	}
}

int vec_put_hint_to_file(const char *hint, int parameter, FILE *fout) {
	if (fout) {
		fprintf(fout, "%%*%s=%d\n", hint, parameter); 
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_hint_to_file: fout == NULL");
		return 1;
	}
}

int vec_put_message_to_file(const char *comment, FILE *fout) {
	if (fout && comment) {
		char *comment_to_output;
		char *c;

		comment_to_output = strdup(comment);
		c = comment_to_output;
		while (*c != '\0') {
			if (*c == '\n') {
				*c = ' ';
			}
			++c;
		}
		fprintf(fout, "%%?%s\n", comment_to_output);
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_message_to_file: fout == NULL");
		return 1;
	}
}

int vec_scan_messages_from_file_and_put_to_file(FILE *fin, FILE *fout) {
	if (fin && fout) {
		while (!feof(fin)) {
			char buff[VEC_MAXIMUM_LINE_LENGTH];
			fgets(&buff[0], VEC_MAXIMUM_LINE_LENGTH, fin);
			if (buff[0] == '%' && buff[1] == '?') {
				fputs(&buff[0], fout);
			}
		}
		return 0;
	}
	else {
		vec_error_handler(1, "vec_scan_messages_from_file_and_put_to_file: fin == fout == NULL");
		return 1;
	}
}

int vec_put_nil_to_file(FILE *fout) {
	if (fout) {
		fputs("nil\n\n", fout);
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_nil_to_file: fout == NULL");
		return 1;
	}
}

int vec_put_float_vector_to_file(size_t n, const float *v, size_t s, FILE *fout) {
	if (fout) {
		fprintf(fout, "%d %% Number of elements\n", n);
		if (n > 0 && v != NULL) {
			if (s > 1) {
				size_t i, j;
				for (i = 0; i < n; i += s) {
					for (j = 0; j < s; ++j) {
						if (i + j < n) {
							fprintf(fout, FORMAT_STR_2 " ", (double)v[i + j]);
						}
					}
					fputc('\n', fout);
				}
			}
			else {
				size_t i;
				for (i = 0; i < n; ++i) {
					fprintf(fout, FORMAT_STR_1 "\n", (double)v[i]);
				}
			}
		}
		else if (n == 0) {
			fputs("0\n", fout);
		}
		else {
			vec_put_nil_to_file(fout);
		}
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_float_vector_to_file: fout == NULL");
		return 1;
	}
}

int
vec_new_float_vector_from_file(size_t *n, float **v, FILE *fin) {
	if (fin) {
		size_t i;
		char *t;
		int c;

		c = fgetc(fin);
		if (c == 'V') {
			vec_error_handler(0, "falls to binary mode");
			ungetc(c, fin);
			return vec_new_float_vector_from_file_binary(n, v, fin);
		}
		else {
			ungetc(c, fin);
			skip_comment(fin);
			t = get_token(fin);
			if (strcmp(t, "nil") != 0) {
				*n = atoi(t);
				if (*n > 0) {
					*v = (float *)calloc(*n, sizeof(float));
					for (i = 0; i < *n; ++i) {
						skip_comment(fin);
						(*v)[i] = (float)atof(get_token(fin));
					}
				}
				else {
					*v = NULL;
				}
			}
			else {
				*n = -1;
				*v = NULL;
			}
			return 0;
		}
	}
	else {
		vec_error_handler(1, "vec_new_float_vector_from_file: fin == NULL");
		return 1;
	}
}

void vec_delete_float_vector(float *v) {
	if (v) {
		free(v);
	}
}

int vec_put_float_vector_to_file_binary(size_t n, const float *v, FILE *fout) {
	if (fout && v && n > 0) {
		fwrite("VCTR****", sizeof(char), 8, fout);
		fwrite(&n, sizeof(size_t), 1, fout);
		fwrite(v, sizeof(float), n, fout);
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_float_vector_to_file_binary: fout == NULL");
		return 1;
	}
}

int vec_new_float_vector_from_file_binary(size_t *n, float **v, FILE *fin) {
	if (fin) {
		char buff[16];
		
		fread(&buff[0], sizeof(char), 8, fin);
		if (strncmp(&buff[0], "VCTR****", 4) == 0) {
			fread(&buff[0], sizeof(size_t), 1, fin);
			*n = *(size_t *)&buff[0];
			*v = (float *)calloc(*n, sizeof(float));
			fread(*v, sizeof(float), *n, fin);
 			return 0;
		}
		else {
			*n = 0;
			*v = NULL;
			vec_error_handler(1, "vec_new_float_vector_from_file_binary: bad magic");
			return 1;
		}
	}
	else {
		vec_error_handler(1, "vec_new_float_vector_from_file_binary: fin == NULL");
		return 1;
	}
}

int vec_put_double_vector_to_file(size_t n, const double *v, size_t s, FILE *fout) {
	if (fout) {
		fprintf(fout, "%d %% Number of elements\n", n);
		if (n > 0 && v != NULL) {
			if (s > 1) {
				size_t i, j;
				for (i = 0; i < n; i += s) {
					for (j = 0; j < s; ++j) {
						if (i + j < n) {
							fprintf(fout, FORMAT_STR_2 " ", v[i + j]);
						}
					}
					fputc('\n', fout);
				}
			}
			else {
				size_t i;
				
				for (i = 0; i < n; ++i) {
					fprintf(fout, FORMAT_STR_1 "\n", v[i]);
				}
			}
 		}
		else if (n == 0) {
 			fputs("0\n", fout);
		}
		else {
			vec_put_nil_to_file(fout);
		}
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_double_vector_to_file: fout == NULL");
 		return 1;
	}
}

int vec_new_double_vector_from_file(size_t *n, double **v, FILE *fin) {
	if (fin) {
		size_t i;
		char *t;
		int c;

		c = fgetc(fin);
		if (c == 'V') {
			vec_error_handler(0, "falls to binary mode");
			ungetc(c, fin);
			return vec_new_double_vector_from_file_binary(n, v, fin);
		}
		else {
			ungetc(c, fin);
			skip_comment(fin);
			t = get_token(fin);
			if (strcmp(t, "nil") != 0) {
				*n = atoi(t);
				if (*n > 0) {
					*v = (double *)calloc(*n, sizeof(double));
					for (i = 0; i < *n; ++i) {
						skip_comment(fin);
						(*v)[i] = atof(get_token(fin));
					}
				}
				else {
					*v = NULL;
				}
			}
			else {
				*n = -1;
				*v = NULL;
			}
			return 0;
		}
	}
	else {
		vec_error_handler(1, "vec_new_double_vector_from_file: fin == NULL");
		return 1;
	}
}

void vec_delete_double_vector(double *v) {
	if (v) {
		free(v);
	}
}

int vec_put_double_vector_to_file_binary(size_t n, const double *v, FILE *fout) {
	if (fout && v && n > 0) {
		fwrite("VCTR****", sizeof(char), 8, fout);
		fwrite(&n, sizeof(size_t), 1, fout);
		fwrite(v, sizeof(double), n, fout);
		return 0;
	}
	else {
		vec_error_handler(1, "vec_put_double_vector_to_file_binary: fout == NULL");
		return 1;
	}
}

int vec_new_double_vector_from_file_binary(size_t *n, double **v, FILE *fin) {
	if (fin) {
		char buff[16];

		fread(&buff[0], sizeof(char), 8, fin);
		if (strncmp(&buff[0], "VCTR****", 4) == 0) {
			fread(&buff[0], sizeof(size_t), 1, fin);
			*n = *(size_t *)&buff[0];
			*v = (double *)calloc(*n, sizeof(double));
			fread(*v, sizeof(double), *n, fin);
			return 0;
		}
		else {
			*n = 0;
			*v = NULL;
			vec_error_handler(1, "vec_new_double_vector_from_file_binary: bad magic");
			return 1;
		}
	}
	else {
		vec_error_handler(1, "vec_new_double_vector_from_file_binary: fin == NULL");
		return 1;
	}
}

int vec_slice_double_vector(double *a, const double *v, size_t offset, size_t length, size_t stride) {
	if (a && v && length > 0 && stride > 0) {
		size_t i;

		for (i = 0; i < length; ++i) {
			size_t j = offset + i * stride;
			a[i] = v[j];
		}
		return 0;
	}
	else {
		if (!a) {
			vec_error_handler(1, "vec_slice_double_vector: destination null");
		}
		if (!v) {
			vec_error_handler(1, "vec_slice_double_vector: source null");
		}
		if (length == 0) {
			vec_error_handler(1, "vec_slice_double_vector: length is 0");
		}
		if (stride == 0) {
			vec_error_handler(1, "vec_slice_double_vector: stride is 0");
		}
		return 1;
	}
}

int vec_add_double_multi_vector_to_multi_vector(double *a, size_t s, size_t n1, const double *v1, size_t n2, const double *v2) {
	if (a && s > 0 && n1 > 0 && v1 && n2 > 0 && v2) {
		size_t n = (n1 < n2) ? n1 : n2;
		size_t i;

		for (i = 0; i < n; ++i) {
		a[i] = v1[i] + v2[i];
	}
	return 0;
 	}
	else {
		vec_error_handler(1, "vec_add_double_multi_vector_to_multi_vector: bad parameters");
		return 1;
	}
}

int vec_add_double_single_vector_to_multi_vector(double *a, size_t s, size_t n1, const double *v1, size_t n2, const double *v2) {
	if (a && s > 0 && n1 >= s && v1 && n2 / s >= n1 / s && v2) {
		size_t i, j;

		for (j = 0; j < n2 / s + 1; ++j) {
			for (i = 0; i < s; ++i) {
				size_t k = j * s + i;
				if (k < n2) {
					a[k] = v1[i] + v2[k];
				}
			}
		}
		return 0;
	}
	else {
		vec_error_handler(1, "vec_add_double_single_vector_to_multi_vector: bad parameters");
		return 1;
	}
}

int vec_multiply_double_multi_matrix_to_multi_vector(double *a, size_t s, size_t nm, const double *m, size_t nv, const double *v, int transpose) {
	if (a && s > 0 && nm / (s * s) >= nv / s && m && nv / s > 0 && v) {
		size_t i, j, k;

		for (k = 0; k < nv; ++k) {
			a[k] = 0;
		}
		for (k = 0; k < nv / s; ++k) {
			for (j = 0; j < s; ++j) {
				for (i = 0; i < s; ++i) {
					if (transpose == 0) {
						a[k * s + j] += m[k * s * s + i * s + j] * v[k * s + i];
					}
					else {
						a[k * s + j] += m[k * s * s + j * s + i] * v[k * s + i];
					}
				}
			}
		}
		return 0;
	}
	else {
		vec_error_handler(1, "vec_multiply_double_multi_matrix_to_multi_vector: bad parameters");
		return 1;
	}
}

int vec_multiply_double_single_matrix_to_multi_vector(double *a, size_t s, size_t nm, const double *m, size_t nv, const double *v, int transpose) {
	if (a && s > 0 && nm >= s * s && m && nv / s >= nm / (s * s) && v) {
		size_t i, j, k;

		for (k = 0; k < nv; ++k) {
			a[k] = 0;
		}
		for (k = 0; k < nv / s; ++k) {	
			for (j = 0; j < s; ++j) {
				for (i = 0; i < s; ++i) {
					if (transpose == 0) {
						a[k * s + j] += m[i * s + j] * v[k * s + i];
					}
					else {
						a[k * s + j] += m[j * s + i] * v[k * s + i];
					}
				}
			}
		}
		return 0;
	}
	else {
		vec_error_handler(1, "vec_multiply_double_single_matrix_to_multi_vector: bad parameters");
		return 1;
	}
}
