## Process this file with automake to generate a Makefile.in

icondir = @datadir@/icons/hicolor/64x64/apps

icon_DATA = com.klausboldt.insensitive.png

EXTRA_DIST = $(icon_DATA)

install-data-local: $(icon_DATA)
	$(mkinstalldirs) $(icondir)
	$(INSTALL_DATA) $(top_builddir)/data/icons/hicolor/64x64/apps/com.klausboldt.insensitive.png $(icondir)/com.klausboldt.insensitive.png
