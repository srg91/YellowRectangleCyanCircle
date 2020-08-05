#include "desktop.test.hpp"

// TODO: Tests for Desktop class

namespace TestDesktop {
	namespace Defaults {
		CreateDeviceContext::CreateDeviceContext() :
			Direct(std::make_shared<MockDirect>()),
			Output(std::make_shared<MockDirectOutput>()),
			Adapter(std::make_shared<MockDirectAdapter>()),
			D11Device(std::make_shared<MockDirect11Device>()),
			DeviceContext(std::make_shared<MockDirect11DeviceContext>())
		{}

		InitializeOnDeviceContext::InitializeOnDeviceContext() :
			D11Device(std::make_shared<MockDirect11Device>()),
			Output(std::make_shared<MockDirectOutput>()),
			Device(std::make_shared<MockDirectDevice>()),
			Adapter(std::make_shared<MockDirectAdapter>()),
			Output1(std::make_shared<MockDirectOutput1>()),
			DeskDupl(std::make_shared<MockDirectOutputDuplication>())
		{}

		EnumAdaptersContext::EnumAdaptersContext() :
			Direct(std::make_shared<MockDirect>()),
			Factory1(std::make_shared<MockDirectFactory1>()),
			Adapter(std::make_shared<MockDirectAdapter>())
		{}

		DesktopContext::DesktopContext() :
			Direct(std::make_shared<MockDirect>()),
			Output(std::make_shared<MockDirectOutput>()),
			Adapter(std::make_shared<MockDirectAdapter>()),
			Factory1(std::make_shared<MockDirectFactory1>()),
			D11Device(std::make_shared<MockDirect11Device>()),
			DeviceContext(std::make_shared<MockDirect11DeviceContext>()),
			Output1(std::make_shared<MockDirectOutput1>()),
			DeskDupl(std::make_shared<MockDirectOutputDuplication>()),
			Resource(std::make_shared<MockDirectResource>()),
			FrameCopy(std::make_shared<DirectTexture2D>()),
			FrameDest(std::make_shared<DirectTexture2D>()),
			DesktopImage(std::make_shared<DirectTexture2D>()),
			Memory(
				static_cast<std::size_t>(DisplayWidth) * 
				static_cast<std::size_t>(DisplayHeight) * 
				4,
				1
			)
		{
			this->MappedResource = Defaults::FormatMappedResource(this->Memory);
		}

		DesktopContext::~DesktopContext() {
			::testing::Mock::VerifyAndClearExpectations(&*this->MappedResource);
			::testing::Mock::VerifyAndClearExpectations(&*this->DesktopImage);
			::testing::Mock::VerifyAndClearExpectations(&*this->FrameDest);
			::testing::Mock::VerifyAndClearExpectations(&*this->FrameCopy);
			::testing::Mock::VerifyAndClearExpectations(&*this->Resource);

			::testing::Mock::VerifyAndClearExpectations(&*this->DeskDupl);
			::testing::Mock::VerifyAndClearExpectations(&*this->Output1);

			::testing::Mock::VerifyAndClearExpectations(&*this->DeviceContext);
			::testing::Mock::VerifyAndClearExpectations(&*this->D11Device);

			::testing::Mock::VerifyAndClearExpectations(&*this->Factory1);
			::testing::Mock::VerifyAndClearExpectations(&*this->Adapter);
			::testing::Mock::VerifyAndClearExpectations(&*this->Output);

			::testing::Mock::VerifyAndClearExpectations(&*this->Direct);
		}

		Dummy::Dummy() :
			D11Device(nullptr),
			DeviceContext(nullptr),
			DeskDuplDesc(nullptr),
			Memory()
		{}

		std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO> FormatFrameInfo(LONGLONG lptQuadPart) {
			auto fi = std::make_shared<DXGI_OUTDUPL_FRAME_INFO>();
			fi->LastPresentTime.QuadPart = lptQuadPart;
			return fi;
		}

		std::shared_ptr<DXGI_OUTDUPL_DESC> FormatDeskDuplDesc(UINT width, UINT height, DXGI_MODE_ROTATION rotation) {
			auto d = std::make_shared<DXGI_OUTDUPL_DESC>();
			d->ModeDesc.Width = width;
			d->ModeDesc.Height = height;
			d->Rotation = rotation;
			return d;
		}

		std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> FormatMappedResource(std::vector<std::uint8_t>& memory) {
			auto d = std::make_shared<D3D11_MAPPED_SUBRESOURCE>();
			d->pData = reinterpret_cast<void*>(std::data(memory));
			return d;
		}

