	}
	if(dist < 0.0f) dist = 0.0f;
	out.distance = dist;
	out.iters = i;
	out.z = z;
	out.distFromOrbitTrap = distFromOrbitTrap;
	return out;
}