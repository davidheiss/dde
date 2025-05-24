#include "convey-hyprland.h"
#include <gio/gio.h>

static void name_acquired_callback(
    GDBusConnection *connection, const gchar *name, gpointer user_data
)
{
    g_autoptr(GError) error = NULL;
    g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(user_data), connection, "/org/dh/convey",
        &error
    );
    if (error) {
        g_warning("%s", error->message);
        return;
    }
}

int main(int argc, char **argv)
{
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, 0);

    GDBusInterfaceSkeleton *compositor = NULL;
    if (convey_hyprland_active()) {
        compositor = G_DBUS_INTERFACE_SKELETON(convey_hyprland_new());
    }
    if (!compositor)
        return EXIT_FAILURE;

    g_bus_own_name(
        G_BUS_TYPE_SESSION, "org.dh.convey", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
        name_acquired_callback, NULL, compositor, NULL
    );

    g_main_loop_run(loop);
    return EXIT_SUCCESS;
}