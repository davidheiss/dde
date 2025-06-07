#include "shell-corners.h"

struct _ShellCorners {
    GtkDrawingArea parent;
    double radius;
};

enum _ShellCornersProperty {
    PROP_RADIUS = 1,
    N_PROPS,
};

typedef enum _ShellCornersProperty ShellCornersProperty;

#define SHELL_CORNERS_PROPERTY(property) ((ShellCornersProperty)(property))

static GParamSpec *properties[N_PROPS];

G_DEFINE_FINAL_TYPE(ShellCorners, shell_corners, GTK_TYPE_DRAWING_AREA);

static void shell_corners_draw_function(
    GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data
)
{
    ShellCorners *self = SHELL_CORNERS(area);
    GdkRGBA color;

    cairo_move_to(cr, 0, 0);
    cairo_arc(
        cr, self->radius, self->radius, self->radius, G_PI_2 * 2, G_PI_2 * 3
    );

    cairo_move_to(cr, width, 0);
    cairo_arc(
        cr, width - self->radius, self->radius, self->radius, G_PI_2 * 3,
        G_PI_2 * 4
    );

    cairo_move_to(cr, width, height);
    cairo_arc(
        cr, width - self->radius, height - self->radius, self->radius,
        G_PI_2 * 4, G_PI_2 * 5
    );

    cairo_move_to(cr, 0, height);
    cairo_arc(
        cr, self->radius, height - self->radius, self->radius, G_PI_2 * 5,
        G_PI_2 * 6
    );

    gtk_widget_get_color(GTK_WIDGET(area), &color);
    gdk_cairo_set_source_rgba(cr, &color);

    cairo_fill(cr);
}

static gboolean shell_corners_contain(GtkWidget *widget, double x, double y)
{
    ShellCorners *self = SHELL_CORNERS(widget);
    int width = gtk_widget_get_width(widget);
    int height = gtk_widget_get_height(widget);

    return (x < self->radius && y < self->radius &&
            hypot(x - self->radius, y - self->radius) >= self->radius) ||
           (x > width - self->radius && y < self->radius &&
            hypot(x - width + self->radius, y - self->radius) >= self->radius
           ) ||
           (x > width - self->radius && y > height - self->radius &&
            hypot(x - width + self->radius, y - height + self->radius) >=
                self->radius) ||
           (x < self->radius && y > height - self->radius &&
            hypot(x - self->radius, y - height + self->radius) >= self->radius);
}

static void shell_corners_get_property(
    GObject *object, guint property_id, GValue *value, GParamSpec *pspec
)
{
    ShellCorners *self = SHELL_CORNERS(object);
    switch (SHELL_CORNERS_PROPERTY(property_id)) {
    default:
        G_OBJECT_CLASS(self)->get_property(object, property_id, value, pspec);
    }
}

static void shell_corners_set_property(
    GObject *object, guint property_id, const GValue *value, GParamSpec *pspec
)
{
    ShellCorners *self = SHELL_CORNERS(object);
    switch (SHELL_CORNERS_PROPERTY(property_id)) {
    case PROP_RADIUS:
        self->radius = g_value_get_double(value);
        break;
    default:
        G_OBJECT_CLASS(self)->set_property(object, property_id, value, pspec);
    }
}

static void shell_corners_class_init(ShellCornersClass *cls)
{
    GObjectClass *object = G_OBJECT_CLASS(cls);
    object->get_property = shell_corners_get_property;
    object->set_property = shell_corners_set_property;
    properties[PROP_RADIUS] = g_param_spec_double(
        "radius", NULL, NULL, 0, INFINITY, 0, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
    );
    g_object_class_install_properties(object, N_PROPS, properties);

    GtkWidgetClass *widget = GTK_WIDGET_CLASS(cls);
    widget->contains = shell_corners_contain;
}

static void shell_corners_init(ShellCorners *self)
{
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(self), shell_corners_draw_function, self, NULL
    );
    gtk_widget_set_hexpand(GTK_WIDGET(self), true);
    gtk_widget_set_vexpand(GTK_WIDGET(self), true);
    gtk_widget_add_css_class(GTK_WIDGET(self), "corners");
}

GtkWidget *shell_corners_new(double radius)
{
    return g_object_new(SHELL_TYPE_CORNERS, "radius", radius, NULL);
}