void test_render_sphere(Renderer * renderer, bool isperspective) {
	Mesh * sphere = createsphere(0.5f, 50, 50);

	render_mesh(renderer, sphere);
	
	#ifdef output
		char * filename = create_string("build/__%u_sphere_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_sphere_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	renderer_clear_frame(renderer);
	
	Mat3 * rotz_mat = create_rot_z_mat(45.f);
	Mat3 * rotx_mat = create_rot_x_mat(225.f);
	mat3_mul(rotz_mat, rotx_mat);
	
	mat_mul_mesh(sphere, rotz_mat);
	free(rotz_mat);
	free(rotx_mat);
	
	render_mesh(renderer, sphere);
	#ifdef output
		filename = create_string("build/__%u_sphere_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_sphere_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	renderer_clear_frame(renderer);
	
	rotz_mat = create_rot_z_mat(65.f);
	rotx_mat = create_rot_x_mat(45.f);
	mat3_mul(rotz_mat, rotx_mat);
	mat_mul_mesh(sphere, rotz_mat);
	free(rotz_mat);
	
	render_mesh(renderer, sphere);
	
	#ifdef output
		filename = create_string("build/__%u_sphere_%uxMSAA_rot65.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_sphere_%uxMSAA_rot65.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
													
	free(rotx_mat);
	free_mesh(sphere);
}
