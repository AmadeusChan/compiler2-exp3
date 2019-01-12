int add(int x, int y) {
	return x + y;
}

int sub(int x, int y) {
	return add(x, -y);
}

int abs(int x) {
	if (x > 0) return x;
	return -x;
}
