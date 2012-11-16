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
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "vec.h"

static bool binary_output = false;

static bool stop_parsing_options = false;
static size_t stride = 0;

void help() {
	std::cerr << "usage: vectorize [-s{STRIDE}] [-B] [--] {FILENAME}\n"
		"\tvectorize reads text file {FILENAME} containing numerical array and\n"
		"\twrites the array in vectorstream format to stdout.\n" 
		"\t-s{STRIDE}: Specifies stride. {STRIDE} must be equal to or greater\n"
		"\tthan 0.\n"
		"\t-B: DO NOT USE. Binary output. Machine dependent.\n"
		"\t-: stdin.\n";
}

void process_istream(std::istream &is) {
	std::vector<double> v;
	while (is) {
		double x;
		is >> x;
		v.push_back(x);
	}
	v.pop_back();

	double *vf = new double[v.size()];
	double *i = vf;
	std::vector<double>::const_iterator j = v.begin();
	while (j != v.end()) {
		*i = *j;
		++i;
		++j;
	}

	if (!binary_output) {
		pid::vec_put_header_to_file(stdout);
		pid::vec_put_hint_to_file("dimension", stride, stdout);
		pid::vec_put_double_vector_to_file(v.size(), vf, stride, stdout);
	}
	else {
		pid::vec_put_double_vector_to_file_binary(v.size(), vf, stdout);
	}
	delete[] vf;
}

void process(const char *filename) {
	std::ifstream is(filename);
	if (is.is_open()) {
		process_istream(is);
		std::fflush(stdout);
	}
	else {
		std::cerr << "vectorize: error: can't open: " << filename << '\n';
		std::exit(1);
	}
}

void parse_option(const char *option) {
  switch (*option) {
  case '\0':
    process_istream(std::cin);
    break;
  case '-':
    stop_parsing_options = true;
    break;
  case 's':
    stride = std::atoi(option + 1);
    break;
  case 'B':
    binary_output = true;
    break;
  case 'h':
    help();
    std::exit(0);
    break;
  default:
    std::cerr << "vectorize: warning: ignoring option: " << option << '\n';
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
    process_istream(std::cin);
  }
  return 0;
}

  
