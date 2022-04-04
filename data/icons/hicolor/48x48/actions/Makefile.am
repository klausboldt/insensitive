## Process this file with automake to generate a Makefile.in

icondir = @datadir@/icons/hicolor/48x48/actions

icon_DATA = insensitive-ispin.png \
	insensitive-ispin-selected.png \
	insensitive-sspin.png \
	insensitive-sspin-selected.png

EXTRA_DIST = $(icon_DATA)

install-data-local: $(icon_DATA)
	$(mkinstalldirs) $(icondir)
	for file in $(icon_DATA); do \
	$(INSTALL_DATA) $(top_builddir)/data/icons/hicolor/48x48/actions/$$file $(icondir)/$$file; \
	done
