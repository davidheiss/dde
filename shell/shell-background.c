#include "shell-background.h"
#include "shell-settings.h"

void shell_background_changed(
    GSettings *settings, char *key, GtkPicture *picture
)
{
    g_autofree char *filename = g_settings_get_string(settings, "background");
    gtk_picture_set_filename(picture, filename);
}

GtkWidget *shell_background_new()
{
    GtkWidget *picture = gtk_picture_new();
    //TODO: Keep alive
    g_autoptr(ShellSettings) settings = shell_settings_new();

    g_signal_connect_object(
        settings, "changed::background", G_CALLBACK(shell_background_changed),
        picture, G_CONNECT_DEFAULT
    );

    g_autofree char *filename = g_settings_get_string(
        G_SETTINGS(settings), "background"
    );
    gtk_picture_set_filename(GTK_PICTURE(picture), filename);

    // TODO: Set also from settings.
    gtk_picture_set_content_fit(GTK_PICTURE(picture), GTK_CONTENT_FIT_COVER);

    return picture;
}
