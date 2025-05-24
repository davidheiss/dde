#include "de-workspaces.h"

DeWorkspaces *de_workspaces_new()
{
    DeWorkspaces *self = g_malloc0(sizeof(DeWorkspaces));
    return self;
};

gboolean de_workspaces_contains(DeWorkspaces *self, gint workspace)
{
    for (gsize i = 0; i < self->len; i++) {
        if (self->data[i] == workspace)
            return TRUE;
    }
    return FALSE;
}

void de_workspaces_append(DeWorkspaces **ptr, gint workspace)
{
    DeWorkspaces *self = *ptr;
    if (de_workspaces_contains(self, workspace))
        return;

    if (self->len == self->cap) {
        *ptr = self = g_realloc(
            self, sizeof(DeWorkspaces) + sizeof(gint) * ++self->cap
        );
    }
    self->data[self->len++] = workspace;
}

gboolean de_workspaces_remove(DeWorkspaces *self, gint workspace)
{
    for (gsize i = 0; i < self->len; i++) {
        if (self->data[i] == workspace) {
            self->data[i] = self->data[--self->len];
            return TRUE;
        }
    }
    return FALSE;
}