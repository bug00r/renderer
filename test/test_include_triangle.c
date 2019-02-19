void test_render_triangle(renderer_t * renderer, bool isperspective) {
	
	vec3_t p1 = { -0.75f, -0.75f, 0.75f };
	vec3_t p2 = { 0.75f, -0.55f, 0.25f };
	vec3_t p3 = { 0.05f, 0.75f, -0.75f };
	
	//1. add triangle
	mesh_t * triangle = create_triangle3(&p1, &p2, &p3);
	
	render_mesh(renderer, triangle);
	
	#ifdef output
		char * filename = create_string("build/__%u_triangle_%uxMSAA.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
	#endif
	free_mesh(triangle);
}