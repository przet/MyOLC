#include <iostream>
#include "olcNoiseMaker.h"
#include <stdexcept>
#include <cmath>

double makeNoise(double time)
{
    return 0.5 * sin(440.0 * 2 * 3.14159 * time);
}

int main()
{
    auto deviceList = olcNoiseMaker<short>::Enumerate();

    if (!deviceList.size())
        throw std::runtime_error("Device list empty!");

    for (const auto& device : deviceList)
    {
        std::wcout << "Output device: " << device << std::endl;
    }

    olcNoiseMaker<short> soundMachine(deviceList[0], 44100, 1, 8, 512);


    soundMachine.SetUserFunction(makeNoise);
    while (1)
    {

    }


    return 0;
}

