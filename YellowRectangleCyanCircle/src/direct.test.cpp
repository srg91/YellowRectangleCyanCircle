#include "direct.test.hpp"

namespace TestDirect {
	TEST(TestDirect, DirectResource) {
		DirectResource r;

		std::shared_ptr<DirectTexture2D> texture;
		EXPECT_EQ(r.QueryInterface(texture), E_FAIL);
	}

	TEST(TestDirect, DirectOutput) {
		DirectOutput o;

		DXGI_OUTPUT_DESC desc;
		std::shared_ptr<DirectAdapter> adapter;
		std::shared_ptr<DirectOutput1> output1;

		EXPECT_EQ(o.GetDesc(desc), E_FAIL);
		EXPECT_EQ(o.GetParent(adapter), E_FAIL);
		EXPECT_EQ(o.QueryInterface(output1), E_FAIL);
	}

	TEST(TestDirect, DirectOutput1) {
		DirectOutput1 o;

		std::shared_ptr<Direct11Device> device;
		std::shared_ptr<DirectOutputDuplication> output;

		EXPECT_EQ(o.DuplicateOutput(device, output), E_FAIL);
	}

	TEST(TestDirect, DirectAdapter) {
		DirectAdapter adapter;

		std::shared_ptr<DirectOutput> output;
		EXPECT_EQ(adapter.EnumOutputs(0, output), DXGI_ERROR_NOT_FOUND);
	}

	TEST(TestDirect, DirectFactory1) {
		DirectFactory1 factory;

		std::shared_ptr<DirectAdapter> adapter;
		EXPECT_EQ(factory.EnumAdapters(0, adapter), DXGI_ERROR_NOT_FOUND);
	}

	TEST(TestDirect, DirectDevice) {
		DirectDevice device;

		std::shared_ptr<DirectAdapter> adapter;
		EXPECT_EQ(device.GetParent(adapter), E_FAIL);
	}

	TEST(TestDirect, Direct11Device) {
		Direct11Device device;

		std::shared_ptr<DirectDevice> d;
		std::shared_ptr<D3D11_TEXTURE2D_DESC> desc;
		std::shared_ptr<DirectTexture2D> texture;

		EXPECT_EQ(device.CreateTexture2D(desc, texture), E_FAIL);
		EXPECT_EQ(device.GetDeviceRemovedReason(), E_FAIL);
		EXPECT_EQ(device.QueryInterface(d), E_FAIL);
	}

	TEST(TestDirect, Direct11DeviceContext) {
		Direct11DeviceContext context;

		std::shared_ptr<DirectTexture2D> texture;
		std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> ms;

		context.CopyTexture(texture, texture);
		EXPECT_EQ(context.SetMultithreadProtected(), E_FAIL);
	
		EXPECT_EQ(context.Map(texture, 0, D3D11_MAP_READ_WRITE, 0, ms), E_FAIL);
		context.Unmap(texture, 0);
	}

	MockDirect::MockDirect() {}
	MockDirect::~MockDirect() {}

	MockDirectResource::MockDirectResource() {}
	MockDirectResource::~MockDirectResource() {}

	MockDirectOutput::MockDirectOutput() {}
	MockDirectOutput::~MockDirectOutput() {}

	MockDirectOutput1::MockDirectOutput1() {}
	MockDirectOutput1::~MockDirectOutput1() {}

	MockDirectAdapter::MockDirectAdapter() {}
	MockDirectAdapter::~MockDirectAdapter() {}

	MockDirectFactory1::MockDirectFactory1() {}
	MockDirectFactory1::~MockDirectFactory1() {}

	MockDirectDevice::MockDirectDevice() {}
	MockDirectDevice::~MockDirectDevice() {}

	MockDirect11Device::MockDirect11Device() {}
	MockDirect11Device::~MockDirect11Device() {}

	MockDirect11DeviceContext::MockDirect11DeviceContext() {}
	MockDirect11DeviceContext::~MockDirect11DeviceContext() {}

	MockDirectOutputDuplication::MockDirectOutputDuplication() {}
	MockDirectOutputDuplication::~MockDirectOutputDuplication() {}
}