#include "desktop.gdi.hpp"

namespace YellowRectangleCyanCircle {
    GDIDesktop::GDIDesktop() :
        available(false),
        dc(nullptr)
    {}

    GDIDesktop::~GDIDesktop() {
        this->uninitialize();
    }

    DesktopInfo GDIDesktop::GetDesktopInfo() const {
        return this->desktopInfo;
    };

    bool GDIDesktop::IsAvailable() const {
        L(trace, "[GDIDesktop::IsAvailable] called, return: {}", this->available);
        return this->available;
    }

    bool GDIDesktop::Duplicate(void* output) {
        L(trace, "[GDIDesktop::Duplicate] called");

        HRESULT hr = S_OK;

        if (!this->dc) {
            L(debug, "[GDIDesktop::Duplicate] has no device context, reinitialize");
            hr = this->initialize();
            if (FAILED(hr)) return false;
        }

        hr = this->duplicate(output);
        if (FAILED(hr)) {
            this->uninitialize();
            return false;
        }

        return true;
    }

    bool GDIDesktop::SetDisplay(std::wstring_view displayName) {
        L(trace, L"[GDIDesktop::SetDisplay] called with display name: {}", displayName);

        this->available = false;
        this->desktopInfo = DesktopInfo(displayName);

        auto hr = this->initialize();
        if (FAILED(hr)) return false;

        this->available = true;
        return this->available;
    }

    HRESULT GDIDesktop::duplicate(void* output) {
        L(trace, "[GDIDesktop::duplicate] called");

        HRESULT hr = S_OK;
        if (!this->dc) {
            L(debug, "[GDIDesktop::duplicate] duplicate called without device context");
            return E_NOT_VALID_STATE;
        }

        auto mdc = CreateCompatibleDC(this->dc);
        if (!mdc) {
            L(debug, "[GDIDesktop::duplicate] failed to create compatible dc");
            return E_FAIL;
        }

        auto width = this->desktopInfo.Rect.width;
        auto height = this->desktopInfo.Rect.height;

        auto comBit = CreateCompatibleBitmap(this->dc, width, height);
        if (!comBit) {
            L(debug, "[GDIDesktop::duplicate] failed to create compatible bitmap");
            DeleteDC(mdc);
            return E_FAIL;
        }

        auto bitmap = static_cast<HBITMAP>(SelectObject(mdc, comBit));
        if (!bitmap || bitmap == HGDI_ERROR) {
            L(debug, "[GDIDesktop::duplicate] failed to select bitmap");
            DeleteDC(mdc);
            return E_FAIL;
        }

        if (!BitBlt(mdc, 0, 0, width, height, this->dc, 0, 0, SRCCOPY)) {
            L(debug, "[GDIDesktop::duplicate] unable to copy with bitblt");
            DeleteDC(mdc);
            return E_FAIL;
        }

        BITMAP raw;
        RtlZeroMemory(&raw, sizeof(raw));
        if (!GetObject(comBit, sizeof(raw), &raw)) {
            L(debug, "[GDIDesktop::duplicate] unable to get bitmap");
            DeleteDC(mdc);
            return E_FAIL;
        }

        BITMAPINFO bmpInfo;
        RtlZeroMemory(&bmpInfo, sizeof(bmpInfo));

        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = raw.bmWidth;
        bmpInfo.bmiHeader.biHeight = -raw.bmHeight;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = raw.bmBitsPixel;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = raw.bmWidth * raw.bmHeight * 4;

        if (!GetDIBits(mdc, comBit, 0, raw.bmHeight, output, &bmpInfo, DIB_RGB_COLORS)) {
            L(debug, "[GDIDesktop::duplicate] unable to get bits");
            DeleteDC(mdc);
            return E_FAIL;
        }

        DeleteDC(mdc);

        L(debug, "[GDIDesktop::duplicate] success");
        return S_OK;
    }

    HRESULT GDIDesktop::initialize() {
        L(trace, L"[GDIDesktop::initialize] called");

        if (this->dc) DeleteDC(this->dc);
        this->dc = CreateDC(std::data(this->desktopInfo.Name), nullptr, nullptr, nullptr);
        if (!this->dc) {
            L(debug, "[GDIDesktop::initialize] failed, unable to create dc");
            return E_FAIL;
        }

        DEVMODE mode;
        RtlZeroMemory(&mode, sizeof(mode));
        if (!EnumDisplaySettings(std::data(this->desktopInfo.Name), ENUM_CURRENT_SETTINGS, &mode)) {
            L(debug, "[GDIDesktop::initialize] failed to enum display settings");
            return E_FAIL;
        }

        auto width = mode.dmPelsWidth;
        auto height = mode.dmPelsHeight;

        int scaledWidth = GetDeviceCaps(this->dc, HORZRES);
        int scaledHeight = GetDeviceCaps(this->dc, VERTRES);

        this->desktopInfo.Rect = Rect::Rect(
            mode.dmPosition.x,
            mode.dmPosition.y,
            width,
            height
        );

        this->desktopInfo.Scale = Point(
            (width * 100) / (scaledWidth > 0 ? scaledWidth : width),
            (height * 100) / (scaledHeight > 0 ? scaledHeight : height)
        );

        L(debug, L"[GDIDesktop::initialize] success: rect({}), scale({})", this->desktopInfo.Rect, this->desktopInfo.Scale);

        return S_OK;
    }

    void GDIDesktop::uninitialize() {
        L(trace, "[GDIDesktop::uninitialize] called");
        if (this->dc) {
            DeleteDC(this->dc);
            this->dc = nullptr;
        }
    }
}
