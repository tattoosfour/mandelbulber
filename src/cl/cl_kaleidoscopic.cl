		if(ifs.absX) z.x = fabs(z.x);
		if(ifs.absY) z.y = fabs(z.y);
		if(ifs.absZ) z.z = fabs(z.z);

		for(int i=0; i<9; i++)
		{
			if(ifs.enabled[i])
			{
				z = Matrix33MulFloat3(ifs.rot[i], z);
				float length = dot(z, ifs.direction[i]);
				if(length < ifs.distance[i])
				{
					z -= ifs.direction[i] * (2.0 * (length - ifs.distance[i]) * ifs.intensity[i]);
				}
			}
		}
		
		z = Matrix33MulFloat3(ifs.mainRot, z - ifs.offset) + ifs.offset;
		
		if(ifs.edge.x > 0) z.x = ifs.edge.x - fabs(ifs.edge.x - z.x);
		if(ifs.edge.y > 0) z.y = ifs.edge.y - fabs(ifs.edge.y - z.y);
		if(ifs.edge.z > 0) z.z = ifs.edge.z - fabs(ifs.edge.z - z.z);
		
		z *= ifs.scale;
		
		if(ifs.mengerSpongeMode)
		{
			z.x -= ifs.offset.x * (ifs.scale - 1.0);
			z.y -= ifs.offset.y * (ifs.scale - 1.0);
			float ztemp = ifs.offset.z * (ifs.scale - 1.0);
			if(z.z > 0.5 * ztemp) z.z -= ztemp;
		}
		else
		{
			z -= ifs.offset * (ifs.scale - 1.0);
		}
		
		DE *= ifs.scale;
		r = fast_length(z);
		
		if(r>1024.0f) 
		{
			distance = (r - 2.0) / DE;
			break;
		}