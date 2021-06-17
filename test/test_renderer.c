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

static float place_of_vec3_z(const vec3_t *  s, const vec3_t *  e, const vec3_t *  p) {
	return (p->z - s->z) * (e->x - s->x) - (p->x - s->x) * (e->z - s->z);
}

static void test_clip_line() {
	int width		= 512;
	int height		= 512;
	vec3_t from 	= { 0.f, 0.f, 0.f };
	vec3_t to 		= { 0.f, 0.f, 1.f };
	cRGB_t bgcolor	= { 0.f, 0.f, 0.f };
	unsigned int samplestep = 1;
	float zoom 		= 1.f;
	float view 		= 4.f;
	float left 		= -view;
	float right 	= view;
	float bottom 	= -view;
	float top 		= view;
	float near		= 1.f;
	float far		= 6.f;
	renderer_t * renderer = create_renderer_perspective( width, height, &from, &to, zoom, left, right, top, bottom, near, far, &bgcolor, samplestep);

	vec3_t *v_start = &renderer->camera.clip_line.start;
	vec3_t *v_end = &renderer->camera.clip_line.end;

	vec3_t *up_start = &renderer->camera.clip_line.up_start;
	vec3_t *up_end = &renderer->camera.clip_line.up_end;

	vec3_print(v_start);
	vec3_print(v_end);
	printf("UP:\n");
	vec3_print(up_start);
	vec3_print(up_end);
	//testline 1: no intersection, in front
	vec3_t p1_s = {-.4f, .5f, 1.8f};
	vec3_t p1_e = {.8f, .2f, 1.8f};
	//testline 2: no intersection,. on the backend, complete ignored
	vec3_t p2_s = {-.4f, .5f, -1.f};
	vec3_t p2_e = {.4f, .5f, -1.f};
	//testline 3: intersection at z = 0, x = 1 and y = 0.5 
		// we have to check if one point is on the left and one on the right
	vec3_t p3_s = {-.4f, .5f, 1.9f};
	vec3_t p3_e = {.8f, .2f, .3f};

	//both in front float place_of_vec3(const vec3_t *  s, const vec3_t *  e, const vec3_t *  p);
	/*
		return (p->x - s->x) * (e->y - s->y) - (p->y - s->y) * (e->x - s->x);
	*/
	float result = place_of_vec3_z(v_start, v_end, &p1_s);
	//printf("test: %f\n", result);
	assert(result >= 0); //< 0 == INSIDE
	result = place_of_vec3_z(v_start, v_end, &p1_e);
	//printf("test: %f\n", result);
	assert(result >= 0); //< 0 == INSIDE

	result = place_of_vec3_z(v_start, v_end, &p2_s);
	//printf("test: %f\n", result);
	assert(result < 0); //< 0 == OUTSIDE
	result = place_of_vec3_z(v_start, v_end, &p2_e);
	//printf("test: %f\n", result);
	assert(result < 0); //< 0 == OUTSIDE

	result = place_of_vec3_z(v_start, v_end, &p3_s);
	//printf("test: %f\n", result);
	assert(result >= 0); //< 0 == INSIDE
	result = place_of_vec3_z(v_start, v_end, &p3_e);
	//printf("test: %f\n", result);
	assert(result < 0); //< 0 == OUTSIDE

	//bool lines_intersect_pt(vec2_t *intersec, vec2_t* l1p1, vec2_t* l1p2, vec2_t* l2p1, vec2_t* l2p2);
	vec2_t inter_p;
	vec2_t v2_start = {v_start->z, v_start->x};
	vec2_t v2_end = {v_end->z, v_end->x};
	vec2_t p2_s2 = {p1_s.z, p1_s.x};
	vec2_t p2_e2 = {p1_e.z, p1_e.x};
	assert(lines_intersect_pt( &inter_p, &v2_start, &v2_end, &p2_s2, &p2_e2) == false);
	//vec2_print(&inter_p);

	p2_s2 = (vec2_t){p2_s.z, p2_s.x};
	p2_e2 = (vec2_t){p2_e.z, p2_e.x};
	assert(lines_intersect_pt( &inter_p, &v2_start, &v2_end, &p2_s2, &p2_e2) == false);
	//vec2_print(&inter_p);

	p2_s2 = (vec2_t){p3_s.z, p3_s.x};
	p2_e2 = (vec2_t){p3_e.z, p3_e.x};
	assert(lines_intersect_pt( &inter_p, &v2_start, &v2_end, &p2_s2, &p2_e2) == true);
	printf("First Z/X interception:\n");
	vec2_print(&inter_p);

	//TODO MOVE UP VEC TO this intersection line and recalC
	vec2_t up_mov_s = { inter_p.y, up_start->y};
	vec2_t up_mov_e = { inter_p.y, up_end->y};

	float saved_z = inter_p.x;

	p2_s2 = (vec2_t){p3_s.x, p3_s.y};
	p2_e2 = (vec2_t){p3_e.x, p3_e.y};
	assert(lines_intersect_pt( &inter_p, &up_mov_s, &up_mov_e, &p2_s2, &p2_e2) == true);
	vec2_print(&inter_p);

	printf("3D intersection:\n");
	vec3_t interception = {inter_p.x, inter_p.y, saved_z};

	vec3_print(&interception);

	renderer_free(renderer);
}

int 
main() {
	#ifdef debug
		printf("Start test renderer\n");
	#endif	
	
	test_clip_line();

	/*test_renderer_creation();
	
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
	*/	
	#ifdef debug
		printf("End test renderer\n");
	#endif	
	
	return 0;
}