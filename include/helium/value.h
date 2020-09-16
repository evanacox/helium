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
bool he_val_is_bool(const he_value *val);

/** @brief Returns if the value is TYPE_INT */
bool he_val_is_int(const he_value *val);

/** @brief Returns if the value is TYPE_FLOAT */
bool he_val_is_float(const he_value *val);

/** @brief Returns if the value is TYPE_STRING */
bool he_val_is_string(const he_value *val);

/** @brief Returns if the value is TYPE_OBJECT */
bool he_val_is_object(const he_value *val);

/** @brief Reads a value as a boolean */
bool he_val_as_bool(const he_value *val);

/** @brief Reads a value as an integer */
int64_t he_val_as_int(const he_value *val);

/** @brief Reads a value as a float */
double he_val_as_float(const he_value *val);

/** @brief Reads a value as a string */
const char *he_val_as_string(const he_value *val);

/** @brief Reads a value as an object */
void *he_val_as_object(const he_value *val);

/** @brief Creates a he_value from a bool */
he_value he_val_from_bool(bool boolean);

/** @brief Creates a he_value from an integer */
he_value he_val_from_int(int64_t integer);

/** @brief Creates a he_value from a float */
he_value he_val_from_float(double fl);

/** @brief Creates a he_value from a string */
he_value he_val_from_string(const char *string);

/** @brief Creates a he_value from a pointer */
he_value he_val_from_object(void *object);

#ifdef __cplusplus
}
#endif

#endif
