#include "direct.hpp"

namespace YellowRectangleCyanCircle {
	// IDXGIResource
	HRESULT DirectResource::QueryInterface(std::shared_ptr<DirectTexture2D>& texture) const {
		if (!this->object) return E_FAIL;

		CComPtr<ID3D11Texture2D> t;
		HRESULT hr = this->object->QueryInterface(IID_PPV_ARGS(&t));
		if (FAILED(hr)) return hr;

		texture = std::make_shared<DirectTexture2D>(t);
		return hr;
	}

	// IDXGIOutput
	HRESULT DirectOutput::GetDesc(DXGI_OUTPUT_DESC& desc) const {
		if (!this->object) return E_FAIL;
		return this->object->GetDesc(&desc);
	}

	HRESULT DirectOutput::GetParent(std::shared_ptr<DirectAdapter>& adapter) const {
		if (!this->object) return E_FAIL;

		CComPtr<IDXGIAdapter> a;
		HRESULT hr = this->object->GetParent(IID_PPV_ARGS(&a));
		if (FAILED(hr)) return hr;

		adapter = std::make_shared<DirectAdapter>(a);
		return hr;
	}

	HRESULT DirectOutput::QueryInterface(std::shared_ptr<DirectOutput1>& output) const {
		if (!this->object) return E_FAIL;

		CComPtr<IDXGIOutput1> o;
		HRESULT hr = this->object->QueryInterface(IID_PPV_ARGS(&o));
		if (FAILED(hr)) return hr;

		output = std::make_shared<DirectOutput1>(o);
		return hr;
	}

	// IDXGIOutput1
	HRESULT DirectOutput1::DuplicateOutput(
		const std::shared_ptr<Direct11Device>& device,
		std::shared_ptr<DirectOutputDuplication>& outputDuplication
	) const {
		if (!this->object) return E_FAIL;
		if (!(device && *device)) return E_FAIL;

		CComPtr<IDXGIOutputDuplication> od;
		HRESULT hr = this->object->DuplicateOutput(device->Get(), &od);
		if (FAILED(hr)) return hr;

		outputDuplication = std::make_shared<DirectOutputDuplication>(od);
		return hr;
	};
	
	// IDXGIAdapter
	HRESULT DirectAdapter::EnumOutputs(UINT index, std::shared_ptr<DirectOutput>& output) const {
		HRESULT hr = DXGI_ERROR_NOT_FOUND;
		if (!this->object) return hr;

		CComPtr<IDXGIOutput> o;
		hr = this->object->EnumOutputs(index, &o);
		if (FAILED(hr)) return hr;

		output = std::make_shared<DirectOutput>(o);
		return hr;
	}

	// IDXGIFactory1
	HRESULT DirectFactory1::EnumAdapters(UINT index, std::shared_ptr<DirectAdapter>& adapter) const {
		HRESULT hr = DXGI_ERROR_NOT_FOUND;
		if (!this->object) return hr;

		CComPtr<IDXGIAdapter> a;
		hr = this->object->EnumAdapters(index, &a);
		if (FAILED(hr)) return hr;

		adapter = std::make_shared<DirectAdapter>(a);
		return hr;
	}

	// IDXGIDevice
	HRESULT DirectDevice::GetParent(std::shared_ptr<DirectAdapter>& adapter) const {
		if (!this->object) return E_FAIL;

		CComPtr<IDXGIAdapter> a;
		HRESULT hr = this->object->GetParent(IID_PPV_ARGS(&a));
		if (FAILED(hr)) return hr;

		adapter = std::make_shared<DirectAdapter>(a);
		return hr;
	}

	// ID3D11Device
	HRESULT Direct11Device::CreateTexture2D(
		const std::shared_ptr<D3D11_TEXTURE2D_DESC>& desc,
		std::shared_ptr<DirectTexture2D>& texture
	) const {
		if (!this->object) return E_FAIL;

		CComPtr<ID3D11Texture2D> t;
		HRESULT hr = this->object->CreateTexture2D(
			desc.get(), 
			nullptr, 
			&t
		);
		if (FAILED(hr)) return hr;

		texture = std::make_shared<DirectTexture2D>(t);
		return hr;
	}

	HRESULT Direct11Device::GetDeviceRemovedReason() const {
		if (!this->object) return E_FAIL;
		return this->GetDeviceRemovedReason();
	}

