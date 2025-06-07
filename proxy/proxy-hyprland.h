#pragma once

#include <de-dbus.h>

G_DECLARE_FINAL_TYPE(
    ProxyHyprland, proxy_hyprland, PROXY, HYPRLAND, DeCompositorSkeleton
);

#define PROXY_TYPE_HYPRLAND (proxy_hyprland_get_type())

ProxyHyprland *proxy_hyprland_new();

gboolean proxy_hyprland_active();