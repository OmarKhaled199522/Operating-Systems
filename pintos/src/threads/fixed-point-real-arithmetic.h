#include <stdint.h>

#define F 16384



int to_fixed_pt(int a);
int to_int_floor(int a);
int to_int_round(int a);
int add_int(int a, int b);
int add_fixed(int a, int b);
int sub_int(int a, int b);
int sub_fixed(int a, int b);
int mul_int(int a, int b);
int mul_fixed(int a, int b);
int div_int(int a, int b);
int div_fixed(int a, int b);
int to_fixed_pt(int a)
{
	return a * F;
}

int to_int_floor(int a)
{
	return a / F;
}

int to_int_round(int a)
{
	if (a>=0)
		return (a + F/2) / F;
	else
		return (a - F/2) / F;
}

int add_int(int a, int b)
{
	return a + (b * F);
}

int add_fixed(int a, int b)
{
	return a + b;
}

int sub_int(int a, int b)
{
	return a - (b * F);
}

int sub_fixed(int a, int b)
{
	return a - b;
}

int mul_int(int a, int b)
{
	return a * b;
}

int mul_fixed(int a, int b)
{
	return ((int64_t) a) * b / F;
}

int div_int(int a, int b)
{
	return a / b;
}

int div_fixed(int a, int b)
{
	return ((int64_t) a) * F / b;
}
