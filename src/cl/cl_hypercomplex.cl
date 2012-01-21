		newz.x = z.x * z.x - z.y * z.y - z.z * z.z - z.w * z.w;
		newz.y = 2.0 * z.x * z.y - 2.0 * z.w * z.z;
		newz.z = 2.0 * z.x * z.z - 2.0 * z.y * z.w;
		newz.w = 2.0 * z.x * z.w - 2.0 * z.y * z.z;
		z = newz + c;
		r = fast_length(z);
		if(r>4000.0f || any(isinf(z))) 
		{
			distance = r;
			break;
		}
		