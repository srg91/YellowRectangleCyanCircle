#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include "utils.hpp"
#include "types.hpp"

#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi1_2.h>

#include <memory>

namespace YellowRectangleCyanCircle {
	// ID3D11Texture2D
	class DirectTexture2D : public ComObject<ID3D11Texture2D> {
	public:
		using ComObject<ID3D11Texture2D>::ComObject;
	};

	// IDXGIResource
	class DirectResource : public ComObject<IDXGIResource> {
	public:
		using ComObject<IDXGIResource>::ComObject;

		virtual HRESULT QueryInterface(std::shared_ptr<DirectTexture2D>& texture) const;
	};

	// IDXGIOutput
	// For DirectOutput.GetParent
	class DirectAdapter;
	// For DirectOutput.QueryInterface
	class DirectOutput1;
	
	class DirectOutput : public ComObject<IDXGIOutput> {
	public:
		using ComObject<IDXGIOutput>::ComObject;

		virtual HRESULT GetDesc(DXGI_OUTPUT_DESC& desc) const;
		virtual HRESULT GetParent(std::shared_ptr<DirectAdapter>& adapter) const;
		virtual HRESULT QueryInterface(std::shared_ptr<DirectOutput1>& output) const;
	};

	// IDXGIOutput1
	// For DirectOutput1.DuplicateOutput
	class Direct11Device;
	class DirectOutputDuplication;

	class DirectOutput1 : public ComObject<IDXGIOutput1> {
	public:
		using ComObject<IDXGIOutput1>::ComObject;

		virtual HRESULT DuplicateOutput(
			const std::shared_ptr<Direct11Device>& device,
			std::shared_ptr< DirectOutputDuplication>& outputDuplication
		) const;
	};

	// IDXGIAdapter
	class DirectAdapter : public ComObject<IDXGIAdapter> {
	public:
		using ComObject<IDXGIAdapter>::ComObject;

		virtual HRESULT EnumOutputs(UINT index, std::shared_ptr<DirectOutput>& output) const;
	};

	// IDXGIFactory1
	class DirectFactory1 : public ComObject<IDXGIFactory1> {
	public:
		using ComObject<IDXGIFactory1>::ComObject;

		virtual HRESULT EnumAdapters(UINT index, std::shared_ptr<DirectAdapter>& adapter) const;
	};

	// IDXGIDevice
	class DirectDevice : public ComObject<IDXGIDevice> {
	public:
		using ComObject<IDXGIDevice>::ComObject;

		virtual HRESULT GetParent(std::shared_ptr<DirectAdapter>& adapter) const;
	};

	// ID3D11Device
	class Direct11Device : public ComObject<ID3D11Device> {
	public:
		using ComObject<ID3D11Device>::ComObject;

		virtual HRESULT CreateTexture2D(
			const std::shared_ptr<D3D11_TEXTURE2D_DESC>& desc,
			std::shared_ptr<DirectTexture2D>& texture
		) const;
		virtual HRESULT GetDeviceRemovedReason() const;
		virtual HRESULT QueryInterface(std::shared_ptr<DirectDevice>& device) const;
	};

	// ID3D11DeviceContext
	class Direct11DeviceContext : public ComObject<ID3D11DeviceContext> {
	public:
		using ComObject<ID3D11DeviceContext>::ComObject;

		virtual void CopyTexture(
			std::shared_ptr<DirectTexture2D>& dst,
			const std::shared_ptr<DirectTexture2D>& src
		) const;

		virtual HRESULT Map(
			const std::shared_ptr<DirectTexture2D>& texture,
			UINT subresource,
			D3D11_MAP mapType,
			UINT mapFlags,
			std::shared_ptr<D3D11_MAPPED_SUBRESOURCE>& mapped
		) const;
		virtual void Unmap(const std::shared_ptr<DirectTexture2D> texture, UINT subresource) const;

		virtual HRESULT SetMultithreadProtected() const;
	};

	// IDXGIOutputDuplication
	class DirectOutputDuplication : public ComObject<IDXGIOutputDuplication> {
	public:
		using ComObject<IDXGIOutputDuplication>::ComObject;
		
		virtual std::shared_ptr<DXGI_OUTDUPL_DESC> GetDesc() const;
		
		virtual HRESULT AcquireNextFrame(
			UINT timeout, 
			std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO>& frameInfo, 
			std::shared_ptr<DirectResource>& desktopResource
		) const;
		virtual HRESULT ReleaseFrame() const;
	};

	// Direct API
	struct IDirect {
		// API
		virtual HRESULT CreateDXGIFactory1(std::shared_ptr<DirectFactory1>& factory) const = 0;
		virtual HRESULT D3D11CreateDevice(
			std::shared_ptr<DirectAdapter> adapter,
			D3D_DRIVER_TYPE driverType,
			UINT flags,
			const D3D_FEATURE_LEVEL* featureLevels,
			UINT featureLevelsCount,
			std::shared_ptr<Direct11Device>& device,
			D3D_FEATURE_LEVEL* featureLevel,
			std::shared_ptr<Direct11DeviceContext>& context
		) const = 0;
	};

	struct Direct : public IDirect {
		HRESULT CreateDXGIFactory1(std::shared_ptr<DirectFactory1>& factory) const override final;
		HRESULT D3D11CreateDevice(
			std::shared_ptr<DirectAdapter> adapter,
			D3D_DRIVER_TYPE driverType,
			UINT flags,
			const D3D_FEATURE_LEVEL* featureLevels,
			UINT featureLevelsCount,
			std::shared_ptr<Direct11Device>& device,
			D3D_FEATURE_LEVEL* featureLevel,
			std::shared_ptr<Direct11DeviceContext>& context
		) const override final;

	};
}
