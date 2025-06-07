project('Davids Desktop Environment', 'c', version: '1.0.0')

gnome = import('gnome')

# gobject = dependency('gobject-2.0')
gio = dependency('gio-2.0 ')
adw = dependency('libadwaita-1')
gtk = dependency('gtk4')
layer_shell = dependency('gtk4-layer-shell-0')
json_glib = dependency('json-glib-1.0')

subdir('de')
subdir('launcher')
subdir('proxy')
subdir('shell')

gnome.post_install(
  glib_compile_schemas: true,
)