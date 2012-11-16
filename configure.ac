AC_PREREQ(2.62)
AC_INIT([vectorstream], [1.8.0], [kanaya@users.sourceforge.net])
AM_INIT_AUTOMAKE([1.10])
AC_CONFIG_SRCDIR([src/vec.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
LT_PREREQ([2.2.4])
LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([memory.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile
	src/Makefile])
AC_OUTPUT
