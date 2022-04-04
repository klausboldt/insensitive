## Process this file with automake to generate a Makefile.in

icondir = @datadir@/icons/hicolor/32x32/actions

icon_DATA = insensitive-composer.png \
	insensitive-matrixcomposer.png \
	insensitive-presentation.png \
	insensitive-pulsesequence.png \
	insensitive-pulseshape.png \
	insensitive-singlespins.png \
	insensitive-spectrum.png \
	insensitive-spinstate.png \
	insensitive-spinsystem.png \
	insensitive-tutorial.png

EXTRA_DIST = $(icon_DATA)

install-data-local: $(icon_DATA)
	$(mkinstalldirs) $(icondir)
	for file in $(icon_DATA); do \
	$(INSTALL_DATA) $(top_builddir)/data/icons/hicolor/32x32/actions/$$file $(icondir)/$$file; \
	done
