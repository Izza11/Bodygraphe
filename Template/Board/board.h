#pragma once
#include "../Input/input.h"
#include <functional>

class RtMidiOut;

class board
{
	friend class board_controller;

protected:
	explicit board(const string& port_name);
	virtual ~board();
	void set_callback(RtMidiOut* out);

public:
	const string port_name;
	virtual input* operator[](const uchar index) = 0;
	void send_message(std::vector<uchar>* data) const;

private:
	RtMidiOut* _out;
};
