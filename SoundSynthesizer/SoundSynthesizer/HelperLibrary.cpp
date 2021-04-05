#include "HelperLibrary.h"
#include <stdexcept>

namespace HelperLibrary
{
    std::vector<wstring> getSoundHW()
    {
        return olcNoiseMaker<short>::Enumerate();
    }

    void printSoundDevice(const std::vector<wstring>& soundDeviceList)
    {
        for (const auto& soundDevice : soundDeviceList)
        {
            std::wcout << "Output device: " << soundDevice << std::endl;
        }
    }

    /*olcNoiseMaker<short> createSoundMachine(const std::vector<wstring>& soundDeviceList)
    {
        if (!soundDeviceList.size())
        {
            throw std::runtime_error("Sound device list is empty!");
        }
        
        return olcNoiseMaker<short>(soundDeviceList[0]);
    }*/
}
