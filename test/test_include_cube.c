void test_render_cube(renderer_t * renderer, bool isperspective) {

	vec3_t center = { 0.f, 0.f, 0.f };
	mesh_t * cube = create_cube3_center(&center, 1.f);
	
	render_mesh(renderer, cube);
	
	#ifdef output
		char * filename = create_string("build/__%u_cube_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cube_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	renderer_clear_frame(renderer);
	
	mat3_t * rotz_mat = create_rot_z_mat(45.f);
	mat3_t * rotx_mat = create_rot_x_mat(45.f);
	mat3_mul(rotz_mat, rotx_mat);
	
	mat_mul_mesh(cube, rotz_mat);
	free(rotz_mat);
	free(rotx_mat);
	render_mesh(renderer, cube);
	
	#ifdef output
		filename = create_string("build/__%u_cube_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cube_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	renderer_clear_frame(renderer);
	
	
	rotz_mat = create_rot_z_mat(65.f);
	rotx_mat = create_rot_x_mat(45.f);
	mat3_mul(rotz_mat, rotx_mat);
	mat_mul_mesh(cube, rotz_mat);
	free(rotz_mat);
	
	render_mesh(renderer, cube);
	
	#ifdef output
		filename = create_string("build/__%u_cube_%uxMSAA_rot65.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cube_%uxMSAA_rot65.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	renderer_clear_frame(renderer);
													
	free(rotx_mat);
	free_mesh(cube);
}