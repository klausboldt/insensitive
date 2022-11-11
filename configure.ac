AC_PREREQ(2.60)
AC_INIT([insensitive], [0.9.34], [klaus.boldt@uni-konstanz.de])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.9.6 -Wall -Werror])

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
PKG_CHECK_MODULES(WEBKIT2GTK, webkit2gtk-4.0)
#PKG_CHECK_MODULES(BLAS, blas, [have_blas=yes], AX_BLAS([have_blas=yes], [have_blas=no]))
#AC_CHECK_LIB([blas],[cblas_caxpy], [HAVE_CBLAS=1], [HAVE_CBLAS=0])
#AS_IF([test "$HAVE_CBLAS" -eq 0], [PKG_CHECK_MODULES(CBLAS, cblas)])
#PKG_CHECK_MODULES(LAPACK, lapack, [have_lapack=yes], AX_LAPACK([have_lapack=yes], [have_lapack=no]))
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