	HRESULT Direct11Device::QueryInterface(std::shared_ptr<DirectDevice>& device) const {
		if (!this->object) return E_FAIL;

		CComPtr<IDXGIDevice> d;
		HRESULT hr = this->object->QueryInterface(IID_PPV_ARGS(&d));
		if (FAILED(hr)) return hr;

		device = std::make_shared<DirectDevice>(d);
		return hr;
	}

	// ID3D11DeviceContext
	void Direct11DeviceContext::CopyTexture(
		std::shared_ptr<DirectTexture2D>& dst,
		const std::shared_ptr<DirectTexture2D>& src
	) const {
		if (!this->object) return;
		this->object->CopyResource(dst->Get(), src->Get());
	}

	HRESULT Direct11DeviceContext::Map(
		const std::shared_ptr<DirectTexture2D>& texture,
		UINT subresource,
		D3D11_MAP mapType,
		UINT mapFlags,
		std::shared_ptr<D3D11_MAPPED_SUBRESOURCE>& mapped
	) const {
		if (!this->object) return E_FAIL;

		auto m = std::make_shared<D3D11_MAPPED_SUBRESOURCE>();
		HRESULT hr = this->object->Map(
			texture->Get(),
			subresource,
			mapType,
			mapFlags,
			m.get()
		);
		if (FAILED(hr)) return hr;

		mapped = m;
		return hr;
	}

	void Direct11DeviceContext::Unmap(
		const std::shared_ptr<DirectTexture2D> texture, 
		UINT subresource
	) const {
		if (!this->object) return;
		this->object->Unmap(texture->Get(), subresource);
	}

	HRESULT Direct11DeviceContext::SetMultithreadProtected() const {
		if (!this->object) return E_FAIL;

		CComPtr<ID3D10Multithread> multithread;
		HRESULT hr = this->object->QueryInterface(IID_PPV_ARGS(&multithread));
		if (FAILED(hr)) return hr;

		multithread->SetMultithreadProtected(true);
		return hr;
	}

	// IDXGIOutputDuplication
	std::shared_ptr<DXGI_OUTDUPL_DESC> DirectOutputDuplication::GetDesc() const {
		auto d = std::make_shared<DXGI_OUTDUPL_DESC>();
		RtlZeroMemory(d.get(), sizeof(DXGI_OUTDUPL_DESC));

		if (this->object) this->object->GetDesc(d.get());
		return d;
	}

	HRESULT DirectOutputDuplication::AcquireNextFrame(
		UINT timeout,
		std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO>& frameInfo,
		std::shared_ptr<DirectResource>& desktopResource
	) const {
		if (!this->object) return E_FAIL;
		
		auto fi = std::make_shared<DXGI_OUTDUPL_FRAME_INFO>();
		CComPtr<IDXGIResource> dr;
		HRESULT hr = this->object->AcquireNextFrame(timeout, fi.get(), &dr);
		if (FAILED(hr)) return hr;

		frameInfo = fi;
		desktopResource = std::make_shared<DirectResource>(dr);
		return hr;
	}

	HRESULT DirectOutputDuplication::ReleaseFrame() const {
		if (!this->object) return E_FAIL;
		return this->object->ReleaseFrame();
	}

	// Direct API
	HRESULT Direct::D3D11CreateDevice(
		std::shared_ptr<DirectAdapter> adapter,
		D3D_DRIVER_TYPE driverType,
		UINT flags,
		const D3D_FEATURE_LEVEL* featureLevels,
		UINT featureLevelsCount,
		std::shared_ptr<Direct11Device>& device,
		D3D_FEATURE_LEVEL* featureLevel,
		std::shared_ptr<Direct11DeviceContext>& context
	) const {
		
		CComPtr<ID3D11Device> d3d11Device;
		CComPtr<ID3D11DeviceContext> d3d11Context;

		HRESULT hr = ::D3D11CreateDevice(
			adapter ? adapter->Get() : nullptr,
			driverType,
			nullptr,
			flags,
			featureLevels,
			featureLevelsCount,
			D3D11_SDK_VERSION,
			&d3d11Device,
			featureLevel,
			&d3d11Context
		);
		if (FAILED(hr)) return hr;

		device = std::make_shared<Direct11Device>(d3d11Device);
		context = std::make_shared<Direct11DeviceContext>(d3d11Context);

		return hr;
	}

	HRESULT Direct::CreateDXGIFactory1(std::shared_ptr<DirectFactory1>& factory) const {
		CComPtr<IDXGIFactory1> f;
		HRESULT hr = ::CreateDXGIFactory1(IID_PPV_ARGS(&f));
		if (FAILED(hr)) return hr;

		factory = std::make_shared<DirectFactory1>(f);
		return hr;
	}
}
