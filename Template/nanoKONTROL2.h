/* 
 * Additional info:
 *		http://www.korg.com/us/support/download/product/0/159/
 * 
 * 
 * 
 * SliderGroup [8] <number + index>
 *	- Slider	:  0 : 0x00
 *  - Knob		: 16 : 0x10
 *  - S			: 32 : 0x20
 *  - M			: 48 : 0x30
 *  - R			: 64 : 0x40
 * 
 * Left:
 *	- Track:
 *		Left	: 58 : 0x3A
 *		Right	: 59 : 0x3B
 *		
 *	- Marker:
 *		Set		: 60 : 0x3C
 *		Left	: 61 : 0x3D
 *		Right	: 62 : 0x3E
 *	
 *  - Main
 *		Back	: 43 : 0x2B
 *		Fwd		: 44 : 0x2C
 *		Stop	: 42 : 0x2A
 *		Play	: 41 : 0x29
 *		Rec		: 45 : 0x2D
 *		
 *	- Cycle		: 46 : 0x2E
 */

#pragma once
#include "Board/board.h"
#include "Input/input.h"
#include "Input/button.h"
#include "Input/toggle.h"
#include "Input/range.h"

#define NANO_KONTROL2_GROUP_SIZE 8

class nanoKONTROL2 : board
{
public:
	struct group
	{
		range * const slider;
		range * const knob;
		toggle * const s;
		toggle * const m;
		toggle * const r;

		explicit group(board* target, const uchar& index) :
			slider(new range(0x00 + index, "Slider")),
			knob(new range(0x10 + index, "Knob")),
			s(new toggle(target, 0x20 + index, "S")),
			m(new toggle(target, 0x30 + index, "M")),
			r(new toggle(target, 0x40 + index, "R")) { }
		~group() { delete slider, knob, s, m, r; }
	};

	nanoKONTROL2();
	~nanoKONTROL2();

	button* const track_left;
	button* const track_right;
	button* const marker_set;
	button* const marker_left;
	button* const marker_right;
	toggle* const cycle;
	toggle* const back;
	toggle* const forward;
	toggle* const stop;
	toggle* const play;
	toggle* const record;
	group** const groups;

	input* operator[](const uchar index) override;
};
