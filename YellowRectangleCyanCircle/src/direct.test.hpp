#pragma once

#include "direct.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace TestDirect {
	using namespace YellowRectangleCyanCircle;

	namespace Defaults {}

	struct MockDirectResource : public DirectResource {
		MockDirectResource();
		virtual ~MockDirectResource();

		MOCK_METHOD(HRESULT, QueryInterface, (std::shared_ptr<DirectTexture2D>&), (const, override));
	};

	struct MockDirectOutput : public DirectOutput {
		MockDirectOutput();
		virtual ~MockDirectOutput();

		MOCK_METHOD(HRESULT, GetDesc, (DXGI_OUTPUT_DESC&), (const, override));
		MOCK_METHOD(HRESULT, GetParent, (std::shared_ptr<DirectAdapter>&), (const, override));
		MOCK_METHOD(HRESULT, QueryInterface, (std::shared_ptr<DirectOutput1>&), (const, override));
	};

	struct MockDirectOutput1 : public DirectOutput1 {
		MockDirectOutput1();
		virtual ~MockDirectOutput1();

		MOCK_METHOD(
			HRESULT,
			DuplicateOutput,
			(const std::shared_ptr<Direct11Device>&, std::shared_ptr< DirectOutputDuplication>&),
			(const, override)
		);
	};

	struct MockDirectAdapter : public DirectAdapter {
		MockDirectAdapter();
		virtual ~MockDirectAdapter();

		MOCK_METHOD(
			HRESULT,
			EnumOutputs,
			(UINT, std::shared_ptr<DirectOutput>&),
			(const, override)
		);
	};

	struct MockDirectFactory1 : public DirectFactory1 {
		MockDirectFactory1();
		virtual ~MockDirectFactory1();

		MOCK_METHOD(HRESULT, EnumAdapters, (UINT, std::shared_ptr<DirectAdapter>&), (const, override));
	};

	struct MockDirectDevice : public DirectDevice {
		MockDirectDevice();
		virtual ~MockDirectDevice();

		MOCK_METHOD(HRESULT, GetParent, (std::shared_ptr<DirectAdapter>&), (const, override));
	};

	struct MockDirect11Device : public Direct11Device {
		MockDirect11Device();
		virtual ~MockDirect11Device();

		MOCK_METHOD(
			HRESULT,
			CreateTexture2D,
			(const std::shared_ptr<D3D11_TEXTURE2D_DESC>&, std::shared_ptr<DirectTexture2D>&),
			(const, override)
		);
		MOCK_METHOD(HRESULT, GetDeviceRemovedReason, (),  (const, override));
		MOCK_METHOD(HRESULT, QueryInterface, (std::shared_ptr<DirectDevice>&), (const, override));
	};

	struct MockDirect11DeviceContext : public Direct11DeviceContext {
		MockDirect11DeviceContext();
		virtual ~MockDirect11DeviceContext();

		MOCK_METHOD(
			void,
			CopyTexture,
			(std::shared_ptr<DirectTexture2D>&, const std::shared_ptr<DirectTexture2D>&),
			(const, override)
		);
		MOCK_METHOD(
			HRESULT,
			Map,
			(
				const std::shared_ptr<DirectTexture2D>&,
				UINT,
				D3D11_MAP,
				UINT,
				std::shared_ptr<D3D11_MAPPED_SUBRESOURCE>&
			),
			(const, override)
		);
		MOCK_METHOD(void, Unmap, (const std::shared_ptr<DirectTexture2D>, UINT), (const, override));
		MOCK_METHOD(HRESULT, SetMultithreadProtected, (), (const, override));
	};

	struct MockDirectOutputDuplication : public DirectOutputDuplication {
		MockDirectOutputDuplication();
		virtual ~MockDirectOutputDuplication();

		MOCK_METHOD(std::shared_ptr<DXGI_OUTDUPL_DESC>, GetDesc, (), (const, override));
		MOCK_METHOD(
			HRESULT,
			AcquireNextFrame,
			(UINT, std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO>&, std::shared_ptr<DirectResource>&),
			(const, override)
		);
		MOCK_METHOD(HRESULT, ReleaseFrame, (), (const, override));
	};

	struct MockDirect : public IDirect {
		MockDirect();
		virtual ~MockDirect();

		MOCK_METHOD(HRESULT, CreateDXGIFactory1, (std::shared_ptr<DirectFactory1>&), (const, override));
		MOCK_METHOD(
			HRESULT, 
			D3D11CreateDevice, 
			(
				std::shared_ptr<DirectAdapter>,
				D3D_DRIVER_TYPE,
				UINT,
				const D3D_FEATURE_LEVEL*,
				UINT,
				std::shared_ptr<Direct11Device>&,
				D3D_FEATURE_LEVEL*,
				std::shared_ptr<Direct11DeviceContext>&
			),
			(const, override)
		);
	};
}
