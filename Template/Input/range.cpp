#include "range.h"

range::range(const uchar& id, const string& name) : input(id, name)
{
	_range_value = 0.0f;
}

float range::range_value() const
{
	return _range_value;
}

void range::change(const bool & new_state, const double & deltatime)
{
	_range_value = static_cast<float>(value()) / SCHAR_MAX;
}
