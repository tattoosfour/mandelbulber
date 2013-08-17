formulaOut CalculateDistance(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam)
{
	float distance;
	float delta = 0.5e-6f;
	float3 dr = 0.0;
	formulaOut out = Fractal(consts, point, calcParam);

	if (out.iters == calcParam->N)
	{
		distance = 0.0f;
	}
	else
	{
		float r = out.distance;
		float r11 = Fractal(consts, point + (float3)	{ delta, 0.0, 0.0}, calcParam).distance;
		float r12 = Fractal(consts, point + (float3)	{ -delta, 0.0, 0.0}, calcParam).distance;
		dr.x = min(fabs(r11 - r), fabs(r12 - r)) / delta;
		float r21 = Fractal(consts, point + (float3)	{ 0.0, delta, 0.0}, calcParam).distance;
		float r22 = Fractal(consts, point + (float3)	{ 0.0, -delta, 0.0}, calcParam).distance;
		dr.y = min(fabs(r21 - r), fabs(r22 - r)) / delta;
		float r31 = Fractal(consts, point + (float3)	{ 0.0, 0.0, delta}, calcParam).distance;
		float r32 = Fractal(consts, point + (float3)	{ 0.0, 0.0, -delta}, calcParam).distance;
		dr.z = min(fabs(r31 - r), fabs(r32 - r)) / delta;
		float d = length(dr);

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