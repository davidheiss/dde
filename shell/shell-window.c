#include "shell-window.h"
#include "shell-background.h"
#include "shell-corners.h"
#include "shell-panel.h"
#include <gtk4-layer-shell.h>
#include <de.h>

static void shell_window_realize(GtkWidget *widget, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(widget);
    gtk_layer_init_for_window(window);
    for (GtkLayerShellEdge edge = 0; edge < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER;
         edge++)
        gtk_layer_set_anchor(window, edge, TRUE);
    gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_BOTTOM);
    gtk_layer_set_exclusive_edge(window, GTK_LAYER_SHELL_EDGE_TOP);
}

static void shell_window_panel_changed(
    GObject *object, GParamSpec *pspec, GtkWindow *window
)
{
    int height;
    g_object_get(object, "height", &height, NULL);
    gtk_layer_set_exclusive_zone(window, height);
}

GtkWidget *shell_window_new(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);

    GtkWidget *panel = shell_panel_new();
    GtkWidget *size = de_size_new(panel);
    g_signal_connect(
        size, "notify::height", G_CALLBACK(shell_window_panel_changed), window
    );

    GtkWidget *corners = shell_corners_new(16.);

    GtkWidget *box = de_containing_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(box), size);
    gtk_box_append(GTK_BOX(box), corners);

    GtkWidget *background = shell_background_new();

    GtkWidget *overlay = gtk_overlay_new();
    gtk_overlay_set_child(GTK_OVERLAY(overlay), background);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    gtk_widget_remove_css_class(window, "background");
    gtk_window_set_child(GTK_WINDOW(window), overlay);
    g_signal_connect(window, "realize", G_CALLBACK(shell_window_realize), NULL);

    return window;
}