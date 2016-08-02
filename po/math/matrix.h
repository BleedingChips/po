#pragma once
#include <array>
#include <iostream>
#include "../tool/type_tool.h"

namespace
{

	template<size_t row_, size_t columns_, typename T> class matrix_view
	{
		T& matrix_;
	public:
		static constexpr size_t row = row_;
		static constexpr size_t columns = columns_;
		using store = typename T::store;
		matrix_view(T& m) :matrix_(m) {}
		matrix_view(const matrix_view& mr) = default;
		decltype(auto) operator()(size_t i, size_t t) { return matrix_(i, t); }
		decltype(auto) operator()(size_t i, size_t t) const { return matrix_(i, t); }
		friend std::ostream& operator<< (std::ostream& os, const matrix_view& ma)
		{
			os << "( ";
			bool finish2 = false;
			for (size_t i = 0; i < row_;++i)
			{
				if (finish2)
					os << " | ";
				else
					finish2 = true;
				bool finish = false;
				for (size_t k = 0; k < columns; ++k)
				{
					if (finish)
						os << ',';
					else
						finish = true;
					os << ma(i, k);
				}
			}
			os << " )";
			return os;
		}
	};

	template<size_t row_, size_t columns_, typename T> class submatrix_view
	{
		T& matrix_;
		size_t start_row;
		size_t start_columns;
	public:
		static constexpr size_t row = row_;
		static constexpr size_t columns = columns_;
		using store = typename T::store;
		submatrix_view(T& m,size_t r,size_t c) :matrix_(m),start_row(r), start_columns(c) {}
		submatrix_view(const submatrix_view& mr) = default;
		decltype(auto) operator()(size_t i, size_t t) { return matrix_(i + start_row, t + start_columns); }
		decltype(auto) operator()(size_t i, size_t t) const { return matrix_(i + start_row, t + start_columns); }
		friend std::ostream& operator<< (std::ostream& os, const submatrix_view& ma)
		{
			return os << matrix_view<row, columns, const submatrix_view>(ma);
		}
	};

	template<size_t row_, size_t columns_, typename T> class matrix_transposed_view
	{
		T& matrix_;
	public:
		static constexpr size_t row = row_;
		static constexpr size_t columns = columns_;
		using store = typename T::store;
		matrix_transposed_view(T& m) :matrix_(m) {}
		matrix_transposed_view(const matrix_transposed_view& mr) = default;
		decltype(auto) operator()(size_t i, size_t t) { return matrix_(t, i); }
		decltype(auto) operator()(size_t i, size_t t) const { return matrix_(t, i); }
		friend std::ostream& operator<< (std::ostream& os, const matrix_transposed_view& ma)
		{
			return os << matrix_view<row, columns, const matrix_transposed_view>(ma);
		}
	};
}






namespace PO
{
	namespace Math
	{

		template<typename T, size_t row_, size_t columns_ > struct matrix : std::array < std::decay_t<T>, row_ * columns_ >
		{
			static_assert(row_ >= 1 && columns_ >= 1, "the row or columns must both bigger then 1");
			
			static constexpr size_t row = row_;
			static constexpr size_t columns = columns_;
			using store = T;

			decltype(auto) operator[](size_t i) {
				return std::array < std::decay_t<T>, row_ * columns_ >
					::operator[](i);
			}
			decltype(auto) operator[](size_t i) const {
				return std::array < std::decay_t<T>, row_ * columns_ >
					::operator[](i);
			}

			decltype(auto) operator()(size_t r, size_t c) {  return (*this)[r * columns + c]; }
			decltype(auto) operator()(size_t r, size_t c) const { return (*this)[r * columns + c]; }
			
			matrix(const std::initializer_list<std::array<T, columns>>& p)
			{
				size_t irow_c = 0;
				for (const auto& irow : p)
				{
					if (irow_c > row)
						break;
					std::copy(irow.begin(), irow.end(), this->begin() + irow_c * columns_);
					++irow_c;
				}
			}

			template<typename K>
			matrix(K&& t)
			{
				for (size_t i = 0; i < row; ++i)
					for (size_t k = 0; k < columns; ++k)
					{
						size_t ti = i, tk = k;
						this->operator()(ti,tk) = t(ti, tk);
					}
			}

			template<typename K>
			matrix(const matrix_view<row,columns,T>&& t)
			{
				for (size_t i = 0; i < row; ++i)
					for (size_t k = 0; k < columns; ++k)
					{
						size_t ti = i, tk = k;
						this->operator()(ti, tk) = t(ti, tk);
					}
			}

			template<typename K>
			matrix(const submatrix_view<row, columns, T>&& t)
			{
				for (size_t i = 0; i < row; ++i)
					for (size_t k = 0; k < columns; ++k)
					{
						size_t ti = i, tk = k;
						this->operator()(ti, tk) = t(ti, tk);
					}
			}

			matrix(const matrix&) = default;
			matrix& operator= (const matrix&) = default;

			template<typename K>
			matrix(const matrix<K, row, columns>& io)
			{
				for (size_t i = 0; i < row; ++i)
				{
					for (size_t k = 0; k < columns; ++k)
					{
						this->operator()(i, k) = io(i, k);
					}
				}
			}

			friend std::ostream& operator<< (std::ostream& os, const matrix& ma)
			{
				return os << matrix_view<row, columns, const matrix>(ma);
			}

			auto operator~() { return matrix_transposed_view<columns, row, matrix>(*this); }
			auto operator~() const { return const matrix_transposed_view<columns, row, const matrix>(*this); }
		};


		//template<  >

	}
}