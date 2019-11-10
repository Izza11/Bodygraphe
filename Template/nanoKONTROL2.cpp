#include "nanoKONTROL2.h"

nanoKONTROL2::nanoKONTROL2() : board("nanoKONTROL2"),
	track_left	(new button(0x3A, "track left")),
	track_right	(new button(0x3B, "track right")),
	marker_set	(new button(0x3C, "marker set")),
	marker_left	(new button(0x3D, "marker left")),
	marker_right(new button(0x3E, "marker right")),
	cycle		(new toggle(this, 0x2E, "cycle")),
	back		(new toggle(this, 0x2B, "back")),
	forward		(new toggle(this, 0x2C, "forward")),
	stop		(new toggle(this, 0x2A, "stop")),
	play		(new toggle(this, 0x29, "play")),
	record		(new toggle(this, 0x2D, "record")),
	groups		(new group*[NANO_KONTROL2_GROUP_SIZE])
{
	// Initialize groups
	for (uchar i = 0; i < NANO_KONTROL2_GROUP_SIZE; i++)
		groups[i] = new group(this, i);
}

nanoKONTROL2::~nanoKONTROL2()
{
	delete
		track_left,
		track_right,
		marker_set,
		marker_left,
		marker_right,
		cycle,
		back,
		forward,
		stop,
		play,
		record;

	for (auto i = 0; i < NANO_KONTROL2_GROUP_SIZE; i++)
		delete groups[i];

	delete[] groups;
}

input* nanoKONTROL2::operator[](const uchar id)
{
	#define NANO_GROUP_GET(i, m) \
		if (i >= 0 && i < NANO_KONTROL2_GROUP_SIZE) \
			return groups[i] ->  m

	switch (id)
	{
		case 0x3A: return track_left;
		case 0x3B: return track_right;
		case 0x3C: return marker_set;
		case 0x3D: return marker_left;
		case 0x3E: return marker_right;
		case 0x2E: return cycle;
		case 0x2B: return back;
		case 0x2C: return forward;
		case 0x2A: return stop;
		case 0x29: return play;
		case 0x2D: return record;

		default:
			auto sid = static_cast<int>(id);
			NANO_GROUP_GET(sid, slider);

			auto index = sid - 0x10;
			NANO_GROUP_GET(index, knob);

			index = sid - 0x20;
			NANO_GROUP_GET(index, s);

			index = sid - 0x30;
			NANO_GROUP_GET(index, m);

			index = sid - 0x40;
			NANO_GROUP_GET(index, r);
			break;
	}

	return nullptr;
}
