// ConsoleWalker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <string>
#include <stdio.h>
#include <stdexcept>
#include <chrono>
#include "ConsoleWalker_HelperFunctions.h"
#include <iostream>

//-----------------------------------------------------------------------------
// Note to reader: Geometry. Recall (0,0) is top left: -> +ve x, v is +ve y
// Use this when following/understanding all the geometry calculations
//-----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Compilation Notes:
// use Unicode for extended ascii set
// http://ascii-table.com/ascii-extended-pc-list.php
//

const int nScreenWidth = 120;
const int nScreenHeight = 40;
const int nCharacterArraySize = nScreenWidth * nScreenHeight;

// Use floating points, or player will "clunk" around
// Init player pos to not be (0,0) (in a wall)
float fPlayerPosX = 8.0f;
float fPlayerPosY = 8.0f;
float fPlayerLookAngle = 0.0f;

const int nMapHeight = 16;
const int nMapWidth = 16;

// Used as a stop condition so we never run into problem of never hitting a wall when raycasting
const float fMaxDepth = 16.0f;

// Pi/4 : 45 deg
float fFOV = 3.14159 / 4.0;

int main()
{
	// Error Code variable
	DWORD Error;

	// Create Screen Buffer
	wchar_t* Screen = new wchar_t[nCharacterArraySize];
	HANDLE ConsoleHandle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	if (!SetConsoleActiveScreenBuffer(ConsoleHandle))
	{
		Error = GetLastError();
		printf("\n Error: error code %d\n", Error);
	}

	DWORD BytesWritten = 0;

	std::wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#....#.....#...#";
	map += L"#....#.....#...#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#........#.....#";
	map += L"#........#.....#";
	map += L"#........#.....#";
	map += L"#######..#######"; 

	// For consistent player movement/framerate
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	// As with any game engine - need a game loop:
	while (1)
	{
		// For consistent player movement/framerate
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		float fElapsedTime = elapsedTime.count();
		tp1 = tp2;

		//---- Controls
		// TODO reveiw C++ vs C char* for string literals: C++ string literal is const char*?
		std::map<const char*, char> mControls{
			{"up", 'W'},
			{ "down", 'S' },
			{ "left", 'A' },
			{ "right", 'D' }
		};

		// Rotation
		// TODO see OLC synthysiser video
		if (GetAsyncKeyState((unsigned short)mControls["left"]) & 0x8000)
			fPlayerLookAngle -= (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)mControls["right"]) & 0x8000)
			fPlayerLookAngle += (0.8f) * fElapsedTime;

		// Forward/Back
		if (GetAsyncKeyState((unsigned short)mControls["up"]) & 0x8000)
		{
			fPlayerPosX += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;

			// Collision Detection
			if (map[(int)fPlayerPosY * nMapWidth + (int)fPlayerPosX] == '#')
			{
				fPlayerPosX -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
				fPlayerPosY -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)mControls["down"]) & 0x8000)
		{
			fPlayerPosX -= sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			fPlayerPosY -= cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;

			// Collision Detection
			if (map[(int)fPlayerPosY * nMapWidth + (int)fPlayerPosX] == '#')
			{
				fPlayerPosX += sinf(fPlayerLookAngle) * 5.0f * fElapsedTime;
				fPlayerPosY += cosf(fPlayerLookAngle) * 5.0f * fElapsedTime;
			}
		}
		//----end controls



		// Raycasting algorithm
		for (int X = 0; X < nScreenWidth; X++)
		{
			// For each column calculate the projected ray angle into world space
			// Note we do not need to check for nScreenWidth zero - we will never get here (loop condition) if it is - 
			// although I guess someone could put in negative... anyway.
			// TODO: the first term implies the angle is measured clockwise from a horizontal (the horiztonal line extending from the 
			// players point): what if anti-clockwise?
			float fRayAngle = (fPlayerLookAngle - fFOV / 2.0f) + ((float)X/ (float)nScreenWidth) * fFOV;

			float fRayDistanceToWall = 0;
			bool bRayHitWall = false;
			// Don't _need_ ExceededMap flag, but I think it makes the logic easier to follow - seperating wall/map
			bool bRayExceededMap = false;
			const float fIncrement = 0.1f;

			// Unit vector for Ray
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bRayHitWall && fRayDistanceToWall < fMaxDepth && !bRayExceededMap)
			{
				fRayDistanceToWall += fIncrement;

				// TODO come back to thinking about what _not_ making this an int does - 
				// for now we are assuming walls etc are on integer boundaries (I want to understand better what
				// _this_ means also)
				int nTestX = (int)(fPlayerPosX + fEyeX * fRayDistanceToWall);
				int nTestY = (int)(fPlayerPosY + fEyeY * fRayDistanceToWall);

				// TODO: do we need to check of TestX, TestY < 0 ? Will they ever be? Yes they could be...
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) // we have exceeded the map limits 
				{
					bRayExceededMap = true;
					fRayDistanceToWall = fMaxDepth;
				}
				else // we are within map boundary
				{
					// !TODO : could be related to my previous !TODO re: cos/sin. I * by nMapHeight, OLC by nMapWidth
					// TestY < nMapHeight here (that's how we got here) so TestY * nMapHeight is the fraction of the nMapHeight
					if (map[nTestY * nMapHeight + nTestX] == '#')
					{
						bRayHitWall = true;
					}

				}
			}
		

			// "Amount" of wall + floor as a function of how far wall is from player
			// TODO : internalise this concept/calculation - the physics of it
			const int nCeiling = (float)(nScreenHeight / 2.0f) - nScreenHeight / (float)fRayDistanceToWall;
			const int nFloor = nScreenHeight - nCeiling;

			// Wall shading as function of distance to wall
			short int nShade = 0x20;

			// Note I actually like this style here of putting the code block of if statement on the same line
			// http://ascii-table.com/ascii-extended-pc-list.php
			std::map<const char*, short int> mExtendedAsciiUHex{
				{"Full block", 0x2588},
				{"Dark shade block", 0x2593},
				{"Medium shade block", 0x2592},
				{"Light shade block", 0x2591},
			};
			std::map<const char*, short int>& mM = mExtendedAsciiUHex;

			if (fRayDistanceToWall <= fMaxDepth / 4.0f)	nShade = mM["Full block"];
			else if (fRayDistanceToWall < fMaxDepth / 3.0f)	nShade = mM["Dark shade block"];
			else if (fRayDistanceToWall < fMaxDepth / 2.0f)	nShade = mM["Medium shade block"];
			else if (fRayDistanceToWall < fMaxDepth )	nShade = mM["Light shade block"];

			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling) // sky(above the ceiling)
					// multiply by screenwidth we want the whole width to be coverred by the appropriate char
					// TODO: for experimentation, add command line args to set the ascii code  
					// TODO: multiply by ScreenHeight to see what happens!
					Screen[y * nScreenWidth + X] = 0x20;
				else if (y <= nFloor) // wall
					Screen[y * nScreenWidth + X] = nShade;
				else
				{
					// Shade floor based on distance
					// TODO. We don't shade based on distace to wall - would this not be better, if we did?
					// TODO understand this calculation; proportion of floor section
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)	nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					Screen[y * nScreenWidth + X] = nShade; // below floor
				}
			}
		}
		Screen[nCharacterArraySize - 1] = '\0';

		// Origin is top-left-hand corner - will stop console from scrolling down
	if (!WriteConsoleOutputCharacter(ConsoleHandle, Screen, nCharacterArraySize, { 0, 0 }, &BytesWritten))
		{
			Error = GetLastError();
			printf("\n Error: error code %d\n", Error);
		}

	}


	return 0;


}

