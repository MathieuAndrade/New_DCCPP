//
// Created by Mathieu Andrade on 28/10/2022.
//

#include "DCCpp_emul.h"
#include "DCCpp_utils.h"

void DCCpp_emul::emulDetectorUpdate(int address) {
    if(DCCpp::emulation) {
        // First reset detector states to prevent false positive
        DCCpp_utils::clearDetectorStates();
        DCCpp::handleDetectorUpdate(DCCpp::detectorStates);

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
        std::string newStates = DCCpp::detectorStates;
        newStates[nibbleIndex] = stringHex[0];

        // Finally perform an update
        DCCpp::handleDetectorUpdate(newStates);
    }
}
