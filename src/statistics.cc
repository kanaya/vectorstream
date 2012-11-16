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
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <valarray>
#include "vec.h"

static bool stop_parsing_options = false;

static bool binary_input = false;

static size_t size_of_vector = 1;

void help() {
  std::cerr << "usage: statistics [-s{STRIDE}] [-b] {FILENAME}\n"
    "\t-b: Binary input.\n"
    "\t-: stdin.\n";
}

void process_vector(size_t N, double *v) {
  std::valarray<double> va(v, N);
	
  for (size_t mu = 0; mu < size_of_vector; ++mu) {
    std::valarray<double> v_sliced(va[std::slice(mu, N / size_of_vector,
						 size_of_vector)]);
    size_t num = v_sliced.size();
    double max = v_sliced.max();
    double min = v_sliced.min();
    double dif = max - min;
    double sum = v_sliced.sum();
    double avr = sum / num;
    v_sliced -= avr;
    for (size_t i = 0; i < v_sliced.size(); ++i) {
      v_sliced[i] = v_sliced[i] * v_sliced[i];
    }
    double var;
    if (num > 1) {
      var = std::sqrt(v_sliced.sum() / (num - 1));
    }
    else {
      var = 0;
    }
    std::cout << "num: " << num << "; max: " << max << "; min: " << min
	      << "; dif: " << dif << "; sum: " << sum << "; avr: " << avr 
	      << "; var: " << var << '\n';
  }
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
    std::cerr << "statistics: warning: can't open: " << filename << '\n';
  }
  else {
    std::cout << filename << '\n';
    process_file(fin);
  }
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
  case 's':
    size_of_vector = std::atoi(option + 1);
    break;
  case 'b':
    binary_input = true;
    break;
  case 'h':
    help();
    std::exit(0);
    break;
  default:
    std::cerr << "statistics: warning: ignoring option: " << option << '\n';
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
