#include "shell-application.h"
#include "shell-window.h"

static void shell_application_activate(GApplication *self, gpointer user_data) {
    GtkWidget *window = shell_window_new(GTK_APPLICATION(self));
    gtk_window_present(GTK_WINDOW(window));
}

GApplication *shell_application_new()
{
    AdwApplication *application = adw_application_new(
        "org.dh.shell", G_APPLICATION_DEFAULT_FLAGS | G_APPLICATION_ALLOW_REPLACEMENT
    );

    g_signal_connect(
        application, "activate", G_CALLBACK(shell_application_activate), NULL
    );

    return G_APPLICATION(application);
}