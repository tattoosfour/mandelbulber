formulaOut CalculateDistance(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam)
{
	float distance;
	float delta = length(point)*1e-5f;
	float3 dr = 0.0;
	formulaOut out = Fractal(consts, point, calcParam);

	if (out.iters == calcParam->N)
	{
		distance = 0.0f;
	}
	else
	{
		float r = length(out.z);
		float r11 = length(Fractal(consts, point + (float3)	{ delta, 0.0, 0.0}, calcParam).z);
		float r12 = length(Fractal(consts, point + (float3)	{ -delta, 0.0, 0.0}, calcParam).z);
		dr.x = min(fabs(r11 - r), fabs(r12 - r)) / delta;
		float r21 = length(Fractal(consts, point + (float3)	{ 0.0, delta, 0.0}, calcParam).z);
		float r22 = length(Fractal(consts, point + (float3)	{ 0.0, -delta, 0.0}, calcParam).z);
		dr.y = min(fabs(r21 - r), fabs(r22 - r)) / delta;
		float r31 = length(Fractal(consts, point + (float3)	{ 0.0, 0.0, delta}, calcParam).z);
		float r32 = length(Fractal(consts, point + (float3)	{ 0.0, 0.0, -delta}, calcParam).z);
		dr.z = min(fabs(r31 - r), fabs(r32 - r)) / delta;
		float d = length(dr);

		if (isinf(r) || isinf(d))
		{
			distance = 0.0f;
		}
		else
		{
			if(consts->fractal.linearDEmode)
			{
				distance = 0.5 * r  / d;
			}
			else
			{
				distance = 0.5 * r * native_log(r) / d;	
			}
		}
		if (distance < 0.0f) distance = 0.0f;
		if (distance > 10.0f) distance = 10.0f;
	}
	out.distance = distance;
	return out;
}