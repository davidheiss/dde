#pragma once

#include <gio/gio.h>

G_DECLARE_FINAL_TYPE(ShellSettings, shell_settings, shell, SETTINGS, GSettings);

#define SHELL_TYPE_SETTINGS (shell_settings_get_type())

ShellSettings* shell_settings_new();