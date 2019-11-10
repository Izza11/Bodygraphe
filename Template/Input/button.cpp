#include "button.h"
#include <iostream>

void button::change(const bool& new_state, const double& deltatime)
{
	std::cout << "Holy shit" << std::endl;
}

button::button(const uchar & id, const string & name) : input(id, name)
{
}

button::~button()
{

}

bool button::on() const
{
	return value() >= threshold;
}
