#pragma once

#include <string>
#include "types.h"

struct move_t {

	uint16_t data;
	//Bits 0-5: Source Square
	//Bits 6-11: Target Square
	//Bits 12-15: Flags (see namespace MoveFlag in types.h)

	move_t() : data{0} {}
	move_t(const uint16_t data): data{data} {}
	move_t(const std::string& code);
	move_t(int from, int to, int flag) { data = (flag << 12) | (to << 6) | from; }

	constexpr int from() const { return data & 0x3F; }
    constexpr int to() const { return (data >> 6) & 0x3F; }
	constexpr int flag() const {return (data >> 12) & 0x3F; }
	
	std::string to_code() const; // return the move in long algebraic UCI format
};
