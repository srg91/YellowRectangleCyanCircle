#pragma once


#include "winapi.hpp"
#include "types.hpp"

#include <Windows.h>

#include <exception>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <sstream>
#include <unordered_map>

namespace YellowRectangleCyanCircle {
	class HookAlreadyRegistered : public std::exception {
	public:
		HookAlreadyRegistered(DWORD eventId);

		const char* what() const;
	private:
		DWORD eventId;
	};

	struct IHook {
		virtual ~IHook() {};
		// virtual void SetCallback(std::function<void(HWND)>) = 0;

		virtual void Enable() = 0;
		virtual void Disable() = 0;

		virtual bool IsEnabled() const noexcept = 0;
		// virtual DWORD GetEventId() const noexcept = 0;
	//protected:
	//	virtual bool canBeEnabled() const noexcept = 0;
	};

	//struct IHookOnHandle {
	//	virtual HWND GetHandle() const noexcept = 0;
	//	virtual void SetHandle(HWND hWnd) noexcept = 0;
	//};

	struct IHookCallbackReceiver {
		virtual ~IHookCallbackReceiver() {};

		virtual void OnWindowCreated(HWND hWnd) {};
		virtual void OnWindowDestroyed(HWND hWnd) {};
		virtual void OnWindowMoved(HWND hWnd) {};
	};

	class Hook : public IHook {
	public:
		Hook(Hook&& other) noexcept;
		Hook(std::shared_ptr<IWinAPI> winAPI, DWORD eventId, HWND hWnd = 0, bool hWndStrict = false, bool assignToWindow = false);
		~Hook() override;
		void SetCallback(std::function<void(HWND, LONG)> func) noexcept;

		bool IsEnabled() const noexcept override final;
		
		void Enable() override;
		void Disable() override;

		HWND GetHandle() const noexcept;
		void SetHandle(HWND hWnd) noexcept;

		virtual void CallBack(HWND hWnd, LONG idObject, LONG idChild);
	protected:
		bool isEnabled;
		DWORD eventId;

		HWND hWnd;
		bool hWndStrict = false;
		bool assignToWindow = false;

		HWINEVENTHOOK hook;

		std::shared_ptr<IWinAPI> winAPI;
		std::function<void(HWND, LONG)> func;

		static std::unordered_map<DWORD, Hook*> instances;
		static std::shared_mutex instancesMutex;

		static Hook* getInstance(DWORD eventId);
		static void registerInstance(DWORD eventId, Hook* instance);
		static void unregisterInstance(DWORD eventId);

		void registerCallback();
		static void CALLBACK staticCallback(
			HWINEVENTHOOK hook,
			DWORD event,
			HWND hWnd,
			LONG idObject,
			LONG idChild,
			DWORD dwEventThread,
			DWORD dwmsEventTime
		);

		bool canBeEnabled() const noexcept;

		Hook(const Hook&) = delete;
	};

	//class HookCreateWindow : public Hook {
	//public:
	//	HookCreateWindow(std::shared_ptr<IWinAPI> winAPI);
	//};

//	class HookDestroyWindow : public Hook, public IHookOnHandle {
//	public:
//		HookDestroyWindow(std::shared_ptr<IWinAPI> winAPI);
//
//		HWND GetHandle() const noexcept override final;
//		void SetHandle(HWND hWnd) noexcept override final;
//	private:
//		HWND hWnd;
//
//		bool canBeEnabled() const noexcept override;
//	};
//
//	class HookMoveWindow : public Hook, public IHookOnHandle {
//	public:
//		HookMoveWindow(std::shared_ptr<IWinAPI> winAPI);
//
//		HWND GetHandle() const noexcept override final;
//		void SetHandle(HWND hWnd) noexcept override final;
//	private:
//		HWND hWnd;
//
//		bool canBeEnabled() const noexcept override;
//	};
}
