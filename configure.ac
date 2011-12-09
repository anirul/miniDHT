#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_boost_base.m4])
m4_include([m4/ax_lib_sqlite3.m4])
m4_include([m4/ax_boost_asio.m4])
m4_include([m4/ax_boost_date_time.m4])
m4_include([m4/ax_boost_filesystem.m4])
m4_include([m4/ax_boost_program_options.m4])
m4_include([m4/ax_boost_serialization.m4])
m4_include([m4/ax_boost_system.m4])
m4_include([m4/ax_boost_thread.m4])
m4_include([m4/ax_check_openssl.m4])
m4_include(m4/wxwin.m4)

AC_PREREQ([2.68])

AC_INIT([BitSmear],[1.0],[frederic.dubouchet@gmail.com])
AM_INIT_AUTOMAKE(BitSmear, 1.0)
AC_CONFIG_SRCDIR([miniDHT_db.h])
AC_CONFIG_HEADERS([config.h])
#AC_LANG([C++])
#AC_SUBST(OSTYPE)

# Checks for programs.
AC_PROG_CXX([clang++ g++])
AC_PROG_CC([clang gcc]) 
#AC_PROG_CPP([clang++ -E])
AC_PROG_CPP([clang++ cpp])
AM_CONDITIONAL(MACOSX, test `uname -s` = "Darwin")

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Checks for libraries.
PKG_CHECK_MODULES([protobuf], [protobuf])
PROTO_CPPFLAGS=`pkg-config --cflags protobuf`
PROTO_LIBS=`pkg-config --libs protobuf`
PROTOC=protoc
AC_SUBST(PROTOC)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
AX_CHECK_OPENSSL([], [AC_MSG_ERROR(You need the OpenSSL development library.)])
AX_LIB_SQLITE3([3.7.0])
AX_BOOST_BASE([1.45.0])
AX_BOOST_ASIO
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS
#AX_BOOST_SERIALIZATION
AX_BOOST_SYSTEM
AX_BOOST_THREAD

BOOST_LIB_DIR=`echo "$BOOST_LDFLAGS"|sed 's/@<:@ @:>@*-L\(@<:@^ @:>@*\).*/\1/'`
AC_SUBST(BOOST_LIB_DIR)
#AM_SUBST_NOTMAKE(BOOST_LIB_DIR)
#TODO : change it!
#PORT_LIB_DIR=/opt/local/lib
AC_SUBST(PORT_LIB_DIR, [/opt/local/lib])
#AM_SUBST_NOTMAKE(PORT_LIB_DIR)

#AC_CHECK_HEADERS([SDL/SDL.h], [], [AC_MSG_ERROR(You need the SDL development library.)])
#AC_CHECK_HEADERS([wx/wx.h], [], [AC_MSG_ERROR(You need the WxWidgets development library.)])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strtol strtoul])

AM_OPTIONS_WXCONFIG
reqwx=2.9.0
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1
then
	AC_MSG_ERROR([
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
else
AC_DEFINE([HAVE_WX], [], [Have the WxWidgets library])
fi
AM_CONDITIONAL(USE_WX, test "$wxWin" = 1)
CPPFLAGS="$CPPFLAGS $SQLITE3_CFLAGS $OPENSSL_INCLUDES $BOOST_CPPFLAGS $WX_CPPFLAGS $PROTO_CPPFLAGS" 
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $SQLITE3_LDFLAGS $OPENSSL_LDFLAGS $OPENSSL_LIBS $BOOST_LDFLAGS $BOOST_ASIO_LIB $BOOST_DATE_TIME_LIB $BOOST_FILESYSTEM_LIB $BOOST_PROGRAM_OPTIONS_LIB $BOOST_SYSTEM_LIB $BOOST_THREAD_LIB $WX_LIBS $PROTO_LIBS"
#LIBS+=$BOOST_SERIALIZATION_LIB
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

