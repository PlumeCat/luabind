// circle.cpp

#include <stdexcept>

extern "C" {
#include "circle.h"
}

float circle_area(const circle& c) {
	if (c.x == 0) {
		throw std::runtime_error("harr");
	}
	return c.x * c.y;
}
