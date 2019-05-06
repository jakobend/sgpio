#include "sgpio.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Blink the LED with 100ms intervals during reboot or shutdown
const struct timespec BLINK_INTERVAL = {0, 10000000};

const sgpio_pin_t POWER_ENABLE_PIN = 4;
const sgpio_pin_t LED_PIN = 14;
const sgpio_pin_t POWER_PIN = 3;
const sgpio_pin_t RESET_PIN = 2;

#define SHUTDOWN_COMMAND "shutdown -h now"
#define REBOOT_COMMAND "reboot"

int led_fd = -1, power_fd = -1, reset_fd = -1;

static void exit_handler()
{
    // We don't particularly care if any of these calls fail, so no error
    // handling here.
    close(led_fd);
    close(power_fd);
    close(reset_fd);

    sgpio_unexport(LED_PIN);
    sgpio_unexport(POWER_PIN);
    sgpio_unexport(RESET_PIN);

    // Unexporting power enable doesn't seem to cut the power supply as you'd
    // expect it to do.
    sgpio_unexport(POWER_ENABLE_PIN);
}

int main(int argc, char **argv)
{
    // If error handling causes a regular exit, clean up
    atexit(&exit_handler);

    // SIGTERM, which will happen during shutdown, will not cause a clean up
    // so the LED stays lit until the system is powered down.
    // This means we have to handle a case where the program was terminated
    // previously and left pins exported (e.g. systemctl restart).

    // Export the power enable pin and set it to high
    if (!sgpio_is_exported(POWER_ENABLE_PIN))
    {
        if (sgpio_export(POWER_ENABLE_PIN) == -1)
        {
            perror("Failed to export power enable pin");
            return EXIT_FAILURE;
        }
    }
    if (sgpio_set_direction(POWER_ENABLE_PIN, SGPIO_DIRECTION_OUT_HIGH) == -1)
    {
        perror("Failed to set power enable pin direction");
        return EXIT_FAILURE;
    }

    // Export the LED pin and set it to high
    if (!sgpio_is_exported(LED_PIN))
    {
        if (sgpio_export(LED_PIN) == -1)
        {
            perror("Failed to export LED pin");
            return EXIT_FAILURE;
        }
    }
    if (sgpio_set_direction(LED_PIN, SGPIO_DIRECTION_OUT_HIGH) == -1)
    {
        perror("Failed to set LED pin direction");
        return EXIT_FAILURE;
    }

    // Export the power pin and set it to generate an interrupt when pressed
    if (!sgpio_is_exported(POWER_PIN))
    {
        if (sgpio_export(POWER_PIN) == -1)
        {
            perror("Failed to export power button pin");
            return EXIT_FAILURE;
        }
    }
    if (sgpio_set_direction(POWER_PIN, SGPIO_DIRECTION_IN) == -1 ||
        sgpio_set_edge(POWER_PIN, SGPIO_EDGE_FALLING) == -1)
    {
        perror("Failed to set power button pin direction/edge");
        return EXIT_FAILURE;
    }

    // Export the reset pin and set it to generate an interrupt when pressed
    if (!sgpio_is_exported(RESET_PIN))
    {
        if (sgpio_export(RESET_PIN) == -1)
        {
            perror("Failed to export reset button pin");
            return EXIT_FAILURE;
        }
    }
    if (sgpio_set_direction(RESET_PIN, SGPIO_DIRECTION_IN) == -1 ||
        sgpio_set_edge(RESET_PIN, SGPIO_EDGE_FALLING) == -1)
    {
        perror("Failed to set reset button pin direction/edge");
        return EXIT_FAILURE;
    }
    
    // Open file descriptors to be used with poll
    if ((led_fd = sgpio_open(LED_PIN, O_WRONLY)) == -1 ||
        (power_fd = sgpio_open(POWER_PIN, O_RDONLY)) == -1 ||
        (reset_fd = sgpio_open(RESET_PIN, O_RDONLY)) == -1)
    {
        perror("Failed to open pin file descriptor");
        return EXIT_FAILURE;
    }

    // This is our set of file descriptors to pass to poll
    struct pollfd pollfds[] = {
        {power_fd, POLLPRI | POLLERR, 0},
        {reset_fd, POLLPRI | POLLERR, 0}
    };

    // Consume any prior interrupts or poll won't work
    enum sgpio_level_t power_level, reset_level;
    if ((power_level = sgpio_read_fd(power_fd)) == -1 ||
        (reset_level = sgpio_read_fd(reset_fd)) == -1)
    {
        perror("Failed to read from pin");
        return EXIT_FAILURE;
    }

    // This is our main loop
    do
    {
        if (poll(pollfds, 2, -1) > 0)
        {
            // Consume interrupts and check levels
            if ((power_level = sgpio_read_fd(power_fd)) == -1 ||
                (reset_level = sgpio_read_fd(reset_fd)) == -1)
            {
                perror("Failed to read from pin");
                return EXIT_FAILURE;
            }
        }
    } while (power_level != SGPIO_LEVEL_LOW && reset_level != SGPIO_LEVEL_LOW);

    if (power_level == SGPIO_LEVEL_LOW)
    {
        printf("Calling %s...\n", SHUTDOWN_COMMAND);
        system(SHUTDOWN_COMMAND);
    }
    else
    {
        printf("Calling %s...\n", REBOOT_COMMAND);
        system(REBOOT_COMMAND);
    }

    // Blink until we receive SIGTERM
    // Unsucessful shutdowns/reboots are not handled, and interfering with them
    // is probably not a good idea anyway.
    // Blinking may not show up if we get terminated early and/or the system is
    // quick to shut down.
    while (1)
    {
        sgpio_write_fd(led_fd, SGPIO_LEVEL_LOW);
        nanosleep(&BLINK_INTERVAL, NULL);
        sgpio_write_fd(led_fd, SGPIO_LEVEL_HIGH);
        nanosleep(&BLINK_INTERVAL, NULL);
    }
    return 0;
}
