#ifndef __GFX_MATRIX3_H__
#define __GFX_MATRIX3_H__

#include "gfx/common.h"

typedef struct {
    float m[3][3];
} mx3_t;

#define MX3(A, I, J) ((A)->m[I][J])

mx3_t *mx3_new();
void mx3_delete(mx3_t *m);

void mx3_mul(mx3_t *a, mx3_t *b, mx3_t *c);
void mx3_transpose(mx3_t *a, mx3_t *b);

void mx3_load_identity(mx3_t *m);
void mx3_load_rotation(mx3_t *m, float angle);
void mx3_load_scaling(mx3_t *m, float sx, float sy);
void mx3_load_translation(mx3_t *m, float tx, float ty);

void mx3_transform(mx3_t *m, point_t *from, point_t *to, int n);

#endif
