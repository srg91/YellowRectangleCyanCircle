#include "application.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	CoInitialize(nullptr);

	int r = 0;
	try {
		YellowRectangleCyanCircle::Application app(hInstance);
		r = app.RunMessageLoop();
	} catch (...) {
		// TODO: MessageBox?
		r = -1;
	}

	CoUninitialize();
	return r;
}
