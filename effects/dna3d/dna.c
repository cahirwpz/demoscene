
#define DNA_VERTS 40

static Point3D _dna_pnts[DNA_VERTS];

Mesh3D dna3d = {
  .vertices = DNA_VERTS,
  .faces = 0,
  .edges = 0,
  .surfaces = 0,
  .images = 0,
  .vertex = _dna_pnts,
  .uv = NULL,
  .faceNormal = NULL,
  .faceSurface = NULL,
  .vertexNormal = NULL,
  .edge = NULL,
  .face = NULL,
  .faceEdge = NULL,
  .faceUV = NULL,
  .vertexFace = NULL,
  .image = NULL,
  .surface = NULL,
};


static void GenerateDnaPoints(Mesh3D *dna, short radius, short angle_offset, short angle_start, short z_offset) {
    short angle = angle_start;
    short z = 0;
    short n = dna->vertices;
    int i = 0;
    for (; i < n; i += 4) {
        dna->vertex[i].x = normfx(SIN(angle) * radius);
        dna->vertex[i].y = normfx(COS(angle) * radius);
        dna->vertex[i].z = z;
        dna->vertex[i].pad = 0;
        dna->vertex[i + 1].x = normfx(SIN(angle + SIN_PI) * radius);
        dna->vertex[i + 1].y = normfx(COS(angle + SIN_PI) * radius);
        dna->vertex[i + 1].z = z;
        dna->vertex[i + 1].pad = 0;

        dna->vertex[i + 2].x = normfx(SIN(-angle) * radius);
        dna->vertex[i + 2].y = normfx(COS(-angle) * radius);
        dna->vertex[i + 2].z = -z;
        dna->vertex[i + 2].pad = 0;
        dna->vertex[i + 3].x = normfx(SIN(-angle - SIN_PI) * radius);
        dna->vertex[i + 3].y = normfx(COS(-angle - SIN_PI) * radius);
        dna->vertex[i + 3].z = -z;
        dna->vertex[i + 3].pad = 0;
        
        angle += angle_offset;
        z += z_offset;
    }
  }
