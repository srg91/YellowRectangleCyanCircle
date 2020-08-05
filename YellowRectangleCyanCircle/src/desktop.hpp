#pragma once

#include "direct.hpp"

#include <chrono>
#include <memory>
#include <vector>
#include <thread>

namespace YellowRectangleCyanCircle {
	namespace DesktopDuplication {
		const D3D_DRIVER_TYPE DriverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};

		const D3D_FEATURE_LEVEL FeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_1
		};

		std::uint32_t CalcDesktopWidth(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc);
		std::uint32_t CalcDesktopHeight(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc);

		HRESULT CreateDevice(
			const std::shared_ptr<IDirect>& direct,
			const std::shared_ptr<DirectOutput>& output,
			std::shared_ptr<Direct11Device>& device,
			std::shared_ptr<Direct11DeviceContext>& context
		);

		HRESULT InitializeOnDevice(
			const std::shared_ptr<Direct11Device>& device,
			std::shared_ptr<DirectOutput> output,
			std::shared_ptr<DirectOutputDuplication>& deskDupl,
			std::shared_ptr<DXGI_OUTDUPL_DESC>& deskDuplDesc
		);

		void FormatFrameDescription(
			const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc,
			std::shared_ptr<D3D11_TEXTURE2D_DESC>& frameDesc,
			std::shared_ptr<D3D11_TEXTURE2D_DESC>& cpuframeDesc
		);

		std::vector<std::shared_ptr<DirectAdapter>> EnumAdapters(
			const std::shared_ptr<IDirect>& direct
		);
		HRESULT OutputByDeviceName(
			const std::shared_ptr<IDirect>& direct, 
			std::wstring_view deviceName, 
			std::shared_ptr<DirectOutput>& output
		);
	}

	class Desktop {
	public:
		Desktop(std::shared_ptr<IDirect> direct);
		Desktop(std::shared_ptr<IDirect> direct, std::wstring_view displayName);

		std::uint32_t GetWidth() const;
		std::uint32_t GetHeight() const;

		void Duplicate(void* output);
		HRESULT SwitchDisplay(std::wstring_view displayName);
	private:
		std::shared_ptr<IDirect> direct;

		std::shared_ptr<Direct11Device> device;
		std::shared_ptr<Direct11DeviceContext> context;
		std::shared_ptr<DirectOutputDuplication> deskDupl;
		std::shared_ptr<DXGI_OUTDUPL_DESC> deskDuplDesc;
		std::shared_ptr<DirectOutput> output;

		std::shared_ptr<D3D11_TEXTURE2D_DESC> frameDesc;
		std::shared_ptr<D3D11_TEXTURE2D_DESC> cpuFrameDesc;

		HRESULT initialize();
		HRESULT duplicate(void* output);
		void uninitialize();
	};
}