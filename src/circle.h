// circle.h

struct circle {
	float x;
	float y;
	float radius;
};
typedef struct circle circle;

float circle_area(const circle& c);