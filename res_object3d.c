#include <proto/exec.h>

#include "engine/mesh.h"
#include "std/memory.h"
#include "std/resource_internal.h"

RSC_TYPE(mesh, MeshT *)
RSC_ADD(mesh, NewMeshFromFile("data/whelpz.robj", MEMF_PUBLIC))
RSC_FREE(mesh, DeleteMesh)
#define mesh_init NULL

RSC_START
RSC(mesh)
RSC_END
