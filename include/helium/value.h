#ifndef HE_VALUE_H
#define HE_VALUE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Tags the `he_value::as` union */
typedef enum he_value_type {
    TYPE_BOOL = 0,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_OBJECT
} he_value_type;

/** @brief Simply a tagged union for a value of some type */
typedef struct he_value {
    /** @brief The type of object the union holds */
    he_value_type type;

    /** @brief Holds the different forms of object */
    union he_value_as {
        bool boolean;
        int64_t integer;
        double floating;
        const char *string;
        void *object;
    } as;
} he_value;

/** @brief Returns if the value is TYPE_BOOL */
inline bool he_val_is_bool(const he_value *val) {
    return val->type == TYPE_BOOL;
}

/** @brief Returns if the value is TYPE_INT */
inline bool he_val_is_int(const he_value *val) {
    return val->type == TYPE_INT;
}

/** @brief Returns if the value is TYPE_FLOAT */
inline bool he_val_is_float(const he_value *val) {
    return val->type == TYPE_FLOAT;
}

/** @brief Returns if the value is TYPE_STRING */
inline bool he_val_is_string(const he_value *val) {
    return val->type == TYPE_STRING;
}

/** @brief Returns if the value is TYPE_OBJECT */
inline bool he_val_is_object(const he_value *val) {
    return val->type == TYPE_OBJECT;
}

/** @brief Reads a value as a boolean */
inline bool he_val_as_bool(const he_value *val) {
    assert(he_val_is_bool(val) && "not attempting to read non-bool as bool");

    return val->as.boolean;
}

/** @brief Reads a value as an integer */
inline int64_t he_val_as_int(const he_value *val) {
    assert(he_val_is_int(val) && "not attempting to read non-int as int");

    return val->as.integer;
}

/** @brief Reads a value as a float */
inline double he_val_as_float(const he_value *val) {
    assert(he_val_is_float(val) && "not attempting to read non-float as float");

    return val->as.floating;
}

/** @brief Reads a value as a string */
inline const char *he_val_as_string(const he_value *val) {
    assert(he_val_is_string(val) && "not attempting to read non-int as int");

    return val->as.string;
}

/** @brief Reads a value as an object */
inline void *he_val_as_object(const he_value *val) {
    assert(he_val_is_object(val) && "not attempting to read non-int as int");

    return val->as.object;
}

/** @brief Creates a he_value from a bool */
inline he_value he_val_from_bool(bool boolean) {
    return (he_value){.type = TYPE_BOOL, .as = {.boolean = boolean}};
}

/** @brief Creates a he_value from an integer */
inline he_value he_val_from_int(int64_t integer) {
    return (he_value){.type = TYPE_INT, .as = {.integer = integer}};
}

/** @brief Creates a he_value from a float */
inline he_value he_val_from_float(double fl) {
    return (he_value){.type = TYPE_FLOAT, .as = {.floating = fl}};
}

/** @brief Creates a he_value from a string */
inline he_value he_val_from_string(const char *string) {
    return (he_value){.type = TYPE_STRING, .as = {.string = string}};
}

/** @brief Creates a he_value from a pointer */
inline he_value he_val_from_object(void *object) {
    return (he_value){.type = TYPE_OBJECT, .as = {.object = object}};
}

#ifdef __cplusplus
}
#endif

#endif
