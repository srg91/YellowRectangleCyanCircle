#pragma once

#include "winapi.hpp"
#include "types.hpp"

#include <Windows.h>

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>

namespace YellowRectangleCyanCircle {
	class HookAlreadyRegistered : public std::exception {
	public:
		HookAlreadyRegistered(HookEventId eventId);

		const char* what() const;
	private:
		HookEventId eventId;
	};

	struct IHook {
		virtual ~IHook() {};

		virtual void Enable() = 0;
		virtual void Disable() = 0;

		virtual bool IsEnabled() const noexcept = 0;
	};

	class Hook : public IHook {
	public:
		Hook(HookEventId eventId, std::shared_ptr<IWinAPI> winAPI, std::function<void(Handle)>&& func);
		~Hook() override;

		bool IsEnabled() const noexcept override final;
	protected:
		bool isEnabled;
		HookEventId eventId;

		HWINEVENTHOOK hook;

		std::shared_ptr<IWinAPI> winAPI;
		std::function<void(Handle)> func;

		static std::unordered_map<HookEventId, Hook*> instances;
		static std::mutex instancesMutex;

		static Hook* getInstance(HookEventId eventId);
		static void registerInstance(HookEventId eventId, Hook* instance);
		static void unregisterInstance(HookEventId eventId);
	};

	class HookCreateWindow : public Hook {
	public:
		HookCreateWindow(std::shared_ptr<IWinAPI> winAPI, std::function<void(Handle)>&& func);

		void Enable() override final;
		void Disable() override final;
	private:
		static void CALLBACK hookCallback(
			HWINEVENTHOOK hook,
			HookEventId event,
			Handle hWnd,
			LONG idObject,
			LONG idChild,
			DWORD dwEventThread,
			DWORD dwmsEventTime
		);
	};
}
