#pragma once

#include <glib-object.h>

G_DECLARE_FINAL_TYPE(ShellTime, shell_time, SHELL, TIME, GObject);

#define SHELL_TYPE_TIME (shell_time_get_type())

ShellTime *shell_time_new();