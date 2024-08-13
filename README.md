# Virtual Data Link

实现数据帧(frame)的封装和解封装, 支持数据帧在相邻节点上传输

``` plaintext

|         MTU OF INTERFACE         |
|         12 BYTE        |         |

| Pre | LEN | Flag | FCS | Payload |
| 4B  | 2B  |  2B  | 4B  | ...     |

```

Pre: 前导码, 4 字节, 0x55 0xAA 0x55 0xAA

LEN: 数据长度, 2 字节, 0x0000 ~ 0xFFFF

Flag: 标志位, 2 字节

FCS: 帧校验码, 4 字节, CRC32

Payload: 数据, 0 ~ 65535 字节

头部长度为12字节, MTU需要根据具体的接口配置, 例如: 1500字节

因此, payload 的最大长度为 1500 - 12 = 1488 字节


