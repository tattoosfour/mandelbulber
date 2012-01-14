	}
	if(distance < 0.0f) distance = 0.0f;
	out.distance = distance;
	out.iters = i;
	out.z = z;
	return out;
}