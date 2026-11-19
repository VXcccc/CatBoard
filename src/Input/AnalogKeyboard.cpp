#include "AnalogKeyboard.h"
#include <iostream>
#include <string>

namespace Analog {
std::string getAnalogResult(WootingAnalogResult res) {
  switch (res) {
  case WootingAnalogResult_UnInitialized:
    return "Item hasn't been initialized";
  case WootingAnalogResult_NoDevices:
    return "No Devices are connected";
  case WootingAnalogResult_DeviceDisconnected:
    return "Device has been disconnected";
  case WootingAnalogResult_Failure:
    return "Generic Failure";
  case WootingAnalogResult_InvalidArgument:
    return "A given parameter was invalid";
  case WootingAnalogResult_NoPlugins:
    return "No Plugins were found";
  case WootingAnalogResult_FunctionNotFound:
    return "The specified function was not found in the library";
  case WootingAnalogResult_NoMapping:
    return "No Keycode mapping to HID was found for the given Keycode";
  case WootingAnalogResult_NotAvailable:
    return "Indicates that it isn't available on this platform";
  case WootingAnalogResult_IncompatibleVersion:
    return "Indicates that the operation that is trying to be used is for an "
           "older version";
  case WootingAnalogResult_DLLNotFound:
    return "Indicates that the Analog SDK could not be found on the system";
    break;
  default:
    return "";
  }
}

bool init() {
  wooting_analog_set_keycode_mode(WootingAnalog_KeycodeType_VirtualKey);

  int result{wooting_analog_initialise()};
  // std::cout << result << '\n';
  return result >= 0;
}

bool isInited() { return wooting_analog_is_initialised(); }

float readKey(unsigned short vk) {
  wooting_analog_set_keycode_mode(WootingAnalog_KeycodeType_VirtualKey);

  float result{wooting_analog_read_analog(vk)};
  // if (result > 0)
  //   std::cout << vk << ": " << result << '\n';
  return result;
}

bool shutdown() {
  WootingAnalogResult res{wooting_analog_uninitialise()};
  return res == WootingAnalogResult_Ok;
}
} // namespace Analog
