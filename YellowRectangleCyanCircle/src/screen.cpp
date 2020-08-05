#include "screen.hpp"

namespace YellowRectangleCyanCircle {
	Screen::Screen(
		std::shared_ptr<IDirect> direct,
		std::shared_ptr<IWinAPI> winAPI,
		HWND hWnd
	) : direct(direct), winAPI(winAPI) {
		this->currentDisplay = this->winAPI->GetApplicationDisplayName(hWnd);
		this->desktop = std::make_unique<Desktop>(
			this->direct,
			this->currentDisplay
		);
	}

	void Screen::OnWindowMoved(HWND hWnd) {
		auto display = this->winAPI->GetApplicationDisplayName(hWnd);
		if (display == this->currentDisplay) return;

		std::unique_lock lock(this->desktopMutex);
		this->currentDisplay = display;
		this->desktop->SwitchDisplay(display);
	}

	void Screen::Perform(IContext& context) {
		std::shared_lock read(this->desktopMutex);

		Mat mat(this->desktop->GetHeight(), this->desktop->GetWidth(), CV_8UC4);
		this->desktop->Duplicate(std::data(mat));

		if (context.IsGameFound())
		{
			auto gameRect = context.GetGameRect();
			cv::resize(mat, mat, cv::Size(gameRect.width, gameRect.height));
		}

		cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);
		context.SetScreenImage(mat);
	}
}
