#pragma once
#include "../RtMidi/RtMidi.h"
#include "board.h"

class board_controller
{
public:
	explicit board_controller(board* board, const bool& use_out = false);
	~board_controller();

	int setup();
	int open();

private:
	const bool _use_out;
	board* const _board;
	uchar _previous_id;

	// Midi In
	RtMidiIn* _in;
	string _port_in_name;
	unsigned _port_in;
	int setup_in();

	// Midi Out
	RtMidiOut* _out;
	string _port_out_name;
	unsigned _port_out;
	int setup_out();

	void update(const uchar& id, const uchar& value, const double& deltatime);
	static void callback(double deltatime, std::vector<unsigned char>* buffer, void* rawdata);
	static void error_in_callback(RtMidiError::Type type, const std::string& message, void* rawdata);
	static void error_out_callback(RtMidiError::Type type, const std::string& message, void* rawdata);
};
