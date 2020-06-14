// ConsoleWalker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <string>
#include <stdio.h>
#include <stdexcept>
#include <chrono>
#include "ConsoleWalker_HelperFunctions.h"
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <utility>
#include <cmath>
#include <initializer_list>

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

// Used as a stop condition so we never run into problem of never hitting a wall when raycasting
const float fMaxDepth = 16.0f;

// Pi/4 : 45 deg
float fFOV = 3.14159 / 4.0;

typedef std::pair<float, float> Coordinate;

struct Cell
{
	/*
	corner1          corner2
    (x2,y0)			(x2,y1)
    -------------------
    |				  |	
	|			      |		
	|                 |
	-------------------
    (x1,y0)			(x1,y1)
    corner0		    corner3
	*/

	Cell(Coordinate rCorner0)
		: mCorner0(rCorner0)
	{
		mCorner1 = { rCorner0.first + 1, rCorner0.second };
		mCorner2 = { rCorner0.first + 1, rCorner0.second + 1 };
		mCorner3 = { rCorner0.first, rCorner0.second + 1 };
		mCornerList = {mCorner0,
					   mCorner1,
					   mCorner2,
					   mCorner3};
	}

	Cell(std::initializer_list<Coordinate> rInitList)
	{
		for (const auto& elem : rInitList)
		{
			mCornerList.push_back(elem);
		}
	}

	Coordinate mCorner0;
	Coordinate mCorner1;
	Coordinate mCorner2;
	Coordinate mCorner3;
	std::vector<Coordinate> mCornerList;

};

template <typename T, typename U>
std::pair<T, U> operator-(const std::pair<T, U>& rCoord1, const std::pair<T, U>& rCoord2)
{
	// Only works for Types with - defined on them
	return { rCoord1.first - rCoord2.first, rCoord1.second - rCoord2.second };
}

struct RayVector
{
	// TODO static assert to make sure we 
	// don't end up with zero length
	RayVector(const Coordinate& rEndCoord, const Coordinate& rStartCoord)
	{
		xLen = (rEndCoord - rStartCoord).first;
		yLen = (rEndCoord - rStartCoord).second;
		Len = sqrt(pow(xLen, 2) + pow(yLen, 2));
		unitXLen = xLen / Len;
		unitYLen = yLen / Len;
	}

	RayVector(const float& rX, const float& rY)
	{
		xLen = rX;
		yLen = rY;
		Len = sqrt(pow(xLen, 2) + pow(yLen, 2));
		unitXLen = xLen / Len;
		unitYLen = yLen / Len;
	}

	RayVector(const Coordinate& rCoord)
	{
		xLen = rCoord.first;
		yLen = rCoord.second;
		Len = sqrt(pow(xLen, 2) + pow(yLen, 2));
		unitXLen = xLen / Len;
		unitYLen = yLen / Len;
	}


	float xLen;
	float yLen;
	float Len;
	float unitXLen;
	float unitYLen;
};


float playerDistance(const Coordinate& rPlayerPos, const Coordinate& rDestination)
{
	return sqrt(pow(rPlayerPos.first - rDestination.first, 2)
				+ pow(rPlayerPos.second - rDestination.second, 2));
}

float angleBetweenRayVectors(const RayVector& rRayVector1, const RayVector& rRayVector2)
{
    return acos( (rRayVector1.unitXLen * rRayVector2.unitXLen) + (rRayVector1.unitYLen * rRayVector2.unitYLen) );
}


