#include "shell-time.h"

struct _ShellTime {
    GObject parent;
    guint id;
    time_t time;
};

enum _ShellTimeProperty {
    PROP_TIME = 1,
    N_PROP,
};

typedef enum _ShellTimeProperty ShellTimeProperty;

#define SHELL_TIME_PROPERTY(property) ((ShellTimeProperty)property)

static ShellTime *instance;
static GParamSpec *properties[N_PROP];

G_DEFINE_FINAL_TYPE(ShellTime, shell_time, G_TYPE_OBJECT)

static gboolean shell_time_update(ShellTime *self)
{
    self->time = time(NULL);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TIME]);
    return TRUE;
}

static void shell_time_get_property(
    GObject *object, guint property_id, GValue *value, GParamSpec *pspec
)
{
    ShellTime *self = SHELL_TIME(object);
    switch (SHELL_TIME_PROPERTY(property_id)) {
    case PROP_TIME:
        g_value_set_long(value, self->time);
        break;
    default:
        G_OBJECT_CLASS(shell_time_parent_class)
            ->get_property(object, property_id, value, pspec);
    }
}

static void shell_time_finalize(GObject *object)
{
    ShellTime *self = SHELL_TIME(object);
    g_source_remove(self->id);
    G_OBJECT_CLASS(shell_time_parent_class)->finalize(object);
    instance = NULL;
}

static void shell_time_class_init(ShellTimeClass *self)
{
    GObjectClass *object = G_OBJECT_CLASS(self);
    object->get_property = shell_time_get_property;
    properties[PROP_TIME] = g_param_spec_long(
        "time", "Time", "The current UNIX time in seconds since epoch", 0,
        LONG_MAX, 0, G_PARAM_READABLE
    );
    g_object_class_install_properties(object, N_PROP, properties);
    object->finalize = shell_time_finalize;
}

static void shell_time_init(ShellTime *self)
{
    self->time = time(NULL);
    self->id = g_timeout_add_seconds(1, G_SOURCE_FUNC(shell_time_update), self);
}

ShellTime *shell_time_new()
{
    if (instance)
        return g_object_ref(instance);
    instance = g_object_new(SHELL_TYPE_TIME, NULL);
    return instance;
}