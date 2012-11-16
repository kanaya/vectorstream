#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "vec.h"

void help() {
  std::cerr << "usage: splice {FILENAME1} {FILENAME2}\n"
    "\tsplice reads vectorstream files FILENAME1 and FILENAME2 and write\n"
    "\tspliced array.\n";
}

void process_files(FILE *fin1, FILE *fin2) {
  size_t N1, N2;
  double *v1, *v2;
  pid::vec_new_double_vector_from_file(&N1, &v1, fin1);
  pid::vec_new_double_vector_from_file(&N2, &v2, fin2);
	
  size_t N = std::min(N1, N2);
  if (N1 != N2) {
    std::cerr << "splice: warning: vector size was adjusted to: " << N << '\n';
  }
	
  double *v = new double[N * 2];
  for (int i = 0; i < N; ++i) {
    v[i * 2 + 0] = v1[i];
    v[i * 2 + 1] = v2[i];
  }
	
  pid::vec_put_header_to_file(stdout);
  pid::vec_put_double_vector_to_file(N * 2, v, 2, stdout);
  std::fflush(stdout);
	
  delete[] v;
  std::free(v1);
  std::free(v2);
}

void process(const char *filename1, const char *filename2) {
  FILE *fin1, *fin2;
  fin1 = std::fopen(filename1, "r");
  fin2 = std::fopen(filename2, "r");
  if (!fin1) {
    std::cerr << "splice: error: can't open:: " << filename1 << '\n';
    std::exit(1);
  }
  if (!fin2) {
    std::cerr << "splice: error: can't open:: " << filename2 << '\n';
    std::exit(1);
  }
  process_files(fin1, fin2);
  std::fclose(fin1);
  std::fclose(fin2);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    help();
    std::exit(0);
  }
  else {
    if (argc > 3) {
      std::cerr << "splice: warning: more than 3 parameters are given.\n";
    }
    process(argv[1], argv[2]);
  }
  return 0;
}

  
