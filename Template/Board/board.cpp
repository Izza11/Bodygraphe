#include "board.h"
#include "../RtMidi/RtMidi.h"
#include <iostream>

board::board(const string& port_name) : port_name(port_name), _out(nullptr)
{
}

board::~board()
{
}

void board::set_callback(RtMidiOut* out)
{
	_out = out;
}

void board::send_message(std::vector<uchar>* data) const
{
	if (_out == nullptr)
	{
		std::cout << "Error: output callback : Instance not set to a reference!" << std::endl;
		return;
	}

	_out->sendMessage(data);
}
