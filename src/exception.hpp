#pragma once
#include "utils.hpp"

#include <Windows.h>

#include <exception>
#include <string>

namespace YellowRectangleCyanCircle {
    class WindowsException : public std::exception {
    public:
        WindowsException();
        const char* what() const throw () override;
    private:
        std::string message;
    };

    // Application exceptions
    struct ApplicationException : public WindowsException {};
    struct UnableToRegisterWindowClass : public ApplicationException {};
    struct UnableToCreateWindow : public ApplicationException {};
}
