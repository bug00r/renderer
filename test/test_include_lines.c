void test_render_lines(Renderer * renderer, bool isperspective) {
	
	Vec3 p = { -1.f, -1.0f, 0.f };
	Vec3 p2 = { -.8f, 1.0f, 0.f };
	Mesh * line = create_line3(&p, &p2);
	
	ColorRGB red = {1.f, 0.f, 0.f};
	set_shape_color(line->shapes[0], &red);
	
	float step = 0.f;
	for ( int i = 0; i < 20; ++i, step+=0.05f) {
		line->shapes[0]->vertices[1]->vec.x += step;
		line->shapes[0]->vertices[1]->vec.y -= step;
		line->shapes[0]->vertices[1]->vec.z += step;
		render_mesh(renderer, line);
	}
	
	#ifdef output
		char * filename = create_string("build/__%u_line_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_line_%uxMSAA.ppm", isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	free_mesh(line);
}
