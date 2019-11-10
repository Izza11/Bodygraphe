#include "board_controller.h"

void board_controller::update(const uchar& id, const uchar& value, const double& deltatime)
{
	auto* target = (*_board)[id];

	if (target == nullptr)
	{
		std::cout << "Input ID \"" << static_cast<int>(id) << "\" not found!" << std::endl;
		return;
	}

	auto new_state = id != _previous_id;
	target->value(value);
	target->change(new_state, deltatime);
	if (target->callback_in != nullptr)
		target->callback_in(this, target, new_state, deltatime);
	_previous_id = id;
}


board_controller::board_controller(board* board, const bool& use_out) : _use_out(use_out), _board(board)
{
	_in = nullptr;
	_out = nullptr;
	_port_in = UINT_MAX;
	_port_out = UINT_MAX;
	_previous_id = 0;
}

board_controller::~board_controller()
{
	if (_in != nullptr)
		delete _in;

	if (_out != nullptr)
		delete _out;
}

int board_controller::setup()
{
	// Setup input
	auto result = setup_in();
	if (result != EXIT_SUCCESS)
		return result;

	// Setup output
	if (_use_out)
		result = setup_out();

	return result;
}

int board_controller::setup_in()
{
	try
	{
		_in = new RtMidiIn();

		// Get ports
		const auto name_size = _board->port_name.size();
		const auto port_count = _in->getPortCount();
		if (port_count == 0)
		{
			std::cout << "Error: There are no in ports found.." << std::endl;
			return EXIT_FAILURE;
		}

		// Log
		std::cout << "There are " << port_count << " input port";
		if (port_count > 1)
			std::cout << "s";
		std::cout << " found:" << std::endl;

		for (auto i = 0u; i < port_count; ++i)
		{
			// Is target pad
			auto port_name = _in->getPortName(i);
			if (port_name.substr(0, name_size) == _board->port_name)
			{
				_port_in = i;
				_port_in_name = port_name;
			}

			// Log
			std::cout << (_port_in == i ? "   ** " : "    - ") << port_name << std::endl;
		}
	}
	catch (RtMidiError& error)
	{
		std::cout << "Error: ";
		error.printMessage();
		return EXIT_FAILURE;
	}

	if (_port_in == UINT_MAX)
	{
		std::cout << "Error: There is no valid input port found.." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << std::endl << std::endl;
	return EXIT_SUCCESS;
}

int board_controller::setup_out()
{
	try
	{
		_out = new RtMidiOut();

		// Get ports
		const auto name_size = _board->port_name.size();
		const auto port_count = _out->getPortCount();
		if (port_count == 0)
		{
			std::cout << "Error: There are no out ports found.." << std::endl;
			return EXIT_FAILURE;
		}

		// Log
		std::cout << "There are " << port_count << " input port";
		if (port_count > 1)
			std::cout << "s";
		std::cout << " found:" << std::endl;

		for (auto i = 0u; i < port_count; ++i)
		{
			// Is target pad
			auto port_name = _out->getPortName(i);
			if (port_name.substr(0, name_size) == _board->port_name)
			{
				_port_out = i;
				_port_out_name = port_name;
			}

			// Log
			std::cout << (_port_out == i ? "   ** " : "    - ") << port_name << std::endl;
		}
	}
	catch (RtMidiError& error)
	{
		std::cout << "Error: ";
		error.printMessage();
		return EXIT_FAILURE;
	}

	if (_port_out == UINT_MAX)
	{
		std::cout << "Error: There is no valid out port found.." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << std::endl << std::endl;
	return EXIT_SUCCESS;
}

int board_controller::open()
{
	try
	{
		if (_in != nullptr)
		{
			_in->openPort(_port_in, _port_in_name);
			_in->setCallback(&callback, this);
			_in->setErrorCallback(&error_in_callback, this);
			_in->ignoreTypes(false, false, false);
		}

		if (_out != nullptr)
		{
			_out->openPort(_port_out, _port_out_name);
			_out->setErrorCallback(&error_out_callback, this);
			_board->set_callback(_out);
		}
	}
	catch (...)
	{
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

void board_controller::callback(double deltatime, std::vector<unsigned char>* buffer, void* rawdata)
{
	std::cout << "Receiving: ";
	auto buff = *buffer;
	for (auto k : buff)
	{
		std::cout << static_cast<int>(k) << " : ";
	}
	std::cout << std::endl;

	try
	{
		auto* const data = static_cast<board_controller*>(rawdata);
		data->update((*buffer)[1], (*buffer)[2], deltatime);
	}
	catch (...)
	{
		std::cout << "An error occurred in retreiving data from the device!" << std::endl;
	}
}

void board_controller::error_in_callback(RtMidiError::Type type, const string& message, void* rawdata)
{
	std::cout << "Error Receiving: " << type << " :: " << message << std::endl;
}

void board_controller::error_out_callback(RtMidiError::Type type, const string& message, void* rawdata)
{
	std::cout << "Error Sending: " << type << " :: " << message << std::endl;
}
