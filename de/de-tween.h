#pragma once

#include <glib.h>

struct _DeTween {
    gsize m, n;
    double (*start)[];
    double (*value)[];
    double (*end)[];
};

typedef struct _DeTween DeTween;

DeTween *de_tween_new(gsize dim, gsize len);
void de_tween_free(DeTween *self);
void de_tween_resize(DeTween *self, gsize m, gsize n);
void de_tween_set(DeTween *self, gsize i, gsize j, double value);
double de_tween_get(DeTween *self, gsize i, gsize j);
void de_tween_updated(DeTween *self);
void de_tween_animate(DeTween *self, double animation);
void de_tween_init_done(DeTween *self);