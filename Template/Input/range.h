#pragma once
#include "input.h"

class range : public input
{
public:
	range(const uchar& id, const string& name);
	float range_value() const;

protected:
	void change(const bool& new_state, const double& deltatime) override;
	
private:
	float _range_value;
};
