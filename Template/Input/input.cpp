#include "input.h"
#include <iostream>

input::input(const uchar& id, const string& name) : _id(id), _name(name)
{
	_value = 0;
}

input::~input()
{

}

std::string input::name() const
{
	return _name;
}

uchar input::id() const
{
	return _id;
}

uchar input::value() const
{
	return _value;
}

void input::value(const uchar& value)
{
	_value = value;
}

bool input::is_min() const
{
	return _value == 0;
}

bool input::is_max() const
{
	return _value == SCHAR_MAX;
}
