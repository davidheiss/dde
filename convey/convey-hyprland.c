#include "convey-hyprland.h"
#include <de-workspaces.h>
#include <json-glib/json-glib.h>

#define HYPR_RESPONSE_DELIM "\n\n\n"
#define HYPR_EVENT_DELIM ">>"

struct _ConveyHyprland {
    DeCompositorSkeleton parent;
    DeWorkspaces *workspaces;
};

G_DEFINE_FINAL_TYPE(ConveyHyprland, convey_hyprland, DE_TYPE_COMPOSITOR_SKELETON);

static void
convey_hyprland_handle_event(ConveyHyprland *self, gchar *event, gchar *data)
{
    if (!strcmp(event, "workspacev2")) {
        *strchr(data, ',') = '\0';
        int workspace = atoi(data);
        g_object_set(self, "active-workspace", workspace, NULL);
    } else if (!strcmp(event, "createworkspacev2")) {
        *strchr(data, ',') = '\0';
        int workspace = atoi(data);
        de_workspaces_append(&self->workspaces, workspace);

        GVariant *workspaces = g_variant_new_fixed_array(
            G_VARIANT_TYPE_INT32, self->workspaces->data, self->workspaces->len,
            sizeof(int)
        );
        g_object_set(self, "workspaces", workspaces, NULL);
    } else if (!strcmp(event, "destroyworkspacev2")) {
        *strchr(data, ',') = '\0';
        int workspace = atoi(data);
        de_workspaces_remove(self->workspaces, workspace);
        GVariant *workspaces = g_variant_new_fixed_array(
            G_VARIANT_TYPE_INT32, self->workspaces->data, self->workspaces->len,
            sizeof(int)
        );
        g_object_set(self, "workspaces", workspaces, NULL);
    }
}

static void convey_hyprland_read_event(
    GObject *stream, GAsyncResult *res, gpointer user_data
)
{
    if (g_input_stream_is_closed(G_INPUT_STREAM(stream)))
        return;

    g_autoptr(GError) error = NULL;
    gsize length;
    g_autofree gchar *event = g_data_input_stream_read_line_finish(
        G_DATA_INPUT_STREAM(stream), res, &length, &error
    );
    if (error) {
        g_warning("Can not read event: %s", error->message);
        return;
    }

    gchar *data = strstr(event, HYPR_EVENT_DELIM);
    if (!data) {
        g_warning("Can not process event: %s", event);
        return;
    }
    *data = '\0';
    data += sizeof(HYPR_EVENT_DELIM) - 1;

    ConveyHyprland *self = CONVEY_HYPRLAND(user_data);
    convey_hyprland_handle_event(self, event, data);

    g_data_input_stream_read_line_async(
        G_DATA_INPUT_STREAM(stream), G_PRIORITY_HIGH, NULL,
        convey_hyprland_read_event, user_data
    );
}

static void convey_hyprland_listen(ConveyHyprland *self)
{
    const char *runtime = g_getenv("XDG_RUNTIME_DIR");
    const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (!runtime) {
        g_warning("Missing environment variables: XDG_RUNTIME_DIR.");
        return;
    }
    if (!his) {
        g_warning("Missing environment variables: HYPRLAND_INSTANCE_SIGNATURE."
        );
        return;
    }

    g_autofree char *path = g_strdup_printf(
        "%s/hypr/%s/.socket2.sock", runtime, his
    );
    g_autoptr(GSocketAddress) address = g_unix_socket_address_new(path);

    g_autoptr(GError) error = NULL;

    g_autoptr(GSocketClient) client = g_socket_client_new();

    g_autoptr(GSocketConnection) connection = g_socket_client_connect(
        client, G_SOCKET_CONNECTABLE(address), NULL, &error
    );
    if (error) {
        g_warning(
            "Failed to connect to Hyprland socket at '%s': %s", path,
            error->message
        );
        return;
    }

    GInputStream *input_stream = g_io_stream_get_input_stream(
        G_IO_STREAM(connection)
    );

    g_autoptr(GDataInputStream)
        data_input_stream = g_data_input_stream_new(input_stream);
    g_data_input_stream_read_line_async(
        data_input_stream, G_PRIORITY_HIGH, NULL, convey_hyprland_read_event,
        self
    );
}

