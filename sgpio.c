#include "sgpio.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

char sgpio_level_to_char(enum sgpio_level_t level)
{
    switch (level)
    {
        case SGPIO_LEVEL_LOW:
            return '0';
        case SGPIO_LEVEL_HIGH:
            return '1';
        default:
            return -1;
    }
}

enum sgpio_level_t sgpio_char_to_level(char level)
{
    switch (level)
    {
        case '0':
            return SGPIO_LEVEL_LOW;
        case '1':
            return SGPIO_LEVEL_HIGH;
        default:
            return SGPIO_LEVEL_ERROR;
    }
}

ssize_t sgpio_direction_to_string(enum sgpio_direction_t direction, const char **string_ptr)
{
    switch (direction)
    {
        case SGPIO_DIRECTION_IN:
            *string_ptr = "in";
            return 3;
        case SGPIO_DIRECTION_OUT:
            *string_ptr = "out";
            return 4;
        case SGPIO_DIRECTION_OUT_LOW:
            *string_ptr = "low";
            return 4;
        case SGPIO_DIRECTION_OUT_HIGH:
            *string_ptr = "high";
            return 5;
        default:
            *string_ptr = NULL;
            return -1;
    }
}

enum sgpio_direction_t sgpio_string_to_direction(char* direction_string)
{
    if (strcmp(direction_string, "in") == 0)
    {
        return SGPIO_DIRECTION_IN;
    }
    else if (strcmp(direction_string, "out") == 0)
    {
        return SGPIO_DIRECTION_OUT;
    }
    else if (strcmp(direction_string, "low") == 0)
    {
        return SGPIO_DIRECTION_OUT_LOW;
    }
    else if (strcmp(direction_string, "high") == 0)
    {
        return SGPIO_DIRECTION_OUT_HIGH;
    }
    return SGPIO_DIRECTION_ERROR;
}

ssize_t sgpio_edge_to_string(enum sgpio_edge_t edge, const char **string_ptr)
{
    switch (edge)
    {
        case SGPIO_EDGE_NONE:
            *string_ptr = "none";
            return 5;
        case SGPIO_EDGE_FALLING:
            *string_ptr = "falling";
            return 8;
        case SGPIO_EDGE_RISING:
            *string_ptr = "rising";
            return 7;
        case SGPIO_EDGE_BOTH:
            *string_ptr = "both";
            return 5;
        default:
            *string_ptr = NULL;
            return -1;
    }
}

enum sgpio_edge_t sgpio_string_to_edge(char* edge_string)
{
    if (strcmp(edge_string, "none") == 0)
    {
        return SGPIO_EDGE_NONE;
    }
    else if (strcmp(edge_string, "falling") == 0)
    {
        return SGPIO_EDGE_FALLING;
    }
    else if (strcmp(edge_string, "rising") == 0)
    {
        return SGPIO_EDGE_RISING;
    }
    else if (strcmp(edge_string, "both") == 0)
    {
        return SGPIO_EDGE_BOTH;
    }
    return SGPIO_EDGE_ERROR;
}