int main()
{
	assert(fPlayerPosX <= nMapWidth && fPlayerPosX >= 0);
	assert(fPlayerPosY <= nMapHeight && fPlayerPosY >= 0);

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

	std::wstring gameMap;

	gameMap += L"#######..#######";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#....#.....#...#";
	gameMap += L"#....#.....#...#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#........#.....#";
	gameMap += L"#........#.....#";
	gameMap += L"#........#.....#";
	gameMap += L"#######..#######"; 
	
	unsigned const gameMapSize = gameMap.size();
	assert(gameMap.size() == nMapHeight * nMapWidth); // _don't_ use defined variable for size in the assert


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

		movePlayer(gameMap, fPlayerLookAngle, fPlayerPosX, fPlayerPosY, fElapsedTime);

		// Raycasting algorithm
		for (int X = 0; X < nScreenWidth; X++)
		{
			float fRayAngle = (fPlayerLookAngle - fFOV / 2.0f) + ((float)X/ (float)nScreenWidth) * fFOV;

			float fRayDistanceToWall = 0;
			bool bRayHitWall = false;
			bool bBoundary = false;

			// Don't _need_ ExceededMap flag, but I think it makes the logic easier to follow - seperating wall/gameMap
			bool bRayExceededMap = false;
			const float fIncrement = 0.1f;

			// Unit vector for Ray
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);
			RayVector mUnitRayVector(fEyeX, fEyeY);

			double mDoubleEqualTolerance = 0.00001;
			assert(mUnitRayVector.unitXLen < fEyeX + mDoubleEqualTolerance && mUnitRayVector.unitYLen < fEyeY + mDoubleEqualTolerance &&
				   mUnitRayVector.unitXLen > fEyeX - mDoubleEqualTolerance && mUnitRayVector.unitYLen > fEyeY - mDoubleEqualTolerance);


			while (!bRayHitWall && fRayDistanceToWall < fMaxDepth && !bRayExceededMap)
			{
				fRayDistanceToWall += fIncrement;

				// TODO come back to thinking about what _not_ making this an int does - 
				// for now we are assuming walls etc are on integer boundaries (I want to understand better what
				// _this_ means also)
				int nTestX = (int)(fPlayerPosX + fEyeX * fRayDistanceToWall);
				int nTestY = (int)(fPlayerPosY + fEyeY * fRayDistanceToWall);

				// TODO: do we need to check of TestX, TestY < 0 ? Will they ever be? Yes they could be...
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) // we have exceeded the gameMap limits 
				{
					bRayExceededMap = true;
					fRayDistanceToWall = fMaxDepth;
				}
				else // we are within gameMap boundary
				{
					// One mapwidth represents 1 unit of height (if you look at the gameMap as a string on one line, this helps)
					if (gameMap[nTestY * nMapWidth + nTestX] == '#')
					{
						bRayHitWall = true;
						
                        // Ray has hit a wall: so first corner is at (nTestX, nTestY)
						Cell mCell({ (float)nTestX, (float)nTestY});

						// (Player distance to corner, dot product ray cast to corner
						// and ray cast from corner.
						std::vector<std::pair<float, float>> p;
						Coordinate mPlayerPos{ fPlayerPosX, fPlayerPosY };
						
						for (const auto& corner : mCell.mCornerList)
						{
							RayVector mCornerToPlayerRay(corner - mPlayerPos);
							p.push_back({ playerDistance(mPlayerPos, corner) ,
										 angleBetweenRayVectors(mUnitRayVector, mCornerToPlayerRay)});
						}
						
						// Sort on player distance to corner
						std::sort(std::begin(p), std::end(p));

						float fTolerance = 0.01;
						bBoundary = p[0].second < fTolerance || p[1].second < fTolerance;
					}
				}
			}
		

			// "Amount" of wall + floor as a function of how far wall is from player
			// Remember (0,0) is top left: this will help in understanding below calculation
			assert(fRayDistanceToWall > 0);
			const int nCeiling = (float)(nScreenHeight / 2.0f) - float(nScreenHeight /fRayDistanceToWall);
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

			// Shade cell edges
			if (bBoundary) nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling) // sky(above the ceiling)
					// multiply by screenwidth we want the whole width to be coverred by the appropriate char
					// TODO: for experimentation, add command line args to set the ascii code  
					// TODO: multiply by ScreenHeight to see what happens!
				{
					Screen[y * nScreenWidth + X] = 0x20;
				}
				else if (y <= nFloor) // wall
				{
					Screen[y * nScreenWidth + X] = nShade;
				}
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

		//---Screen Output---
		std::string playerPosXStr = std::to_string(fPlayerPosX);
		std::string playerPosXStrTitle = "Player Position X = ";
		std::string playerPosYStr = std::to_string(fPlayerPosY);
		std::string playerPosYStrTitle = "Player Position Y = ";
		std::string playerPosLookAngleStr = std::to_string(fPlayerLookAngle);
		std::string playerLookAngleStrTitle = "Player Look Angle = ";

		// Write to screen
		// XPos
		assert(playerPosXStr.length() + playerPosXStrTitle.length() < nScreenWidth);
		int i = 0;
		for (char character : playerPosXStrTitle) { Screen[i++] = character;}
		for (char character : playerPosXStr) { Screen[i++] = character;}
		// YPos
		assert(playerPosYStr.length() + playerPosYStrTitle.length() < nScreenWidth);
		i = nScreenWidth;
		for (char character : playerPosYStrTitle) { Screen[i++] = character;}
		for (char character : playerPosYStr) { Screen[i++] = character;}
		// Look Angle
		assert(playerPosYStr.length() + playerPosYStrTitle.length() < nScreenWidth);
		i = nScreenWidth*2;
		for (char character : playerLookAngleStrTitle) { Screen[i++] = character;}
		for (char character : playerPosLookAngleStr) { Screen[i++] = character;}

		// Origin is top-left-hand corner - will stop console from scrolling down
        if (!WriteConsoleOutputCharacter(ConsoleHandle, Screen, nCharacterArraySize, { 0, 0 }, &BytesWritten))
		{
			Error = GetLastError();
			printf("\n Error: error code %d\n", Error);
		}

	}


	return 0;


}

