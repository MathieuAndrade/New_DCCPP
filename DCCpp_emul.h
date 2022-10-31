//
// Created by Mathieu Andrade on 28/10/2022.
//

#ifndef DDGI_DCCPP_DCCPP_EMUL_H
#define DDGI_DCCPP_DCCPP_EMUL_H

#include <bitset>
#include <cmath>
#include <string>
#include <algorithm>
#include "DCCpp.h"

class DCCpp_emul
{
  public:
    static void emulResponse(const std::string &command);
    static void emulDetectorUpdate(int address);
};

#endif // DDGI_DCCPP_DCCPP_EMUL_H
