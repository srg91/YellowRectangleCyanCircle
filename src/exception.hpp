#pragma once
#include "types.hpp"
#include "utils.hpp"

#include <exception>
#include <string>

namespace YellowRectangleCyanCircle {
    class COMException : public std::exception {
    public:
        COMException(HRESULT hr);
        const char* what() const throw() override;
    private:
        std::string message;
    };

    class WindowsException : public std::exception {
    public:
        WindowsException();
        const char* what() const throw() override;
    private:
        std::string message;
    };

    // Application exceptions
    struct ApplicationException : public WindowsException {};
    struct UnableToRegisterWindowClass : public ApplicationException {};
    struct UnableToCreateWindow : public ApplicationException {};

    // Custom exceptions
    class HookAlreadyRegistered : public std::exception {
    public:
        HookAlreadyRegistered(DWORD eventId);

        const char* what() const;
    private:
        DWORD eventId;
    };
}
