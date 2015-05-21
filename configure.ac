#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(ssimCheck, 0.3, thijs.boonstra@npo.nl)
AC_CONFIG_SRCDIR(src/ssimCheck.cpp)
AM_INIT_AUTOMAKE
AC_OUTPUT(Makefile src/Makefile)
AC_CONFIG_SRCDIR([src/ssimCheck.cpp])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([sqrt strdup])

AC_OUTPUT
