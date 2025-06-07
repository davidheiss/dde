sources = [
    'main.c',
    'proxy-hyprland.c',
]

dbus_service_conf = configuration_data()
dbus_service_conf.set('bindir', get_option('prefix') / get_option('bindir'))

configure_file(
  input: 'proxy.service.in',
  output: 'org.dde.proxy.service',
  configuration: dbus_service_conf,
  install: true,
  install_dir: '/usr/share/dbus-1/services'
)

executable(
    'proxy',
    sources,
    dependencies: [gio, json_glib, de],
    install: true,
)