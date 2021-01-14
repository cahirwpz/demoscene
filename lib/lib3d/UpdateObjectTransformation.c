#include <3d.h>
#include <fx.h>

void UpdateObjectTransformation(Object3D *object) {
  Point3D *rotate = &object->rotate;
  Point3D *scale = &object->scale;
  Point3D *translate = &object->translate;

  /* object -> world: Rx * Ry * Rz * S * T */
  {
    Matrix3D *m = &object->objectToWorld;
    LoadRotate3D(m, rotate->x, rotate->y, rotate->z);
    Scale3D(m, scale->x, scale->y, scale->z);
    Translate3D(m, translate->x, translate->y, translate->z);
  }

  /* world -> object: T * S * Rz * Ry * Rx */
  {
    Matrix3D *m = &object->worldToObject;
    Matrix3D m_scale, m_rotate;

    LoadIdentity3D(&m_scale);

    /*
     * Translation is formally first, and we achieve that in ReverseTransform3D.
     * Matrix3D is used only to store the vector.
     */
    m_scale.x = -translate->x;
    m_scale.y = -translate->y;
    m_scale.z = -translate->z;

    m_scale.m00 = div16(1 << 24, scale->x);
    m_scale.m11 = div16(1 << 24, scale->y);
    m_scale.m22 = div16(1 << 24, scale->z);

    LoadReverseRotate3D(&m_rotate, -rotate->x, -rotate->y, -rotate->z);
    Compose3D(m, &m_scale, &m_rotate);
  }

  /* calculate camera position in object space */ 
  {
    Matrix3D *M = &object->worldToObject;
    short *camera = (short *)&object->camera;

    /* camera position in world space is (0, 0, 0) */
    short cx = M->x;
    short cy = M->y;
    short cz = M->z;

    *camera++ = normfx(M->m00 * cx + M->m01 * cy + M->m02 * cz);
    *camera++ = normfx(M->m10 * cx + M->m11 * cy + M->m12 * cz);
    *camera++ = normfx(M->m20 * cx + M->m21 * cy + M->m22 * cz);
  }
}
