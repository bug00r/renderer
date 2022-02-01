void test_render_quad(renderer_t * renderer, bool isperspective) {

	vec3_t p1 = { -0.75f, -0.75f, 0.75f };
	vec3_t p2 = { 0.75f, -0.75f, 0.75f };
	vec3_t p3 = { -0.75f, 0.75f, 0.75f };
	vec3_t p4 = { 0.75f, 0.75f, 0.75f };
	mesh_t * quad = create_quad3( &p1, &p2, &p3, &p4);
	
	render_mesh(renderer, quad);
	
	#ifdef output
		char * filename = create_string("build/__%u_quad_%uxMSAA.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_quad_%uxMSAA.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	renderer_clear_frame(renderer);
	
	mat3_t * rotz_mat = create_rot_z_mat(40.f);
	mat_mul_mesh(quad, rotz_mat);
	free(rotz_mat);
	
	render_mesh(renderer, quad);
	
	#ifdef output
		filename = create_string("build/__%u_quad_%uxMSAA_rot45.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_quad_%uxMSAA_rot45.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif	
	
	renderer_clear_frame(renderer);
	
	rotz_mat = create_rot_z_mat(65.f);
	mat_mul_mesh(quad, rotz_mat);
	free(rotz_mat);
	
	render_mesh(renderer, quad);
	
	#ifdef output
		filename = create_string("build/__%u_quad_%uxMSAA_rot65.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_quad_%uxMSAA_rot65.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	free_mesh(quad);
	
}