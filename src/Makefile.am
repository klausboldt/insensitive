## Process this file with automake to generate a Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
datadir = @datadir@
datarootdir= @datarootdir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" -D_FILE_OFFSET_BITS=64 @DEFS@

bin_PROGRAMS = insensitive

insensitive_SOURCES = \
	insensitive-resources.c insensitive-resources.h \
	main.c insensitive.h \
	insensitive-window.c insensitive-window.h \
	insensitive-controller.c insensitive-controller.h \
	insensitive-library.c insensitive-library.h \
	insensitive-pulsesequence.c insensitive-pulsesequence.h \
	insensitive-settings.c insensitive-settings.h \
	insensitive-spinsystem.c insensitive-spinsystem.h \
	insensitive-pulseshaper.c insensitive-pulseshaper.h \
	insensitive-singlespins.c insensitive-singlespins.h \
	insensitive-tutorial.c insensitive-tutorial.h \
	insensitive-composer.c insensitive-composer.h \
	insensitive-preferences.c insensitive-preferences.h \
	levmarq.c levmarq.h

insensitive_CFLAGS = @GLIB_CFLAGS@ @GIO_CFLAGS@ @GTK3_CFLAGS@ \
	@FFTW3_CFLAGS@ @CAIRO_CFLAGS@ @WEBKIT2GTK_CFLAGS@ @XML2_CFLAGS@

insensitive_LDADD = @LIBINTL@ @GLIB_LIBS@ @GIO_LIBS@ @GTK3_LIBS@ \
	@FFTW3_LIBS@ @CAIRO_LIBS@ @WEBKIT2GTK_LIBS@ @XML2_LIBS@

insensitive_LDFLAGS = -rdynamic

insensitive-resources.c: insensitive.gresource.xml
	@GLIB_COMPILE_RESOURCES@ --sourcedir=$(top_builddir)/src --target=$@ --generate-source --c-name insensitive $<

insensitive-resources.h: insensitive.gresource.xml
	@GLIB_COMPILE_RESOURCES@ --sourcedir=$(top_builddir)/src --target=$@ --generate-header --c-name insensitive $<

EXTRA_DIST = insensitive.gresource.xml \
	insensitive-composer-row.ui \
	insensitive-pulseshaper.ui \
	insensitive-tutorial.ui \
	insensitive-composer.ui \
	insensitive-singlespins.ui \
	insensitive-preferences.ui \
	insensitive-window.ui \
	meson.build

CLEANFILES = *~ insensitive-resources.c insensitive-resources.h
