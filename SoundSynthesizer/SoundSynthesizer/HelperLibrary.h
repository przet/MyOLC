#ifndef HELPER_LIBRARY__H
#define HELPER_LIBRARY__H

#include "olcNoiseMaker.h"
#include <vector>
#include <string>

namespace HelperLibrary
{
    std::vector<wstring> getSoundHW();

    void printSoundDevice(const std::vector<wstring>& soundDeviceList);

    olcNoiseMaker<short> createSoundMachine(const std::vector<wstring>& soundDevices);
}


#endif // !HELPER_LIBRARY__H
