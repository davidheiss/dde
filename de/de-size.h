#pragma once

#include <adwaita.h>

G_DECLARE_FINAL_TYPE(DeSize, de_size, DE, SIZE, GtkWidget);

#define DE_TYPE_SIZE (de_size_get_type())

GtkWidget *de_size_new(GtkWidget *child);