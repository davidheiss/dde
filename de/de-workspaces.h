#pragma once

#include <glib.h>

struct _DeWorkspaces {
    gsize len;
    gsize cap;
    gint data[];
};

typedef struct _DeWorkspaces DeWorkspaces;

DeWorkspaces *de_workspaces_new();
void de_workspaces_append(DeWorkspaces **ptr, gint workspace);
gboolean de_workspaces_remove(DeWorkspaces *self, gint workspace);