#pragma once

#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(LauncherWindow, launcher_window, LAUNCHER, WINDOW, GtkApplicationWindow);

#define LAUNCHER_TYPE_WINDOW (launcher_window_get_type())

GtkWidget* launcher_window_new(GtkApplication *app);