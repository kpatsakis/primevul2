GLSurfaceOzoneSurfaceless::~GLSurfaceOzoneSurfaceless() {
  Destroy();  // The EGL surface must be destroyed before SurfaceOzone.
}
