#include "de-size.h"

struct _DeSize {
    GtkWidget parent;
    GtkWidget *child;
    int width;
    int height;
};

enum _DeSizeProperty {
    PROP_CHILD = 1,
    PROP_WIDTH,
    PROP_HEIGHT,
    N_PROPS,
};

typedef enum _DeSizeProperty DeSizeProperty;

#define DE_SIZE_PROPERTY(property) ((DeSizeProperty)property)

static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE(DeSize, de_size, GTK_TYPE_WIDGET);

static void de_size_measure(
    GtkWidget *widget, GtkOrientation orientation, int for_size, int *minimum,
    int *natural, int *minimum_baseline, int *natural_baseline
)
{
    DeSize *self = DE_SIZE(widget);

    gtk_widget_measure(
        self->child, orientation, for_size, minimum, natural, minimum_baseline,
        natural_baseline
    );
}

static void
de_size_allocate(GtkWidget *widget, int width, int height, int baseline)
{
    GObject *object = G_OBJECT(widget);
    DeSize *self = DE_SIZE(widget);

    if (self->width != width) {
        self->width = width;
        g_object_notify_by_pspec(object, properties[PROP_WIDTH]);
    }

    if (self->height != height) {
        self->height = height;
        g_object_notify_by_pspec(object, properties[PROP_HEIGHT]);
    }

    gtk_widget_allocate(self->child, width, height, baseline, NULL);
}

static void de_size_get_property(
    GObject *object, guint property_id, GValue *value, GParamSpec *pspec
)
{
    DeSize *self = DE_SIZE(object);
    switch (DE_SIZE_PROPERTY(property_id)) {
    case PROP_WIDTH:
        g_value_set_int(value, self->width);
        break;
    case PROP_HEIGHT:
        g_value_set_int(value, self->height);
        break;
    default:
        G_OBJECT_CLASS(de_size_parent_class)
            ->get_property(object, property_id, value, pspec);
    }
}

static void de_size_set_property(
    GObject *object, guint property_id, const GValue *value, GParamSpec *pspec
)
{
    DeSize *self = DE_SIZE(object);
    switch (DE_SIZE_PROPERTY(property_id)) {
    case PROP_CHILD:
        if (self->child)
            gtk_widget_unparent(self->child);
        self->child = g_value_get_object(value);
        gtk_widget_set_parent(self->child, GTK_WIDGET(self));
        break;
    default:
        G_OBJECT_CLASS(de_size_parent_class)
            ->set_property(object, property_id, value, pspec);
    }
}

static void de_size_dispose(GObject *object)
{
    DeSize *self = DE_SIZE(object);
    if (self->child) {
        gtk_widget_unparent(self->child);
        self->child = NULL;
    }
    G_OBJECT_CLASS(de_size_parent_class)->dispose(object);
}

static void de_size_class_init(DeSizeClass *cls)
{
    GObjectClass *object = G_OBJECT_CLASS(cls);

    object->get_property = de_size_get_property;
    object->set_property = de_size_set_property;

    properties[PROP_CHILD] = g_param_spec_object(
        "child", NULL, NULL, G_TYPE_OBJECT,
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
    );
    properties[PROP_WIDTH] = g_param_spec_int(
        "width", NULL, NULL, 0, INT32_MAX, 0, G_PARAM_READABLE
    );
    properties[PROP_HEIGHT] = g_param_spec_int(
        "height", NULL, NULL, 0, INT32_MAX, 0, G_PARAM_READABLE
    );
    g_object_class_install_properties(object, N_PROPS, properties);

    object->dispose = de_size_dispose;

    GtkWidgetClass *widget = GTK_WIDGET_CLASS(cls);
    widget->measure = de_size_measure;
    widget->size_allocate = de_size_allocate;
}

static void de_size_init(DeSize *self) {}

GtkWidget *de_size_new(GtkWidget *child)
{
    return g_object_new(DE_TYPE_SIZE, "child", child, NULL);
}