int sgpio_export(sgpio_pin_t pin)
{
    int fd = open(SGPIO_EXPORT_DEVICE_PATH, O_WRONLY);
    if (fd == -1)
    {
        return -1;
    }

    char pin_buffer[SGPIO_PIN_STRING_MAX + 1];
    ssize_t bytes_written = snprintf(pin_buffer, SGPIO_PIN_STRING_MAX + 1, SGPIO_PIN_FMT, pin);
    if (write(fd, pin_buffer, bytes_written) == -1)
    {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int sgpio_unexport(sgpio_pin_t pin)
{
    int fd = open(SGPIO_UNEXPORT_DEVICE_PATH, O_WRONLY);
    if (fd == -1)
    {
        return -1;
    }

    char pin_buffer[SGPIO_PIN_STRING_MAX + 1];
    ssize_t bytes_written = snprintf(pin_buffer, SGPIO_PIN_STRING_MAX + 1, SGPIO_PIN_FMT, pin);
    if (write(fd, pin_buffer, bytes_written) == -1)
    {
        return -1;
    }
    close(fd);
    return 0;
}

bool sgpio_is_exported(sgpio_pin_t pin)
{
    struct stat statbuf;
    char path_buffer[SGPIO_PATH_STRING_MAX + 1];
    snprintf(path_buffer, SGPIO_PATH_STRING_MAX + 1, SGPIO_PIN_PATH, pin);
    return stat(path_buffer, &statbuf) == 0 && S_ISDIR(statbuf.st_mode);
}


int sgpio_open(sgpio_pin_t pin, int flags)
{
    char path_buffer[SGPIO_PATH_STRING_MAX + 1];
    snprintf(path_buffer, SGPIO_PATH_STRING_MAX + 1, SGPIO_VALUE_PATH, pin);
    return open(path_buffer, flags);
}

enum sgpio_level_t sgpio_read_fd(int fd)
{
    off_t offset = lseek(fd, 0, SEEK_SET);
    if (offset != 0)
    {
        return SGPIO_LEVEL_ERROR;
    }
    char level_char;
    ssize_t bytes_read = read(fd, &level_char, 1);
    if (bytes_read != 1)
    {
        return SGPIO_LEVEL_ERROR;
    }
    return sgpio_char_to_level(level_char);
}

enum sgpio_level_t sgpio_read(sgpio_pin_t pin)
{
    int fd = sgpio_open(pin, O_RDONLY);
    if (fd == -1)
    {
        return SGPIO_LEVEL_ERROR;
    }

    enum sgpio_level_t level = sgpio_read_fd(fd);
    close(fd);
    return level;
}

enum sgpio_level_t sgpio_write_fd(int fd, enum sgpio_level_t level)
{
    char level_char = sgpio_level_to_char(level);
    ssize_t bytes_written = write(fd, &level_char, 1);
    if (bytes_written != 1)
    {
        return SGPIO_LEVEL_ERROR;
    }
    return level;
}

enum sgpio_level_t sgpio_write(sgpio_pin_t pin, enum sgpio_level_t level)
{
    int fd = sgpio_open(pin, O_WRONLY);
    if (fd == -1)
    {
        return SGPIO_LEVEL_ERROR;
    }
    level = sgpio_write_fd(fd, level);
    close(fd);
    return level;
}


int sgpio_open_direction(sgpio_pin_t pin, int flags)
{
    char path_buffer[SGPIO_PATH_STRING_MAX + 1];
    snprintf(path_buffer, SGPIO_PATH_STRING_MAX + 1, SGPIO_DIRECTION_PATH, pin);
    return open(path_buffer, flags);
}

enum sgpio_direction_t sgpio_set_direction_fd(int fd, enum sgpio_direction_t direction)
{
    const char *direction_string;
    ssize_t direction_length = sgpio_direction_to_string(direction, &direction_string);
    if (direction_length == -1)
    {
        return SGPIO_DIRECTION_ERROR;
    }
    if (write(fd, direction_string, direction_length - 1) == -1)
    {
        return SGPIO_DIRECTION_ERROR;
    }
    return direction;
}

enum sgpio_direction_t sgpio_set_direction(sgpio_pin_t pin, enum sgpio_direction_t direction)
{
    int fd = sgpio_open_direction(pin, O_WRONLY);
    if (fd == -1)
    {
        return SGPIO_DIRECTION_ERROR;
    }
    direction = sgpio_set_direction_fd(fd, direction);
    close(fd);
    return direction;
}

enum sgpio_direction_t sgpio_get_direction_fd(int fd)
{
    off_t offset = lseek(fd, 0, SEEK_SET);
    if (offset != 0)
    {
        return SGPIO_DIRECTION_ERROR;
    }
    char direction_buffer[SGPIO_DIRECTION_STRING_MAX + 1];
    ssize_t bytes_read = read(fd, (void *)direction_buffer, SGPIO_DIRECTION_STRING_MAX);
    if (bytes_read == -1)
    {
        return SGPIO_DIRECTION_ERROR;
    }
    direction_buffer[bytes_read] = '\0';
    return sgpio_string_to_direction(direction_buffer);
}

enum sgpio_direction_t sgpio_get_direction(sgpio_pin_t pin)
{
    int fd = sgpio_open_direction(pin, O_WRONLY);
    if (fd == -1)
    {
        return SGPIO_DIRECTION_ERROR;
    }
    enum sgpio_direction_t direction = sgpio_get_direction_fd(fd);
    close(fd);
    return direction;
}


int sgpio_open_edge(sgpio_pin_t pin, int flags)
{
    char path_buffer[SGPIO_PATH_STRING_MAX + 1];
    snprintf(path_buffer, SGPIO_PATH_STRING_MAX + 1, SGPIO_EDGE_PATH, pin);
    return open(path_buffer, flags);
}

enum sgpio_edge_t sgpio_set_edge_fd(int fd, enum sgpio_edge_t edge)
{
    const char *edge_string;
    ssize_t edge_length = sgpio_edge_to_string(edge, &edge_string);
    if (edge_length == -1)
    {
        return SGPIO_EDGE_ERROR;
    }
    if (write(fd, edge_string, edge_length) == -1)
    {
        return SGPIO_EDGE_ERROR;
    }
    return edge;
}

enum sgpio_edge_t sgpio_set_edge(sgpio_pin_t pin, enum sgpio_edge_t edge)
{
    int fd = sgpio_open_edge(pin, O_WRONLY);
    if (fd == -1)
    {
        return SGPIO_EDGE_ERROR;
    }
    edge = sgpio_set_edge_fd(fd, edge);
    close(fd);
    return edge;
}

enum sgpio_edge_t sgpio_get_edge_fd(int fd)
{
    off_t offset = lseek(fd, 0, SEEK_SET);
    if (offset != 0)
    {
        return SGPIO_EDGE_ERROR;
    }
    char edge_buffer[SGPIO_EDGE_STRING_MAX + 1];
    ssize_t bytes_read = read(fd, edge_buffer, SGPIO_EDGE_STRING_MAX);
    if (bytes_read == -1)
    {
        return SGPIO_EDGE_ERROR;
    }
    edge_buffer[bytes_read] = '\0';
    return sgpio_string_to_edge(edge_buffer);
}

enum sgpio_edge_t sgpio_get_edge(sgpio_pin_t pin)
{
    int fd = sgpio_open_edge(pin, O_RDONLY);
    if (fd == -1)
    {
        return SGPIO_EDGE_ERROR;
    }
    enum sgpio_edge_t edge = sgpio_get_edge_fd(fd);
    close(fd);
    return edge;
}
