dnl ###########
dnl ## SETUP ###########################################################
dnl ###########

AC_INIT([ezstream], [0.6.0],
	[https://trac.xiph.org/newticket?component=ezstream])
AC_REVISION([$Id$])
AC_PREREQ([2.61])
AC_CONFIG_SRCDIR([src/ezstream.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_LIBOBJ_DIR([src])
AM_MAINTAINER_MODE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC_STDC
AC_PROG_FGREP
AC_CANONICAL_HOST

BUILD_DATE=$(date '+%B %d, %Y')
AC_SUBST([BUILD_DATE])

EXAMPLES_DIR="\$(datadir)/examples/${PACKAGE_TARNAME}"
AC_ARG_ENABLE([examplesdir],
	[AS_HELP_STRING([--enable-examplesdir=DIR],
		[examples installation directory (default: DATADIR/examples/ezstream)])],
[case "$enableval" in
	yes) ;;
	no) AC_MSG_ERROR([Must have an install directory for the examples.]) ;;
	*) EXAMPLES_DIR="$enableval" ;;
esac], [])
AC_SUBST([EXAMPLES_DIR])

ez_enable_debug=no
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
		[enable memory debugging (default: no)])],
[case "$enableval" in
	no) ;;
	*) ez_enable_debug=yes ;;
esac], [])
AC_MSG_CHECKING([whether to enable debugging])
if test x"${ez_enable_debug}" = "xyes"; then
	AC_DEFINE([XALLOC_DEBUG], [1],
		[Define whether to build with XALLOC debugging])
fi
AC_MSG_RESULT([$ez_enable_debug])

EZ_CFLAGS=""
EZ_CPPFLAGS=""
EZ_LDFLAGS=""
EZ_LIBS=""


dnl ##############
dnl ## COMPILER ########################################################
dnl ##############

if test -n "${GCC}"; then
	for _flag in fstack-protector fstrict-aliasing Wall W Wstrict-aliasing \
		Wwrite-strings Wpointer-arith Wshadow Wconversion Wcast-qual \
		Wcast-align Wbounded Wmissing-declarations Wmissing-prototypes \
		Wstrict-prototypes
	do
		AX_CHECK_CFLAG([${_flag}],
			[AX_UNIQVAR_APPEND([EZ_CFLAGS], [-${_flag}])], [])
	done
	AX_CHECK_CFLAG([Wformat=2],
		[AX_UNIQVAR_APPEND([EZ_CFLAGS], [-Wformat=2])],
		[
		  AX_CHECK_CFLAG([Wformat],
			[AX_UNIQVAR_APPEND([EZ_CFLAGS], [-Wformat])], [])
		])
else
	case "${host}" in
	*-irix*)
		EZ_CFLAGS="-fullwarn"
		;;
	*-solaris*)
		EZ_CFLAGS="-v"
		;;
	*)
		;;
	esac
fi

AC_C_CONST
AC_C_VOLATILE


dnl #############
dnl ## HEADERS #########################################################
dnl #############

AC_CHECK_HEADERS([ \
	sys/time.h signal.h langinfo.h libgen.h locale.h paths.h \
], [], [],
[
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
])

COMPAT_INCLUDES=""
if test x"${ez_enable_debug}" = "xyes"; then
	AX_HEADER_TREE([RB_FOREACH], [],
		[
		  COMPAT_INCLUDES="-I\$(top_srcdir)/compat"
		])
fi
AC_SUBST([COMPAT_INCLUDES])


dnl ###########
dnl ## TYPES ###########################################################
dnl ###########

AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T


dnl ###############
dnl ## LIBRARIES #######################################################
dnl ###############

AC_CHECK_LIB([gen], [basename],
	[ AX_UNIQVAR_PREPEND([EZ_LIBS], [-lgen]) ])

AX_CHECK_LIBSHOUT([], [],
	[AC_MSG_ERROR([libshout 2.2.x is required.], [1])])
AX_UNIQVAR_APPEND([EZ_CPPFLAGS], [${LIBSHOUT_CPPFLAGS}])
AX_UNIQVAR_APPEND([EZ_CFLAGS], [${LIBSHOUT_CFLAGS}])
AX_UNIQVAR_APPEND([EZ_LDFLAGS], [${LIBSHOUT_LDFLAGS}])
AX_UNIQVAR_PREPEND([EZ_LIBS], [${LIBSHOUT_LIBS}])

AX_CHECK_LIBXML2([], [],
	[AC_MSG_ERROR([libxml 2.x is required.], [1])])
AX_UNIQVAR_APPEND([EZ_CPPFLAGS], [${LIBXML2_CPPFLAGS}])
AX_UNIQVAR_APPEND([EZ_CFLAGS], [${LIBXML2_CFLAGS}])
AX_UNIQVAR_APPEND([EZ_LDFLAGS], [${LIBXML2_LDFLAGS}])
AX_UNIQVAR_PREPEND([EZ_LIBS], [${LIBXML2_LIBS}])

