/* Author: Tobi Vollebregt */

#ifndef GPIO_H
#define GPIO_H

int gpio_name_to_idx(const char* name);
const char* gpio_idx_to_name(int idx);

int gpio_open(const char* name, int flags);
int gpio_close(const char *name, int fd);

#endif
