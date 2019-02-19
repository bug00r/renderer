void test_render_test_scene(renderer_t * renderer, bool isperspective) {
	//scene_t * scene = scene_create_test();
	scene_t * scene = scene_create_test_all();
	
	render_scene(renderer, scene);
	
	#ifdef output
		char * filename = create_string("build/__%u_testscene_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_testscene_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	renderer_clear_frame(renderer);
	
	//addBackgroundToRenderer(renderer);
	
	mat3_t * rotz_mat = create_rot_z_mat(0.f);
	mat3_t * rotx_mat = create_rot_x_mat(45.f);
	mat3_mul(rotz_mat, rotx_mat);
	
	mat_mul_scene(scene, rotz_mat);
	free(rotz_mat);
	free(rotx_mat);
	
	renderer_clear_frame(renderer);
	//addBackgroundToRenderer(renderer);
	render_scene(renderer, scene);
	
	#ifdef output
		filename = create_string("build/__%u_testscene_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_testscene_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	renderer_clear_frame(renderer);
	//addBackgroundToRenderer(renderer);
	rotx_mat = create_rot_x_mat(-45.f);
	mat_mul_scene(scene, rotx_mat);
	free(rotx_mat);
	
	rotz_mat = create_rot_y_mat(45.f);
	rotx_mat = create_rot_x_mat(45.f);
	mat3_mul(rotz_mat, rotx_mat);
	mat_mul_scene(scene, rotz_mat);
	
	free(rotz_mat);
	free(rotx_mat);
	
	render_scene(renderer, scene);
	
	#ifdef output
		filename = create_string("build/__%u_testscene_%uxMSAA_rot65.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_testscene_%uxMSAA_rot65.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	free_scene(scene);
}
