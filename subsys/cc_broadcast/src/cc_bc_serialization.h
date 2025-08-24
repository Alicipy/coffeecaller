#pragma once
#include <stddef.h>

// serialized JSON length, consisting of type, message, and extra for key names
#define CC_BROADCAST_MAX_SERIALIZED_MSG_SIZE (CONFIG_CC_BROADCAST_SUBSYS_MAX_TYPE_LEN + CONFIG_CC_BROADCAST_SUBSYS_MAX_PAYLOAD_LEN + 100)

typedef struct {
    const char *type;
    uint64_t sender_id;
    char *message;
} cc_bc_message_parts_t;

int serialize_cc_broadcast_message(char *buffer, size_t buffer_size, const cc_bc_message_parts_t *message);

int deserialize_cc_broadcast_message(cc_bc_message_parts_t *message, char *json_message);
