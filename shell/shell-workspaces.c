#include "shell-workspaces.h"
#include "shell-settings.h"
#include <de.h>

#define SHELL_WORKSPACES_SIZE 10
#define SHELL_WORKSPACES_EXPAND 15
#define SHELL_WORKSPACES_SPACING 3
#define SHELL_WORKSPACES_WIDTH(n)                                              \
    (SHELL_WORKSPACES_SIZE * n + SHELL_WORKSPACES_SPACING * (n - 1) +          \
     SHELL_WORKSPACES_EXPAND)

struct _ShellWorkspaces {
    GtkButton parent;
    GtkWidget *drawing;

    ShellSettings *settings;
    DeCompositor *compositor;

    DeTween *tween;
    AdwAnimation *animation;
};

G_DEFINE_FINAL_TYPE(ShellWorkspaces, shell_workspaces, GTK_TYPE_BUTTON)

static void shell_workspaces_draw_func(
    GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height,
    gpointer user_data
)
{
    ShellWorkspaces *self = SHELL_WORKSPACES(user_data);

    double x = 0;
    double y = height / 2.;

    GdkRGBA color;
    gtk_widget_get_color(self->drawing, &color);

    for (gsize i = 0; i < self->tween->n; i++) {
        double active = de_tween_get(self->tween, 0, i);
        double occupied = de_tween_get(self->tween, 1, i);

        x += SHELL_WORKSPACES_SIZE / 2.;
        double expand = active * SHELL_WORKSPACES_EXPAND;
        cairo_arc(cr, x, y, SHELL_WORKSPACES_SIZE / 2., G_PI_2, G_PI_2 * 3);
        cairo_arc(
            cr, x + expand, y, SHELL_WORKSPACES_SIZE / 2., G_PI_2 * 3, G_PI_2
        );
        double alpha = (1. + active + occupied) / 3.;
        cairo_set_source_rgba(
            cr, color.red, color.green, color.blue, color.alpha * alpha
        );
        cairo_fill(cr);
        x += SHELL_WORKSPACES_SIZE / 2. + expand + SHELL_WORKSPACES_SPACING;
    }
}

static void shell_workspaces_changed_active(
    ShellWorkspaces *self, GParamSpec *pspec, GObject *object
)
{
    gint workspace = de_compositor_get_active_workspace(self->compositor);
    if (workspace == 0) {
        for (gsize i = 0; i < self->tween->n; i++)
            de_tween_set(self->tween, 0, i, 0.);
        goto end;
    }

    for (gsize i = 0; i < self->tween->n; i++)
        de_tween_set(self->tween, 0, i, i + 1 == workspace ? 1. : 0.);

end:
    de_tween_updated(self->tween);
    adw_animation_play(self->animation);
}

static gboolean contains(const int *array, gsize len, int value)
{
    for (gsize i = 0; i < len; i++) {
        if (array[i] == value)
            return TRUE;
    }
    return FALSE;
}

static void shell_workspaces_changed_workspaces(
    ShellWorkspaces *self, GParamSpec *pspec, GObject *object
)
{
    GVariant *variant = de_compositor_get_workspaces(self->compositor);
    if (variant == NULL) {
        for (gsize i = 0; i < self->tween->n; i++)
            de_tween_set(self->tween, 1, i, 0.);
        goto end;
    }

    gsize len;
    const gint *workspaces = g_variant_get_fixed_array(
        variant, &len, sizeof(gint)
    );

    for (gsize i = 0; i < self->tween->n; i++)
        de_tween_set(
            self->tween, 1, i, contains(workspaces, len, i + 1) ? 1. : 0.
        );
end:
    de_tween_updated(self->tween);
    adw_animation_play(self->animation);
}

static void shell_workspaces_animate(double animation, gpointer user_date)
{
    ShellWorkspaces *self = SHELL_WORKSPACES(user_date);
    de_tween_animate(self->tween, animation);
    gtk_widget_queue_draw(self->drawing);
}

