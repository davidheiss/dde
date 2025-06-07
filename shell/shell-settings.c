#include "shell-settings.h"

struct _ShellSettings {
    GSettings parent;
};

ShellSettings *instance;

G_DEFINE_FINAL_TYPE(ShellSettings, shell_settings, G_TYPE_SETTINGS)

void shell_settings_finalize(GObject *self)
{
    instance = NULL;
    G_OBJECT_CLASS(shell_settings_parent_class)->finalize(self);
}

void shell_settings_class_init(ShellSettingsClass *cls)
{
    GObjectClass *object = G_OBJECT_CLASS(cls);
    object->finalize = shell_settings_finalize;
}

void shell_settings_init(ShellSettings *self) {}

ShellSettings *shell_settings_new()
{
    if (instance)
        return g_object_ref(instance);

    instance = g_object_new(
        SHELL_TYPE_SETTINGS, "schema-id", "org.dh.shell", NULL
    );

    return instance;
}