## Process this file with automake to generate a Makefile.in

icondir = @datadir@/icons/hicolor/24x24/actions

icon_DATA = insensitive-acquisition.png \
	insensitive-add.png \
	insensitive-chemicalshift.png \
	insensitive-coupling.png \
	insensitive-equilibrium.png \
	insensitive-evolution.png \
	insensitive-gradient.png \
	insensitive-hideparameters.png \
	insensitive-peakpicking.png \
	insensitive-play.png \
	insensitive-pulse.png \
	insensitive-record.png \
	insensitive-relaxation.png \
	insensitive-remove.png \
	insensitive-reset.png \
	insensitive-step.png \
	insensitive-stop.png \
	insensitive-trash.png \
	insensitive-undo.png

EXTRA_DIST = $(icon_DATA)

install-data-local: $(icon_DATA)
	$(mkinstalldirs) $(icondir)
	for file in $(icon_DATA); do \
	$(INSTALL_DATA) $(top_builddir)/data/icons/hicolor/24x24/actions/$$file $(icondir)/$$file; \
	done
