#pragma once

#include <set>
#include <vector>

template <typename T>
bool contains(const std::set<T> &set, const T &val)
{
    return std::find(set.begin(), set.end(), val) != set.end();
}

template <typename T>
bool contains(const std::vector<T> &vec, const T &val)
{
    return std::find(vec.begin(), vec.end(), val) != vec.end();
}
