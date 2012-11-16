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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "vec.h"

static bool stop_parsing_options = false;

static bool binary_input = false;
static bool binary_output = false;

static size_t offset = 0;
static size_t length = 0;
static size_t stride = 1;

void help() {
  std::cerr << "usage: slice [-o{OFFSET}] [-l{LENGTH}] [-s{STRIDE}] [-b]\n"
    "\t[-B]\n {FILENAME}\n"
    "\tslice slices vectorstream file {FILENAME} (or stdin if {FILENAME}\n"
    "\twas -) with offset OFFSET, length LENGTH, and stride STRIDE.\n"
    "\t-o{OFFSET}: Sets offset. {OFFSET} must be equal to or greater than 0.\n"
    "\t(Default: 0).\n"
    "\t-l{LENGTH}: Sets length. {LENGTH} must be equal to or greater than 0.\n"
    "\tIf 0 is given, the length is automatically calculated. (Default: 0).\n"
    "\t-s{STRIDE}: Sets stride. {STRIDE} must be greater than 0.\n"
    "\t(Default: 1)\n"
    "\t-b: Binary input.\n"
    "\t-B: DO NOT USE. Binary output. Machine dependent.\n"
    "\t-: stdin.\n";
}

void process_vector(size_t N, double *v) {
  if (length == 0) {
    length = N / stride;
  }
  double *vs = new double[length];
  pid::vec_slice_double_vector(vs, v, offset, length, stride);
  if (!binary_output) {
    pid::vec_put_header_to_file(stdout);
    pid::vec_put_hint_to_file("dimension", stride, stdout);
    pid::vec_put_double_vector_to_file(length, vs, 0, stdout);
    std::fflush(stdout);
  }
  else {
    pid::vec_put_double_vector_to_file_binary(length, vs, stdout);
    std::fflush(stdout);
  }
  delete[] vs;
}

void process_file(FILE *fin) {
  size_t N;
  double *v;
	
  if (!binary_input) {
    pid::vec_new_double_vector_from_file(&N, &v, fin);
  }
  else {
    pid::vec_new_double_vector_from_file_binary(&N, &v, fin);
  }
  process_vector(N, v);
  std::free(v);
}

void process(const char *filename) {
  FILE *fin;
  fin = std::fopen(filename, "r");
  if (!fin) {
    std::cerr << "slice: error: can't open: " << filename << '\n';
    std::exit(1);
  }
  process_file(fin);
  pid::vec_scan_messages_from_file_and_put_to_file(fin, stdout);
  std::fclose(fin);
}

void parse_option(const char *option) {
  switch (*option) {
  case '\0':
    process_file(stdin);
    break;
  case '-':
    stop_parsing_options = true;
    break;
  case 'o':
    offset = std::atoi(option + 1);
    break;
  case 'l':
    length = std::atoi(option + 1);
    break;
  case 's':
    stride = std::atoi(option + 1);
    break;
  case 'b':
    binary_input = true;
    break;
  case 'B':
    binary_output = true;
    break;
  case 'h':
    help();
    std::exit(0);
    break;
  default:
    std::cerr << "add: warning: ignoring option: " << option << '\n';
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