use_metadata="No"
AX_CHECK_TAGLIB_C([], [
	AC_DEFINE([HAVE_TAGLIB], [1],
		[Define to 1 if you have the libtag_c library.]
	)
	AX_UNIQVAR_APPEND([EZ_CFLAGS], [${TAGLIB_CFLAGS}])
	AX_UNIQVAR_APPEND([EZ_CPPFLAGS], [${TAGLIB_CPPFLAGS}])
	AX_UNIQVAR_APPEND([EZ_LDFLAGS], [${TAGLIB_LDFLAGS}])
	AX_UNIQVAR_PREPEND([EZ_LIBS], [${TAGLIB_C_LIBS}])
	use_metadata="TagLib"
], [
	AX_CHECK_LIBVORBISFILE([], [
		AC_DEFINE([HAVE_VORBISFILE], [1],
			[Define to 1 if you have the libvorbisfile library.])
		AX_UNIQVAR_APPEND([EZ_CPPFLAGS], [${VORBIS_CPPFLAGS}])
		AX_UNIQVAR_APPEND([EZ_CFLAGS], [${VORBIS_CFLAGS}])
		AX_UNIQVAR_APPEND([EZ_LDFLAGS], [${VORBIS_LDFLAGS}])
		AX_UNIQVAR_PREPEND([EZ_LIBS], [${LIBVORBISFILE_LIBS}])
		use_metadata="libvorbis"
	], [])
])

# Not using AM_ICONV to check for iconv(), with or without libiconv(),
# is not a viable option. Something in the past went horribly wrong;
# the whole topic is extremely complex and not worth rewriting.
# Instead, work around the buggy no-op that is --with-libiconv-prefix,
# and at least try to find it in --prefix. Thankfully, it is and will
# remain optional in ezstream.

INCICONV=""
LDICONV=""
LIBICONV=""
LTLIBICONV=""
EZ_LIBICONV=""
AM_ICONV
if test x"${am_cv_func_iconv}" != "xyes"; then
	unset am_cv_func_iconv
	unset am_cv_lib_iconv
	if test x"${prefix}" = "xNONE"; then
		libiconv_prefix="/usr/local"
	else
		libiconv_prefix="${prefix}"
	fi
	AC_MSG_NOTICE([Looking for libiconv in ${libiconv_prefix} ...])
	libiconv_save_CPPFLAGS="${CPPFLAGS}"
	libiconv_save_LDFLAGS="${LDFLAGS}"
	libiconv_save_LIBS="${LIBS}"
	CPPFLAGS="${CPPFLAGS} -I${libiconv_prefix}/include"
	LDFLAGS="${LDFLAGS} -L${libiconv_prefix}/lib"
	LIBS="-liconv"
	AM_ICONV
	if test x"${am_cv_func_iconv}" = "xyes"; then
		INCICONV="-I${libiconv_prefix}/include"
		LDICONV="-L${libiconv_prefix}/lib"
		LIBICONV="-liconv"
	fi
	CPPFLAGS="${libiconv_save_CPPFLAGS}"
	LDFLAGS="${libiconv_save_LDFLAGS}"
	LIBS="${libiconv_save_LIBS}"
fi
if test -n "${LTLIBICONV}"; then
	EZ_LIBICONV="${LTLIBICONV}"
else
	EZ_LIBICONV="${LIBICONV}"
fi
AX_UNIQVAR_APPEND([EZ_CPPFLAGS], [$INCICONV])
AX_UNIQVAR_APPEND([EZ_LDFLAGS], [$LDICONV])
AX_UNIQVAR_PREPEND([EZ_LIBS], [$EZ_LIBICONV])
use_iconv="No"
if test x"${am_cv_func_iconv}" = "xyes"; then
	use_iconv="Yes"
fi


dnl #######################
dnl ## FUNCTIONS / MACROS ##############################################
dnl #######################

AC_CHECK_FUNCS([ \
	arc4random \
	basename \
	getopt \
	gettimeofday \
	nl_langinfo \
	pclose \
	_pclose \
	popen \
	_popen \
	random \
	setlocale \
	snprintf \
	_snprintf \
	srandomdev \
	stat \
	_stat \
	strncasecmp \
	strnicmp \
	strtoll \
	_strtoi64 \
])

AC_REPLACE_FUNCS([ \
	strlcat \
	strlcpy \
	strtonum \
])

AX_FUNC_VA_COPY

AC_CHECK_FUNCS([sigaction], [
	AC_DEFINE([HAVE_SIGNALS], [1],
		[Define to 1 if we have BSD signals.])
	], [], [
	  #ifdef HAVE_SIGNAL_H
	  # include <signal.h>
	  #endif
	])


dnl ##################
dnl ## MISCELLANEOUS ###################################################
dnl ##################

AX_CHECK___PROGNAME


dnl ############
dnl ## OUTPUT ##########################################################
dnl ############

AC_SUBST([EZ_CPPFLAGS])
AC_SUBST([EZ_CFLAGS])
AC_SUBST([EZ_LDFLAGS])
AC_SUBST([EZ_LIBS])

AC_CONFIG_FILES([
	Makefile
	build-aux/Makefile
	compat/Makefile
	compat/sys/Makefile
	doc/Makefile
	doc/ezstream-file.sh.1.in
	doc/ezstream.1.in
	examples/Makefile
	m4/Makefile
	src/Makefile
	src/ezstream-file.sh
	win32/Makefile
	win32/shout/Makefile
])

AC_OUTPUT

cat << __EOT

${PACKAGE_STRING} has been configured successfully.

Configuration:
    Ogg Vorbis metadata support ... : ${use_metadata}
    Charset conversion support .... : ${use_iconv}
    Prefix ........................ : ${prefix}

__EOT
