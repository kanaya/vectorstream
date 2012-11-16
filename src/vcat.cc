/*
 *  VectorStream 1.8
 *  Vector streaming library.
 *  Copyright (C) 2002-2010 Ichiroh Kanaya
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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "vec.h"

static bool stop_parsing_options = false;

static bool verbose = false;
static bool unvectorize = false;
static bool unvectorize_with_scheme_format = false;
static bool binary_input = false;
static bool binary_float_input = false;
static bool binary_output = false;
static bool binary_float_output = false;
static size_t stride = 1;
static const char *name = 0;

void help() {
  std::cerr << "usage: vcat [-v] [-u|-U] [-s{STRIDE}] [-b[s]] [-B[s]] [--] {FILENAME}\n"
    "\tvcat reads vector file {FILENAME} and writes it to stdout.\n"
    "\t-v: Verbose mode.\n"
    "\t-u: Unvectorize.\n"
    "\t-U: Unvectorize with Scheme format.\n"
    "\t-s{STRIDE}: Specifies stride. {STRIDE} must be equal to or greater\n"
    "\tthan 0.\n"
    "\t-b: Binary input.\n"
    "\t-bs: Binary input (single precision).\n"
    "\t-B: DO NOT USE. Binary output. Machine dependent.\n"
    "\t-Bs: DO NOT USE. Binary output (single precision). Machine dependent.\n"
    "\n"
    "\t*You can merge more than two vector files by:\n"
    "\t\tvcat -u {FILE1} {FILE2} | vectorize - > {OUTPUT}\n";
}

void process_vector(size_t N, double *v) {
  if (unvectorize || unvectorize_with_scheme_format) {
    if (stride < 2) {
      if (unvectorize_with_scheme_format) {
	std::fprintf(stdout, "#( ; %d nodes\n", N);
      }
      for (size_t i = 0; i < N; ++i) {
	std::fprintf(stdout, "%f\n", static_cast<double>(v[i]));
      }
      if (unvectorize_with_scheme_format) {
	std::fputs(")\n", stdout);
      }
    }
    else {
      if (unvectorize_with_scheme_format) {
	std::fprintf(stdout, "#( ; %d nodes\n", N / stride);
      }
      for (size_t i = 0; i < N / stride; ++i) {
	if (unvectorize_with_scheme_format) {
	  std::fputs("#(", stdout);
	}
	for (size_t j = 0; j < stride; ++j) {
	  int k = i * stride + j;
	  std::fprintf(stdout, "%16f ", static_cast<double>(v[k]));
	}
	if (unvectorize_with_scheme_format) {
	  std::fputs(")\n", stdout);
	}
	else {
	  std::fputc('\n', stdout);
	}
      }
      if (unvectorize_with_scheme_format) {
	std::fputs(")\n", stdout);
      }
    }
  }
  else {
    if (!binary_output) {
      pid::vec_put_header_to_file(stdout);
      pid::vec_put_hint_to_file("dimension", stride, stdout);
      pid::vec_put_double_vector_to_file(N, v, stride, stdout);
    }
    else {
      if (!binary_float_output) {
	pid::vec_put_double_vector_to_file_binary(N, v, stdout);
      }
      else {
	float *fv = new float[N];
	for (size_t i = 0; i < N; ++i) {
	  fv[i] = v[i];
	}
	pid::vec_put_float_vector_to_file_binary(N, fv, stdout);
	delete[] fv;
      }
    }
  }
}

void process_file(FILE *fin) {
  size_t N;
  double *v;

  if (!binary_input) {
    pid::vec_new_double_vector_from_file(&N, &v, fin);
  }
  else {
    if (!binary_float_input) {
      pid::vec_new_double_vector_from_file_binary(&N, &v, fin);
    }
    else {
      float *fv;
      
      pid::vec_new_float_vector_from_file(&N, &fv, fin);
      v = static_cast<double *>(std::malloc(N * sizeof(double)));
      for (size_t i = 0; i < N; ++i) {
	v[i] = fv[i];
      }
      pid::vec_delete_float_vector(fv);
    }
  }
  process_vector(N, v);
  std::free(v);
}

void process(const char *filename) {
  if (verbose) {
    std::cerr << "processing " << filename << "... ";
  }
  FILE *fin;
  fin = std::fopen(filename, "r");
  if (!fin) {
    std::cerr << "vcat: error: can't open: " << filename << '\n';
    std::exit(1);
  }
  process_file(fin);
  if (!(unvectorize || unvectorize_with_scheme_format) && !binary_output) {
    std::rewind(fin);
    pid::vec_scan_messages_from_file_and_put_to_file(fin, stdout);
    std::fprintf(stdout, "%%? This file was created from %s\n", filename);
  }
  std::fflush(stdout);
  std::fclose(fin);
  if (verbose) {
    std::cerr << "done\n";
  }
}

void parse_option(const char *option) {
  switch (*option) {
  case '\0':
    process_file(stdin);
    break;
  case '-':
    stop_parsing_options = true;
    break;
  case 'v':
    verbose = true;
    break;
  case 'u':
    unvectorize = true;
    break;
  case 'U':
    unvectorize_with_scheme_format = true;
    break;
  case 's':
    stride = std::atoi(option + 1);
    break;
  case 'b':
    binary_input = true;
    if (*++option == 's') {
      binary_float_input = true;
    }
    break;
  case 'B':
    binary_output = true;
    if (*++option == 's') {
      binary_float_output = true;
    }
    break;
  case 'h':
    help();
    std::exit(0);
    break;
  default:
    std::cerr << "vcat: warning: ignoring option: " << option << '\n';
    break;
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    help();
    std::exit(0);
  }
  int file_count = 0;
  while (--argc) {
    ++argv;
    if (!stop_parsing_options && **argv == '-') {
      parse_option(*argv + 1);
    }
    else {
      ++file_count;
      process(*argv);
    }
  }
  if (file_count == 0) {
    process_file(stdin);
  }
  return 0;
}
