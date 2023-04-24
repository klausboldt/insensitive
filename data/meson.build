desktop_file = i18n.merge_file(
  input: 'com.klausboldt.insensitive.desktop.in',
  output: 'com.klausboldt.insensitive.desktop',
  type: 'desktop',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications')
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
    args: [desktop_file]
  )
endif

appstream_file = i18n.merge_file(
  input: 'com.klausboldt.insensitive.appdata.xml.in',
  output: 'com.klausboldt.insensitive.appdata.xml',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(get_option('datadir'), 'appdata')
)

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate', appstream_file]
  )
endif

install_data('com.klausboldt.insensitive.gschema.xml',
  install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas')
)

install_data('com.klausboldt.insensitive.mime.xml',
  install_dir: join_paths(get_option('datadir'), 'mime/packages'),
)

foreach file: [
  'fid.wav',
]
  install_data(
    file, install_dir: join_paths(get_option('datadir'), 'insensitive')
  )
endforeach

compile_schemas = find_program('glib-compile-schemas', required: false)
if compile_schemas.found()
  test('Validate schema file', compile_schemas,
    args: ['--strict', '--dry-run', meson.current_source_dir()]
  )
endif

subdir('icons')
subdir('doc')
