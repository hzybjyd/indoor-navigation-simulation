#include "Point.h"
#include <math.h>

Point::Point(int x, int y):x(x),y(y) {

}
double Point::distance(const Point& p1,const Point& p2) {
	int x = (p1.x > p2.x) ? (p1.x - p2.x) : (p2.x - p1.x);
	int y = (p1.y > p2.y) ? (p1.y - p2.y) : (p2.y - p1.y);

	double result = sqrt(x * x + y * y);
	return result;
}