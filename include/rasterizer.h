#ifndef RASTERIZER_H
#define RASTERIZER_H
/**

    The Rasterizer offers Interfaces for drawing Objects to an Buffer in dependency
    from some raster Options like Anti Aliasing, Wireframe or Point Frame output.

    Furthermore it offers a raster state Object which includes calculations of
    Natural Device, Raster Coordinates and a lots of other calculations. There
    are interfaces to handle precalculation for Objects. This could be interesting
    for multithreaded or KI based rasterization. Internal Flags will be handling 
    pre calculations. This could be also useful if you want to draw 2D Raster based
    Informations like Text Labels or 2D Bounding Rectangles. 

*/

#include <stdbool.h>


typedef struct {
    //ndc coords of object
    //raster coords of objects
    //(maybe)z-values of object coordinates
    int dummy;
} raster_state_t;

/**

    Summary of Options which will be take effect on the rasterization Process.

*/
typedef struct {
    //Coloring: Vertex Color or Texture Colors or maybe a user Function?
    //Antialiasing: 1, 2, 4 depends on Buffer Size
    //Render Type: solid, wireframe, point
    int dummy;
} raster_opt_t;


/**
    A Raster Context contains all needed Data for rastering Objects.
    Objects could be a point, a line or a triangle
    


*/
typedef struct {
    //frameBuffer Pointer: currently and cRGB_t Array, maybe extraction to BufferInterface
    //                for handling different type of buffers
    //zBuffer Pointer: same thoughts like frameBuffer
    //Translation Matrices: currently there is only the multiplication of all.
    //                      Useful to add projection, view and translation

    raster_opt_t opts;
    raster_state_t state;
} raster_ctx_t;

void raster(raster_ctx_t *ctx);
void raster_precalc(raster_ctx_t *ctx);

#endif