#pragma once
#include <map>
#define StringLiteral const char*
#define Flt float
#define Map_Str_To_Char std::map<StringLiteral, char>

void Win32MoveCharacter(
	StringLiteral sDirection,
	Flt fLeftMoveIncrement,
	Flt fRightMoveIncrement,
	Flt fUpMoveIncrement,
	Flt fDownMoveIncrement,
	Flt& PlayerLookAngle,
    const Map_Str_To_Char& mControls 	
)
{
	

}

