#pragma once

#include <de-dbus.h>

G_DECLARE_FINAL_TYPE(
    ConveyHyprland, convey_hyprland, CONVEY, HYPRLAND, DeCompositorSkeleton
);

#define CONVEY_TYPE_HYPRLAND (convey_hyprland_get_type())

ConveyHyprland *convey_hyprland_new();

gboolean convey_hyprland_active();