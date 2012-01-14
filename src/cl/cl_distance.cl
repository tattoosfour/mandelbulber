formulaOut CalculateDistance(float4 point, sClFractal *fractal)
{
	formulaOut out=Fractal(point, fractal);
	if(out.distance<0.0f) out.distance = 0.0f;
	if(out.distance>10.0f) out.distance = 10.0f;
	return out;
}