		DXGI_OUTPUT_DESC Defaults::FormatOutputDesc(std::wstring_view deviceName) {
			DXGI_OUTPUT_DESC d;
			RtlZeroMemory(&d, sizeof(DXGI_OUTPUT_DESC));
			wcsncpy_s(d.DeviceName, std::data(deviceName), sizeof(d.DeviceName) / sizeof(wchar_t));
			return d;
		}

		std::shared_ptr<DesktopContext> PrepareDesktopContext() {
			auto c = std::make_shared<DesktopContext>();

			EXPECT_CALL(*c->Direct, CreateDXGIFactory1).
				WillRepeatedly(DoAll(
					SetArgReferee<0>(c->Factory1),
					Return(S_OK)
				));

			EXPECT_CALL(*c->Factory1, EnumAdapters).
				WillOnce(DoAll(
					SetArgReferee<1>(c->Adapter),
					Return(S_OK)
				)).
				WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

			EXPECT_CALL(*c->Adapter, EnumOutputs).
				WillOnce(DoAll(
					SetArgReferee<1>(c->Output),
					Return(S_OK)
				)).
				WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

			EXPECT_CALL(*c->Output, GetDesc).
				WillRepeatedly(DoAll(
					SetArgReferee<0>(Defaults::FormatOutputDesc(DisplayName)),
					Return(S_OK)
				));

			EXPECT_CALL(*c->Direct, D3D11CreateDevice).
				WillRepeatedly(DoAll(
					SetArgReferee<5>(c->D11Device),
					SetArgReferee<7>(c->DeviceContext),
					Return(S_OK)
				));

			EXPECT_CALL(*c->DeviceContext, SetMultithreadProtected).Times(1);

			EXPECT_CALL(*c->Output, GetParent).
				WillRepeatedly(DoAll(
					SetArgReferee<0>(c->Adapter),
					Return(S_OK)
				));

			EXPECT_CALL(*c->Output, QueryInterface).
				WillRepeatedly(DoAll(
					SetArgReferee<0>(c->Output1),
					Return(S_OK)
				));

			EXPECT_CALL(*c->Output1, DuplicateOutput).
				WillRepeatedly(DoAll(
					SetArgReferee<1>(c->DeskDupl),
					Return(S_OK)
				));

			EXPECT_CALL(*c->DeskDupl, GetDesc).
				WillRepeatedly(Return(FormatDeskDuplDesc(c->DisplayWidth, c->DisplayHeight)));

			EXPECT_CALL(*c->DeskDupl, ReleaseFrame).Times(2);

			EXPECT_CALL(*c->DeskDupl, AcquireNextFrame).
				WillOnce(DoAll(
					SetArgReferee<1>(FormatFrameInfo()),
					SetArgReferee<2>(c->Resource),
					Return(S_OK)
				));

			EXPECT_CALL(*c->D11Device, GetDeviceRemovedReason).Times(0);

			EXPECT_CALL(*c->D11Device, CreateTexture2D).
				WillOnce(DoAll(
					SetArgReferee<1>(c->FrameCopy),
					Return(S_OK)
				)).
				WillOnce(DoAll(
					SetArgReferee<1>(c->FrameDest),
					Return(S_OK)
				));

			EXPECT_CALL(*c->Resource, QueryInterface).
				WillOnce(DoAll(
					SetArgReferee<0>(c->DesktopImage),
					Return(S_OK)
				));

			EXPECT_CALL(*c->DeviceContext, Map(Eq(c->FrameDest), _, _, _, _)).
				WillOnce(DoAll(
					SetArgReferee<4>(c->MappedResource),
					Return(S_OK)
				));

			EXPECT_CALL(*c->DeviceContext, Unmap(Eq(c->FrameDest), _)).Times(1);

			return c;
		};
	}

	TEST(TestDesktopDuplication, CalcDesktopSize) {
		std::uint32_t originalWidth = 100;
		std::uint32_t originalHeight = 200;

		std::vector<std::tuple<DXGI_MODE_ROTATION, UINT, UINT>> cases = {
			std::make_tuple(DXGI_MODE_ROTATION_UNSPECIFIED, 100, 200),
			std::make_tuple(DXGI_MODE_ROTATION_ROTATE180, 100, 200),
			std::make_tuple(DXGI_MODE_ROTATION_ROTATE90, 200, 100),
			std::make_tuple(DXGI_MODE_ROTATION_ROTATE270, 200, 100),
		};

		for (const auto [rotation, expectedWidth, expectedHeight] : cases) {
			auto d = Defaults::FormatDeskDuplDesc(originalWidth, originalHeight, rotation);
			auto w = DesktopDuplication::CalcDesktopWidth(d);
			auto h = DesktopDuplication::CalcDesktopHeight(d);

			EXPECT_EQ(w, expectedWidth);
			EXPECT_EQ(h, expectedHeight);
		}
	}

