formulaOut CalculateDistance(float3 point, sClFractal *fractal)
{
	float distance;
	float delta = 1e-6;
	float3 dr = 0.0;
	formulaOut out = Fractal(point, fractal);

	if (out.iters == fractal->N)
	{
		distance = 0.0f;
	}
	else
	{
		float r = out.distance;
		float r1 = Fractal(point + (float3)
		{ delta, 0.0, 0.0}, fractal).distance;
		dr.x = fabs(r1 - r) / delta;
		float r2 = Fractal(point + (float3)
		{ 0.0, delta, 0.0}, fractal).distance;
		dr.y = fabs(r2 - r) / delta;
		float r3 = Fractal(point + (float3)
		{ 0.0, 0.0, delta}, fractal).distance;
		dr.z = fabs(r3 - r) / delta;
		float d = fast_length(dr);

		if (isinf(r) || isinf(d))
		{
			distance = 0.0f;
		}
		else
		{
			distance = 0.5 * r * native_log(r) / d;
		}
		if (distance < 0.0f) distance = 0.0f;
		if (distance > 10.0f) distance = 10.0f;
	}
	out.distance = distance;
	return out;
}