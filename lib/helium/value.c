#include "helium/value.h"

bool he_val_is_bool(const he_value *val) {
    return val->type == TYPE_BOOL;
}

bool he_val_is_int(const he_value *val) {
    return val->type == TYPE_INT;
}

bool he_val_is_float(const he_value *val) {
    return val->type == TYPE_FLOAT;
}

bool he_val_is_string(const he_value *val) {
    return val->type == TYPE_STRING;
}

bool he_val_is_object(const he_value *val) {
    return val->type == TYPE_OBJECT;
}

bool he_val_as_bool(const he_value *val) {
    assert(he_val_is_bool(val) && "not attempting to read non-bool as bool");

    return val->as.boolean;
}

int64_t he_val_as_int(const he_value *val) {
    assert(he_val_is_int(val) && "not attempting to read non-int as int");

    return val->as.integer;
}

double he_val_as_float(const he_value *val) {
    assert(he_val_is_float(val) && "not attempting to read non-float as float");

    return val->as.floating;
}

const char *he_val_as_string(const he_value *val) {
    assert(he_val_is_string(val) && "not attempting to read non-int as int");

    return val->as.string;
}

void *he_val_as_object(const he_value *val) {
    assert(he_val_is_object(val) && "not attempting to read non-int as int");

    return val->as.object;
}

he_value he_val_from_bool(bool boolean) {
    return (he_value){.type = TYPE_BOOL, .as = {.boolean = boolean}};
}

he_value he_val_from_int(int64_t integer) {
    return (he_value){.type = TYPE_INT, .as = {.integer = integer}};
}

he_value he_val_from_float(double fl) {
    return (he_value){.type = TYPE_FLOAT, .as = {.floating = fl}};
}

he_value he_val_from_string(const char *string) {
    return (he_value){.type = TYPE_STRING, .as = {.string = string}};
}

he_value he_val_from_object(void *object) {
    return (he_value){.type = TYPE_OBJECT, .as = {.object = object}};
}
