#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "renderer.h"
#include "scene_builder.h"
#include "texture.h"

char * create_string(const char * msg, ...) {
	va_list vl;
	va_start(vl, msg);
	int buffsize = vsnprintf(NULL, 0, msg, vl);
	va_end(vl);
	buffsize += 1;
	char * buffer = malloc(buffsize);
	va_start(vl, msg);
	vsnprintf(buffer, buffsize, msg, vl);
	va_end( vl);
	return buffer;
}

#if 0
	//include renderer obj functions
#endif
#include "test_include_renderer.c"
#include "test_include_renderer_creation.c"
#include "test_include_add_bkgrnd.c"
#include "test_include_points.c"
#include "test_include_lines.c"
#include "test_include_triangle.c"
#include "test_include_quad.c"
#include "test_include_cube.c"
#include "test_include_sphere.c"
#include "test_include_test_scene.c"
#include "test_include_texture_cube.c"
#include "test_include_cylinder.c"
#include "test_include_cone.c"
#include "test_include_square_block.c"

int 
main() {
	#ifdef debug
		printf("Start test renderer\n");
	#endif	
	
	test_renderer_creation();
	
	renderer_t *renderer = create_test_base_renderer(1);
	
	test_render_cylinder(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_triangle(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_test_scene(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cone(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_square_block(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_quad(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cube(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_sphere(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_points(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_lines(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	renderer_free(renderer);
    
	renderer = create_test_base_renderer(4);
	
	//addBackgroundToRenderer(renderer);
	test_render_test_scene(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cylinder(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_triangle(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cone(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_square_block(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_quad(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cube(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_sphere(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_points(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_lines(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	renderer_free(renderer);
	
	//perspective rendering
	renderer = create_test_base_renderer_perspective(1);
	
	test_render_test_scene(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cylinder(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cone(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_triangle(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_square_block(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_quad(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cube(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_sphere(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_points(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_lines(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	renderer_free(renderer);
	
	renderer = create_test_base_renderer_perspective(4);
	
	test_render_test_scene(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cylinder(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cone(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_triangle(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_square_block(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_quad(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_cube(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_sphere(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_points(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	test_render_lines(renderer, renderer->projection == RP_PERSPECTIVE);
	renderer_clear_frame(renderer);
	
	renderer_free(renderer);
	
	test_render_texture_cube(1);
	test_render_texture_cube(4);
	
	test_render_texture_cube_perspective(1);
	test_render_texture_cube_perspective(4);
		
	#ifdef debug
		printf("End test renderer\n");
	#endif	
	
	return 0;
}