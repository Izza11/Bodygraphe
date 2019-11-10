#pragma once
#include "input.h"

class board;

class toggle : public input
{
public:
	toggle(board* target, const uchar& id, const string& name);
	bool toggle_value() const;
	void toggle_value(const bool& value);

protected:
	void change(const bool& new_state, const double& deltatime) override;

private:
	board* const _target;
	bool _toggle_value;
	bool _ignore_input;
};
