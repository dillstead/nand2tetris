#ifndef UTILS_H_
#define UTILS_H_

using namespace std;

// Allows use of enum classes as keys in a map.
// http://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
struct EnumClassHash
{
    template <typename T> size_t operator()(T t) const
    {
        return static_cast<size_t>(t);
    }
};

#endif
