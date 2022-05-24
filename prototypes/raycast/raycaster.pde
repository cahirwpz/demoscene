/*
 * Parametric line equation:
 *
 *   x := x_0 + t * a
 *   y := y_0 + t * b
 *   z := z_0 + t * c
 */

// https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#SECTION00023000000000000000

/*
 * Plane equation:
 *
 *   A * x + B * y + C * z = D
 *
 *  ... derived from point (p) normal (n) form:
 *
 *   A = n_x
 *   B = n_y
 *   C = n_z
 *   D = -(A * p_x + B * p_y + C * p_z)
 *
 * Plane-line intersection parametric equation:
 * 
 *           A * x_0 + B * y_0 + C * z_0 - D
 *   t := - ---------------------------------
 *               A * a + B * b + C * c
 */

/*
 * Infinite cylinder (aka tunnel) equation:
 *
 *   x ^ 2 + y ^ 2 = 1
 *
 * Cylinder-line intersection parametric equation:
 *
 *   (a^2 + b^2) * t^2 + 2 * (a * x0 + b * y0) * t + (x0^2 + y0^2 - 1) = 0
 *
 *   A := 2 * (a^2 + b^2)
 *   B := -2 * (a * x0 + b * y0)
 *   C := x0^2 + y0^2 - 1
 *   t := (B +/- sqrt(sq(B) - 2 * A * C)) / A
 */

/*
 * Sphere equation:
 *
 *   x ^ 2 + y ^ 2 + z ^ 2 = 1
 *
 * Sphere-line intersection parametric equation:
 *
 *   (a^2 + b^2 + c^2) * t^2 + 2 * (a * x0 + b * y0 + c * z0) * t + (x0^2 + y0^2 + z0^2 - 1) = 0
 *
 *   A := 2 * (a^2 + b^2 + c^2)
 *   B := -2 * (a * x0 + b * y0 + c * z0)
 *   C := x0^2 + y0^2 + z0^2 - 1
 *   t := (B +/- sqrt(sq(B) - 2 * A * C)) / A
 */


abstract class RayCaster {
  PVector eye;
  PVector[] view;
 
  RayCaster() {
    eye = new PVector(0.0, 0.0, 0.0);
    view = new PVector[4];
    
    for (int i = 0; i < view.length; i++)
      view[i] = new PVector();
  }

  void setEyeX(float x) {
    eye.x = x;
  }

  void setEyeY(float y) {
    eye.y = y; 
  }

  void setEyeZ(float z) {
    eye.z = z; 
  }

  void recalcView(PMatrix3D matrix) {
    float ratio = float(height) / float(width);

    matrix.mult(new PVector(-1.0, -ratio, 1.0), view[0]);
    matrix.mult(new PVector(-1.0,  ratio, 1.0), view[1]);
    matrix.mult(new PVector( 1.0, -ratio, 1.0), view[2]);
    matrix.mult(new PVector( 1.0,  ratio, 1.0), view[3]);
  }

  abstract float intersection(PVector ray);
  abstract PVector textureCoord(PVector p);
  
  PVector castRay(PVector ray) {
    float t = intersection(ray);     
    PVector p = PVector.add(eye, PVector.mult(ray, t));
    return textureCoord(p);
  }
}

class RayPlane extends RayCaster {
  PVector N;
  float A, B, C, D;
  
  RayPlane() {
    N = new PVector();
  }
  
  void calculateForm(PVector p, PVector n) {
    N.x = n.x;
    N.y = n.y;
    N.z = n.z;
    N.normalize();
    
    A = N.x;
    B = N.y;
    C = N.z;
    D = - PVector.dot(p, N);
  }  
    
  float intersection(PVector ray) {
    float n = D - A * eye.x + B * eye.y + C * eye.z;
    float d = A * ray.x + B * ray.y + C * ray.z;
    
    if (d == 0.0)
      return Float.POSITIVE_INFINITY;

    float t = n / d;
    
    if (t < 0)
      return Float.POSITIVE_INFINITY;
      
    return t;
  }
  
  PVector textureCoord(PVector p) {
    float d = PVector.dist(p, eye) - 1.0;
    
    if (d > 0)
      d = constrain(d - 1.0, 0.0, 1.0); 
    else
      d = 0.0;
      
    PVector UAxis = new PVector(N.y, N.z, -N.x);
    PVector VAxis = UAxis.get().cross(N);
    
    float u = PVector.dot(p, UAxis) * 0.5;
    float v = PVector.dot(p, VAxis) * 0.5;

    return new PVector(u, v, d);
  }
}

class RayTunnel extends RayCaster {
  float fallOff = 6.0;
  boolean inside;
  
  void setEyeX(float x) {
    eye.x = constrain(x, -1.95, 1.95);
  }

  void setEyeY(float y) {
    eye.y = constrain(y, -.95, .95); 
  }

  void recalcView(PMatrix3D matrix) {
    super.recalcView(matrix);

    inside = eye.x * eye.x + eye.y * eye.y < 1;
  }
  
  float intersection(PVector ray) {
    float a = ray.x;
    float b = ray.y;
    float x = eye.x;
    float y = eye.y;

    float A = 2 * (sq(a) + sq(b));
    float B = -2 * (a * x + b * y);
    float C = sq(x) + sq(y) - 1;
    float Z = sq(B) - 2 * A * C;

    if (Z < 0)
      return Float.POSITIVE_INFINITY;
  
    float t = (inside ? (B + sqrt(Z)) : (B - sqrt(Z))) / A;
    
    if (t < 0)
      return Float.POSITIVE_INFINITY;
    
    return t;
  }
  
  PVector textureCoord(PVector p) {
    float u = atan2(p.x, p.y) / TWO_PI;
    float v = p.z / fallOff;
    float d = abs(p.z - eye.z) - fallOff;
    
    if (d > 0)
      d = constrain(d - 1.0, 0.0, 1.0); 
    else
      d = 0.0;

    return new PVector(u, v, d);
  }
}

class RaySphere extends RayCaster {
  float fallOff = 2.0;
  boolean inside;
  
  void recalcView(PMatrix3D matrix) {
    super.recalcView(matrix);

    inside = sq(eye.x) + sq(eye.y) + sq(eye.z) < 1;
  }
  
  float intersection(PVector ray) {
    float a = ray.x;
    float b = ray.y;
    float c = ray.z;
    float x = eye.x;
    float y = eye.y;
    float z = eye.z;

    float A = 2 * (sq(a) + sq(b) + sq(c));
    float B = -2 * (a * x + b * y + c * z);
    float C = sq(x) + sq(y) + sq(z) - 1;
    float Z = sq(B) - 2 * A * C;

    if (Z < 0)
      return Float.POSITIVE_INFINITY;
  
    float t = (inside ? (B + sqrt(Z)) : (B - sqrt(Z))) / A;
    
    if (t < 0)
      return Float.POSITIVE_INFINITY;
    
    return t;
  }
  
  PVector textureCoord(PVector p) {
    float u = atan2(p.x, p.y) / TWO_PI;
    float v = atan2(p.z, sqrt(sq(p.x) + sq(p.y))) / TWO_PI;
    float d = abs(p.z - eye.z) - fallOff;
    
    if (d > 0)
      d = constrain(d - 1.0, 0.0, 1.0); 
    else
      d = 0.0;

    return new PVector(frpart(u * 2.0), frpart(v * 2.0), d);
  }
}

