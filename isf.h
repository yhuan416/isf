#ifndef _ISF_H_
#define _ISF_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct isfcb isfcb;
typedef struct isf_interface isf_interface;

typedef enum isf_state
{
    ISF_WAIT_MAGIC1 = 0,
    ISF_WAIT_MAGIC2,
    ISF_WAIT_MAGIC3,
    ISF_WAIT_MAGIC4,
    ISF_WAIT_HEADER,
    ISF_WAIT_PAYLOAD,
    ISF_MAX,
} isf_state;

// callback fn
typedef int (*output_fn_t)(isfcb *self, uint8_t *data, int len);
typedef int (*on_frame_fn_t)(isfcb *self, uint8_t *frame, int frame_size);
typedef int (*get_mtu_fn_t)(isfcb *self);

// callback fn
struct isf_interface
{
    output_fn_t output;
    on_frame_fn_t on_frame;
    get_mtu_fn_t get_mtu;
};

struct isfcb
{
    struct isf_interface *i;

    uint8_t *buffer;
    int buffer_size;
    int pos;

    isf_state state;
    int wait_len;

    void *priv;
};

extern int isf_init(isfcb *cb, struct isf_interface *i, void *priv);

extern int isf_deinit(isfcb *cb);

extern int isf_on_data(isfcb *cb, uint8_t *data, int len);

extern int isf_send(isfcb *cb, uint8_t *data, int data_len);

extern int isf_get_mtu(isfcb *cb);

#endif // _STREAM_FRAME_H_
