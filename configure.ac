AC_PREREQ(2.60)
AC_INIT([insensitive], [0.9.34], [klaus.boldt@uni-rostock.de])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9.6 -Wall -Werror])

AC_ARG_ENABLE(
  [webkit],
  [AS_HELP_STRING([--disable-webkit], [Disable support for Webkit2Gtk. Use this
                                       option on Windows or macOS systems for
                                       which WebKit2Gtk has not been ported.])],
  [enable_webkit=$enableval],
  [enable_webkit=yes]
)
case "$target_os" in
    cygwin* | mingw* | msys*)
		[enable_webkit=no]
		[platform=windows]
		;;
    darwin*)
    	[enable_webkit=no]
		[platform=macos]
		;;
    *)
		[platform=posix]
    	;;
esac
AM_CONDITIONAL([WINDOWS], [test x$platform = xwindows])
AM_CONDITIONAL([APPLE], [test x$platform = xmacos])

AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources, no)
if test x$GLIB_COMPILE_RESOURCES = xno; then
	AC_MSG_ERROR(Could not find a glib-compile-resources in your PATH)
fi

AX_CHECK_COMPILE_FLAG([-std=gnu11], [
  CFLAGS+=" -std=gnu11"
], [
  echo "C compiler cannot compile Gnu-C11 code"
  exit -1
])

AC_CHECK_HEADERS([stdio.h stdlib.h string.h math.h float.h complex.h])

AC_FUNC_MALLOC

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

GETTEXT_PACKAGE=insensitive
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [insensitive])

ACX_PTHREAD
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"

AC_CHECK_LIB([m],[cos])

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0)
PKG_CHECK_MODULES(GIO, gio-2.0 >= 2.50)
PKG_CHECK_MODULES(GTK3, gtk+-3.0 >= 3.22)
PKG_CHECK_MODULES(CAIRO, cairo)
PKG_CHECK_MODULES(PANGO, pango)
if test x$enable_webkit != xno ; then
    PKG_CHECK_MODULES(WEBKIT2GTK, webkit2gtk-4.0, enable_webkit="yes", enable_webkit="no")
fi
AM_CONDITIONAL(USE_WEBKIT_GTK, test x$enable_webkit = xyes)
if test "x$enable_webkit" == xyes ; then
    AC_DEFINE(USE_WEBKIT_GTK, [], [Build Insensitive with Webkit2Gtk support.])
fi
if test x$platform = xmacos ; then
    PKG_CHECK_MODULES(GTKMACINTEGRATION, gtk-mac-integration-gtk3)
fi
AX_BLAS()
if test "$ax_blas_ok" = "no"; then
    AC_MSG_ERROR([
  ------------------------------------------
   A BLAS library and header file required to build insensitive. Stopping...
   Check 'config.log' for more information.
  ------------------------------------------])
fi
AX_LAPACK()
if test "$ax_lapack_ok" = "no"; then
    AC_MSG_ERROR([
  ------------------------------------------
   A LAPACK library and header file required to build insensitive. Stopping...
   Check 'config.log' for more information.
  ------------------------------------------])
fi
PKG_CHECK_MODULES(FFTW3, fftw3)
PKG_CHECK_MODULES(XML2, libxml-2.0)

AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strstr])
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

AC_SUBST([LIBINTL])
AC_SUBST([top_builddir])
AC_SUBST([POMAKEFILEDEPS])
AC_SUBST([XGETTEXT_EXTRA_OPTIONS])

AC_PATH_PROG([XDG_MIME],[xdg-mime])
AC_SUBST(XDG_MIME)
AC_PATH_PROG([XDG_ICON_RESOURCE],[xdg-icon-resource])
AC_SUBST(XDG_ICON_RESOURCE)
AC_PATH_PROG([UPDATE_DESKTOP_DATABASE],[update-desktop-database])
AC_SUBST(UPDATE_DESKTOP_DATABASE)
AC_PATH_PROG([UPDATE_MIME_DATABASE],[update-mime-database])
AC_SUBST(UPDATE_MIME_DATABASE)

CFLAGS="$CFLAGS $BLAS_CFLAGS"
LIBS="$LAPACK_LIBS $BLAS_LIBS $FLIBS $LIBS"

AC_CONFIG_HEADERS([insensitive-config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	po/Makefile.in
	po/Makefile
	data/Makefile
	data/doc/Makefile
	data/icons/Makefile
	data/icons/hicolor/16x16/mimetypes/Makefile
	data/icons/hicolor/32x32/mimetypes/Makefile
	data/icons/hicolor/128x128/mimetypes/Makefile
	data/icons/hicolor/256x256/mimetypes/Makefile
	data/icons/hicolor/16x16/actions/Makefile
	data/icons/hicolor/24x24/actions/Makefile
	data/icons/hicolor/32x32/actions/Makefile
	data/icons/hicolor/48x48/actions/Makefile
	data/icons/hicolor/16x16/apps/Makefile
	data/icons/hicolor/24x24/apps/Makefile
	data/icons/hicolor/32x32/apps/Makefile
	data/icons/hicolor/64x64/apps/Makefile
	data/icons/hicolor/128x128/apps/Makefile
	data/icons/hicolor/256x256/apps/Makefile
	data/icons/hicolor/512x512/apps/Makefile
	examples/Makefile
])
AC_OUTPUT
