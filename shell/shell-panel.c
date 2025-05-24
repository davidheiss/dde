#include "shell-panel.h"
#include "shell-time-label.h"
#include "shell-workspaces.h"

GtkWidget *shell_panel_new()
{
    GtkWidget *workspaces = shell_workspaces_new();

    GtkWidget *start = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(start), workspaces);

    GtkWidget *time = shell_time_label_new();

    GtkWidget *center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(center), time);

    GtkWidget *center_box = gtk_center_box_new();
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(center_box), start);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(center_box), center);
    gtk_widget_set_size_request(center_box, -1, 28);
    gtk_widget_add_css_class(center_box, "panel");
    return center_box;
}