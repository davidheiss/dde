sources = [
    'main.c',
    'convey-hyprland.c',
]

dbus_service_conf = configuration_data()
dbus_service_conf.set('bindir', get_option('prefix') / get_option('bindir'))

configure_file(
  input: 'convey.service.in',
  output: 'org.dh.convey.service',
  configuration: dbus_service_conf,
  install: true,
  install_dir: '/usr/share/dbus-1/services'
)

executable(
    'convey',
    sources,
    dependencies: [gio, json_glib, de],
    install: true,
)