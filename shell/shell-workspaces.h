#pragma once

#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(ShellWorkspaces, shell_workspaces, SHELL, WORKSPACES, GtkButton)

#define SHELL_TYPE_WORKSPACES (shell_workspaces_get_type())

GtkWidget *shell_workspaces_new();