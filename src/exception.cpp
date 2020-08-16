#include "exception.hpp"

namespace YellowRectangleCyanCircle {
    WindowsException::WindowsException() {
        DWORD lastError = GetLastError();
        LPVOID msgBuf;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            lastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&msgBuf),
            0,
            nullptr
        );

        std::wstring message(reinterpret_cast<LPWSTR>(msgBuf));
        this->message = utf8_encode(message);
    }

    const char* WindowsException::what() const throw () {
        return std::data(this->message);
    }
}
