application_id = 'com.klausboldt.insensitive'

foreach s: [16, 24, 32, 64, 128, 256, 512]
  s = '@0@x@0@'.format(s.to_string())
  icon_dir = join_paths('hicolor', s, 'apps')
  install_data(
    join_paths(icon_dir, ('@0@.png').format(application_id)),
    install_dir: join_paths(get_option('datadir'), 'icons', icon_dir)
  )
endforeach

foreach icon: [
  'insensitive-acquisition',
  'insensitive-equilibrium',
  'insensitive-remove',
  'insensitive-evolution',
  'insensitive-reset',
  'insensitive-add',
  'insensitive-gradient',
  'insensitive-undo',
  'insensitive-chemicalshift',
  'insensitive-pulse',
  'insensitive-coupling',
  'insensitive-relaxation',
  'insensitive-peakpicking',
  'insensitive-hideparameters',
  'insensitive-play',
  'insensitive-record',
  'insensitive-stop',
  'insensitive-step',
  'insensitive-trash'
]
  icon_dir = join_paths('hicolor', '24x24', 'actions')
  install_data(
    join_paths(icon_dir, ('@0@.png').format(icon)),
    install_dir: join_paths(get_option('datadir'), 'icons', icon_dir)
  )
endforeach

foreach icon: [
  'insensitive-spinsystem',
  'insensitive-spinstate',
  'insensitive-pulsesequence',
  'insensitive-spectrum',
  'insensitive-matrixcomposer',
  'insensitive-pulseshape',
  'insensitive-singlespins',
  'insensitive-presentation',
  'insensitive-composer',
  'insensitive-tutorial'
]
  icon_dir = join_paths('hicolor', '32x32', 'actions')
  install_data(
    join_paths(icon_dir, ('@0@.png').format(icon)),
    install_dir: join_paths(get_option('datadir'), 'icons', icon_dir)
  )
endforeach

foreach icon: [
  'insensitive-ispin',
  'insensitive-ispin-selected',
  'insensitive-sspin',
  'insensitive-sspin-selected'
]
  icon_dir = join_paths('hicolor', '48x48', 'actions')
  install_data(
    join_paths(icon_dir, ('@0@.png').format(icon)),
    install_dir: join_paths(get_option('datadir'), 'icons', icon_dir)
  )
endforeach
