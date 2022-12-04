## Process this file with automake to generate a Makefile.in

icondir = @datadir@/icons/hicolor/16x16/actions

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
	insensitive-undo.png \
	insensitive-triangleleft.png \
	insensitive-triangleright.png

EXTRA_DIST = $(icon_DATA)

install-data-local: $(icon_DATA)
	$(mkinstalldirs) $(icondir)
	for file in $(icon_DATA); do \
	$(INSTALL_DATA) $(top_builddir)/data/icons/hicolor/16x16/actions/$$file $(icondir)/$$file; \
	done

