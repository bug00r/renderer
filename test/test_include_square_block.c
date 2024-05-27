void test_render_square_block(Renderer * renderer, bool isperspective) {
	#ifdef debug
		printf(">> test render square block\n");
	#endif

	Vec3 center = { 0.f, 0.f, -2.f };
	Mesh * square_block = create_square_block(&center, 1.f, 2.f, 3.f, 1, 2, 3);
	
	render_mesh(renderer, square_block);
	
	#ifdef output
		char * filename = create_string("build/__%u_square_block_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_square_block_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	#ifdef debug
		printf("without rotation:\n");
		printf("min_z:\t%f\n", renderer->min_z);
		printf("max_z:\t%f\n", renderer->max_z);
	#endif
	
	Mat3 * rotz_mat = create_rot_z_mat(0.f);
	Mat3 * rotx_mat = create_rot_x_mat(20.f);
	Mat3 * roty_mat = create_rot_y_mat(20.f);
	mat3_mul(rotz_mat, rotx_mat);
	mat3_mul(rotz_mat, roty_mat);
	mat_mul_mesh(square_block, rotz_mat);
	
	free(rotz_mat);
	free(rotx_mat);
	free(roty_mat);	
	
	renderer_clear_frame(renderer);
	render_mesh(renderer, square_block);
	
	#ifdef output
		filename = create_string("build/__%u_square_block_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_square_block_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	#ifdef debug
		printf("with rotation:\n");
		printf("min_z:\t%f\n", renderer->min_z);
		printf("max_z:\t%f\n", renderer->max_z);
	#endif
	
	free_mesh(square_block);
	
	#ifdef debug
		printf("<< test render square block\n");
	#endif
}
