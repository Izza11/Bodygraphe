#pragma once
#include "input.h"

class button : public input
{
protected:
	void change(const bool& new_state, const double& deltatime) override;

public:
	const static uchar threshold = 64;

	explicit button(const uchar& id, const string& name);
	virtual ~button();

	bool on() const;
};
