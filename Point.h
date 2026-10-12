#pragma once
class Point {
public:
	Point(int x = 0, int y = 0);
	static double distance(const Point& p1,const Point& p2);
	int x;
	int y;
};