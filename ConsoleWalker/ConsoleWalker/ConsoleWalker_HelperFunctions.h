#pragma once
#include <map>
#include <cmath>
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

// Use floating points, or player will "clunk" around
// Init player pos to not be (0,0) (in a wall)
float fPlayerPosX = 8.0f;
float fPlayerPosY = 8.0f;
float fPlayerLookAngle = 0.0f;

const int nMapHeight = 16;
const int nMapWidth = 16;
int playerPosInGameMap(int playerPosX = fPlayerPosX, int playerPosY = fPlayerPosY, int gameMapWidth = nMapWidth)
{
	return int(playerPosX) + (int)playerPosY * gameMapWidth;
}

void movePlayer(const std::wstring& gameMap,
                float& fPlayerLookAngle, float& fPlayerPosX,
                float& fPlayerPosY, float& fElapsedTime)
{
    std::map<const char*, char> mControls{
        {"up", 'W'},
        { "down", 'S' },
        { "left", 'A' },
        { "right", 'D' },
        { "left_strafe", 'Z'},
        { "right_strafe", 'X'}
    };

	//---Rotation---
		// TODO see OLC synthysiser video
	if (GetAsyncKeyState((unsigned short)mControls["left"]) & 0x8000)
	{
		fPlayerLookAngle -= (0.8f) * fElapsedTime;
	}

	if (GetAsyncKeyState((unsigned short)mControls["right"]) & 0x8000)
	{
		fPlayerLookAngle += (0.8f) * fElapsedTime;
	}

	//---Forward/Back---
	if (GetAsyncKeyState((unsigned short)mControls["up"]) & 0x8000)
	{
		fPlayerPosX += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		fPlayerPosY += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;

		// Out Of Bounds Handling
		if (playerPosInGameMap() > gameMap.size()
			|| playerPosInGameMap() < 0)
		{
			fPlayerPosX -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}

		// Collision Detection
		if (gameMap[playerPosInGameMap()] == '#')
		{
			fPlayerPosX -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}
	}

	if (GetAsyncKeyState((unsigned short)mControls["down"]) & 0x8000)
	{
		fPlayerPosX -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		fPlayerPosY -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;

		// Out Of Bounds Handling
		if (playerPosInGameMap() > gameMap.size()
			|| playerPosInGameMap() < 0)
		{
			fPlayerPosX += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}

		// Collision Detection
		if (gameMap[(int)fPlayerPosY * nMapWidth + (int)fPlayerPosX] == '#')
		{
			fPlayerPosX += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}
	}

	//---Strafe---
	if (GetAsyncKeyState((unsigned short)mControls["right_strafe"]) & 0x8000)
	{
		fPlayerPosX += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		fPlayerPosY -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;

		// Out Of Bounds Handling
		if (playerPosInGameMap() > gameMap.size()
			|| playerPosInGameMap() < 0)
		{
			fPlayerPosX -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}

		// Collision Detection
		if (gameMap[(int)fPlayerPosY * nMapWidth + (int)fPlayerPosX] == '#')
		{
			fPlayerPosX -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}
	}

	if (GetAsyncKeyState((unsigned short)mControls["left_strafe"]) & 0x8000)
	{
		fPlayerPosX -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		fPlayerPosY += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;

		// Out Of Bounds Handling
		if (playerPosInGameMap() > gameMap.size()
			|| playerPosInGameMap() < 0)
		{
			fPlayerPosX += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}

		// Collision Detection
		if (gameMap[(int)fPlayerPosY * nMapWidth + (int)fPlayerPosX] == '#')
		{
			fPlayerPosX += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
		}
	}

}

bool floatEqual(float r1, float r2, float rDelta = 10e-10)
{
	return fabs(r1 - r2) <= rDelta;
}

bool angleInSector(const float& rAngle, float rSectorStart, float rSectorEnd, bool rIncludeBoundaries = false)
{
	return rIncludeBoundaries ? rAngle <= rSectorEnd && rAngle >= rSectorStart :
                                rAngle < rSectorEnd && rAngle > rSectorStart ;

}


