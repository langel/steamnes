
float curve_apply(float val, float curve) {
	return powf(val, curve);
}

float curve_get(float min, float max, float mid) {
	return log(0.5f) / log((mid-min)/(max-min));
}

float curve_inverse(float curve) {
	return 1.f / curve;
}
