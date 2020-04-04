#ifndef AVERAGE_H
#define AVERAGE_H

#include <cstdint>

template <typename T>
class Average
{
public:
    bool operator<(const Average<T> &other) const
    {
        return _value < other._value;
    }

    bool operator>(const Average<T> &other) const
    {
        return _value > other._value;
    }

    bool operator<(const T &value) const
    {
        return _value < value;
    }

    bool operator>(const T &value) const
    {
        return _value > value;
    }

    void add(T value)
    {
        _value = _value * _count + value;
        _value /= ++_count;
    }

    void add(Average<T> other)
    {
        if (other._count > 0) {
            _value = _value * _count + other._value * other._count;
            _count += other._count;
            _value /= _count;
        }
    }

    void remove(T value)
    {
        if (_count > 1) {
            _value = _value * _count - value;
            _value /= --_count;
        }
        else {
            _value = 0;
            _count = 0;
        }
    }

    void remove(Average<T> other)
    {
        if (other._count > 0) {
            if (_count > other._count) {
                _value = _value * _count - other._value * other._count;
                _count -= other._count;
                _value /= _count;
            }
            else {
                _value = 0;
                _count = 0;
            }
        }
    }

    void replace(T valueFrom, T valueTo)
    {
        if (_count > 1) {
            _value = _value * _count - valueFrom + valueTo;
            _value /= _count;
        }
        else {
            _value = valueTo;
            _count = 1;
        }
    }

    void reset()
    {
        _value = 0;
        _count = 0;
    }

    T value() const
    {
        return _value;
    }

    uint64_t count() const
    {
        return _count;
    }

private:
    T _value = 0;
    uint64_t _count = 0;
};

#endif // AVERAGE_H
