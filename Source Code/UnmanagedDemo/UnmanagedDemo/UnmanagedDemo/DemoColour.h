#pragma once
class DemoColour
{
public:
	DemoColour(void);
	DemoColour(float, float, float, float);
	~DemoColour(void);
	float& R() { return rgba[0]; }
	float& G() { return rgba[1]; }
	float& B() { return rgba[2]; }
	float& A() { return rgba[3]; }
	float* AsArray() { return rgba; }
private:
	float rgba[4];
};

