#include "3d.h"
#include "ffp.h"
#include "reader.h"
#include "memory.h"
#include "io.h"

typedef struct {
  const char *name;
  BOOL (*func)(char **, Mesh3D *);
} ParserT;

static BOOL ParseImageCount(char **data, Mesh3D *mesh) {
  WORD n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  Log("[3D] Mesh has %ld images\n", (LONG)n);
  mesh->images = n;
  mesh->image = MemAlloc(sizeof(PixmapT *) * n, MEMF_PUBLIC|MEMF_CLEAR);
  return TRUE;
}

static BOOL ParseSurfaceCount(char **data, Mesh3D *mesh) {
  WORD n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  Log("[3D] Mesh has %ld surfaces\n", (LONG)n);
  mesh->surfaces = n;
  mesh->surface = MemAlloc(sizeof(MeshSurfaceT) * n, MEMF_PUBLIC|MEMF_CLEAR);
  return TRUE;
}

static BOOL ParseVertices(char **data, Mesh3D *mesh) {
  FLOAT scale = SPMul(mesh->scale, SPFlt(16));
  Point3D *vertex;
  WORD n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  Log("[3D] Mesh has %ld points\n", (LONG)n);
  mesh->vertices = n;
  mesh->vertex = MemAlloc(sizeof(Point3D) * n, MEMF_PUBLIC);

  vertex = mesh->vertex;

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    FLOAT x, y, z;

    if (!(ReadFloat(data, &x) && ReadFloat(data, &y) && ReadFloat(data, &z) &&
          EndOfLine(data)))
      return FALSE;

    vertex->x = SPFix(SPMul(x, scale));
    vertex->y = SPFix(SPMul(y, scale));
    vertex->z = SPFix(SPMul(z, scale));
    vertex++;
    n--;
  }

  return n == 0;
}

static BOOL ParseVertexUVs(char **data, Mesh3D *mesh) {
  FLOAT scale_u = SPFlt(16);
  FLOAT scale_v = SPFlt(16);
  UVCoord *uv;
  WORD n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  Log("[3D] Mesh has %ld uv coordinates\n", (LONG)n);
  mesh->uv = MemAlloc(sizeof(Point2D) * n, MEMF_PUBLIC);

  uv = mesh->uv ;

  while (NextLine(data) && !MatchString(data, "@end")) {
    FLOAT u, v;

    if (!(ReadFloat(data, &u) && ReadFloat(data, &v) && EndOfLine(data)))
      return FALSE;

    uv->u = SPFix(SPMul(u, scale_u));
    uv->v = SPFix(SPMul(v, scale_v));
    uv++;
    n--;
  }

  return n == 0;
}

static BOOL ParseFaces(char **data, Mesh3D *mesh) {
  IndexListT **faces;
  UBYTE *faceSurface;
  WORD *index;
  WORD n, m;

  if (!(ReadShort(data, &n) && ReadShort(data, &m) && EndOfLine(data)))
    return FALSE;

  Log("[3D] Mesh has %ld polygons\n", (LONG)n);

  mesh->faces = n;
  mesh->face = MemAlloc(sizeof(IndexListT *) * (n + 1) +
                        sizeof(WORD) * (m + n), MEMF_PUBLIC|MEMF_CLEAR);
  mesh->faceSurface = MemAlloc(n, MEMF_PUBLIC);

  faces = mesh->face;
  faceSurface = mesh->faceSurface;
  index = (WORD *)&mesh->face[n + 1];

  while (NextLine(data) && !MatchString(data, "@end") && n > 0) {
    IndexListT *face = (IndexListT *)index++;

    if (!ReadByte(data, faceSurface++))
      return FALSE;

    while (!EndOfLine(data)) {
      if (!ReadShort(data, index++))
        return FALSE;
      face->count++, m--;
    }

    *faces++ = face, n--;
  }

  return (n == 0) && (m == 0);
}

static BOOL ParseFaceUVs(char **data, Mesh3D *mesh) {
  IndexListT **faceUVs;
  WORD *index;
  WORD n, m;

  if (!(ReadShort(data, &n) && ReadShort(data, &m) && EndOfLine(data)))
    return FALSE;

  mesh->faceUV = MemAlloc(sizeof(IndexListT *) * (n + 1) +
                          sizeof(WORD) * (m + n), MEMF_PUBLIC|MEMF_CLEAR);

  faceUVs = mesh->faceUV;
  index = (WORD *)&mesh->faceUV[n + 1];

  while (NextLine(data) && !MatchString(data, "@end")) {
    IndexListT *faceUV = (IndexListT *)index++;

    while (!EndOfLine(data)) {
      if (!ReadShort(data, index++))
        return FALSE;
      faceUV->count++, m--;
    }

    *faceUVs++ = faceUV, n--;
  }

  return (n == 0) && (m == 0);
}

static BOOL ParseSurface(char **data, Mesh3D *mesh) {
  MeshSurfaceT *surface;
  WORD n;

  if (!(ReadShort(data, &n) && EndOfLine(data)))
    return FALSE;

  surface = &mesh->surface[n];

  while (NextLine(data) && !MatchString(data, "@end")) {
    if (MatchString(data, "color")) {
      if (!(ReadByte(data, &surface->r) && 
            ReadByte(data, &surface->g) &&
            ReadByte(data, &surface->b) &&
            EndOfLine(data)))
        return FALSE;
    } else if (MatchString(data, "side")) {
      if (!(ReadByte(data, &surface->sideness) && EndOfLine(data)))
        return FALSE;
    } else if (MatchString(data, "texture")) {
      if (!(ReadShort(data, &surface->texture) && EndOfLine(data)))
        return FALSE;
    } else {
      SkipLine(data);
    }
  }

  return TRUE;
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

__regargs Mesh3D *LoadMesh3D(char *filename, FLOAT scale) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  char *data = file;
  Mesh3D *mesh = MemAlloc(sizeof(Mesh3D), MEMF_PUBLIC|MEMF_CLEAR);

  Log("[3D] Parsing '%s' file\n", filename);

  mesh->scale = scale;

  while (NextLine(&data)) {
    ParserT *parser = TopLevelParser;
    
    for (; parser->name; parser++) {
      if (!MatchString(&data, parser->name))
        continue;
      if (parser->func(&data, mesh))
        break;
      Log("[3D] Syntax error at %ld position!\n", (LONG)(data - file));
      DeleteMesh3D(mesh);
      return NULL;
    }
  }

  return mesh;
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
