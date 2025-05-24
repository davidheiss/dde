#include "de-tween.h"

DeTween *de_tween_new(gsize m, gsize n)
{
    g_assert(m > 0 && n > 0);
    DeTween *self = g_malloc(sizeof(DeTween));
    self->m = m;
    self->n = n;

    gsize size = sizeof(double) * m * n;
    self->start = g_malloc0(size);
    self->value = g_malloc0(size);
    self->end = g_malloc0(size);

    return self;
}

void de_tween_free(DeTween *self)
{
    free(self->start);
    free(self->value);
    free(self->end);
    free(self);
}

static void
resize(double (**mem)[], gsize old_m, gsize old_n, gsize new_m, gsize new_n)
{
    double(*new_mem)[new_n] = g_malloc0(sizeof(gdouble) * new_m * new_n);
    for (gsize i = 0; i < MIN(old_m, new_m); i++)
        for (gsize j = 0; j < MIN(old_n, new_n); j++)
            ((double(*)[new_n])new_mem
            )[i][j] = ((double(*)[old_n]) * mem)[i][j];
    free(*mem);
    *mem = new_mem;
}

void de_tween_resize(DeTween *self, gsize m, gsize n)
{
    resize(&self->start, self->m, self->n, m, n);
    resize(&self->value, self->m, self->n, m, n);
    resize(&self->end, self->m, self->n, m, n);
    self->m = m;
    self->n = n;
}

void de_tween_set(DeTween *self, gsize i, gsize j, double value)
{
    g_assert(i < self->m && j < self->n);
    double(*end)[self->n] = self->end;
    end[i][j] = value;
}

double de_tween_get(DeTween *self, gsize i, gsize j)
{
    g_assert(i < self->m && j < self->n);
    return ((double(*)[self->n])self->value)[i][j];
}

void de_tween_updated(DeTween *self)
{
    double(*start)[self->n] = self->start;
    double(*value)[self->n] = self->value;

    for (gsize i = 0; i < self->m; i++)
        for (gsize j = 0; j < self->n; j++)
            start[i][j] = value[i][j];
}

void de_tween_init_done(DeTween *self)
{
    double(*start)[self->n] = self->start;
    double(*value)[self->n] = self->value;
    double(*end)[self->n] = self->end;

    for (gsize i = 0; i < self->m; i++)
        for (gsize j = 0; j < self->n; j++)
            start[i][j] = value[i][j] = end[i][j];
}

void de_tween_animate(DeTween *self, double animation)
{
    double(*start)[self->n] = self->start;
    double(*value)[self->n] = self->value;
    double(*end)[self->n] = self->end;

    for (gsize i = 0; i < self->m; i++)
        for (gsize j = 0; j < self->n; j++)
            value[i][j] = (1. - animation) * start[i][j] +
                          animation * end[i][j];
}