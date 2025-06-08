#include "launcher-window.h"
#include <adwaita.h>
#include <gtk4-layer-shell/gtk4-layer-shell.h>

struct _LauncherWindow {
    GtkApplicationWindow window;
    GtkWidget *text;
    GtkCustomFilter *filter;
    GtkSelectionModel *selection;
    GtkWidget *list;
    GtkWidget *stack;
    char *path;
    GKeyFile *key_file;
};

G_DEFINE_FINAL_TYPE(
    LauncherWindow, launcher_window, GTK_TYPE_APPLICATION_WINDOW
);

static void launcher_window_clicked(
    GtkGestureClick *self, gint n_press, gdouble x, gdouble y,
    gpointer user_data
)
{

    GtkWidget *child = gtk_window_get_child(GTK_WINDOW(user_data));

    graphene_point_t point = GRAPHENE_POINT_INIT(x, y);
    graphene_rect_t rect;
    if (gtk_widget_compute_bounds(
            GTK_WIDGET(child), GTK_WIDGET(user_data), &rect
        ))
        if (!graphene_rect_contains_point(&rect, &point))
            gtk_window_close(GTK_WINDOW(user_data));
}

gboolean launcher_window_key_pressed(
    GtkEventControllerKey *controller, guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data
)
{
    LauncherWindow *self = LAUNCHER_WINDOW(user_data);

    if (state & GDK_CONTROL_MASK && keyval == GDK_KEY_n) {
        guint n_items = g_list_model_get_n_items(G_LIST_MODEL(self->selection));

        guint selection = gtk_single_selection_get_selected(
            GTK_SINGLE_SELECTION(self->selection)
        );
        if (selection + 1 < n_items)
            gtk_list_view_scroll_to(
                GTK_LIST_VIEW(self->list), selection + 1,
                GTK_LIST_SCROLL_FOCUS | GTK_LIST_SCROLL_SELECT, NULL
            );
        return GDK_EVENT_STOP;
    }

    if (state & GDK_CONTROL_MASK && keyval == GDK_KEY_p) {
        guint n_items = g_list_model_get_n_items(G_LIST_MODEL(self->selection));
        if (n_items == 0)
            return GDK_EVENT_STOP;

        guint selection = gtk_single_selection_get_selected(
            GTK_SINGLE_SELECTION(self->selection)
        );
        if (selection >= 1)
            gtk_list_view_scroll_to(
                GTK_LIST_VIEW(self->list), selection - 1,
                GTK_LIST_SCROLL_FOCUS | GTK_LIST_SCROLL_SELECT, NULL
            );

        return GDK_EVENT_STOP;
    }

    if (keyval == GDK_KEY_Return) {
        guint selection = gtk_single_selection_get_selected(
            GTK_SINGLE_SELECTION(self->selection)
        );
        g_signal_emit_by_name(self->list, "activate", selection);

        return GDK_EVENT_STOP;
    }

    if (keyval == GDK_KEY_Escape) {
        gtk_window_close(GTK_WINDOW(self));
        return GDK_EVENT_STOP;
    }

    gtk_text_grab_focus_without_selecting(GTK_TEXT(self->text));

    return GDK_EVENT_PROPAGATE;
}

static GListStore *launcher_window_create_application_store()
{
    GListStore *store = g_list_store_new(G_TYPE_APP_INFO);
    g_autoptr(GList) list = g_app_info_get_all();
    for (GList *item = list; item != NULL; item = item->next)
        g_list_store_append(store, item->data);
    return store;
}

static gboolean launcher_window_filter(gpointer item, gpointer user_data)
{
    GAppInfo *app = G_APP_INFO(item);

    g_autofree char *text;
    g_object_get(user_data, "text", &text, NULL);

    const char *name = g_app_info_get_name(app);

    return strncasecmp(text, name, strlen(text)) == 0;
}

gint launcher_window_sorter(
    gconstpointer a, gconstpointer b, gpointer user_data
)
{
    LauncherWindow *self = LAUNCHER_WINDOW(user_data);

    GAppInfo *app_a = G_APP_INFO(a);
    GAppInfo *app_b = G_APP_INFO(b);

    guint64 score_a = g_key_file_get_uint64(
        self->key_file, "Application", g_app_info_get_id(app_a), NULL
    );
    guint64 score_b = g_key_file_get_uint64(
        self->key_file, "Application", g_app_info_get_id(app_b), NULL
    );

    if (score_a == score_b) {
        return 0;
    }

    return score_a < score_b ? 1 : -1;
}

