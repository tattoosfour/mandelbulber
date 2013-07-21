formulaOut CalculateDistance(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam)
{
	float distance;
	float delta = 1e-6;
	float3 dr = 0.0;
	formulaOut out = Fractal(consts, point, calcParam);

	if (out.iters == calcParam->N)
	{
		distance = 0.0f;
	}
	else
	{
		float r = out.distance;
		float r1 = Fractal(consts, point + (float3)
		{ delta, 0.0, 0.0}, calcParam).distance;
		dr.x = fabs(r1 - r) / delta;
		float r2 = Fractal(consts, point + (float3)
		{ 0.0, delta, 0.0}, calcParam).distance;
		dr.y = fabs(r2 - r) / delta;
		float r3 = Fractal(consts, point + (float3)
		{ 0.0, 0.0, delta}, calcParam).distance;
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