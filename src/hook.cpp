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
    std::unordered_map<DWORD, std::atomic<bool>> Hook::callbackBreaker;

    Hook::Hook(Hook&& other) noexcept {
        this->isEnabled = other.isEnabled;
        this->eventId = other.eventId;

        this->hWnd = other.hWnd;
        this->hWndStrict = other.hWndStrict;
        this->assignToWindow = other.assignToWindow;

        this->hook = other.hook;

        this->func = other.func;

        Hook::unregisterInstance(other.eventId);
        Hook::registerInstance(this->eventId, this);
    }

    Hook::Hook(DWORD eventId, HWND hWnd, bool hWndStrict, bool assignToWindow) :
        isEnabled(false),
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
        if (this->hook) ::UnhookWinEvent(this->hook);
        this->isEnabled = false;
    }

    HWND Hook::GetHandle() const noexcept {
        return this->hWnd;
    }

    void Hook::SetHandle(HWND hWnd) noexcept {
        this->hWnd = hWnd;
    }

    void Hook::CallBack(HWND hWnd, LONG idObject, LONG idChild) {
        if (this->hWndStrict && this->hWnd)
            if (this->hWnd != hWnd) return;

        if (this->assignToWindow) {
            auto acc = WinAPI::AccessibleObject::FromEvent(hWnd, idObject, idChild);
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
        Hook::callbackBreaker[eventId] = false;
    }

    void Hook::unregisterInstance(DWORD eventId) {
        std::unique_lock lock(Hook::instancesMutex);

        if (Hook::instances.find(eventId) == Hook::instances.end()) return;

        Hook::instances.erase(eventId);
        Hook::callbackBreaker[eventId] = false;
    }

    void Hook::registerCallback() {
        DWORD threadId = 0;
        if (this->assignToWindow && this->hWnd)
            threadId = ::GetWindowThreadProcessId(this->hWnd, nullptr);

        this->hook = ::SetWinEventHook(
            this->eventId, this->eventId,
            nullptr,
            Hook::staticCallback,
            0,
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
        if (Hook::callbackBreaker[event]) return;
        Hook::callbackBreaker[event] = true;

        auto instance = Hook::getInstance(event);
        if (!instance) return;

        try
        {
            instance->CallBack(hWnd, idObject, idChild);
        }
        catch (...) {
            Hook::callbackBreaker[event] = false;
            throw;
        }
        Hook::callbackBreaker[event] = false;
    }

    bool Hook::canBeEnabled() const noexcept {
        return !this->IsEnabled()
            && this->func
            && (this->hWndStrict && this->hWnd || !this->hWndStrict);
    }
}