static void
launcher_window_notify(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    LauncherWindow *self = LAUNCHER_WINDOW(user_data);

    gtk_filter_changed(GTK_FILTER(self->filter), GTK_FILTER_CHANGE_DIFFERENT);

    guint n_items = g_list_model_get_n_items(G_LIST_MODEL(self->selection));
    if (n_items > 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(self->stack), "list");
        gtk_list_view_scroll_to(
            GTK_LIST_VIEW(self->list), 0,
            GTK_LIST_SCROLL_FOCUS | GTK_LIST_SCROLL_SELECT, NULL
        );
    } else {
        gtk_stack_set_visible_child_name(GTK_STACK(self->stack), "empty");
    }
}

static void launcher_window_setup(
    GtkSignalListItemFactory *self, GtkListItem *list_item, gpointer user_data
)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_size_request(box, -1, 46);
    gtk_widget_set_margin_start(box, 6);
    gtk_widget_set_margin_end(box, 6);
    gtk_list_item_set_child(list_item, box);

    GtkWidget *image = gtk_image_new();
    gtk_box_append(GTK_BOX(box), image);
    gtk_image_set_icon_size(GTK_IMAGE(image), GTK_ICON_SIZE_LARGE);

    GtkWidget *label = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *name = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(name), 0);
    gtk_label_set_ellipsize(GTK_LABEL(name), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(label), name);

    GtkWidget *description = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(description), 0);
    gtk_label_set_ellipsize(GTK_LABEL(description), PANGO_ELLIPSIZE_END);
    gtk_widget_add_css_class(description, "caption");
    gtk_widget_add_css_class(description, "dim-label");
    gtk_box_append(GTK_BOX(label), description);
}

static void launcher_window_bind(
    GtkSignalListItemFactory *self, GtkListItem *list_item, gpointer user_data
)
{
    GAppInfo *item = gtk_list_item_get_item(list_item);

    GtkWidget *image = gtk_widget_get_first_child(
        gtk_list_item_get_child(list_item)
    );
    gtk_image_set_from_gicon(GTK_IMAGE(image), g_app_info_get_icon(item));

    GtkWidget *name = gtk_widget_get_first_child(
        gtk_widget_get_next_sibling(image)
    );
    const char *str_name = g_app_info_get_name(item);
    gtk_label_set_label(GTK_LABEL(name), str_name);

    GtkWidget *description = gtk_widget_get_next_sibling(name);

    const char *str_desctiption = g_app_info_get_description(item);
    gtk_label_set_label(
        GTK_LABEL(description), str_desctiption ? str_desctiption : str_name
    );
}

static void
launcher_window_activate(GtkListView *list, guint position, gpointer user_data)
{
    LauncherWindow *self = LAUNCHER_WINDOW(user_data);

    GAppInfo *app = g_list_model_get_item(
        G_LIST_MODEL((gtk_list_view_get_model(list))), position
    );
    GError *error = NULL;
    g_app_info_launch(app, NULL, NULL, &error);
    if (error) {
        g_warning("%s", error->message);
        g_clear_error(&error);
    }

    const char *id = g_app_info_get_id(app);
    guint64 value = g_key_file_get_uint64(
        self->key_file, "Application", id, NULL
    );
    g_key_file_set_uint64(self->key_file, "Application", id, value + 1);
    g_mkdir_with_parents(g_path_get_dirname(self->path), 0755);
    g_key_file_save_to_file(self->key_file, self->path, &error);
    if (error) {
        g_warning("%s", error->message);
        g_clear_error(&error);
    }

    g_print("%lu\n", value);
    gtk_window_close(GTK_WINDOW(user_data));
}

void launcher_window_realize(GtkWidget *widget)
{
    GtkWindow *window = GTK_WINDOW(widget);

    gtk_layer_init_for_window(window);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, true);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, true);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, true);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, true);
    gtk_layer_set_exclusive_zone(window, -1);
    gtk_layer_set_keyboard_mode(
        window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE
    );
    gtk_layer_set_namespace(window, "dde");

    GTK_WIDGET_CLASS(launcher_window_parent_class)->realize(widget);
}

void launcher_window_dispose(GObject *object)
{
    G_OBJECT_CLASS(launcher_window_parent_class)->dispose(object);
}

void launcher_window_finalize(GObject *object)
{
    LauncherWindow *self = LAUNCHER_WINDOW(object);
    g_free(self->path);
    g_key_file_free(self->key_file);
    G_OBJECT_CLASS(launcher_window_parent_class)->dispose(object);
}

static void launcher_window_class_init(LauncherWindowClass *cls)
{
    GtkWidgetClass *widget = GTK_WIDGET_CLASS(cls);
    widget->realize = launcher_window_realize;

    GObjectClass *object = G_OBJECT_CLASS(cls);
    object->dispose = launcher_window_dispose;
    object->finalize = launcher_window_finalize;
}

