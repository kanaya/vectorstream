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

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "vec.h"

static bool binary_input = false;
static bool binary_output = false;

static bool stop_parsing_options = false;
static bool verbose = false;
static bool add_all = false;
static bool negative = false;

void help() {
  std::cerr << "usage: add [-v] [-a] [-n] [-b] [-B] {FILENAME1} {FILENAME2}\n"
    "\tadd reads vectorstream files {FILENAME1} and {FILENAME2} and add each\n"
    "\telements of the arrays.\n";
}

void process_vectors(size_t N1, double *v1, size_t N2, double *v2) {
  size_t N, n;
  double *v_out = 0;
	
  if (negative) {
    for (size_t i = 0; i < N1; ++i) {
      v1[i] = -v1[i];
    }
  }
  if (add_all) {
    double *V, *v;
    if (N1 < N2) {
      n = N1;
      N = N2;
      v = v1;
      V = v2;
    }
    else {
      n = N2;
      N = N1;
      v = v2;
      V = v1;
    }
    v_out = new double[N];
    pid::vec_add_double_single_vector_to_multi_vector(v_out, n, n, v, N, V);
    if (!binary_output) {
      pid::vec_put_header_to_file(stdout);
      pid::vec_put_double_vector_to_file(N, v_out, n, stdout);
    }
    else {
      pid::vec_put_double_vector_to_file_binary(N, v_out, stdout);
    }
  }
  else {
    if (N1 < N2) {
      n = N1;
      N = N2;
    }
    else {
      n = N2;
      N = N1;
    }
    v_out = new double[n];
    pid::vec_add_double_multi_vector_to_multi_vector(v_out, n, n, v1, n, v2);
    if (!binary_output) {
      pid::vec_put_header_to_file(stdout);
      pid::vec_put_double_vector_to_file(n, v_out, 0, stdout);
    }
    else {
      pid::vec_put_double_vector_to_file_binary(n, v_out, stdout);
    }
  }
  
  std::fflush(stdout);
  delete[] v_out;
}

void process_files(FILE *fin1, FILE *fin2) {
  size_t N1, N2;
  double *v1, *v2;
  if (!binary_input) {
    pid::vec_new_double_vector_from_file(&N1, &v1, fin1);
    pid::vec_new_double_vector_from_file(&N2, &v2, fin2);
  }
  else {
    pid::vec_new_double_vector_from_file_binary(&N1, &v1, fin1);
    pid::vec_new_double_vector_from_file_binary(&N2, &v2, fin2);
  }
  process_vectors(N1, v1, N2, v2);
  pid::vec_scan_messages_from_file_and_put_to_file(fin1, stdout);
  pid::vec_scan_messages_from_file_and_put_to_file(fin2, stdout);
  std::free(v1);
  std::free(v2);
}

void process(const char *filename1, const char *filename2) {
  if (filename1[0] == '-' && filename1[1] == '\0') {
    FILE *fin2;
    fin2 = std::fopen(filename2, "r");
    if (!fin2) {
      std::cerr << "add: error: can't open: " << filename2 << '\n';
      std::exit(1);
    }
    process_files(stdin, fin2);
    std::fclose(fin2);
  }
  else if (filename2[0] == '-' && filename2[1] == '\0') {
    FILE *fin1;
    fin1 = std::fopen(filename1, "r");
    if (!fin1) {
      std::cerr << "add: error: can't open: " << filename1 << '\n';
      std::exit(1);
    }
    process_files(fin1, stdin);
    std::fclose(fin1);
  }
  else {
    FILE *fin1, *fin2;
    fin1 = std::fopen(filename1, "r");
    fin2 = std::fopen(filename2, "r");
    if (!fin1) {
      std::cerr << "add: error: can't open: " << filename1 << '\n';
      std::exit(1);
    }
    if (!fin2) {
      std::cerr << "add: error: can't open: " << filename2 << '\n';
      std::exit(1);
    }
    process_files(fin1, fin2);
    std::fclose(fin1);
    std::fclose(fin2);
  }
}

void parse_option(const char *option) {
  switch (*option) {
  case '-':
    stop_parsing_options = true;
    break;
  case 'a':
    add_all = true;
    break;
  case 'v':
    verbose = true;
    break;
  case 'n':
    negative = true;
    break;
  case 'b':
    binary_input = true;
    break;
  case 'B':
    binary_output = true;
    break;
  default:
    std::cerr << "add: warning: ignoring option: " << option << '\n';
    break;
  }
}

int main(int argc, char **argv) {
  char *filenames[2];
  int fnc = 0;
	
  if (argc < 3) {
    help();
    std::exit(0);
  }
  
  while (--argc) {
    ++argv;
    if (!stop_parsing_options && **argv == '-' && *(*argv + 1) != '\0') {
      parse_option(*argv + 1);
    }
    else {
      if (fnc < 2) {
	filenames[fnc] = *argv;
	++fnc;
      }
      else {
	std::cerr << "add: warning: ignoring filename: " << *argv << '\n';
      }
    }
  }
  process(filenames[0], filenames[1]);
  return 0;
}

  