static void shell_workspaces_changed_num_workspaces(
    ShellWorkspaces *self, gchar *key, GSettings *settings
)
{
    gsize workspaces_len = g_settings_get_uint64(settings, "workspaces");
    de_tween_resize(self->tween, 2, workspaces_len);

    gint active = de_compositor_get_active_workspace(self->compositor);
    if (active > 0 && active <= workspaces_len) {
        de_tween_set(self->tween, 0, active - 1, 1.);
    }

    GVariant *variant = de_compositor_get_workspaces(self->compositor);
    if (!variant)
        goto end;

    gsize n_workspaces;
    const gint *workspaces = g_variant_get_fixed_array(
        variant, &n_workspaces, sizeof(gint)
    );
    for (gsize i = 0; i < n_workspaces; i++) {
        int workspace = workspaces[i];
        if (workspace > 0 && workspace <= workspaces_len)
            de_tween_set(self->tween, 1, workspace - 1, 1.0);
    }

end:
    de_tween_init_done(self->tween);

    gtk_widget_set_size_request(
        self->drawing, SHELL_WORKSPACES_WIDTH(workspaces_len),
        SHELL_WORKSPACES_SIZE
    );
    gtk_widget_queue_draw(self->drawing);
};

static void shell_workspaces_dispose(GObject *object)
{
    ShellWorkspaces *self = SHELL_WORKSPACES(object);
    if (self->compositor) {
        g_object_unref(self->compositor);
        self->compositor = NULL;
    }
    if (self->settings) {
        g_object_unref(self->settings);
        self->settings = NULL;
    }
    if (self->animation) {
        g_object_unref(self->animation);
        self->animation = NULL;
    }
    G_OBJECT_CLASS(shell_workspaces_parent_class)->dispose(object);
}

static void shell_workspaces_finalize(GObject *object)
{
    ShellWorkspaces *self = SHELL_WORKSPACES(object);
    de_tween_free(self->tween);
    G_OBJECT_CLASS(shell_workspaces_parent_class)->finalize(object);
}

static void shell_workspaces_class_init(ShellWorkspacesClass *cls)
{
    GObjectClass *object = G_OBJECT_CLASS(cls);
    object->dispose = shell_workspaces_dispose;
    object->finalize = shell_workspaces_finalize;
}

static void shell_workspaces_init(ShellWorkspaces *self)
{
    GError *error = NULL;

    self->settings = shell_settings_new();
    g_signal_connect_object(
        self->settings, "changed::workspaces",
        G_CALLBACK(shell_workspaces_changed_num_workspaces), self,
        G_CONNECT_SWAPPED
    );

    self->compositor = de_compositor_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, "org.dh.convey",
        "/org/dh/convey", NULL, &error
    );
    if (error) {
        g_warning(
            "Can not create new proxy for compositor: %s", error->message
        );
        g_clear_error(&error);
    }
    g_signal_connect_object(
        self->compositor, "notify::active-workspace",
        G_CALLBACK(shell_workspaces_changed_active), self, G_CONNECT_SWAPPED
    );
    g_signal_connect_object(
        self->compositor, "notify::workspaces",
        G_CALLBACK(shell_workspaces_changed_workspaces), self, G_CONNECT_SWAPPED
    );

    gsize workspaces_len = g_settings_get_uint64(
        G_SETTINGS(self->settings), "workspaces"
    );
    self->tween = de_tween_new(2, workspaces_len);

    gint active = de_compositor_get_active_workspace(self->compositor);
    if (active > 0 && active <= workspaces_len) {
        de_tween_set(self->tween, 0, active - 1, 1.);
    }

    GVariant *variant = de_compositor_get_workspaces(self->compositor);
    gsize n_workspaces;
    const gint *workspaces = g_variant_get_fixed_array(
        variant, &n_workspaces, sizeof(gint)
    );
    for (gsize i = 0; i < n_workspaces; i++) {
        int workspace = workspaces[i];
        if (workspace > 0 && workspace <= workspaces_len)
            de_tween_set(self->tween, 1, workspace - 1, 1.0);
    }

    de_tween_init_done(self->tween);

    self->drawing = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(self->drawing), shell_workspaces_draw_func, self, NULL
    );
    gtk_widget_set_size_request(
        self->drawing, SHELL_WORKSPACES_WIDTH(workspaces_len),
        SHELL_WORKSPACES_SIZE
    );

    AdwAnimationTarget *target = adw_callback_animation_target_new(
        shell_workspaces_animate, self, NULL
    );
    self->animation = adw_timed_animation_new(
        self->drawing, 0., 1., 250, target
    );

    gtk_widget_set_valign(GTK_WIDGET(self), GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(GTK_WIDGET(self), "flat");
    gtk_widget_add_css_class(GTK_WIDGET(self), "small");
    gtk_button_set_child(GTK_BUTTON(self), self->drawing);
}

GtkWidget *shell_workspaces_new()
{
    return g_object_new(SHELL_TYPE_WORKSPACES, NULL);
}