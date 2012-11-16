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

#ifndef __VECXX_H
#define __VECXX_H

#include <vec.h>

#ifdef __cplusplus

#include <cstdlib>
#include <map>
#include <string>
#include <stdexcept>
#include <valarray>
#include <vector>

// C++ users can use the short-cut versions of the vector stream functions.

namespace pid {
  
  inline int put_header(FILE *fout) {
    return pid::vec_put_header_to_file(fout);
  }

  inline int put_hint(const char *hint, int parameter, FILE *fout) {
    return pid::vec_put_hint_to_file(hint, parameter, fout);
  }

  inline int put_message(const char *message, FILE *fout) {
    return pid::vec_put_message_to_file(message, fout);
  }

  inline int scan_messages_and_put(FILE *fin, FILE *fout) {
    return pid::vec_scan_messages_from_file_and_put_to_file(fin, fout);
  }

  inline int put_nil(FILE *fout) {
    return pid::vec_put_nil_to_file(fout);
  }

  inline int put_vector(size_t n, const float *v, size_t s, FILE *fout) {
    return pid::vec_put_float_vector_to_file(n, v, s, fout);
  }

  inline int put_vector(size_t n, const double *v, size_t s, FILE *fout) {
    return pid::vec_put_double_vector_to_file(n, v, s, fout);
  }

  inline int new_vector(size_t *n, float **v, FILE *fin) {
    return pid::vec_new_float_vector_from_file(n, v, fin);
  }

  inline int new_vector(size_t *n, double **v, FILE *fin) {
    return pid::vec_new_double_vector_from_file(n, v, fin);
  }

  inline void delete_vector(float *v) {
    pid::vec_delete_float_vector(v);
  }

  inline void delete_vector(double *v) {
    pid::vec_delete_double_vector(v);
  }

  inline int slice_vector(double *a, const double *v, size_t offset,
			  size_t length, size_t stride) {
    return pid::vec_slice_double_vector(a, v, offset, length, stride);
  }
	
  inline int add_multi_vector_to_multi_vector(double *a, size_t s, size_t n1,
					      const double *v1, size_t n2,
					      const double *v2) {
    return pid::vec_add_double_multi_vector_to_multi_vector(a, s, n1, v1, n2,
							    v2);
  }

  inline int add_single_vector_to_multi_vector(double *a, size_t s, size_t n1,
					       const double *v1, size_t n2,
					       const double *v2) {
    return pid::vec_add_double_single_vector_to_multi_vector(a, s, n1, v1, n2,
							     v2);
  }

  inline int multiply_multi_matrix_to_malti_vector(double *a, size_t s,
						   size_t nm, const double *m,
						   size_t nv, const double *v,
						   bool transpose) {
    int t = transpose ? 1 : 0;
    return pid::vec_multiply_double_multi_matrix_to_multi_vector(a, s, nm, m,
								 nv, v, t);
  }

  inline int multiply_single_matrix_to_multi_vector(double *a, size_t s,
						    size_t nm, const double *m,
						    size_t nv, const double *v,
						    int transpose) {
    int t = transpose ? 1 : 0;
    return pid::vec_multiply_double_single_matrix_to_multi_vector(a, s, nm, m,
								  nv, v, t);
  }

  // Utility functions and functors

  class string_to_string {
  public:
    std::string
    operator () (const std::string &s) {
      return std::string(s);
    }
  };

  class string_to_int {
  public:
    int
    operator () (const std::string &s) {
      return std::atoi(s.c_str());
    }
  };

  class string_to_double {
  public:
    double
    operator () (const std::string &s) {
      return std::atof(s.c_str());
    }
  };

  template <typename T, typename C> void
  parse_multiple_parameters(std::valarray<T> **v, C converter, char *s) {
    std::vector<std::string> v0;
    char *e = s;
    while (*e++);
    while (s != e) {
      char *p = s;
      while (*p != ':' && *p != '\0') {
	++p;
      }
      *p = '\0';
      v0.push_back(std::string(s));
      s = p + 1;
    }
    *v = new std::valarray<T>(v0.size());
    for (size_t i = 0; i < v0.size(); ++i) {
      (**v)[i] = converter(v0[i]);
    }
  }

  // vector_loader class

  template <typename real> class vector_loader {
  private:
    std::valarray<real> *_values;
    std::map<std::string, std::string> *_options;
    std::map<std::string, std::string> *_hints;
    std::vector<std::string> *_messages;
    
    void register_parameter(std::map<std::string, std::string> *map,
			    char *line) {
      char *s1 = line;
      char *s2 = line;
      while (*s2 != '\0' && *s2 != '=') {
	++s2;
      }
      if (*s2 == '\0') {
	s2 = "1";
      }
      else {
	*s2 = '\0';
	++s2;
	if (*s2 == '\0') {
	  s2 = "0";
	}
      }
      map->insert(std::pair<std::string, std::string>(std::string(s1),
						      std::string(s2)));
    }

    void process_special_line(char *line) {
      switch (line[0]) {
      case '-':
	register_parameter(_options, line + 1);
	break;
      case '*':
	register_parameter(_hints, line + 1);
	break;
      case '?':
	_messages->push_back(std::string(line + 1));
	break;
      default:
	break;
      }
    }

    void cut_lf(char *line) {
      size_t l = std::strlen(line);
      if (line[l - 1] == '\n') {
	line[l - 1] = '\0';
      }
    }

  public:
    vector_loader(const std::string &filename) throw (std::exception) {
      _values = 0;
      _options = new std::map<std::string, std::string>();
      _hints = new std::map<std::string, std::string>();
      _messages = new std::vector<std::string>();
      FILE *fin = std::fopen(filename.c_str(), "r");
      if (!fin) {
	throw std::invalid_argument(filename);
      }
      // First scan
      char buff[VEC_MAXIMUM_LINE_LENGTH];
      while (!std::feof(fin)) {
	buff[VEC_MAXIMUM_LINE_LENGTH - 1] = '\0';
	fgets(buff, VEC_MAXIMUM_LINE_LENGTH, fin);
	if (buff[VEC_MAXIMUM_LINE_LENGTH - 1] != '\0') {
	  throw std::length_error(filename);
	}
	if (buff[0] == '%') {
	  cut_lf(buff + 1);
	  process_special_line(buff + 1);
	}
      }
      // Second scan
      std::rewind(fin);
      size_t n;
      real *v;
      int result = new_vector(&n, &v, fin);
      std::fclose(fin);
      if (result == 0) {
	_values = new std::valarray<real>(v, n);
	delete_vector(v);
      }
      else {
	// should thorw some exception
      }
    }

    ~vector_loader() {
      delete _values;
      delete _options;
      delete _hints;
      delete _messages;
      _values = 0;
      _options = 0;
      _hints = 0;
      _messages = 0;
    }
    
    std::valarray<real> *values() {
      return _values;
    }
    
    std::map<std::string, std::string> *hints() {
      return _hints;
    }
    
    std::vector<std::string> *messages() {
      return _messages;
    }
  };

}

#endif  
#endif
