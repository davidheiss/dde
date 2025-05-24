#include "shell-time-label.h"
#include "shell-time.h"

static gboolean shell_time_label_transform(
    GBinding *binding, const GValue *from_value, GValue *to_value,
    gpointer user_data
)
{
    time_t time = g_value_get_long(from_value);
    struct tm tm;
    localtime_r(&time, &tm);

    size_t maxsize = 1024;
    g_autofree char *s = g_malloc(maxsize);
    size_t len = strftime(s, maxsize, "%b %d %R", &tm);
    s = g_realloc(s, len);

    g_value_set_string(to_value, s);

    return TRUE;
}

GtkWidget *shell_time_label_new()
{
    ShellTime *time = shell_time_new();
    GtkWidget *label = gtk_label_new(NULL);
    g_object_bind_property_full(
        time, "time", label, "label", G_BINDING_SYNC_CREATE,
        shell_time_label_transform, NULL, time, g_object_unref
    );
    return label;
}