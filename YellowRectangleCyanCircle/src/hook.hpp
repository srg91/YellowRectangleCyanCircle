#pragma once

#include "interface.hpp"
#include "winapi.hpp"
#include "types.hpp"

#include <Windows.h>

#include <atomic>
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

	class Hook : public IHook {
	public:
		Hook(Hook&& other) noexcept;
		Hook(std::shared_ptr<IWinAPI> winAPI, DWORD eventId, HWND hWnd = 0, bool hWndStrict = false, bool assignToWindow = false);
		~Hook();
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

		// Looks like then we processing callback it can be interrupted
		// This is little way to recursive calls
		static std::unordered_map<DWORD, std::atomic<bool>> callbackBreaker;

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
}
