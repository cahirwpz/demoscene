#pragma once

#include <SDL.h>
#include <math.h>
#include <stdbool.h>

static const int WIDTH = 320;
static const int HEIGHT = 240;

static const int MAX_STEPS = 1000;
static const float MAX_DIST = 100.0;
static const float SURF_DIST = 0.01;

static float iTime;

typedef struct vec3 {
  float x, y, z, w;
} vec3;

typedef struct mat4 {
  float m00, m01, m02, m03;
  float m10, m11, m12, m13;
  float m20, m21, m22, m23;
  float m30, m31, m32, m33;
} mat4;

/* https://en.wikipedia.org/wiki/Fast_inverse_square_root */
float Q_rsqrt(float number) {
  union {
    float f;
    uint32_t i;
  } conv = {.f = number};
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f *= 1.5f - (number * 0.5f * conv.f * conv.f);
  return conv.f;
}

float clamp(float v, float min, float max) {
  return fmin(fmax(v, min), max);
}

vec3 v3_abs(vec3 p) {
  return (vec3){fabs(p.x), fabs(p.y), fabs(p.z)};
}

vec3 v3_max(vec3 p, float v) {
  return (vec3){fmax(p.x, v), fmax(p.y, v), fmax(p.z, v)};
}

vec3 v3_add(vec3 a, vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 v3_sub(vec3 a, vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 v3_mul(vec3 a, float v) {
  return (vec3){a.x * v, a.y * v, a.z * v};
}

float v3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 v3_cross(vec3 a, vec3 b) {
  return (vec3){a.y * b.z - b.y * a.z,
                a.z * b.x - b.z * a.x,
                a.x * b.y - b.x * a.y};
}

float v3_length(vec3 p) {
  return sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
}

vec3 v3_normalize(vec3 p) {
#if 1
  return v3_mul(p, 1.0f / v3_length(p));
#else
  return v3_mul(p, Q_rsqrt(p.x * p.x + p.y * p.y + p.z * p.z));
#endif
}

mat4 m4_mul(mat4 a, mat4 b) {
  mat4 c;

  float *ma = (float *)&a.m00;
  float *mb = (float *)&b.m00;
  float *mc = (float *)&c.m00;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      mc[i * 4 + j] = 0;
      for (int k = 0; k < 4; k++) {
        mc[i * 4 + j] += ma[i * 4 + k] * mb[k * 4 + j];
      }
    }
  }

  return c;
}

/* @brief Rotate around X axis */
mat4 m4_rotate_x(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1};
}

/* @brief Rotate around Y axis */
mat4 m4_rotate_y(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1};
}

/* @brief Rotate around Z axis */
mat4 m4_rotate_z(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

mat4 m4_move(float x, float y, float z) {
  return (mat4){1, 0, 0, -x, 0, 1, 0, -y, 0, 0, 1, -z, 0, 0, 0, 1};
}

mat4 m4_rotate(float x, float y, float z) {
  return m4_mul(m4_mul(m4_rotate_x(x), m4_rotate_y(y)), m4_rotate_z(z));
}

vec3 m4_translate(vec3 p, mat4 a) {
  vec3 r;

  p.w = 1.0;

  float *vp = (float *)&p.x;
  float *vr = (float *)&r.x;
  float *ma = (float *)&a.m00;

  for (int i = 0; i < 4; i++) {
    vr[i] = 0;
    for (int j = 0; j < 4; j++) {
      vr[i] += ma[i * 4 + j] * vp[j];
    }
  }

  return r;
}

// Position:
//  x: left (-) <=> right (+)
//  y: down (-) <=> up (+)
//  z: back (-) <=> front (+)
mat4 m4_camera_lookat(vec3 pos, vec3 target, float rot) {
  // camera direction aka forward
	vec3 f = v3_normalize(v3_sub(target, pos));
  // camera up vector
	vec3 up = (vec3){sinf(rot), cosf(rot), 0.0};

	vec3 r = v3_normalize(v3_cross(f, up));
	vec3 u = v3_cross(r, f);

  return (mat4){
    r.x, u.x, f.x, 0.0,
    r.y, u.y, f.y, 0.0,
    r.z, u.z, f.z, 0.0,
    0.0, 0.0, 0.0, 1.0};
}

/* @brief Calculate distance from sphere
 * @param p point of interest
 * @param r sphere radius
 * @return distance of s from p
 */
float SphereDist(vec3 p, float r) {
  return v3_length(p) - r;
}

/* @brief Calculate distance from plane at (0,0,0) origin
 * @param p point of interest
 * @return distance of plane from p
 */
float PlaneDist(vec3 p) {
  return p.y;
}

/* @brief Calculate distance from a capsule */
float CapsuleDist(vec3 p, vec3 a, vec3 b, float r) {
  vec3 ab = v3_sub(b, a);
  vec3 ap = v3_sub(p, a);

  float t = v3_dot(ab, ap) / v3_dot(ab, ab);

  t = clamp(t, 0.0, 1.0);

  vec3 c = v3_add(a, v3_mul(ab, t));

  return v3_length(v3_sub(p, c)) - r;
}

/* @brief Calculate distance from a torus */
float TorusDist(vec3 p, float r1, float r2) {
  float r = sqrtf(p.x * p.x + p.z * p.z) - r1;
  float q = sqrtf(r * r + p.y * p.y);
  return q - r2;
}

/* @brief Calculate distance from a box */
float BoxDist(vec3 p, vec3 s) {
  vec3 d = v3_sub(v3_abs(p), s);
  float e = v3_length(v3_max(d, 0.0));
  float i = fmin(fmax(d.x, fmax(d.y, d.z)), 0.0);
  return e + i;
}

/* @brief Calculate distance from a cylinder */
float CylinderDist(vec3 p, vec3 a, vec3 b, float r) {
  vec3 ab = v3_sub(b, a);
  vec3 ap = v3_sub(p, a);

  float t = v3_dot(ab, ap) / v3_dot(ab, ab);

  vec3 c = v3_add(a, v3_mul(ab, t));
  float x = v3_length(v3_sub(p, c)) - r;
  float y = (fabs(t - 0.5) - 0.5) * v3_length(ab);
  float e = v3_length(v3_max((vec3){x, y}, 0.0));
  float i = fmin(fmax(x, y), 0.0);

  return e + i;
}
