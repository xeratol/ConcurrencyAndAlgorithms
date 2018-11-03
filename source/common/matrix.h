#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <exception>

template <typename T>
class matrix
{
private:
    typedef typename std::vector<T>::size_type size_type;
    std::vector<T> _data; // row-major
    // (x, y) -> _data[y * _width + x]
    size_type _width; // num elements per row
    size_type _height; // num elements per col (height)

public:
    matrix() : _data(), _width(), _height() {}
    matrix(size_type width, size_type height) : _data(width * height), _width(width), _height(height) {}
    matrix(size_type width, size_type height, const std::vector<T>& data) :
        _data(data), _width(width), _height(height)
    {
        if (_data.size() != _width * _height)
        {
            throw std::exception("Mismatching dimensions");
        }
    }
    matrix(size_type width, size_type height, std::vector<T>&& data) :
        _data(std::move(data)), _width(width), _height(height)
    {
        if (_data.size() != _width * _height)
        {
            throw std::exception("Mismatching dimensions");
        }
    }
    
    void Resize(size_type cols, size_type rows)
    {
        std::vector<T> newData(cols * rows);
        auto smallerRow = (rows < _height) ? rows : _height;
        auto smallerCol = (cols < _width) ? cols : _width;
        for (auto row = 0u; row < smallerRow; ++row)
        {
            for (auto col = 0u; col < smallerCol; ++col)
            {
                newData[row * cols + col] = _data[row * _width + col];
            }
        }

        _data = newData;
        _width = cols;
        _height = rows;
    }

    std::vector<T> ToVector() const
    {
        return _data;
    }

    T& At(size_type x, size_type y)
    {
        return _data[y * _width + x];
    }

    const T& At(size_type x, size_type y) const
    {
        return _data[y * _width + x];
    }

    void Print(const std::string& elementDelimiter = ", ", const std::string& lineDelimiter = "\n") const
    {
        size_type c = 0;
        std::for_each(_data.begin(), _data.end(), [&, this](auto d)
        {
            std::cout << d << elementDelimiter;
            ++c;
            if (c == _width)
            {
                std::cout << lineDelimiter;
                c = 0;
            }
        });
    }

    void Row(size_type y, const std::vector<T>& newRowData)
    {
        if (y < 0 || y >= _height)
        {
            throw std::out_of_range("matrix::Row()");
        }

        if (newRowData.size() > _width)
        {
            throw std::out_of_range("matrix::Row(): new data is larger than row capacity");
        }

        for (auto x = 0u; x < _width; ++x)
        {
            At(x, y) = newRowData[x];
        }
    }

    std::vector<T> Row(size_type y) const
    {
        if (y < 0 || y >= _height)
        {
            throw std::out_of_range("matrix::Row()");
        }
        return std::vector<T>(_data.begin() + y * _width,
            _data.begin() + (y + 1) * _width);
    }

    void Col(size_type x, const std::vector<T>& newColData)
    {
        if (x < 0 || x >= _width)
        {
            throw std::out_of_range("matrix::Col()");
        }

        if (newColData.size() > _height)
        {
            throw std::out_of_range("matrix::Col(): new data is larger than column capacity");
        }

        for (auto y = 0u; y < _height; ++y)
        {
            At(x, y) = newColData[y];
        }
    }

    std::vector<T> Col(size_type x) const
    {
        if (x < 0 || x >= _width)
        {
            throw std::out_of_range("matrix::Col()");
        }
        std::vector<T> d(_height);
        for (auto row = 0u; row < _height; ++row)
        {
            d[row] = At(x, row);
        }
        return d;
    }

    size_type Width() const
    {
        return _width;
    }

    size_type Height() const
    {
        return _height;
    }

    const std::vector<T>& Raw() const
    {
        return _data;
    }

    /*
    Apply unary op per element and store in dstIt
    */
    template <typename UnaryOp, typename DestinationIterator>
    void Transform(UnaryOp f, DestinationIterator dstIt) const
    {
        std::transform(_data.cbegin(), _data.cend(), dstIt, f);
    }

    /*
    Apply unary op per element in-place
    */
    template <typename UnaryOp>
    void Transform(UnaryOp f)
    {
        std::transform(_data.begin(), _data.end(), _data.begin(), f);
    }

    void Normalize()
    {
        T highest = std::numeric_limits<T>::min();
        T lowest = std::numeric_limits<T>::max();

        std::for_each(_data.begin(), _data.end(), [&](const T& el)
        {
            if (el > highest)
            {
                highest = el;
            }
            if (el < lowest)
            {
                lowest = el;
            }
        });
        T range = highest - lowest;
        if (range <= std::numeric_limits<T>::epsilon())
        {
            return;
        }

        Transform([&](const T& el)
        {
            return (el - lowest) / range;
        });
    }
};
