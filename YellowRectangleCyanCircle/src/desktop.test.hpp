#pragma once

#include "desktop.hpp"
#include "direct.test.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tuple>
#include <vector>

namespace TestDesktop {
	using namespace YellowRectangleCyanCircle;

	using ::testing::_;
	using ::testing::Eq;
	using ::testing::DoAll;
	using ::testing::Return;
	using ::testing::SaveArg;
	using ::testing::SetArgReferee;

	using TestDirect::MockDirectResource;
	using TestDirect::MockDirectOutput;
	using TestDirect::MockDirectOutput1;
	using TestDirect::MockDirectAdapter;
	using TestDirect::MockDirectFactory1;
	using TestDirect::MockDirectDevice;
	using TestDirect::MockDirect11Device;
	using TestDirect::MockDirect11DeviceContext;
	using TestDirect::MockDirectOutputDuplication;
	using TestDirect::MockDirect;

	namespace Defaults {
		std::wstring DisplayName = L"\\\\.\\DISPLAY1";

		struct CreateDeviceContext {
			CreateDeviceContext();

			std::shared_ptr<MockDirect> Direct;

			std::shared_ptr<MockDirectOutput> Output;
			std::shared_ptr<MockDirectAdapter> Adapter;

			std::shared_ptr<MockDirect11Device> D11Device;
			std::shared_ptr<MockDirect11DeviceContext> DeviceContext;
		};

		struct InitializeOnDeviceContext {
			InitializeOnDeviceContext();

			std::shared_ptr<MockDirect11Device> D11Device;

			std::shared_ptr<MockDirectOutput> Output;
			std::shared_ptr<MockDirectDevice> Device;
			std::shared_ptr<MockDirectAdapter> Adapter;
			std::shared_ptr<MockDirectOutput1> Output1;

			std::shared_ptr<MockDirectOutputDuplication> DeskDupl;
		};

		struct EnumAdaptersContext {
			EnumAdaptersContext();

			std::shared_ptr<MockDirect> Direct;
			std::shared_ptr<MockDirectFactory1> Factory1;
			std::shared_ptr<MockDirectAdapter> Adapter;
		};

		struct DesktopContext {
			DesktopContext();
			~DesktopContext();

			UINT DisplayWidth = 100;
			UINT DisplayHeight = 200;

			std::shared_ptr<MockDirect> Direct;

			std::shared_ptr<MockDirectOutput> Output;
			std::shared_ptr<MockDirectAdapter> Adapter;
			std::shared_ptr<MockDirectFactory1> Factory1;

			std::shared_ptr<MockDirect11Device> D11Device;
			std::shared_ptr<MockDirect11DeviceContext> DeviceContext;

			std::shared_ptr<MockDirectOutput1> Output1;
			std::shared_ptr<MockDirectOutputDuplication> DeskDupl;

			std::shared_ptr<MockDirectResource> Resource;
			std::shared_ptr<DirectTexture2D> FrameCopy;
			std::shared_ptr<DirectTexture2D> FrameDest;
			std::shared_ptr<DirectTexture2D> DesktopImage;
			std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> MappedResource;

			std::vector<std::uint8_t> Memory;
		};

		struct Dummy {
			Dummy();

			std::shared_ptr<Direct11Device> D11Device;
			std::shared_ptr<Direct11DeviceContext> DeviceContext;

			std::shared_ptr<DirectOutputDuplication> DeskDupl;
			std::shared_ptr<DXGI_OUTDUPL_DESC> DeskDuplDesc;

			std::vector<std::uint8_t> Memory;
		};

		std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO> FormatFrameInfo(LONGLONG lptQuadPart = 1);
		std::shared_ptr<DXGI_OUTDUPL_DESC> FormatDeskDuplDesc(UINT width, UINT height, DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED);
		std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> FormatMappedResource(std::vector<std::uint8_t>& memory);
		DXGI_OUTPUT_DESC FormatOutputDesc(std::wstring_view deviceName);
		std::shared_ptr<DesktopContext> PrepareDesktopContext();
	};
}
