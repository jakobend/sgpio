#ifndef SGPIO_H
#define SGPIO_H

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <poll.h>

#ifndef SGPIO_BASE_PATH
#define SGPIO_BASE_PATH "/sys/class/gpio"
#endif

// Type declaration for pins
typedef uint8_t sgpio_pin_t;
// Format specifier for sgpio_pin_t
#define SGPIO_PIN_FMT "%" PRId8
// Maximum length of a string formatted by SGPIO_PIN_FMT without trailing zero
#define SGPIO_PIN_STRING_MAX 3

// Type declaration for pin levels
enum sgpio_level_t {
    SGPIO_LEVEL_ERROR = -1,
    SGPIO_LEVEL_LOW = 0,
    SGPIO_LEVEL_HIGH = 1
};

// Type declaration for pin directions
enum sgpio_direction_t {
    SGPIO_DIRECTION_ERROR = -1,
    SGPIO_DIRECTION_IN = 0,
    SGPIO_DIRECTION_OUT = 1,
    SGPIO_DIRECTION_OUT_LOW = 2,
    SGPIO_DIRECTION_OUT_HIGH = 3
};
// Maximum length of a string returned by sgpio_direction_to_string or read from
// the direction endpoint without trailing zero
#define SGPIO_DIRECTION_STRING_MAX 4

// Type declaration for pin edges
enum sgpio_edge_t {
    SGPIO_EDGE_ERROR = -1,
    SGPIO_EDGE_NONE = 0,
    SGPIO_EDGE_FALLING = 1,
    SGPIO_EDGE_RISING = 2,
    SGPIO_EDGE_BOTH = 3
};
// Maximum length of a string returned by sgpio_edge_to_string or read from
// the edge endpoint without trailing zero  
#define SGPIO_EDGE_STRING_MAX 7

// Path to the device used to export pins
#define SGPIO_EXPORT_DEVICE_PATH SGPIO_BASE_PATH "/export"
// Path to the device used to unexport pins
#define SGPIO_UNEXPORT_DEVICE_PATH SGPIO_BASE_PATH "/unexport"

#define SGPIO_PIN_PATH SGPIO_BASE_PATH "/gpio" SGPIO_PIN_FMT
// Formatting template for paths to devices used to set the direction of a pin
#define SGPIO_DIRECTION_PATH SGPIO_PIN_PATH "/direction"
// Formatting template for paths to devices used to set the value/level of a pin
#define SGPIO_VALUE_PATH SGPIO_PIN_PATH "/value"
// Formatting template for paths to devices used to set the edge of a pin
#define SGPIO_EDGE_PATH SGPIO_PIN_PATH "/edge"
// Maximum length of a formatted path
#define SGPIO_PATH_STRING_MAX (sizeof(SGPIO_BASE_PATH) + 5 + SGPIO_PIN_STRING_MAX + 10)

char sgpio_level_to_char(enum sgpio_level_t level);
enum sgpio_level_t sgpio_char_to_level(char level);
ssize_t sgpio_direction_to_string(enum sgpio_direction_t direction, const char **string_ptr);
enum sgpio_direction_t sgpio_string_to_direction(char* direction_string);
ssize_t sgpio_edge_to_string(enum sgpio_edge_t edge, const char **string_ptr);
enum sgpio_edge_t sgpio_string_to_edge(char* edge_string);

int sgpio_export(sgpio_pin_t pin);
int sgpio_unexport(sgpio_pin_t pin);
bool sgpio_is_exported(sgpio_pin_t pin);

int sgpio_open(sgpio_pin_t pin, int flags);
enum sgpio_level_t sgpio_read_fd(int fd);
enum sgpio_level_t sgpio_read(sgpio_pin_t pin);
enum sgpio_level_t sgpio_write_fd(int fd, enum sgpio_level_t level);
enum sgpio_level_t sgpio_write(sgpio_pin_t pin, enum sgpio_level_t level);

int sgpio_open_direction(sgpio_pin_t pin, int flags);
enum sgpio_direction_t sgpio_set_direction_fd(int fd, enum sgpio_direction_t direction);
enum sgpio_direction_t sgpio_set_direction(sgpio_pin_t pin, enum sgpio_direction_t direction);
enum sgpio_direction_t sgpio_get_direction_fd(int fd);
enum sgpio_direction_t sgpio_get_direction(sgpio_pin_t pin);

int sgpio_open_edge(sgpio_pin_t pin, int flags);
enum sgpio_edge_t sgpio_set_edge_fd(int fd, enum sgpio_edge_t edge);
enum sgpio_edge_t sgpio_set_edge(sgpio_pin_t pin, enum sgpio_edge_t edge);
enum sgpio_edge_t sgpio_get_edge_fd(int fd);
enum sgpio_edge_t sgpio_get_edge(sgpio_pin_t pin);

#endif
