project(
	'maus',
	'c',
	default_options: [
		'c_std=c89',
		'warning_level=2',
		'optimization=2',
	]
)

backend = get_option('window_backend')
cc = meson.get_compiler('c')

inc = include_directories('include')
common_sources = [
	'source/maus.c',
	'source/maus_font.c',
	'source/utils.c',
]
backend_sources = []
deps = []
c_args = []
backend_override_options = ['c_std=c89']
lib_name = 'maus_' + backend
if backend == 'windows'
	lib_name = 'maus_win'
endif

if backend == 'x11'
	c_args += ['-DBACKEND_X11']
	deps += [
		dependency('x11'),
		dependency('xext'),
		dependency('xi'),
	]
	backend_sources += ['source/maus_x11.c']


elif backend == 'wayland'
	c_args += ['-DBACKEND_WAY']
	backend_override_options = ['c_std=c99']
	deps += [ dependency('wayland-client'), dependency('wayland-cursor'), dependency('xkbcommon'), ]

	wayland = import('wayland')
	backend_sources += wayland.scan_xml(wayland.find_protocol('xdg-shell'))
	backend_sources += wayland.scan_xml(wayland.find_protocol('pointer-constraints', state: 'unstable', version: 1))
	backend_sources += wayland.scan_xml(wayland.find_protocol('relative-pointer', state: 'unstable', version: 1))

	backend_sources += ['source/maus_wayland.c']


elif backend == 'windows'
	c_args += ['-DBACKEND_WIN']
	deps += [
		cc.find_library('gdi32'),
		cc.find_library('user32'),
	]
	backend_sources += ['source/maus_win.c']
endif

common = static_library(
	'maus_common_' + backend,
	common_sources,
	include_directories: inc,
	c_args: c_args,
	override_options: ['c_std=c89'],
)

backend_lib = static_library(
	'maus_backend_' + backend,
	backend_sources,
	include_directories: inc,
	c_args: c_args,
	dependencies: deps,
	override_options: backend_override_options,
)

maus = static_library(
	lib_name,
	objects: [
		common.extract_all_objects(recursive: false),
		backend_lib.extract_all_objects(recursive: false),
	],
	dependencies: deps,
)

maus_dep = declare_dependency(
	include_directories: inc,
	link_with: maus,
	dependencies: deps,
	compile_args: c_args,
)

