#pragma once

#include "Utils/Math.h"

#include <memory>
#include <cassert>

namespace Expanse
{
	template<class T>
	using initializer_list_2d = std::initializer_list<std::initializer_list<T>>;

	template<class Elem>
	class Array2D
	{
	public:
		Array2D() = default;

		Array2D(const Rect& rect)
			: _rect(rect)
		{
			assert(rect.w > 0 && rect.h > 0);
			_data.reset(new Elem[Size()]);
		}

		template<class T>
		Array2D(const Rect& rect, const T& value)
			: Array2D(rect)
		{
			std::fill(begin(), end(), value);
		}

		Array2D(const Array2D& other)
			: Array2D(other._rect)
		{
			std::copy(other.begin(), other.end(), begin());
		}

		Array2D& operator=(const Array2D& other)
		{
			_rect = other._rect;
			const auto new_size = other.Size();
			if (new_size > 0) {
				_data.reset(new Elem[new_size]);
				std::copy(other.begin(), other.end(), begin());
			}
			else {
				_data.reset();
			}
			return *this;
		}

		Array2D(Array2D&& other)
			: _rect(other._rect)
			, _data(std::move(other._data))
		{}

		Array2D& operator=(Array2D&& other)
		{
			_rect = other._rect;
			_data = std::move(other._data);
			return *this;
		}

		template<class T>
		Array2D(initializer_list_2d<T> rows)
		{
			const int height = static_cast<int>(rows.size());

			int width = 0;
			for (const auto& row : rows) {
				width = std::max(width, static_cast<int>(row.size()));
			}

			_rect = { 0, 0, width, height };
			_data.reset(new Elem[Size()]);

			int y = 0;
			for (const auto& row : rows) {
				Elem* row_begin = _data.get() + PointToIndex({ 0, y++ });
				std::copy(row.begin(), row.end(), row_begin);
			}
		}

		bool operator==(const Array2D& other) const
		{
			return (_rect == other._rect) && std::equal(begin(), end(), other.begin());
		}

		bool IndexIsValid(Point pt) const { return Contains(_rect, pt); }

		Elem& operator[](Point pt) { return _data[PointToIndex(pt)]; }
		const Elem& operator[](Point pt) const { return _data[PointToIndex(pt)]; }

		const Rect& GetRect() const { return _rect; }
		Point Origin() const { return { _rect.x, _rect.y }; }
		int Width() const { return _rect.w; }
		int Height() const { return _rect.h; }
		bool Empty() const { return _data == nullptr; }

		void MoveOrigin(Point new_origin)
		{
			_rect.x = new_origin.x;
			_rect.y = new_origin.y;
		}

		size_t Size() const { return static_cast<size_t>(_rect.w * _rect.h); }

		Elem* begin() { return _data.get(); }
		Elem* end() { return _data.get() + Size(); }
		const Elem* begin() const { return _data.get(); }
		const Elem* end() const { return _data.get() + Size(); }

	private:
		Rect _rect{ 0, 0, 0, 0 };
		std::unique_ptr<Elem[]> _data;

		size_t PointToIndex(Point pt) const { return static_cast<size_t>(_rect.w * (pt.y - _rect.y) + (pt.x - _rect.x)); }
	};

	template<class T>
	void CopyArrayData(const Array2D<T>& src, Array2D<T>& dst, const Rect& rect)
	{
		assert(Contains(src.GetRect(), rect));
		assert(Contains(dst.GetRect(), rect));
	
		const Point pt{ rect.x, rect.y };
		auto* src_line = &src[pt];
		auto* dst_line = &dst[pt];
		const auto src_row_size = static_cast<size_t>(src.GetRect().w);
		const auto dst_row_size = static_cast<size_t>(dst.GetRect().w);
		const auto row_size = static_cast<size_t>(rect.w);

		for (int i = 0; i < rect.h; ++i) {
			std::copy_n(src_line, row_size, dst_line);
			src_line += src_row_size;
			dst_line += dst_row_size;
		}
	}
}