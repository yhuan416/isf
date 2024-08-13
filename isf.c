#include "isf.h"

#include <string.h>

/*
|         12 BYTE        |         |

| Pre | LEN | Flag | FCS | Payload |
| 4B  | 2B  |  2B  | 4B  | ...     |
*/

#pragma pack(push, 1)
typedef struct frame
{
    uint32_t magic; /*!< magic */
    uint16_t len;   /*!< len  */
    uint16_t flag;  /*!< flag  */

    uint32_t check_sum; /*!< check sum */

    uint8_t payload[0]; /*!< payload */
} _frame;
#pragma pack(pop)

#define FRAME_MAGIC (*(uint32_t *)"ISF0")

#define FRAME_HEADER_SIZE (12)

#define FRAME_LEN(frame) ((frame)->len)
#define FRAME_FLAG(frame) ((frame)->flag)
#define FRAME_CHECK_SUM(frame) ((frame)->check_sum)
#define FRAME_PAYLOAD(frame) (uint8_t *)((frame)->payload)

static int _isf_parse_frame(struct frame *frame)
{
    return 0;
}

static uint32_t _isf_calc_check_sum(struct frame *frame)
{
    return 0;
}

static inline void _isf_clear_buffer(isfcb *cb)
{
    cb->pos = 0;
}

static inline void _isf_append_buffer(isfcb *cb, uint8_t *data, int len)
{
    memcpy(cb->buffer + cb->pos, data, len);
    cb->pos += len;
}

int isf_init(isfcb *cb, struct isf_interface *i, void *priv)
{
    cb->i = i;
    cb->priv = priv;

    cb->buffer_size = 1024;
    cb->pos = 0;
    cb->state = ISF_WAIT_MAGIC1;

    cb->buffer = malloc(cb->buffer_size);
    if (cb->buffer == NULL)
    {
        return -1;
    }

    return 0;
}

int isf_deinit(isfcb *cb)
{
    free(cb->buffer);
    return 0;
}

int isf_on_data(isfcb *cb, uint8_t *data, int len)
{
    struct frame *frame = NULL;
    int frame_size = 0;
    int process_len = 0;
    int offset = 0;

    while (len > 0)
    {
        switch (cb->state)
        {
        case ISF_WAIT_MAGIC1:
            if (*data == ((uint8_t *)&FRAME_MAGIC)[0])
            {
                _isf_append_buffer(cb, data, 1);
                cb->state = ISF_WAIT_MAGIC2;
            }
            process_len = 1;
            break;

        case ISF_WAIT_MAGIC2:
            if (*data == ((uint8_t *)&FRAME_MAGIC)[1])
            {
                _isf_append_buffer(cb, data, 1);
                cb->state = ISF_WAIT_MAGIC3;
            }
            else
            {
                _isf_clear_buffer(cb);
                cb->state = ISF_WAIT_MAGIC1;
            }
            process_len = 1;
            break;

        case ISF_WAIT_MAGIC3:
            if (*data == ((uint8_t *)&FRAME_MAGIC)[2])
            {
                _isf_append_buffer(cb, data, 1);
                cb->state = ISF_WAIT_MAGIC4;
            }
            else
            {
                _isf_clear_buffer(cb);
                cb->state = ISF_WAIT_MAGIC1;
            }
            process_len = 1;
            break;

        case ISF_WAIT_MAGIC4:
            if (*data == ((uint8_t *)&FRAME_MAGIC)[3])
            {
                _isf_append_buffer(cb, data, 1);
                cb->state = ISF_WAIT_HEADER;
            }
            else
            {
                _isf_clear_buffer(cb);
                cb->state = ISF_WAIT_MAGIC1;
            }
            process_len = 1;
            break;

        case ISF_WAIT_HEADER:
            if (cb->pos + len >= FRAME_HEADER_SIZE)
            {
                // 追加头部剩余数据
                offset = FRAME_HEADER_SIZE - cb->pos;
                _isf_append_buffer(cb, data, offset);
                process_len = offset;

                // 解析头部
                frame = (struct frame *)cb->buffer;
                frame_size = FRAME_HEADER_SIZE + FRAME_LEN(frame);

                // 检查帧大小
                if (frame_size > cb->buffer_size)
                {
                    // 帧太大，清空缓冲区
                    _isf_clear_buffer(cb);
                    cb->state = ISF_WAIT_MAGIC1;
                    break;
                }

                // 切换状态
                cb->state = ISF_WAIT_PAYLOAD;
            }
            else
            {
                _isf_append_buffer(cb, data, len);
                process_len = len;
            }
            break;

        case ISF_WAIT_PAYLOAD:
            frame = (struct frame *)cb->buffer;
            frame_size = FRAME_HEADER_SIZE + FRAME_LEN(frame);

            if (cb->pos + len >= frame_size)
            {
                // 追加负载剩余数据
                offset = frame_size - cb->pos;
                _isf_append_buffer(cb, data, offset);
                process_len = offset;

                // 解析帧
                frame = (struct frame *)cb->buffer;
                if (_isf_parse_frame(frame) == 0)
                {
                    // 调用回调函数处理帧
                    cb->i->on_frame(cb, FRAME_PAYLOAD(frame), FRAME_LEN(frame));
                }

                // 清空缓冲区，等待下一个帧
                _isf_clear_buffer(cb);
                cb->state = ISF_WAIT_MAGIC1;
            }
            else
            {
                _isf_append_buffer(cb, data, len);
                process_len = len;
            }
            break;

        default:
            break;
        }

        data += process_len;
        len -= process_len;
    }

    return 0;
}

int isf_send(isfcb *cb, uint8_t *data, int data_len)
{
    // create a buffer to hold the frame
    int ret;
    int frame_size = data_len + FRAME_HEADER_SIZE;
    struct frame *frame = (struct frame *)malloc(frame_size);

    // fill in the frame
    frame->magic = FRAME_MAGIC;
    frame->len = data_len;
    frame->flag = 0;
    frame->check_sum = 0;
    memcpy(frame->payload, data, data_len);

    // calculate the check sum
    frame->check_sum = _isf_calc_check_sum(frame);

    // call the output function
    ret = cb->i->output(cb, (uint8_t *)frame, frame_size);

    free(frame);
    return ret;
}

int isf_get_mtu(isfcb *cb)
{
    return cb->i->get_mtu(cb);
}
