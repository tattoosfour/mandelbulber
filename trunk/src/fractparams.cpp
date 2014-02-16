/*********************************************************
 /                   MANDELBULBER
 / definition of structures for image and effect
 / parameters
 /
 / author: Krzysztof Marczak
 / contact: buddhi1980@gmail.com
 / licence: GNU GPL v3.0
 /
 ********************************************************/
#include "fractparams.hpp"

void InitParams(parameters::container *par)
{
	//image
	par->addParam("image_width", 800, 32, 1000000, false);
	par->addParam("image_height", 600, 32, 1000000, false);
	par->addParam("tiles", 1, 1, 64, false);

	//animation
	par->addParam("start_frame", 0, 0, 99999, false);
	par->addParam("end_frame", 1000, 0, 99999, false);
	par->addParam("frames_per_keyframe", 100, 1, 99999, false);

	par->addParam("view_point", CVector3(0.0, 0.0, 0.0), true);
	par->addParam("angle_alpha", -20.0, true);
	par->addParam("angle_beta", 30.0, true);
	par->addParam("angle_gamma", 0.0, true);
	par->addParam("zoom", 2.5, 0.0, 1e15, true);
	par->addParam("perspective", 0.5, 0.0, 100.0, true);
	par->addParam("limit_min", CVector3(-10.0, -10.0, -10.0), true);
	par->addParam("limit_max", CVector3(10.0, 10.0, 10.0), true);
	par->addParam("limits_enabled", false, true);

	//general fractal and engine
	par->addParam("formula", (int)fractal::trig_optim, false);
	par->addParam("power", 9.0, true);
	par->addParam("julia_mode", false, true);
	par->addParam("julia", CVector3(0.0, 0.0, 0.0), true);
	par->addParam("N", 250, 0, 65536, true);
	par->addParam("minN", 1, 0, 65536, true);
	par->addParam("fractal_constant_factor", 1.0, true);
	par->addParam("quality", 1.0, true);
	par->addParam("smoothness", 1.0, true);
	par->addParam("iteration_threshold_mode", false, true);
	par->addParam("analityc_DE_mode", true, false);
	par->addParam("DE_factor", 1.0, 0.0, 1e15, true);
	par->addParam("slow_shading", false, true);

	//foldings
	par->addParam("tglad_folding_mode", false, true);
	par->addParam("folding_limit", 1.0, true);
	par->addParam("folding_value", 2.0, true);
	par->addParam("spherical_folding_mode", false, true);
	par->addParam("spherical_folding_fixed", 1.0, true);
	par->addParam("spherical_folding_min", 0.5, true);
	par->addParam("IFS_folding_mode", false, true);

	//image effects
	par->addParam("brightness", 1.0, 0.0, 1e15, true);
	par->addParam("contrast", 1.0, 0.0, 1e15, true);
	par->addParam("gamma", 1.0, 0.0, 1e15, true);
	par->addParam("hdr", false, true);
	par->addParam("ambient", 0.0, true);
	par->addParam("reflect", 0.0, true);
	par->addParam("shadows_intensity", 0.7, true);
	par->addParam("shadows_cone_angle", 1.0, 0.0, 180.0, true);
	par->addParam("ambient_occlusion", 1.0, true);
	par->addParam("ambient_occlusion_quality", 4, 1, 10, true);
	par->addParam("ambient_occlusion_fast_tune", 1.0, true);
	par->addParam("ambient_occlusion_enabled", false, true);
	par->addParam("fast_ambient_occlusion_mode", false, true);
	par->addParam("shading", 1.0, true);
	par->addParam("specular", 1.0, true);
	par->addParam("glow_intensity", 0.0, true);
	par->addParam("glow_color", 1, (sRGB){40984, 44713, 49490}, true);
	par->addParam("glow_color", 2, (sRGB){57192, 60888, 62408}, true);
	par->addParam("background_color", 1, (sRGB){0, 38306, 65535}, true);
	par->addParam("background_color", 2, (sRGB){65535, 65535, 65535}, true);
	par->addParam("background_color", 3, (sRGB){0, 10000, 500}, true);
	par->addParam("fog_color", 1, (sRGB){20000, 20000, 20000}, true);
	par->addParam("fog_color", 2, (sRGB){0, 30000, 65535}, true);
	par->addParam("fog_color", 3, (sRGB){65535, 65535, 65535}, true);
	par->addParam("textured_background", false, true);
	par->addParam("background_as_fuldome", false, false);
	par->addParam("shadows_enabled", true, true);

	//coloring
	par->addParam("fractal_color", true, true);
	par->addParam("coloring_random_seed", 123456, false);
	par->addParam("coloring_saturation", 1.0, true);
	par->addParam("coloring_speed", 1.0, true);
	par->addParam("coloring_palette_offset", 0.0, 0.0, 256.0, true);

	//fog
	par->addParam("post_fog_enabled", false, true);
	par->addParam("post_fog_visibility", 20.0, true);
	par->addParam("post_fog_color", (sRGB){59399, 61202, 65535}, true);
	par->addParam("post_SSAO_enabled", true, true);
	par->addParam("post_SSAO_quality", 20, 1, 100, true);
	par->addParam("post_DOF_enabled", false, true);
	par->addParam("post_DOF_focus", 166.0, 0.0, 200.0, true);
	par->addParam("post_DOF_radius", 10.0, 0.0, 200.0, true);

	//main light
	par->addParam("main_light_intensity", 1.0, true);
	par->addParam("main_light_alfa", -45.0, true);
	par->addParam("main_light_beta", 45.0, true);
	par->addParam("main_light_colour", (sRGB){65535, 65535, 65535}, true);

	//aux lights
	par->addParam("aux_light_intensity", 1.0, true);
	par->addParam("aux_light_random_seed", 1234, false);
	par->addParam("aux_light_number", 0, 0, 9999, false);
	par->addParam("aux_light_max_dist", 0.1, 0.0, 1e15, false);
	par->addParam("aux_light_distribution_radius", 3.0, 0.0, 1e15, false);
	par->addParam("aux_light_random_center", CVector3(0.0, 0.0, 0.0), true);
	par->addParam("aux_light_visibility", 1.0, true);

	par->addParam("aux_light_predefined_position", 1, CVector3(3.0, -3.0, -3.0), true);
	par->addParam("aux_light_predefined_position", 2, CVector3(-3.0, -3.0, 0.0), true);
	par->addParam("aux_light_predefined_position", 3, CVector3(-3.0, 3.0, 1.0), true);
	par->addParam("aux_light_predefined_position", 4, CVector3(0.0, -1.0, 3.0), true);
	par->addParam("aux_light_predefined_intensity", 1, 1.3, true);
	par->addParam("aux_light_predefined_intensity", 2, 1.0, true);
	par->addParam("aux_light_predefined_intensity", 3, 3, true);
	par->addParam("aux_light_predefined_intensity", 4, 2, true);
	par->addParam("aux_light_predefined_enabled", 1, false, true);
	par->addParam("aux_light_predefined_enabled", 2, false, true);
	par->addParam("aux_light_predefined_enabled", 3, false, true);
	par->addParam("aux_light_predefined_enabled", 4, false, true);
	par->addParam("aux_light_predefined_colour", 1, (sRGB){45761, 53633, 59498}, true);
	par->addParam("aux_light_predefined_colour", 2, (sRGB){62875, 55818, 50083}, true);
	par->addParam("aux_light_predefined_colour", 3, (sRGB){64884, 64928, 48848}, true);
	par->addParam("aux_light_predefined_colour", 4, (sRGB){52704, 62492, 45654}, true);

	//IFS formula
	par->addParam("IFS_scale", 2.0, true);
	par->addParam("IFS_rot_alpha", 0.0, true);
	par->addParam("IFS_rot_beta", 0.0, true);
	par->addParam("IFS_rot_gamma", 0.0, true);
	par->addParam("IFS_offset", CVector3(1.0, 0.0, 0.0), true);
	par->addParam("IFS_edge", CVector3(0.0, 0.0, 0.0), true);
	par->addParam("IFS_abs_X", false, true);
	par->addParam("IFS_abs_Y", false, true);
	par->addParam("IFS_abs_Z", false, true);
	par->addParam("IFS_menger_sponge_mode", false, true);

	for(int i = 0; i < IFS_VECTOR_COUNT; i++)
	{
		par->addParam("IFS_direction", i, CVector3(1.0, 0.0, 0.0), true);
		par->addParam("IFS_alpha", i, 0.0, true);
		par->addParam("IFS_beta", i, 0.0, true);
		par->addParam("IFS_gamma", i, 0.0, true);
		par->addParam("IFS_distance", i, 0.0, true);
		par->addParam("IFS_intensity", i, 1.0, true);
		par->addParam("IFS_enabled", i, false, true);
	}

	//Hybrid formula
	for(int i = 0; i < HYBRID_COUNT; i++)
	{
		par->addParam("hybrid_formula", i, (int)fractal::none, true);
	}
}


