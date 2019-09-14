#include "3d.h"
#include "ffp.h"
#include "reader.h"
#include "memory.h"
#include "io.h"

typedef struct {
  const char *name;
  bool (*func)(char **, Mesh3D *);
} ParserT;

static bool ParseImageCount(char **data, Mesh3D *mesh) {
  short n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  Log("[3D] Mesh has %d images\n", n);
  mesh->images = n;
  mesh->image = MemAlloc(sizeof(PixmapT *) * n, MEMF_PUBLIC|MEMF_CLEAR);
  return true;
}

static bool ParseSurfaceCount(char **data, Mesh3D *mesh) {
  short n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  Log("[3D] Mesh has %d surfaces\n", n);
  mesh->surfaces = n;
  mesh->surface = MemAlloc(sizeof(MeshSurfaceT) * n, MEMF_PUBLIC|MEMF_CLEAR);
  return true;
}

static bool ParseVertices(char **data, Mesh3D *mesh) {
  float scale = SPMul(mesh->scale, SPFlt(16));
  Point3D *vertex;
  short n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  Log("[3D] Mesh has %d points\n", n);
  mesh->vertices = n;
  mesh->vertex = MemAlloc(sizeof(Point3D) * n, MEMF_PUBLIC);

  vertex = mesh->vertex;

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    float x, y, z;

    if (!(ReadFloat(data, &x) && ReadFloat(data, &y) && ReadFloat(data, &z) &&
          EndOfLine(data)))
      return false;

    vertex->x = SPFix(SPMul(x, scale));
    vertex->y = SPFix(SPMul(y, scale));
    vertex->z = SPFix(SPMul(z, scale));
    vertex++;
    n--;
  }

  return n == 0;
}

static bool ParseVertexUVs(char **data, Mesh3D *mesh) {
  float scale_u = SPFlt(16);
  float scale_v = SPFlt(16);
  UVCoord *uv;
  short n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  Log("[3D] Mesh has %d uv coordinates\n", n);
  mesh->uv = MemAlloc(sizeof(Point2D) * n, MEMF_PUBLIC);

  uv = mesh->uv ;

  while (NextLine(data) && !MatchString(data, "@end")) {
    float u, v;

    if (!(ReadFloat(data, &u) && ReadFloat(data, &v) && EndOfLine(data)))
      return false;

    uv->u = SPFix(SPMul(u, scale_u));
    uv->v = SPFix(SPMul(v, scale_v));
    uv++;
    n--;
  }

  return n == 0;
}

static bool ParseFaces(char **data, Mesh3D *mesh) {
  IndexListT **faces;
  u_char *faceSurface;
  short *index;
  short n, m;

  if (!(ReadShort(data, &n) && ReadShort(data, &m) && EndOfLine(data)))
    return false;

  Log("[3D] Mesh has %d polygons\n", n);

  mesh->faces = n;
  mesh->face = MemAlloc(sizeof(IndexListT *) * (n + 1) +
                        sizeof(short) * (m + n), MEMF_PUBLIC|MEMF_CLEAR);
  mesh->faceSurface = MemAlloc(n, MEMF_PUBLIC);

  faces = mesh->face;
  faceSurface = mesh->faceSurface;
  index = (short *)&mesh->face[n + 1];

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    IndexListT *face = (IndexListT *)index++;

    if (!ReadByteU(data, faceSurface++))
      return false;

    while (!EndOfLine(data)) {
      if (!ReadShort(data, index++))
        return false;
      face->count++, m--;
    }

    *faces++ = face, n--;
  }

  return (n == 0) && (m == 0);
}

static bool ParseFaceUVs(char **data, Mesh3D *mesh) {
  IndexListT **faceUVs;
  short *index;
  short n, m;

  if (!(ReadShort(data, &n) && ReadShort(data, &m) && EndOfLine(data)))
    return false;

  mesh->faceUV = MemAlloc(sizeof(IndexListT *) * (n + 1) +
                          sizeof(short) * (m + n), MEMF_PUBLIC|MEMF_CLEAR);

  faceUVs = mesh->faceUV;
  index = (short *)&mesh->faceUV[n + 1];

  while (NextLine(data) && !MatchString(data, "@end")) {
    IndexListT *faceUV = (IndexListT *)index++;

    while (!EndOfLine(data)) {
      if (!ReadShort(data, index++))
        return false;
      faceUV->count++, m--;
    }

    *faceUVs++ = faceUV, n--;
  }

  return (n == 0) && (m == 0);
}

static bool ParseSurface(char **data, Mesh3D *mesh) {
  MeshSurfaceT *surface;
  short n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return false;

  surface = &mesh->surface[n];

  while (NextLine(data) && !MatchString(data, "@end")) {
    if (MatchString(data, "color")) {
      if (!(ReadByteU(data, &surface->r) && 
            ReadByteU(data, &surface->g) &&
            ReadByteU(data, &surface->b) &&
            EndOfLine(data)))
        return false;
    } else if (MatchString(data, "side")) {
      if (!(ReadByteU(data, &surface->sideness) && EndOfLine(data)))
        return false;
    } else if (MatchString(data, "texture")) {
      if (!(ReadShortU(data, &surface->texture) && EndOfLine(data)))
        return false;
    } else {
      SkipLine(data);
    }
  }

  return true;
}

static ParserT TopLevelParser[] = {
  { "@image.cnt", &ParseImageCount },
  { "@surf.cnt", &ParseSurfaceCount },
  // { "@image", &ParseImage },
  { "@surf", &ParseSurface },
  { "@pnts", &ParseVertices },
  { "@pnts.uv", &ParseVertexUVs },
  { "@pols", &ParseFaces },
  { "@pols.uv", &ParseFaceUVs },
  { NULL, NULL }
};

__regargs Mesh3D *LoadMesh3D(const char *filename, float scale) {
  char *file = LoadFile(filename, MEMF_PUBLIC);

  if (file) {
    Mesh3D *mesh = MemAlloc(sizeof(Mesh3D), MEMF_PUBLIC|MEMF_CLEAR);
    char *data = file;

    Log("[3D] Parsing '%s' file\n", filename);

    mesh->scale = scale;

    while (NextLine(&data)) {
      ParserT *parser = TopLevelParser;
      
      for (; parser->name; parser++) {
        if (!MatchString(&data, parser->name))
          continue;
        if (parser->func(&data, mesh))
          break;
        Log("[3D] Syntax error at %ld position!\n", (ptrdiff_t)(data - file));
        DeleteMesh3D(mesh);
        return NULL;
      }
    }

    return mesh;
  }

  return NULL;
}

__regargs void DeleteMesh3D(Mesh3D *mesh) {
  if (mesh) {
    MemFree(mesh->vertexFace);
    MemFree(mesh->vertexNormal);
    MemFree(mesh->faceSurface);
    MemFree(mesh->faceNormal);
    MemFree(mesh->faceEdge);
    MemFree(mesh->face);
    MemFree(mesh->edge);
    MemFree(mesh->vertex);
    MemFree(mesh);
  }
}
