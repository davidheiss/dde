shell_resources = gnome.compile_resources(
    'shell-resources',
    'resources/shell.gresource.xml',
    source_dir: 'resources',
)

sources = [
    'main.c',
    'shell-application.c',
    'shell-window.c',
    'shell-panel.c',
    'shell-corners.c',
    'shell-background.c',
    'shell-time.c',
    'shell-time-label.c',
    'shell-settings.c',
    'shell-workspaces.c',
]

executable(
    'shell',
    sources,
    shell_resources,
    install: true,
    dependencies: [adw, gtk, layer_shell, de],
    link_args: ['-lm'],
)

install_data(
  'org.dh.shell.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0/schemas',
)