#pragma once
class DemoRectangle
{
public:
	DemoRectangle(void);
	DemoRectangle(int a_x, int a_y, int a_width, int a_height);
	~DemoRectangle(void);
	int& X() { return x; }
	int& Y() { return y; }
	int& Width() { return width; }
	int& Height() { return height; }
private:
	int x;
	int y;
	int width;
	int height;
};

