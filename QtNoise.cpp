/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

extern "C" {
#include "putty.h"
}

void noise_get_heavy(void (*func) (void *, int))
{
}

void random_save_seed(void)
{
/*
    int len;
    void *data;

    if (random_active) {
    random_get_savedata(&data, &len);
    write_random_seed(data, len);
    sfree(data);
    }
*/
}

void noise_get_light(void (*func) (void *, int))
{
}

void noise_regular(void)
{
}

/*void noise_ultralight(unsigned long data)
{
}*/
