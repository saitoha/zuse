#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.68)
LT_PREREQ([2.2.6b])
AC_INIT(zuse, 0.1.4, saitoha@gmail.com)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(foreign)
#AM_INIT_AUTOMAKE
AT_INIT
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_TESTDIR(tests)
AC_CONFIG_MACRO_DIR([m4])


# libtool support
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

AM_CONDITIONAL(BUILD_EXTRA_MODULES, false)

# disable default conmpiler option "-g -O2"
AC_SUBST(CXXFLAGS)
if test "" = "$CXXFLAGS"; then
	CXXFLAGS=" "
fi

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_MKDIR_P

# Checks for libraries.
AC_SUBST(HAVE_READLINE)
AC_CHECK_HEADER(readline/readline.h,
  AC_CHECK_LIB([readline], [readline],
               [AC_DEFINE(HAVE_READLINE, [], [readline]) 
                AM_CONDITIONAL(HAVE_LIBREADLINE, true)], 
               [AC_MSG_WARN([*** READLINE not found ! ***])
                AM_CONDITIONAL(HAVE_LIBREADLINE, false)]))
AC_CHECK_LIB(SDL, SDL_Init, 
             [AM_CONDITIONAL(HAVE_LIBSDL, true)], 
             [AC_MSG_WARN([*** SDL not found ! ***])
              AM_CONDITIONAL(HAVE_LIBSDL, false)])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
#AC_CHECK_FUNC(isfinite, AC_DEFINE(HAVE_ISFINITE, [], []), [Define if isfinite is there])
#AC_CHECK_FUNC(isnan, AC_DEFINE(HAVE_ISNAN, [], []), [Define if isnan is there])
#AC_CHECK_FUNC(isinf, AC_DEFINE(HAVE_ISNAN, [], []), [Define if isinf is there])
AC_CHECK_FUNCS([floor getcwd pow realpath sqrt strdup strstr isfinite isnan isinf])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile
                 tests/atlocal])
AC_OUTPUT
