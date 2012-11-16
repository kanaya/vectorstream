bin_PROGRAMS = vcat vectorize slice gslice add multiply statistics splice
# lib_LIBRARIES = libvec.a
lib_LTLIBRARIES = libvec.la
include_HEADERS = vec.h vec++.hh

vectorize_SOURCES = vectorize.cc
vectorize_LDFLAGS = libvec.la

slice_SOURCES = slice.cc
slice_LDFLAGS = libvec.la

gslice_SOURCES = gslice.cc
gslice_LDFLAGS = libvec.la

vcat_SOURCES = vcat.cc
vcat_LDFLAGS = libvec.la

add_SOURCES = add.cc
add_LDFLAGS = libvec.la

multiply_SOURCES = multiply.cc
multiply_LDFLAGS = libvec.la

statistics_SOURCES = statistics.cc
statistics_LDFLAGS = libvec.la -lm

splice_SOURCES = splice.cc
splice_LDFLAGS = libvec.la

# libvec_a_SOURCES = vec.c vec.h
libvec_la_SOURCES = vec.c vec.h
libvec_la_LDFLAGS = -version-info 1:7:0

