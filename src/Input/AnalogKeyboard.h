#pragma once

#include "wooting-analog-sdk.h"
#include <iostream>
#include <string>


namespace Analog {
std::string getAnalogResult(WootingAnalogResult res);

bool init();
bool isInited();

float readKey(unsigned short vk);

bool shutdown();

// class Keyboard{
// 	public:
// 		Keyboard() : {};
//
// 	private:
//
// };

} // namespace Wooting
