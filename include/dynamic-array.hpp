#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <string>
#include <cstdlib>
#include <new>
#include <utility>
#include <stdexcept>

template <typename T>
class dynamic_array
{
    int _size;
    int _capacity;
    T *data;

    void reallocate(int new_capacity)
    {
        // Handle reducing the capacity to zero
        if (new_capacity == 0)
        {
            for (int i = 0; i < _size; i++)
            {
                data[i].~T();
            }
            std::free(data);
            data = nullptr;
            _capacity = 0;
            _size = 0;
            return;
        }

        // Allocate raw memory for the new capacity
        void *raw_memory = std::malloc(new_capacity * sizeof(T));
        if (!raw_memory)
        {
            throw std::string("Memory allocation failed during reallocation.");
        }

        T *new_data = static_cast<T *>(raw_memory);

        // Move existing elements into the new memory block
        for (int i = 0; i < _size; i++)
        {
            new (&new_data[i]) T(std::move(data[i]));
            // Explicitly call the destructor on the old object
            data[i].~T();
        }

        std::free(data);
        data = new_data;
        _capacity = new_capacity;
    }

public:
    dynamic_array()
    {
        _size = 0;
        _capacity = 0;
        data = nullptr;
    }

    // Copy Constructor
    dynamic_array(const dynamic_array &other)
    {
        _size = other._size;
        _capacity = other._capacity;

        if (_capacity > 0)
        {
            void *raw_memory = std::malloc(_capacity * sizeof(T));
            if (!raw_memory)
            {
                throw std::string("Memory allocation failed during copy construction.");
            }
            data = static_cast<T *>(raw_memory);

            // Use placement new to copy construct each element
            for (int i = 0; i < _size; i++)
            {
                new (&data[i]) T(other.data[i]);
            }
        }
        else
        {
            data = nullptr;
        }
    }

    // Assignment Operator
    dynamic_array &operator=(const dynamic_array &other)
    {
        // Guard against self-assignment
        if (this != &other)
        {
            // Destroy current elements and free existing memory
            for (int i = 0; i < _size; i++)
            {
                data[i].~T();
            }
            std::free(data);

            _size = other._size;
            _capacity = other._capacity;

            if (_capacity > 0)
            {
                void *raw_memory = std::malloc(_capacity * sizeof(T));
                if (!raw_memory)
                {
                    throw std::string("Memory allocation failed during assignment.");
                }
                data = static_cast<T *>(raw_memory);

                for (int i = 0; i < _size; i++)
                {
                    new (&data[i]) T(other.data[i]);
                }
            }
            else
            {
                data = nullptr;
            }
        }
        return *this;
    }

    // Destructor
    ~dynamic_array()
    {
        for (int i = 0; i < _size; i++)
        {
            data[i].~T();
        }
        std::free(data);
    }

    int length() const
    {
        return _size;
    }

    int capacity() const
    {
        return _capacity;
    }

    void add(T value)
    {
        if (_size >= _capacity)
        {
            int new_cap = (_capacity == 0) ? 1 : _capacity * 2;
            reallocate(new_cap);
        }

        // Construct the new element directly in the raw memory slot
        new (&data[_size]) T(std::move(value));
        _size++;
    }

    void remove(int index)
    {
        if (index < 0 || index >= _size)
        {
            throw std::string("Accessed invalid array index " + std::to_string(index));
        }

        // Shift elements down to fill the gap
        for (int i = index + 1; i < _size; i++)
        {
            data[i - 1] = std::move(data[i]);
        }

        // Explicitly destruct the last element which is now a moved-from shell
        data[_size - 1].~T();
        _size--;

        // Halve the capacity if the array is 1/4 full or less
        if (_capacity > 0 && _size <= _capacity / 4)
        {
            reallocate(_capacity / 2);
        }
    }

    T &get(int index)
    {
        if (index < 0 || index >= _size)
        {
            throw std::string("Accessed invalid array index " + std::to_string(index));
        }
        return data[index];
    }

    const T &get(int index) const
    {
        if (index < 0 || index >= _size)
        {
            throw std::string("Accessed invalid array index " + std::to_string(index));
        }
        return data[index];
    }

    T &operator[](int index)
    {
        return get(index);
    }

    const T &operator[](int index) const
    {
        return get(index);
    }

    void fill(int count, T value)
    {
        // Ensure there is enough capacity to hold the required amount
        while (_size + count > _capacity)
        {
            int new_cap = (_capacity == 0) ? 1 : _capacity * 2;
            reallocate(new_cap);
        }

        for (int i = 0; i < count; i++)
        {
            add(value);
        }
    }
};

#endif