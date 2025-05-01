#include "colors.inc"
#include "shapes.inc"
#include "textures.inc"

#declare R = 4;
#declare r = 2;

camera {   
  location <0,0,10>
  look_at <0,0,0>
}                                             

light_source {
  <0,0,20>
  color White
}

torus {
  R r
  rotate 45*x
  rotate -22.5*y
  translate <-0.4, 1.25, 0>  
  uv_mapping
  pigment{      
   image_map {png "bake2.png"}
  }
  finish {
    ambient .45
  }
}

background {
  Green
}
