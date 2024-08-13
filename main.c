#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "isf.h"

int store_len;
uint8_t store_data[1024];

int output_fn(isfcb *self, uint8_t *data, int len)
{
    printf("output_fn\n");

    store_len = len;
    memcpy(store_data, data, len);

    int i;
    for (i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
    }

    printf("\n");

    return 0;
}

int on_frame_fn(isfcb *self, uint8_t *frame, int frame_size)
{
    printf("on_frame_fn\n");

    int i;
    for (i = 0; i < frame_size; i++)
    {
        printf("%02x ", frame[i]);
    }

    printf("\n");

    return 0;
}

int get_mtu_fn(isfcb *self)
{
    return 1500;
}

struct isf_interface i = {
    .output = output_fn,
    .on_frame = on_frame_fn,
    .get_mtu = get_mtu_fn,
};

int main(int argc, char const *argv[])
{
    int ret;
    isfcb cb;

    isf_init(&cb, &i, NULL);

    isf_send(&cb, (uint8_t *)"hello", 5);

    // isf_on_data(&cb, store_data, store_len);
    int i;
    for (i = 0; i < store_len; i++)
    {
        ret = isf_on_data(&cb, store_data + i, 1);
        printf("ret: %d, i: %d, d: %02x\n", ret, i, store_data[i]);
    }

    return 0;
}
