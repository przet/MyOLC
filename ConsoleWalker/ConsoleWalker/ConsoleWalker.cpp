// ConsoleWalker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <string>
#include <stdio.h>

const int ScreenWidth = 120;
const int ScreenHeight = 40;
const int CharacterArraySize = ScreenWidth * ScreenHeight;

// Use floating points, or player will "clunk" around
float PlayerPosX = 0.0f;
float PlayerPosY = 0.0f;
float PlayerLookAngle = 0.0f;

const int MapHeight = 16;
const int MapWidth = 16;

int main()
{
	// Error Code variable
	DWORD Error;

	// Create Screen Buffer
	wchar_t* Screen = new wchar_t[CharacterArraySize];
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
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	// As with any game engine - need a game loop:
	while (1)
	{
		Screen[CharacterArraySize - 1] = '\0';

		// Origin is top-left-hand corner - will stop console from scrolling down
		if (!WriteConsoleOutputCharacter(ConsoleHandle, Screen, CharacterArraySize, { 0, 0 }, &BytesWritten))
		{
			Error = GetLastError();
			printf("\n Error: error code %d\n", Error);
		}

	}


	
	 


	return 0;


}

