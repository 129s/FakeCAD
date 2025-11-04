#include "Circle.h"

Circle::Circle(const QPointF& c, double r) : center_(c), radius_(r) { ++kCount; }

Circle::~Circle() { --kCount; }

