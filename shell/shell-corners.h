#pragma once

#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(ShellCorners, shell_corners, SHELL, CORNERS, GtkDrawingArea);

#define SHELL_TYPE_CORNERS (shell_corners_get_type())

GtkWidget* shell_corners_new(double radius);