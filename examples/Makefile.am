## Process this file with automake to generate a Makefile.in

examplesdir = $(datadir)/@PACKAGE@/examples

examples_DATA = 13C_decoupled.ipp \
	13C_inverse_gated_decoupling.ipp \
	1H.ipp \
	1_Spin.iss \
	2_Spins_A2.iss \
	2_Spins_AB.iss \
	2_Spins_AX.iss \
	2_Spins_heteronuclear.iss \
	3_Spins_A2X.iss \
	3_Spins_A3.iss \
	3_Spins_AMX.iss \
	3_Spins_AX+A.iss \
	3_Spins_H-C-C.iss \
	3_Spins_heteronuclear.iss \
	4_Spins_1,1-difluoroallene.iss \
	4_Spins_1,1-difluoroethylene.iss \
	4_Spins_A4.iss \
	4_Spins_AABB.iss \
	4_Spins_AAXX.iss \
	4_Spins_ABMX.iss \
	4_Spins_ABXY.iss \
	4_Spins_INADEQUATE.iss \
	ABMX_1D.igg \
	ABMX_COSY-45.igg \
	ABMX_COSY.igg \
	ABMX_DQF-COSY.igg \
	ABMX_E.COSY.igg \
	ABMX_SECSY.igg \
	ABMX_TOCSY.igg \
	ABMX_TQF-COSY.igg \
	ABMX_z-COSY.igg \
	AMX_1D.igg \
	AMX_COSY-45.igg \
	AMX_COSY.igg \
	AMX_DQF-COSY.igg \
	AMX_gCOSY.igg \
	AMX_J-resolved.igg \
	AMX_NOESY.igg \
	AMX_Relayed_COSY.igg \
	AMX_ROESY.igg \
	AMX_SECSY.igg \
	AMX_TOCSY.igg \
	AMX_TQF-COSY.igg \
	AMX_z-COSY.igg \
	APT.ipp \
	AX+A_DOSY.igg \
	AXAX_2D-INADEQUATE.igg \
	AX_INADEQUATE.igg \
	BIRD.ipp \
	COSY-45.ipp \
	COSY.ipp \
	CPMG_1D.ipp \
	CPMG.ipp \
	DEPT-135.ipp \
	DEPT-45.ipp \
	DEPT-90.ipp \
	DOSY.ipp \
	DQF-COSY.ipp \
	gCOSY.ipp \
	gDQF-COSY.ipp \
	H-C-C_HMBC.igg \
	H-C-C_HMQC.igg \
	H-C-C_HSQC.igg \
	H-C-C_HXCO.igg \
	HETCOR.ipp \
	HMBC.ipp \
	HMQC.ipp \
	HSQC.ipp \
	INADEQUATE_2D.ipp \
	INADEQUATE.ipp \
	INEPT.ipp \
	INEPT_decoupled.ipp \
	INEPT_transfer_only.ipp \
	Inversion-Recovery_1D.ipp \
	Inversion-Recovery.ipp \
	J-modulated_(SEFT).ipp \
	J-resolved.ipp \
	Long-Range_COSY.ipp \
	NOESY.ipp \
	PENDANT.ipp \
	QQF-COSY.ipp \
	Relayed_COSY.ipp \
	ROESY.ipp \
	SCOSY.ipp \
	SECSY.ipp \
	TANGO.ipp \
	TOCSY.ipp \
	TQF-COSY.ipp \
	z-COSY.ipp

EXTRA_DIST = $(examples_DATA) meson.build

install-data-local: $(examples_DATA)
	$(mkinstalldirs) $(examplesdir) $(examplesdir)/spin_systems $(examplesdir)/pulse_programs $(examplesdir)/spectra
	for file in $(top_builddir)/examples/*.iss ; do \
	basefile=`basename "$$file"`; \
	$(INSTALL_DATA) "$(top_builddir)/examples/$$file" "$(examplesdir)/spin_systems/$$basefile"; \
	done
	for file in $(top_builddir)/examples/*.ipp ; do \
	basefile=`basename "$$file"`; \
	$(INSTALL_DATA) "$(top_builddir)/examples/$$file" "$(examplesdir)/pulse_programs/$$basefile"; \
	done
	for file in $(top_builddir)/examples/*.igg ; do \
	basefile=`basename "$$file"`; \
	$(INSTALL_DATA) "$(top_builddir)/examples/$$file" "$(examplesdir)/spectra/$$basefile"; \
	done
