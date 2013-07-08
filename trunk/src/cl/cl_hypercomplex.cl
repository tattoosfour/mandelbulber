		newz.x = z.x * z.x - z.y * z.y - z.z * z.z - w * w;
		newz.y = 2.0 * z.x * z.y - 2.0 * w * z.z;
		newz.z = 2.0 * z.x * z.z - 2.0 * z.y * w;
		neww = 2.0 * z.x * w - 2.0 * z.y * z.z;
		z = newz + c;
		w = neww;
		r = fast_length(z);
		r = sqrt(r * r + w * w);
		if (r < colourMin) colourMin = r;
		if(r>4000.0f || any(isinf(z))) 
		{
			distance = r;
			out.colourIndex = colourMin * 5000.0;
			break;
		}
		