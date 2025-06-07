#include "launcher-application.h"
#include "launcher-window.h"

static GtkWidget *window;

static void launcher_application_startup(GApplication *app, gpointer user_data)
{
    window = launcher_window_new(GTK_APPLICATION(app));
}

static void launcher_application_activate(GApplication *app, gpointer user_data)
{
    gtk_window_present(GTK_WINDOW(window));
}

AdwApplication *launcher_application_new()
{
    AdwApplication *app = adw_application_new(
        "org.dde.launcher", G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app, "startup", G_CALLBACK(launcher_application_startup), NULL
    );

    g_signal_connect(
        app, "activate", G_CALLBACK(launcher_application_activate), NULL
    );

    return app;
}