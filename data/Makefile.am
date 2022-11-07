## Process this file with automake to generate a Makefile.in
SUBDIRS = doc \
	icons \
	icons/hicolor/16x16/mimetypes \
	icons/hicolor/32x32/mimetypes \
	icons/hicolor/128x128/mimetypes \
	icons/hicolor/256x256/mimetypes \
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
	com.klausboldt.insensitive.mime.xml \
	meson.build

install-data-local: $(insensitive_DATA) $(desktop_DATA)
	$(mkinstalldirs) $(insensitivedir) $(desktopdir)
	$(INSTALL_DATA) $(top_builddir)/data/com.klausboldt.insensitive.desktop.in $(desktopdir)/com.klausboldt.insensitive.desktop
	$(UPDATE_DESKTOP_DATABASE) -q $(desktopdir)

install-data-hook: install-mime-database
uninstall-hook: uninstall-mime-database

install-mime-database: com.klausboldt.insensitive.mime.xml $(XDG_MIME) $(XDG_ICON_RESOURCE)
	$(XDG_MIME) install --novendor com.klausboldt.insensitive.mime.xml
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 16 icons/hicolor/16x16/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 16 icons/hicolor/16x16/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 16 icons/hicolor/16x16/mimetypes/insensitive-igg.png insensitive-igg
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 icons/hicolor/32x32/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 icons/hicolor/32x32/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 icons/hicolor/32x32/mimetypes/insensitive-igg.png insensitive-igg
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 128 icons/hicolor/128x128/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 128 icons/hicolor/128x128/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 128 icons/hicolor/128x128/mimetypes/insensitive-igg.png insensitive-igg
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 256 icons/hicolor/256x256/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 256 icons/hicolor/256x256/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) install --context mimetypes --size 256 icons/hicolor/256x256/mimetypes/insensitive-igg.png insensitive-igg
	$(UPDATE_MIME_DATABASE) $(datadir)/mime

uninstall-mime-database: com.klausboldt.insensitive.mime.xml $(XDG_MIME) $(XDG_ICON_RESOURCE)
	$(XDG_MIME) uninstall --novendor com.klausboldt.insensitive.mime.xml
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 16 icons/hicolor/16x16/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 16 icons/hicolor/16x16/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 16 icons/hicolor/16x16/mimetypes/insensitive-igg.png insensitive-igg
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 icons/hicolor/32x32/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 icons/hicolor/32x32/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 icons/hicolor/32x32/mimetypes/insensitive-igg.png insensitive-igg
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 128 icons/hicolor/128x128/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 128 icons/hicolor/128x128/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 128 icons/hicolor/128x128/mimetypes/insensitive-igg.png insensitive-igg
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 256 icons/hicolor/256x256/mimetypes/insensitive-iss.png insensitive-iss
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 256 icons/hicolor/256x256/mimetypes/insensitive-ipp.png insensitive-ipp
	$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 256 icons/hicolor/256x256/mimetypes/insensitive-igg.png insensitive-igg
	$(UPDATE_MIME_DATABASE) $(datadir)/mime
