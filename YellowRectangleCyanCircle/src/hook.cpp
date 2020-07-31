#include "hook.hpp"

namespace YellowRectangleCyanCircle {
	HookAlreadyRegistered::HookAlreadyRegistered(HookEventId eventId) :
		eventId(eventId)
	{}

	const char* HookAlreadyRegistered::what() const {
		std::ostringstream message;
		message << "Unable to register a hook with ID " << this->eventId << ": already exists";
		return std::data(message.str());
	}

	std::unordered_map<HookEventId, Hook*> Hook::instances;
	std::mutex Hook::instancesMutex;

	Hook::Hook(HookEventId eventId, std::shared_ptr<IWinAPI> winAPI, std::function<void(Handle)>&& hookFunc) :
		isEnabled(false),
		eventId(eventId),
		winAPI(winAPI),
		func(std::move(hookFunc))
	{
		this->registerInstance(eventId, this);
	}

	Hook::~Hook() {
		if (this->IsEnabled()) this->Disable();
		if (Hook::getInstance(this->eventId)) Hook::unregisterInstance(this->eventId);
	}

	bool Hook::IsEnabled() const noexcept {
		return this->isEnabled;
	}

	Hook* Hook::getInstance(HookEventId eventId) {
		std::lock_guard<std::mutex> lock(Hook::instancesMutex);

		if (Hook::instances.find(eventId) == Hook::instances.end())
			return nullptr;

		return Hook::instances[eventId];
	}

	void Hook::registerInstance(HookEventId eventId, Hook* instance) {
		std::lock_guard<std::mutex> lock(Hook::instancesMutex);

		if (Hook::instances.find(eventId) != Hook::instances.end())
			throw HookAlreadyRegistered(eventId);

		Hook::instances[eventId] = instance;
	}

	void Hook::unregisterInstance(HookEventId eventId) {
		std::lock_guard<std::mutex> lock(Hook::instancesMutex);

		if (Hook::instances.find(eventId) == Hook::instances.end()) return;

		Hook::instances.erase(eventId);
	}

	HookCreateWindow::HookCreateWindow(std::shared_ptr<IWinAPI> winAPI, std::function<void(Handle)>&& func) :
		Hook(EVENT_OBJECT_CREATE, winAPI, std::move(func))
	{}

	void HookCreateWindow::Enable() {
		if (this->IsEnabled()) return;
		this->hook = this->winAPI->SetWinEventHook(this->eventId, HookCreateWindow::hookCallback);
		this->isEnabled = true;
	}

	void HookCreateWindow::Disable() {
		if (!this->IsEnabled()) return;
	}

	void CALLBACK HookCreateWindow::hookCallback(
		HWINEVENTHOOK hook, 
		HookEventId event, 
		Handle hWnd, 
		LONG idObject, 
		LONG idChild, 
		DWORD dwEventThread, 
		DWORD dwmsEventTime
	) {
		//123
	}

}
