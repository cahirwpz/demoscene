#include "colors.inc"
#include "shapes.inc"
#include "textures.inc"

#declare R = 4;
#declare r = 2;

#declare rad = function(x,y) { atan2(x, y) / (2 * pi) }
#declare dist = function(x,y) { sqrt(x * x + y * y) }
                                                               
#declare torus_u = function(x,y,z) { rad(z, x) }
#declare torus_v = function(x,y,z) { rad(y, dist(x, z) - R) }
                                                               
camera {   
  location <0,0,10>
  look_at <0,0,0>
}                                             

torus {
  R r
  uv_mapping
  pigment {
#ifdef (VMAP)
      // torus_v(x,y,z)
      gradient -v
#end
#ifdef (UMAP)
      // torus_u(x,y,z)
      gradient u
#end
  }
  rotate 45*x
  rotate -22.5*y
  translate <-0.4, 1.25, 0>
  finish {
    ambient 1.0
  }
}

background {
  Green
}