static void launcher_window_init(LauncherWindow *self)
{
    gchar **environ = g_get_environ();
    const char *XDG_DATA_HOME = g_environ_getenv(environ, "XDG_DATA_HOME");
    if (XDG_DATA_HOME) {
        self->path = g_strdup_printf("%s/dde/launcher/data.ini", XDG_DATA_HOME);
    } else {
        const char *HOME = g_environ_getenv(environ, "HOME");
        self->path = g_strdup_printf(
            "%s/.local/share/dde/launcher/data.init", HOME
        );
    }

    self->key_file = g_key_file_new();
    g_key_file_load_from_file(
        self->key_file, self->path, G_KEY_FILE_NONE, NULL
    );

    GtkGesture *click = gtk_gesture_click_new();
    gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(click));
    g_signal_connect_object(
        click, "pressed", G_CALLBACK(launcher_window_clicked), self,
        G_CONNECT_DEFAULT
    );

    GtkEventController *key = gtk_event_controller_key_new();
    gtk_widget_add_controller(GTK_WIDGET(self), key);
    gtk_event_controller_set_propagation_phase(key, GTK_PHASE_CAPTURE);
    g_signal_connect_object(
        key, "key-pressed", G_CALLBACK(launcher_window_key_pressed), self,
        G_CONNECT_DEFAULT
    );

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(box, 600, 395);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(box, 200);
    gtk_widget_add_css_class(box, "view");
    gtk_widget_add_css_class(box, "frame");
    gtk_widget_add_css_class(box, "card");
    gtk_widget_set_overflow(box, GTK_OVERFLOW_HIDDEN);
    gtk_window_set_child(GTK_WINDOW(self), box);

    GtkWidget *entry = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(entry, 12);
    gtk_widget_set_margin_bottom(entry, 12);
    gtk_widget_set_margin_start(entry, 12);
    gtk_widget_set_margin_end(entry, 12);
    gtk_box_append(GTK_BOX(box), entry);

    GtkWidget *icon = gtk_image_new_from_icon_name("edit-find-symbolic");
    gtk_box_append(GTK_BOX(entry), icon);

    self->text = gtk_text_new();
    gtk_widget_set_hexpand(self->text, true);
    gtk_text_set_placeholder_text(GTK_TEXT(self->text), "Search for anything");
    gtk_box_append(GTK_BOX(entry), self->text);

    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_append(GTK_BOX(box), separator);

    GListStore *store = launcher_window_create_application_store();

    self->filter = gtk_custom_filter_new(
        launcher_window_filter, self->text, NULL
    );
    g_signal_connect(
        self->text, "notify::text", G_CALLBACK(launcher_window_notify), self
    );

    GtkCustomSorter *sorter = gtk_custom_sorter_new(
        launcher_window_sorter, self, NULL
    );

    self->selection = GTK_SELECTION_MODEL(
        gtk_single_selection_new(G_LIST_MODEL(gtk_filter_list_model_new(
            G_LIST_MODEL(
                gtk_sort_list_model_new(G_LIST_MODEL(store), GTK_SORTER(sorter))
            ),
            GTK_FILTER(self->filter)
        )))
    );

    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(launcher_window_setup), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(launcher_window_bind), NULL);

    self->list = gtk_list_view_new(self->selection, factory);
    g_signal_connect(
        self->list, "activate", G_CALLBACK(launcher_window_activate), self
    );
    gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(self->list), TRUE);

    GtkWidget *scrollable = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrollable, true);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollable), self->list);

    GtkWidget *status = adw_status_page_new();
    adw_status_page_set_title(ADW_STATUS_PAGE(status), "No Results Found");
    adw_status_page_set_description(
        ADW_STATUS_PAGE(status), "Try a different search"
    );
    adw_status_page_set_icon_name(
        ADW_STATUS_PAGE(status), "edit-find-symbolic"
    );
    gtk_widget_add_css_class(status, "compact");

    self->stack = gtk_stack_new();
    gtk_stack_add_named(GTK_STACK(self->stack), scrollable, "list");
    gtk_stack_add_named(GTK_STACK(self->stack), status, "empty");
    gtk_box_append(GTK_BOX(box), self->stack);

    gtk_widget_remove_css_class(GTK_WIDGET(self), "background");
}

GtkWidget *launcher_window_new(GtkApplication *app)
{
    GType type = LAUNCHER_TYPE_WINDOW;
    return g_object_new(type, "application", app, NULL);
}