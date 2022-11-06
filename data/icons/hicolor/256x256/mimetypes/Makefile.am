## Process this file with automake to generate a Makefile.in

icondir = @datadir@/icons/hicolor/256x256/mimetypes

icon_DATA = insensitive-iss.png \
	insensitive-ipp.png \
	insensitive-igg.png

EXTRA_DIST = $(icon_DATA)

install-data-local: $(icon_DATA)
	$(mkinstalldirs) $(icondir)
	for file in $(icon_DATA); do \
	$(INSTALL_DATA) $(top_builddir)/data/icons/hicolor/256x256/mimetypes/$$file $(icondir)/$$file; \
	done
