sources = [
    'de-size.c',
    'de-containing-box.c',
    'de-workspaces.c',
    'de-tween.c',
]

dbus = gnome.gdbus_codegen(
    'de-dbus',
    sources: [
        'dbus/compositor.xml',
    ],
    interface_prefix: 'org.dh',
    namespace: 'De',
)

de_lib = library(
    'de',
    sources,
    dbus,
    dependencies: [adw, gtk],
    install: true,
)

de = declare_dependency(
    link_with: de_lib,
    include_directories: include_directories('.'),
    sources: [dbus],
)