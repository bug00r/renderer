void test_render_cone(renderer_t * renderer, bool isperspective) {
	//	mesh_t * createcone(float radius, float height, unsigned int lats, bool showbottom);
	mesh_t * cone = createcone(0.5f, 1.5f, 10, true);
	
	render_mesh(renderer, cone);
	
	#ifdef output
		char * filename = create_string("build/__%u_cone_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cone_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	mat3_t * rotx_mat = create_rot_x_mat(-45.f);
	mat_mul_mesh(cone, rotx_mat);
	free(rotx_mat);
	
	
	renderer_clear_frame(renderer);
	render_mesh(renderer, cone);
	
	#ifdef output
		filename = create_string("build/__%u_cone_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cone_%uxMSAA_rot45.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	free_mesh(cone);
}
