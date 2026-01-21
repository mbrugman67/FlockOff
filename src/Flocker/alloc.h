#ifndef ALLOC_H_
#define ALLOC_H_
#include "Arduino.h"  // for ps_malloc()

#include <type_traits>
#include <string>
#include <limits>

template <class T>
class psramAlloc
{
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t ;
        using propagate_on_container_move_assignment  = std::true_type;

       
        T* allocate(size_type n, const void* hint = 0)
        {
            return static_cast<T*>(ps_malloc(n*sizeof(T)));
        }

        void deallocate(T* p, size_type n) { free (p); }
        size_type max_size() const { return size_type(std::numeric_limits<unsigned int>::max() / sizeof(T)); }
        void construct(T* p, const T& value) { _construct(p, value); }
        void destroy(T* p) { _destroy(p); }
};

template<class T, class U>
bool operator==(const psramAlloc <T>&, const psramAlloc <U>&) noexcept { return true; }

template<class T, class U>
bool operator!=(const psramAlloc <T>&, const psramAlloc <U>&) noexcept { return false; }

// for std::map using a const char* as the key.  Be very careful doing that!
struct cmp_const_char
{
  bool operator()(const char* a, const char* b) const
  {
    return (strcmp(a, b) < 0);
  }
};

namespace utils
{
// to allocate std::string in external RAM instead of onboard SRAM.  Slower, but SRAM is a limited resource
  typedef std::basic_string<char, std::char_traits<char>, psramAlloc<std::string::value_type>> string;
};

#endif // ALLOC_H_