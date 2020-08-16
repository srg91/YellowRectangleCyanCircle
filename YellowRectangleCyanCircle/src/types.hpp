#pragma once

#include <atlbase.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <Windows.h>

namespace YellowRectangleCyanCircle {
	enum class DetectorType {
		Area,
		Fingerprint,
		Keypad
	};

	using Mat = cv::Mat;

	namespace Rect {
		using Rect = cv::Rect;

		Rect FromRECT(const RECT& rect);
	}

	template <class T>
	class ComObject {
	public:
		ComObject() :
			ComObject<T>(nullptr)
		{}

		ComObject(CComPtr<T> object) :
			object(object)
		{}

		CComPtr<T> Get() const {
			return this->object;
		}

		void Set(CComPtr<T> value) {
			this->object = value;
		}

		operator bool() const { return bool(this->object); };
	protected:
		CComPtr<T> object;
	};
}

namespace std {
	std::uint8_t* data(YellowRectangleCyanCircle::Mat mat);
}
