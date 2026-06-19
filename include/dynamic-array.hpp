#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <cstdlib>
#include <string>
#include <utility>

/*
dynamic_array is a custom-made version of std::vector. It is a resizable array
that dynamically grows and shrinks.
*/

template <typename T> class dynamic_array {
  int _size;     // The current number of elements in the array
  int _capacity; // The total number of elements the array can hold
  T *data; // The pointer to the memory chunk containing all the data in the
           // array

  // The reallocate method is the private method that handles
  // expansion/reduction of memory
  // @param int new_capacity - the target new total capacity the array should
  // have, can be higher or lower
  void reallocate(int new_capacity) {
    // Handle reducing the capacity to zero
    if (new_capacity == 0) {
      // In this case we just want to delete all the data
      for (int i = 0; i < _size; i++) {
        data[i].~T();
      }

      // Free the data
      std::free(data);
      data = nullptr;

      // Set array params
      _capacity = 0;
      _size = 0;
      return;
    } // If the new capacity isn't zero, we can proceed..

    // Allocate raw memory for the new capacity using malloc
    void *raw_memory = std::malloc(new_capacity * sizeof(T));
    if (!raw_memory) {
      // Handle if there's not enough memory
      throw std::string("Memory allocation failed during reallocation.");
    }

    // The raw memory has void type, so cast it into the target data type
    T *new_data = static_cast<T *>(raw_memory);

    // Move existing elements into the new memory block
    for (int i = 0; i < _size; i++) {
      // The normal new command allocates memory and constructs the object
      // automatically But since we already did that with malloc, we have to use
      // the 'placement new' This syntax is basically like new (address);,
      // meaning don't allocate anything, just construct T at the target
      // address. the std::move part copies the data over, and deletes the old
      // data
      new (&new_data[i]) T(std::move(data[i]));

      // Explicitly call the destructor on the old object, just in case
      data[i].~T();
    }

    // Free the old data and reassign the data
    std::free(data);
    data = new_data;
    _capacity = new_capacity;
  }

public:
  // Default constructor: no parameters, empty array
  dynamic_array() : _size(0), _capacity(0), data(nullptr) {}

  // Copy constructor, allow a new array to be created by assignment
  // @param const dynamic_array &other - the array to be copied
  dynamic_array(const dynamic_array &other) {
    // Set the same size and capacity
    _size = other._size;
    _capacity = other._capacity;

    if (_capacity > 0) {
      // Use the same techniques as in reallocate()

      void *raw_memory = std::malloc(_capacity * sizeof(T));
      if (!raw_memory) {
        throw std::string("Memory allocation failed during copy construction.");
      }
      data = static_cast<T *>(raw_memory);

      // Use placement new to copy construct each element
      for (int i = 0; i < _size; i++) {
        new (&data[i]) T(other.data[i]);
        // We don't delete the old data, because this is explicitly for copying
        // (we are supposed to duplicate elements)
      }
    } else {
      data = nullptr;
    }
  }

  // Very similar to copy constructor, but runs when the object being
  // overwritten already exists
  // @param const dynamic_array &other - the array to be copied
  dynamic_array &operator=(const dynamic_array &other) {
    // Guard against self-assignment
    if (this != &other) {
      // Destroy current elements and free existing memory
      for (int i = 0; i < _size; i++) {
        data[i].~T();
      }
      std::free(data);

      _size = other._size;
      _capacity = other._capacity;

      if (_capacity > 0) {
        void *raw_memory = std::malloc(_capacity * sizeof(T));
        if (!raw_memory) {
          throw std::string("Memory allocation failed during assignment.");
        }
        data = static_cast<T *>(raw_memory);

        for (int i = 0; i < _size; i++) {
          new (&data[i]) T(other.data[i]);
        }
      } else {
        data = nullptr;
      }
    }
    return *this;
  }

  // Destructor, just frees memory
  ~dynamic_array() {
    for (int i = 0; i < _size; i++) {
      data[i].~T();
    }
    std::free(data);
  }

  // Helper method to get the size of the array
  int length() const { return _size; }

  // Helper method to get the array capacity in memory
  int capacity() const { return _capacity; }

  // The add() method needs to add the data to the array and also increase the
  // capacity if its ful
  // @param T value - the value to add
  void add(T value) {
    // If the new element will cause the array to exceed capacity, we double
    // capacity
    if (_size >= _capacity) {
      int new_cap = (_capacity == 0) ? 1 : _capacity * 2;
      reallocate(new_cap); // Reallocate is called to map all the data to a new
                           // array with a new size
    }

    // Construct the new element directly in the raw memory slot using placement
    // new again
    new (&data[_size]) T(std::move(value));
    _size++;
  }

  // Remvoe does pretty much the opposite from above, but instead of adding a
  // specific value, we assume the user already knows the index of the element
  // they want to remove
  // @param int index - the index to remove
  void remove(int index) {
    // Check bounds
    if (index < 0 || index >= _size) {
      throw std::string("Accessed invalid array index " +
                        std::to_string(index));
    }

    // Shift elements down to fill the gap
    for (int i = index + 1; i < _size; i++) {
      data[i - 1] = std::move(data[i]);
    }

    // Explicitly destruct the last element which has nothing in it now
    data[_size - 1].~T();
    _size--;

    // Halve the capacity if the array is 1/4 full or less
    if (_capacity > 0 && _size <= _capacity / 4) {
      reallocate(_capacity / 2);
    }
  }

  // Retrieve the value at a specific index in the arrray
  // @param int index = the target index
  // @return T - the value at that index as a reference
  // Key point: This method is called when you want to reassign an element of
  // the array
  T &get(int index) {
    // Check bounds
    if (index < 0 || index >= _size) {
      throw std::string("Accessed invalid array index " +
                        std::to_string(index));
    }

    // Call the lower level c-style [] operator on the data.
    return data[index];
  }

  // Retrieve the value at a specific index in the arrray
  // @param int index = the target index
  // @return const T - the value at that index, read-only reference
  // Key point: different to above, this is called when you only want to fetch
  // (not change) the array value
  const T &get(int index) const {
    if (index < 0 || index >= _size) {
      throw std::string("Accessed invalid array index " +
                        std::to_string(index));
    }
    return data[index];
  }

  // Helper [] operator to call the non-const get method
  // @param int index = the target index
  // @return T - the value at that index as a reference
  T &operator[](int index) { return get(index); }

  // Helper [] operator to call the const get method
  // @param int index = the target index
  // @return const T - the value at that index as a reference, read-only
  const T &operator[](int index) const { return get(index); }

  // This method lets us fill up the entire array with a set of the same value
  // @param int count - the amount of values to fill
  // @param T value - the value to put in each element
  void fill(int count, T value) {
    // Ensure there is enough capacity to hold the required amount
    while (_size + count > _capacity) {
      int new_cap = (_capacity == 0) ? 1 : _capacity * 2;
      reallocate(new_cap);
    }

    for (int i = 0; i < count; i++) {
      add(value);
    }
  }

  /*
  In many cases I am using the dynamic array to hold numerical values. In these
  cases it benefits me to allow methods like min and max, but they should only
  be enabled if the data type is numerical. The standard library allows for this
  by filtering the template type.
  */

  // Retrieve minimum value iff array type is double
  // @return double - the minimum array value
  template <typename U = T,
            typename = std::enable_if_t<std::is_same<U, double>::value>>
  double min() const {
    if (_size == 0)
      return 0.0;

    // Iterate through the entire array, setting any new lower values to the
    // global min
    double minimum = data[0];
    for (int i = 0; i < length(); i++) {
      if (data[i] < minimum) {
        minimum = data[i];
      }
    }
    return minimum;
  }

  // Retrieve maximum value iff array type is double
  // @return double - the maximum array value
  template <typename U = T,
            typename = std::enable_if_t<std::is_same<U, double>::value>>
  double max() const {
    if (_size == 0)
      return 0.0;

    // Iterate through the entire array, setting any new higher values to the
    // global max
    double maximum = data[0];
    for (int i = 0; i < length(); i++) {
      if (data[i] > maximum) {
        maximum = data[i];
      }
    }
    return maximum;
  }
};

#endif
