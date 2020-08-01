#include "hook.hpp"

namespace YellowRectangleCyanCircle {
	HookAlreadyRegistered::HookAlreadyRegistered(DWORD eventId) :
		eventId(eventId)
	{}

	const char* HookAlreadyRegistered::what() const {
		std::ostringstream message;
		message << "Unable to register a hook with ID " << this->eventId << ": already exists";
		return std::data(message.str());
	}

	std::unordered_map<DWORD, Hook*> Hook::instances;
	std::shared_mutex Hook::instancesMutex;

	Hook::Hook(Hook&& other) noexcept {
		this->isEnabled = other.isEnabled;
		this->eventId = other.eventId;

		this->hWnd = other.hWnd;
		this->hWndStrict = other.hWndStrict;
		this->assignToWindow = other.assignToWindow;

		this->hook = other.hook;

		this->winAPI = other.winAPI;
		this->func = other.func;

		Hook::unregisterInstance(other.eventId);
		Hook::registerInstance(this->eventId, this);
	}

	Hook::Hook(std::shared_ptr<IWinAPI> winAPI, DWORD eventId, HWND hWnd, bool hWndStrict, bool assignToWindow) :
		isEnabled(false),
		winAPI(winAPI),
		eventId(eventId),
		hWnd(hWnd),
		hWndStrict(hWndStrict),
		assignToWindow(assignToWindow)
	{
		this->registerInstance(eventId, this);
	}

	Hook::~Hook() {
		if (this->IsEnabled()) this->Disable();
		if (Hook::getInstance(this->eventId)) Hook::unregisterInstance(this->eventId);
	}

	void Hook::SetCallback(std::function<void(HWND, LONG)> func) noexcept {
		this->func = func;
	}

	bool Hook::IsEnabled() const noexcept {
		return this->isEnabled;
	}

	void Hook::Enable() {
		if (!this->canBeEnabled()) return;
		this->registerCallback();
		if (this->hook) this->isEnabled = true;
	}

	void Hook::Disable() {
		if (!this->IsEnabled()) return;
		if (this->hook) this->winAPI->UnhookWinEvent(this->hook);
		this->isEnabled = false;
	}

	HWND Hook::GetHandle() const noexcept {
		return this->hWnd;
	}

	void Hook::SetHandle(HWND hWnd) noexcept {
		this->hWnd = hWnd;
	}

	void Hook::CallBack(HWND hWnd, LONG idObject, LONG idChild) {
		if (this->assignToWindow) {
			auto acc = this->winAPI->AccessibleObjectFromEvent(hWnd, idObject, idChild);
			if (!(acc && acc->IsWindow())) return;
		}

		if (this->func) std::invoke(this->func, hWnd, idObject);
	}

	Hook* Hook::getInstance(DWORD eventId) {
		std::shared_lock lock(Hook::instancesMutex);

		if (Hook::instances.find(eventId) == Hook::instances.end())
			return nullptr;

		return Hook::instances[eventId];
	}

	void Hook::registerInstance(DWORD eventId, Hook* instance) {
		std::unique_lock lock(Hook::instancesMutex);

		if (Hook::instances.find(eventId) != Hook::instances.end())
			throw HookAlreadyRegistered(eventId);

		Hook::instances[eventId] = instance;
	}

	void Hook::unregisterInstance(DWORD eventId) {
		std::unique_lock lock(Hook::instancesMutex);

		if (Hook::instances.find(eventId) == Hook::instances.end()) return;

		Hook::instances.erase(eventId);
	}

	void Hook::registerCallback() {
		DWORD threadId = 0;
		if (this->assignToWindow && this->hWnd)
			threadId = this->winAPI->GetWindowThreadProcessId(this->hWnd);

		this->hook = this->winAPI->SetWinEventHook(
			this->eventId,
			Hook::staticCallback,
			threadId,
			WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
		);
	}

	void CALLBACK Hook::staticCallback(
		HWINEVENTHOOK hook,
		DWORD event, 
		HWND hWnd, 
		LONG idObject, 
		LONG idChild,
		DWORD dwEventThread, 
		DWORD dwmsEventTime
	) {
		std::shared_lock lock(Hook::instancesMutex);
		auto instance = Hook::getInstance(event);
		if (!instance) return;

		instance->CallBack(hWnd, idObject, idChild);
	}

	bool Hook::canBeEnabled() const noexcept {
		return !this->IsEnabled() 
			&& this->func 
			&& (this->hWndStrict && this->hWnd || !this->hWndStrict);
	}
}