	TEST(TestDesktopDuplication, CreateDeviceWithOutput) {
		Defaults::CreateDeviceContext c;

		EXPECT_CALL(*c.Output, GetParent).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Adapter),
				Return(S_OK)
			));

		EXPECT_CALL(*c.Direct, D3D11CreateDevice(Eq(c.Adapter), Eq(D3D_DRIVER_TYPE_UNKNOWN), _, _, _, _, _, _))
			.Times(1)
			.WillOnce(DoAll(
				SetArgReferee<5>(c.D11Device),
				SetArgReferee<7>(c.DeviceContext),
				Return(S_OK)
			));

		EXPECT_CALL(*c.DeviceContext, SetMultithreadProtected)
			.Times(1);

		Defaults::Dummy d;
		auto hr = DesktopDuplication::CreateDevice(c.Direct, c.Output, d.D11Device, d.DeviceContext);
		EXPECT_EQ(hr, S_OK);
		EXPECT_EQ(d.D11Device, c.D11Device);
		EXPECT_EQ(d.DeviceContext, c.DeviceContext);
	}

	TEST(TestDesktopDuplication, CreateDeviceWithoutOutput) {
		Defaults::CreateDeviceContext c;

		EXPECT_CALL(*c.Output, GetParent).Times(0);
		EXPECT_CALL(*c.Direct, D3D11CreateDevice(Eq(nullptr), Eq(D3D_DRIVER_TYPE_HARDWARE), _, _, _, _, _, _))
			.Times(1)
			.WillOnce(DoAll(
				SetArgReferee<5>(c.D11Device),
				SetArgReferee<7>(c.DeviceContext),
				Return(S_OK)
			));
		EXPECT_CALL(*c.DeviceContext, SetMultithreadProtected)
			.Times(1);

		Defaults::Dummy d;
		auto hr = DesktopDuplication::CreateDevice(c.Direct, nullptr, d.D11Device, d.DeviceContext);
		EXPECT_EQ(hr, S_OK);
		EXPECT_EQ(d.D11Device, c.D11Device);
		EXPECT_EQ(d.DeviceContext, c.DeviceContext);
	}

	TEST(TestDesktopDuplication, CreateDeviceFailed) {
		Defaults::CreateDeviceContext c;

		auto createDeviceCallCount = static_cast<int>(std::size(DesktopDuplication::DriverTypes));

		EXPECT_CALL(*c.Direct, D3D11CreateDevice)
			.Times(createDeviceCallCount)
			.WillRepeatedly(Return(E_FAIL));
		EXPECT_CALL(*c.DeviceContext, SetMultithreadProtected).Times(0);

		Defaults::Dummy d;
		auto hr = DesktopDuplication::CreateDevice(c.Direct, nullptr, d.D11Device, d.DeviceContext);
		EXPECT_EQ(hr, E_FAIL);
	}

	TEST(TestDesktopDuplication, InitializeOnDeviceWithOutput) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.D11Device, QueryInterface).Times(0);
		EXPECT_CALL(*c.Device, GetParent).Times(0);
		EXPECT_CALL(*c.Adapter, EnumOutputs).Times(0);

		EXPECT_CALL(*c.Output, QueryInterface).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Output1),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Output1, DuplicateOutput).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<1>(c.DeskDupl),
				Return(S_OK)
			));

		auto expectedDesc = std::make_shared<DXGI_OUTDUPL_DESC>();
		EXPECT_CALL(*c.DeskDupl, GetDesc).
			Times(1).
			WillOnce(Return(expectedDesc));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			c.Output,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, S_OK);
		EXPECT_EQ(d.DeskDupl, c.DeskDupl);
		EXPECT_EQ(d.DeskDuplDesc, expectedDesc);
	}

	TEST(TestDesktopDuplication, InitializeOnDeviceWithoutOutput) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.D11Device, QueryInterface).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Device),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Device, GetParent).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Adapter),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Adapter, EnumOutputs).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<1>(c.Output),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Output, QueryInterface).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Output1),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Output1, DuplicateOutput).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<1>(c.DeskDupl),
				Return(S_OK)
			));

		auto expectedDesc = std::make_shared<DXGI_OUTDUPL_DESC>();
		EXPECT_CALL(*c.DeskDupl, GetDesc).
			Times(1).
			WillOnce(Return(expectedDesc));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			nullptr,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, S_OK);
		EXPECT_EQ(d.DeskDupl, c.DeskDupl);
		EXPECT_EQ(d.DeskDuplDesc, expectedDesc);
	}


	TEST(TestDesktopDuplication, InitializeOnDeviceUnableDuplicate) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.D11Device, QueryInterface).Times(0);
		EXPECT_CALL(*c.Device, GetParent).Times(0);
		EXPECT_CALL(*c.Adapter, EnumOutputs).Times(0);
		EXPECT_CALL(*c.Output1, DuplicateOutput).Times(0);
		EXPECT_CALL(*c.DeskDupl, GetDesc).Times(0);

		EXPECT_CALL(*c.Output, QueryInterface).
			Times(1).
			WillOnce(Return(E_FAIL));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			c.Output,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, E_FAIL);
	}

	TEST(TestDesktopDuplication, InitializeOnDeviceUnableQueryOutput1) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.D11Device, QueryInterface).Times(0);
		EXPECT_CALL(*c.Device, GetParent).Times(0);
		EXPECT_CALL(*c.Adapter, EnumOutputs).Times(0);
		EXPECT_CALL(*c.DeskDupl, GetDesc).Times(0);

		EXPECT_CALL(*c.Output, QueryInterface).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Output1),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Output1, DuplicateOutput).
			Times(1).
			WillOnce(Return(E_FAIL));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			c.Output,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, E_FAIL);
	}

	TEST(TestDesktopDuplication, InitializeOnDeviceUnableEnumOutputs) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.Output1, DuplicateOutput).Times(0);
		EXPECT_CALL(*c.DeskDupl, GetDesc).Times(0);

		EXPECT_CALL(*c.D11Device, QueryInterface).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Device),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Device, GetParent).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Adapter),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Adapter, EnumOutputs).
			Times(1).
			WillOnce(Return(E_FAIL));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			nullptr,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, E_FAIL);
	}

	TEST(TestDesktopDuplication, InitializeOnDeviceUnableGetParent) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.Adapter, EnumOutputs).Times(0);
		EXPECT_CALL(*c.Output1, DuplicateOutput).Times(0);
		EXPECT_CALL(*c.DeskDupl, GetDesc).Times(0);

		EXPECT_CALL(*c.D11Device, QueryInterface).
			Times(1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Device),
				Return(S_OK)
			));
		EXPECT_CALL(*c.Device, GetParent).
			Times(1).
			WillOnce(Return(E_FAIL));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			nullptr,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, E_FAIL);
	}

	TEST(TestDesktopDuplication, InitializeOnDeviceUnableQueryInterface) {
		Defaults::InitializeOnDeviceContext c;

		EXPECT_CALL(*c.Device, GetParent).Times(0);
		EXPECT_CALL(*c.Adapter, EnumOutputs).Times(0);
		EXPECT_CALL(*c.Output1, DuplicateOutput).Times(0);
		EXPECT_CALL(*c.DeskDupl, GetDesc).Times(0);

		EXPECT_CALL(*c.D11Device, QueryInterface).
			Times(1).
			WillOnce(Return(E_FAIL));

		Defaults::Dummy d;
		auto hr = DesktopDuplication::InitializeOnDevice(
			c.D11Device,
			nullptr,
			d.DeskDupl,
			d.DeskDuplDesc
		);
		EXPECT_EQ(hr, E_FAIL);
	}

	TEST(TestDesktopDuplication, FormatFrameDescription) {
		auto desc = Defaults::FormatDeskDuplDesc(100, 200);

		std::shared_ptr<D3D11_TEXTURE2D_DESC> frame, cpuFrame;
		DesktopDuplication::FormatFrameDescription(desc, frame, cpuFrame);

		EXPECT_NE(frame, nullptr);
		EXPECT_NE(cpuFrame, nullptr);

		EXPECT_EQ(frame->Width, desc->ModeDesc.Width);
		EXPECT_EQ(frame->Height, desc->ModeDesc.Height);
		EXPECT_EQ(frame->BindFlags, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET);
		EXPECT_EQ(frame->CPUAccessFlags, 0);
		EXPECT_EQ(frame->Usage, D3D11_USAGE_DEFAULT);

		EXPECT_EQ(cpuFrame->Width, desc->ModeDesc.Width);
		EXPECT_EQ(cpuFrame->Height, desc->ModeDesc.Height);
		EXPECT_EQ(cpuFrame->BindFlags, 0);
		EXPECT_EQ(cpuFrame->CPUAccessFlags, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
		EXPECT_EQ(cpuFrame->Usage, D3D11_USAGE_STAGING);
	}

	TEST(TestDesktopDuplication, EnumAdapters) {
		Defaults::EnumAdaptersContext c;

		EXPECT_CALL(*c.Factory1, EnumAdapters).
			WillOnce(DoAll(
				SetArgReferee<1>(c.Adapter),
				Return(S_OK)
			)).
			WillOnce(DoAll(
				SetArgReferee<1>(c.Adapter),
				Return(S_OK)
			)).
			WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

		EXPECT_CALL(*c.Direct, CreateDXGIFactory1).
			WillOnce(DoAll(
				SetArgReferee<0>(c.Factory1),
				Return(S_OK)
			));

		auto adapters = DesktopDuplication::EnumAdapters(c.Direct);
		EXPECT_EQ(std::size(adapters), 2);
	}

	TEST(TestDesktopDuplication, OutputByDeviceName) {
		auto direct = std::make_shared<MockDirect>();
		auto factory = std::make_shared<MockDirectFactory1>();

		auto expectedAdapter = std::make_shared<MockDirectAdapter>();
		auto otherAdapter = std::make_shared<MockDirectAdapter>();

		auto expectedOutput = std::make_shared<MockDirectOutput>();
		auto otherOutput = std::make_shared<MockDirectOutput>();

		EXPECT_CALL(*otherOutput, GetDesc).
			WillOnce(DoAll(
				SetArgReferee<0>(
					Defaults::FormatOutputDesc(L"\\\\.\\DISPLAY1")
				),
				Return(S_OK)
			)).
			WillOnce(DoAll(
				SetArgReferee<0>(
					Defaults::FormatOutputDesc(L"\\\\.\\DISPLAY2")
					),
				Return(S_OK)
			));
		EXPECT_CALL(*expectedOutput, GetDesc).
			WillOnce(DoAll(
				SetArgReferee<0>(
					Defaults::FormatOutputDesc(L"\\\\.\\DISPLAY3")
					),
				Return(S_OK)
			)).
			WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

		EXPECT_CALL(*otherAdapter, EnumOutputs).
			WillOnce(DoAll(
				SetArgReferee<1>(otherOutput),
				Return(S_OK)
			)).
			WillOnce(DoAll(
				SetArgReferee<1>(otherOutput),
				Return(S_OK)
			)).
			WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

		EXPECT_CALL(*expectedAdapter, EnumOutputs).
			WillOnce(DoAll(
				SetArgReferee<1>(expectedOutput),
				Return(S_OK)
			)).
			WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

		EXPECT_CALL(*factory, EnumAdapters).
			WillOnce(DoAll(
				SetArgReferee<1>(otherAdapter),
				Return(S_OK)
			)).
			WillOnce(DoAll(
				SetArgReferee<1>(expectedAdapter),
				Return(S_OK)
			)).
			WillRepeatedly(Return(DXGI_ERROR_NOT_FOUND));

		EXPECT_CALL(*direct, CreateDXGIFactory1).
			WillOnce(DoAll(
				SetArgReferee<0>(factory),
				Return(S_OK)
			));

		std::wstring expectedDeviceName = L"\\\\.\\DISPLAY3";
		std::shared_ptr<DirectOutput> resultOutput;
		auto hr = DesktopDuplication::OutputByDeviceName(direct, expectedDeviceName, resultOutput);
		EXPECT_EQ(resultOutput, expectedOutput);
	}

	TEST(TestDesktop, DuplicateSuccess) {
		auto c = Defaults::PrepareDesktopContext();
		Desktop desktop(c->Direct, Defaults::DisplayName);

		std::shared_ptr<DirectTexture2D> fctFA, fctSA;
		std::shared_ptr<DirectTexture2D> sctFA, sctSA;

		EXPECT_CALL(*c->DeviceContext, CopyTexture).
			Times(2).
			WillOnce(DoAll(
				SaveArg<0>(&fctFA),
				SaveArg<1>(&fctSA)
			)).
			WillOnce(DoAll(
				SaveArg<0>(&sctFA),
				SaveArg<1>(&sctSA)
			));

		Defaults::Dummy d;
		d.Memory.resize(
			static_cast<std::size_t>(desktop.GetWidth()) * 
			static_cast<std::size_t>(desktop.GetHeight()) *
			4
		);
		desktop.Duplicate(std::data(d.Memory));
		EXPECT_EQ(d.Memory, c->Memory);

		EXPECT_EQ(fctFA, c->FrameCopy);
		EXPECT_EQ(fctSA, c->DesktopImage);

		EXPECT_EQ(sctFA, c->FrameDest);
		EXPECT_EQ(sctSA, c->FrameCopy);
	}
}
