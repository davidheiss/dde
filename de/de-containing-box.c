#include "de-containing-box.h"

struct _DeContainingBox {
    GtkBox parent;
};

G_DEFINE_TYPE(DeContainingBox, de_containing_box, GTK_TYPE_BOX)

static gboolean de_containing_box_contains(GtkWidget *widget, double x, double y)
{
    return FALSE;
}

static void de_containing_box_class_init(DeContainingBoxClass *cls)
{
    GtkWidgetClass *widget = GTK_WIDGET_CLASS(cls);
    widget->contains = de_containing_box_contains;
}

static void de_containing_box_init(DeContainingBox *self) {}

GtkWidget *de_containing_box_new(GtkOrientation orientation, int spacing)
{
    return g_object_new(
        DE_TYPE_CONTAINING_BOX, "orientation", orientation, "spacing", spacing,
        NULL
    );
}