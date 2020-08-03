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
	using ::testing::SetArgPointee;
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
		std::shared_ptr<DXGI_OUTDUPL_DESC> FormatDeskDuplDesc(UINT Width, UINT Height, DXGI_MODE_ROTATION Rotation = DXGI_MODE_ROTATION_UNSPECIFIED);
		DXGI_OUTPUT_DESC FormatOutputDesc(std::wstring_view deviceName);

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

		struct Dummy {
			Dummy();

			std::shared_ptr<Direct11Device> D11Device;
			std::shared_ptr<Direct11DeviceContext> DeviceContext;

			std::shared_ptr<DirectOutputDuplication> DeskDupl;
			std::shared_ptr<DXGI_OUTDUPL_DESC> DeskDuplDesc;
		};

	};
}
