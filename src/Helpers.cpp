#include "Helpers.h"
#include <cstdlib>
#include <windows.h>
#include <winuser.h>

namespace Helper {
[[noreturn]] void fatalError(const wchar_t *message) {
  MessageBox(NULL, message, L"Catboard Fatal Error", MB_OK | MB_ICONERROR);
  std::exit(EXIT_FAILURE);
}

void errorMesBox(const wchar_t *message){
  MessageBox(NULL, message, L"Catboard Error", MB_OK | MB_ICONERROR);
}
} // namespace Helper
