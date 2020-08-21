#include "main.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    YellowRectangleCyanCircle::Logging::Initialize(pCmdLine);

    try {
        YellowRectangleCyanCircle::Application app(hInstance);
        return app.RunMessageLoop();
    }
    catch (const std::exception& exc) {
        std::wstringstream buf;
        buf << "Application exception: " << YellowRectangleCyanCircle::utf8_decode(exc.what());
        L(critical, buf.str());
        ::MessageBox(0, std::data(buf.str()), L"Yellow Rectangle Cyan Circle", MB_ICONERROR | MB_OK);
        return -1;
    }
}
