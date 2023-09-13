#ifndef ARRAYSIZE_H_
#define ARRAYSIZE_H_

/**
 * The undef is needed in case some other header already defined a macro like this.
 */
#undef ARRAYSIZE

/**
 * @brief Computes the number of elements in a statically allocated array.
 *
 * Example:
 *
 *    int array[10];
 *    size_t array_size = ARRAYSIZE(array);
 *
 * @note Ensure that 'array' is actually an array, not a pointer. Otherwise,
 *       ARRAYSIZE will produce erroneous results if 'array' is a pointer.
 *       Pay attention to the fact that an array passed as a parameter to a
 *       function decays to a pointer, which means that within the function
 *       you can't use ARRAYSIZE.
 *
 * @param[in] _array The name of the array.
 *
 * @return    The number of elements in the array, expressed as a size_t.
 */
#define ARRAYSIZE(_array) (sizeof(_array) / sizeof(*(_array)))

#endif /* ARRAYSIZE_H_ */
