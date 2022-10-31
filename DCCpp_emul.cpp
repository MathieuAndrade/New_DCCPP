//
// Created by Mathieu Andrade on 28/10/2022.
//

#include "DCCpp_emul.h"
#include "DCCpp_utils.h"

void DCCpp_emul::emulResponse(const std::string &command) {
    if (command.rfind("<s>", 1) == 0)
    {
        // CMD_STATION_VERSION_REQUEST
        DCCpp_commands::cmdToParse.emplace_back("<iDCCpp V-1.0.0 >");
    }
    else if (command.rfind("<1>", 1) == 0)
    {
        // POWER_ON
        DCCpp_commands::cmdToParse.emplace_back("<p1>");
    }
    else if (command.rfind("<0>", 1) == 0)
    {
        // POWER_OFF
        DCCpp_commands::cmdToParse.emplace_back("<p0>");
    }
    else if (command.rfind("<y", 0) == 0)
    {
        // INIT_S88
        DCCpp_utils::clearDetectorStates();
        DCCpp_commands::cmdToParse.emplace_back("<y " + DCCpp::detectorStates + ">");
    }
    else if (command.rfind("<t", 0) == 0)
    {
        // LOCO_SPEED
        std::string locoSpeed = command;
        std::replace( locoSpeed.begin(), locoSpeed.end(), 't', 'T');
        DCCpp_commands::cmdToParse.emplace_back(locoSpeed);
    }
    else if (command.rfind("<f", 0) == 0 && command.size() > 9)
    {
        // LOCO_FUNCTION
        std::string locoFunc = command;
        DCCpp_utils::removeCharsFromString(locoFunc, (char *)"<f>");
        DCCpp_commands::cmdToParse.emplace_back("<F0" + locoFunc + " -1>");
    }
    else if (command.rfind("<T", 0) == 0)
    {
        // ACCESSORY_OPERATION
        std::string acc = command;
        DCCpp_utils::removeCharsFromString(acc, (char *)"<T");
        DCCpp_commands::cmdToParse.emplace_back("<H" + acc);
    }
}

void DCCpp_emul::emulDetectorUpdate(int address) {
    if(DCCpp::emulation) {
        // Calculate nibble index in detector states according to target address
        int nibbleIndex = std::floor((address / 4.0) - 0.25);
        // Get first address of target nibble
        int firstAddOfNibble = (nibbleIndex * 4) + 1;
        // Get corresponding address byte index in nibble
        int byteIndex = address - firstAddOfNibble;

        // Init a nibble and set the target address to high
        auto bytes = std::bitset<4>(0);
        bytes[byteIndex] = true;

        // Convert bytes to hex value
        std::stringstream hex;
        hex << std::hex << bytes.to_ulong();
        std::string stringHex = hex.str();

        // Set new nibble in detector states
        // Use new array nibble to ensure clear old states
        // Number of nibbles is known, because in emulation mode we use 288 detectors
        // 288 detectors = 36 modules = 72 nibbles
        std::string newStates = std::bitset<72>(0).to_string();
        newStates[nibbleIndex] = stringHex[0];

        // Finally perform an update
        DCCpp_commands::cmdToParse.emplace_back("<y " + newStates + ">");
    }
}
