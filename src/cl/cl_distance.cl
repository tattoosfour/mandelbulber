float CalculateDistance(float4 point, sClFractal *fractal)
{
	float distance = Fractal(point, fractal);
	if(distance<0.0f) distance = 0.0f;
	if(distance>10.0f) distance = 10.0f;
	return distance;
}