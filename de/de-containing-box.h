#pragma once

#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(
    DeContainingBox, de_containing_box, DE, CONTAINING_BOX, GtkBox
);

#define DE_TYPE_CONTAINING_BOX (de_containing_box_get_type())

GtkWidget *de_containing_box_new(GtkOrientation orientation, int spacing);