static void convey_hyprland_handle_command(
    ConveyHyprland *self, gchar *command, gchar *data
)
{
    g_autoptr(GError) error = NULL;
    g_autoptr(JsonParser) parser = json_parser_new_immutable();
    json_parser_load_from_data(parser, data, -1, &error);
    if (error) {
        g_warning(
            "Failed to parse JSON for command '%s': %s", command, error->message
        );
        return;
    }

    JsonNode *node = json_parser_get_root(parser);

    if (!g_strcmp0(command, "activeworkspace")) {
        JsonObject *object = json_node_get_object(node);
        gint64 id = json_object_get_int_member(object, "id");
        g_object_set(self, "active-workspace", id, NULL);
    } else if (!g_strcmp0(command, "workspaces")) {
        JsonArray *array = json_node_get_array(node);
        guint length = json_array_get_length(array);
        for (guint i = 0; i < length; i++) {
            JsonNode *node = json_array_get_element(array, i);
            JsonObject *object = json_node_get_object(node);
            int workspace = json_object_get_int_member(object, "id");
            de_workspaces_append(&self->workspaces, workspace);
        }

        GVariant *variant = g_variant_new_fixed_array(
            G_VARIANT_TYPE_INT32, self->workspaces->data, self->workspaces->len,
            sizeof(int)
        );
        g_object_set(self, "workspaces", variant, NULL);
    }
}

static void
convey_hyprland_commands(ConveyHyprland *self, gchar **commands, gsize n)
{
    const char *runtime = g_getenv("XDG_RUNTIME_DIR");
    const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (!runtime) {
        g_warning("Missing environment variables: XDG_RUNTIME_DIR.");
        return;
    }
    if (!his) {
        g_warning("Missing environment variables: HYPRLAND_INSTANCE_SIGNATURE."
        );
        return;
    }

    g_autofree char *path = g_strdup_printf(
        "%s/hypr/%s/.socket.sock", runtime, his
    );
    g_autoptr(GSocketAddress) address = g_unix_socket_address_new(path);

    g_autoptr(GString) command = g_string_new("[[BATCH]]");
    for (gsize i = 0; i < n; i++) {
        g_string_append_len(command, ";j/", 3);
        g_string_append(command, commands[i]);
    }

    g_autoptr(GError) error = NULL;

    g_autoptr(GSocketClient) client = g_socket_client_new();
    g_autoptr(GSocketConnection) connection = g_socket_client_connect(
        client, G_SOCKET_CONNECTABLE(address), NULL, &error
    );
    if (error) {
        g_warning(
            "Failed to connect to Hyprland socket at '%s': %s", path,
            error->message
        );
        return;
    }

    GOutputStream *output_stream = g_io_stream_get_output_stream(
        G_IO_STREAM(connection)
    );
    g_output_stream_write_all(
        output_stream, command->str, command->len, NULL, NULL, &error
    );
    if (error) {
        g_warning(
            "Failed to send command batch to Hyprland: %s", error->message
        );
        return;
    }

    GInputStream *input_stream = g_io_stream_get_input_stream(
        G_IO_STREAM(connection)
    );

    g_autoptr(GString) response = g_string_new(NULL);
    while (1) {
        g_autofree gchar *buffer = g_malloc(4096);
        gsize read;
        g_input_stream_read_all(
            input_stream, buffer, 4096, &read, NULL, &error
        );
        if (error) {
            g_warning(
                "Failed to read response from Hyprland socket: %s",
                error->message
            );
            return;
        }
        g_string_append_len(response, buffer, read);
        if (read < 4096)
            break;
    }

    g_io_stream_close(G_IO_STREAM(connection), NULL, &error);
    if (error) {
        g_warning(
            "Failed to close connection to Hyprland socket: %s", error->message
        );
        return;
    }

    gchar *data = response->str;
    gsize i = 0;
    do {
        gchar *next_data = strstr(data, HYPR_RESPONSE_DELIM);
        if (next_data) {
            *next_data = '\0';
            next_data += sizeof(HYPR_RESPONSE_DELIM) - 1;
        }
        convey_hyprland_handle_command(self, commands[i++], data);
        data = next_data;
    } while (data);

    if (i != n)
        g_warning(
            "Expected to handle %lu commands, but only processed %lu.", i, n
        );
}

static void convey_hyprland_finalize(GObject *object)
{
    ConveyHyprland *self = CONVEY_HYPRLAND(object);
    g_free(self->workspaces);
    G_OBJECT_CLASS(convey_hyprland_parent_class)->finalize(object);
}

static void convey_hyprland_class_init(ConveyHyprlandClass *cls)
{
    GObjectClass *object = G_OBJECT_CLASS(cls);
    object->finalize = convey_hyprland_finalize;
}

static void convey_hyprland_init(ConveyHyprland *self)
{
    self->workspaces = de_workspaces_new();
    gchar *commands[] = {
        "activeworkspace",
        "workspaces",
    };
    convey_hyprland_commands(self, commands, 2);
    convey_hyprland_listen(self);
}

ConveyHyprland *convey_hyprland_new()
{
    return g_object_new(CONVEY_TYPE_HYPRLAND, NULL);
}

gboolean convey_hyprland_active() {
    return g_getenv("HYPRLAND_INSTANCE_SIGNATURE") != NULL;
}