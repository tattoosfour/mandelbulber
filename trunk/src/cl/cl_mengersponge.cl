		z = fabs(z);
		
		if (z.x - z.y < 0)
		{
			temp = z.y;
			z.y = z.x;
			z.x = temp;
		}
		if (z.x - z.z < 0)
		{
			temp = z.z;
			z.z = z.x;
			z.x = temp;
		}
		if (z.y - z.z < 0)
		{
			temp = z.z;
			z.z = z.y;
			z.y = temp;
		}
		
		r = fast_length(z);
		if (r < colourMin) colourMin = r;
		
		z *= 3.0;

		z.x -= 2.0;
		z.y -= 2.0;
		if (z.z > 1.0) z.z -= 2.0;
		r = fast_length(z);
		DE *= 3.0;

		if (r > 1024.0f)
		{
			distance = r / fabs(DE);
			out.colourIndex = colourMin * 1000.0;
			break;
		}
		