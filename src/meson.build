insensitive_sources = [
  'main.c',
  'insensitive-window.c',
  'insensitive-controller.c',
  'insensitive-library.c',
  'insensitive-pulsesequence.c',
  'insensitive-settings.c',
  'insensitive-spinsystem.c',
  'insensitive-pulseshaper.c',
  'insensitive-singlespins.c',
  'insensitive-composer.c',
  'insensitive-preferences.c',
  'levmarq.c',
]

insensitive_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('blas'),
  dependency('cblas', required: false),
  dependency('lapack'),
  dependency('fftw3'),
  dependency('cairo'),
  dependency('pango'),
  dependency('libxml-2.0'),
]

webkit_dep = dependency('webkit2gtk-4.0', required : false)
if webkit_dep.found()
    insensitive_sources += ['insensitive-tutorial.c',]
    insensitive_deps += webkit_dep
    add_project_arguments('-DUSE_WEBKIT_GTK', language : 'c')
endif

cc = meson.get_compiler('c')
if cc.get_argument_syntax() == 'gcc'
    compiler_args = cc.get_supported_arguments([
        '-Wno-unused-parameter',
        '-Wno-unused-variable',
        '-Wno-unused-function',
        '-Wno-unused-but-set-variable'
    ])
endif
add_project_arguments(compiler_args, language: 'c')
libm = cc.find_library('m')

gnome = import('gnome')

insensitive_sources += gnome.compile_resources('insensitive-resources',
  'insensitive.gresource.xml',
  c_name: 'insensitive'
)

executable('insensitive', insensitive_sources,
  dependencies: [insensitive_deps, libm],
  install: true,
  link_args: '-rdynamic',
)
