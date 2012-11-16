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

static bool multiply_all = false;
static bool transpose = false;

static bool stop_parsing_options = false;
static bool verbose = false;
static size_t size_of_vector = 3;

void help() {
  std::cerr << "usage: multiply [-s{SIZE_OF_VECTOR}] [-v] [-a] [-t] [-b]\n"
    "\t[-B] [--] {FILENAME1} {FILENAME2}\n";
}

void process_vectors(size_t n1, double *v1, size_t n2, double *v2) {
  double *v = new double[n2];

  if (multiply_all) {
    pid::vec_multiply_double_single_matrix_to_multi_vector(v, size_of_vector,
							   n1, v1, n2, v2,
							   static_cast<int>(transpose));
  }
  else {
    pid::vec_multiply_double_multi_matrix_to_multi_vector(v, size_of_vector,
							  n1, v1, n2, v2,
							  static_cast<int>(transpose));
  }
		
  if (!binary_output) {
    pid::vec_put_header_to_file(stdout);
    pid::vec_put_hint_to_file("dimension", size_of_vector, stdout);
    pid::vec_put_double_vector_to_file(n2, v, size_of_vector, stdout);
  }
  else {
    pid::vec_put_double_vector_to_file_binary(n2, v, stdout);
  }
  std::fflush(stdout);
  
  delete[] v;
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
      std::cerr << "multiply: error: can't open: " << filename2 << '\n';
      std::exit(1);
    }
    process_files(stdin, fin2);
    std::fclose(fin2);
  }
  else if (filename2[0] == '-' && filename2[1] == '\0') {
    FILE *fin1;
    fin1 = std::fopen(filename1, "r");
    if (!fin1) {
      std::cerr << "multiply: error: can't open: " << filename1 << '\n';
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
      std::cerr << "multiply: error: can't open: " << filename1 << '\n';
      std::exit(1);
    }
    if (!fin2) {
      std::cerr << "multiply: error: can't open: " << filename2 << '\n';
      std::exit(1);
    }
    process_files(fin1, fin2);
    std::fclose(fin1);
    std::fclose(fin2);
  }
}

static void parse_option(const char *option) {
  switch (*option) {
  case '-':
    stop_parsing_options = true;
    break;
  case 's':
    size_of_vector = std::atoi(option + 1);
    if (size_of_vector < 2) {
      std::cerr << "multiply: error: size of vector must be greater than 1.\n";
      std::exit(1);
    }
    break;
  case 'v':
    verbose = true;
    break;
  case 'a':
    multiply_all = true;
    break;
  case 't':
    transpose = true;
    break;
  case 'b':
    binary_input = true;
    break;
  case 'B':
    binary_output = true;
    break;
  case 'h':
    help();
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
	std::cerr << "multiply: warning: ignoring filename: " << *argv << '\n';
      }
    }
  }
  process(filenames[0], filenames[1]);
  return 0;
}
