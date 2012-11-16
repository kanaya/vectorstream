#include <iostream>
#include <fstream>
#include <valarray>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "vec.h"

static bool binary_input = false;
static bool binary_output = false;

static bool stop_parsing_options = false;

static size_t offset = 0;
static std::valarray<size_t> *lengths = 0;
static std::valarray<size_t> *strides = 0;

void help() {
  std::cerr << "usage: gslice [-o{OFFSET}] [-L{LENGTHS}] [-S{STRIDES}] [-b]\n"
    "\t[-B] [--] {FILENAME}\n"
    "\tslice slices vectorstream file {FILENAME} with offset {OFFSET},\n"
    "\tlength {LENGTH}, and stride {STRIDE}.\n"
    "\t-o{OFFSET}: Specifies offset. {OFFSET} must be equal to or greater\n"
    "\tthan 0. (Default: 0).\n"
    "\t-L{LENGTHS}: Specifies lengths. {LENGTHS} must be separated by ':',\n"
    "\te.g. -L5:4. Each length must be equal to or greater than 0.\n"
    "\t-S{STRIDES}: Specifies strides. {STRIDES} must be separated by ':',\n"
    "\te.g. -S2:3:5. Each stride must be greater than 0.\n"
    "\t-b: Binary input.\n"
    "\t-B: DO NOT USE. Binary output. Machine dependent.\n";
}

void process_vector(size_t N, double *v) {
  if ((*lengths)[0] == 0) {
    (*lengths)[0] = N / (*strides)[0];
  }

  std::valarray<double> va(v, N);
  std::valarray<double> sliced(va[std::gslice(offset, *lengths, *strides)]);

  double *vf = static_cast<double *>(calloc(sliced.size(), sizeof(double)));
  for (size_t i = 0; i < sliced.size(); ++i) {
    vf[i] = sliced[i];
  }
  if (!binary_output) {
    pid::vec_put_header_to_file(stdout);
    pid::vec_put_double_vector_to_file(sliced.size(), vf, 0, stdout);
  }
  else {
    pid::vec_put_double_vector_to_file_binary(sliced.size(), vf, stdout);
  }
  std::fflush(stdout);
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
  pid::vec_scan_messages_from_file_and_put_to_file(fin, stdout);
  std::free(v);
}

void process(const char *filename) {
  FILE *fin;
  fin = std::fopen(filename, "r");
  if (!fin) {
    std::cerr << "gslice: error: can't open: " << filename << '\n';
    std::exit(1);
  }
  process_file(fin);
  std::fclose(fin);
}

void parse_numbers(std::valarray<size_t> **v, char *s) {
  std::vector<size_t> v0;
  char *e = s;
  while (*e++);
  while (s != e) {
    char *p = s;
    while (*p != ':' && *p != '\0') ++p;
    *p = '\0';
    v0.push_back(std::atoi(s));
    s = p + 1;
  }
  
  *v = new std::valarray<size_t>(v0.size());
  for (size_t i = 0; i < v0.size(); ++i) {
    (**v)[i] = v0[i];
  }
}

void parse_option(char *option) {
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
  case 'L':
    parse_numbers(&lengths, option + 1);
    break;
  case 'S':
    parse_numbers(&strides, option + 1);
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
      if (lengths && strides) {
	++file_count;
	process(*argv);
      }
      else {
	help();
	std::exit(0);
      }
    }
  }
  if (file_count == 0 && lengths && strides) {
    process_file(stdin);
  }
  delete lengths;
  delete strides;
  return 0;
}

  
