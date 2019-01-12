int neg(int x) {
	return -x;
}

int abs(int x) {
	int y;
	if (x >= 0) {
		y = x;
	} else {
		y = neg(x);
	}
	return y;
}

void test(int x) {
	int addx = x + 1;
	int subx = x - 1;
	int mulx = addx * subx;
	int shlx = addx << 1;
}
