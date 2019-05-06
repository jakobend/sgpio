# sgpio
A low-level wrapper around the sysfs GPIO interface with a NESPi button/LED service as demo

## Example
```c
sgpio_pin_t my_pin = 4;
sgpio_export(my_pin);
sgpio_set_direction(my_pin, SGPIO_DIRECTION_OUT);
sgpio_write(my_pin, SGPIO_LEVEL_HIGH);
// ...
sgpio_set_direction(my_pin, SGPIO_DIRECTION_IN);
enum sgpio_level_t level = sgpio_read(my_pin);
// ...
sgpio_unexport(my_pin);
```
