#pragma once

/// Remap a numeric value from one range to another.
/// For example:
///
///   value = 5
///   current_minimum = 0, current_maximum = 10
///   new_minimum = 0, new_maximum = 100
///   result = 50
///
///   value = 7
///   current_minimum = 0, current_maximum = 10
///   new_minimum = 100, new_maximum = 200
///   result = 170
///
/// @param value the value that will be mapped to the new range
/// @param current_minimum current minimum value that the value can reach
/// @param current_maximum current maximum value that the value can reach
/// @param new_minimum the new minimum value to scale and shift the old value to
/// @param new_maximum the new maximum value to scale and shift the old value to
template <typename T, typename U>
inline U Map(T value, T current_minimum, T current_maximum, U new_minimum,
             U new_maximum);