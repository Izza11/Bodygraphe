#include "toggle.h"
#include "../Board/board.h"
#include "button.h"

toggle::toggle(board* target, const uchar& id, const string& name) : input(id, name), _target(target)
{
	_toggle_value = false;
	_ignore_input = false;
}

bool toggle::toggle_value() const
{
	return _toggle_value;
}

void toggle::toggle_value(const bool& value)
{
	_toggle_value = value;

	std::vector<uchar> message =
	{
		0xB0,
		id(),
		_toggle_value ? uchar(0x7F) : uchar(0x00)
	};

	_target->send_message(&message);
}

void toggle::change(const bool& new_state, const double& deltatime)
{
	auto target = value() > button::threshold;
	if (!_toggle_value && target)
	{
		toggle_value(true);
		_ignore_input = true;
	}
	else if (!_ignore_input && _toggle_value && !target)
		toggle_value(false);
	else if (_ignore_input && _toggle_value && !target)
		_ignore_input = false;
}
