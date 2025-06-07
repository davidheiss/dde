sources = [
    'main.c',
    'launcher-application.c',
    'launcher-window.c',
]

launcher_resources = gnome.compile_resources(
    'launcher-resources',
    'resources/launcher.gresource.xml',
    source_dir: 'resources',
)

executable(
    'launcher',
    sources,
    launcher_resources,
    install: true,
    dependencies: [adw, layer_shell],
)