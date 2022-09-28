## Process this file with automake to generate a Makefile.in
SUBDIRS = doc \
	icons \
	icons/hicolor/16x16/actions \
	icons/hicolor/24x24/actions \
	icons/hicolor/32x32/actions \
	icons/hicolor/48x48/actions \
	icons/hicolor/16x16/apps \
	icons/hicolor/24x24/apps \
	icons/hicolor/32x32/apps \
	icons/hicolor/64x64/apps \
	icons/hicolor/128x128/apps \
	icons/hicolor/256x256/apps \
	icons/hicolor/512x512/apps

insensitivedir = @datadir@/@PACKAGE@
insensitive_DATA = fid.wav

desktopdir = $(datadir)/applications

EXTRA_DIST = $(insensitive_DATA) \
	com.klausboldt.insensitive.desktop.in \
	com.klausboldt.insensitive.gschema.xml \
	com.klausboldt.insensitive.appdata.xml.in \
	meson.build

install-data-local: $(insensitive_DATA) $(desktop_DATA)
	$(mkinstalldirs) $(insensitivedir) $(desktopdir)
	$(INSTALL_DATA) $(top_builddir)/data/com.klausboldt.insensitive.desktop.in $(desktopdir)/com.klausboldt.insensitive.desktop
