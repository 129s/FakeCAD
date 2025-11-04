#include "Rectangle.h"

Rectangle::Rectangle(const QRectF& r) : rect_(r) { ++kCount; }

Rectangle::~Rectangle() { --kCount; }

