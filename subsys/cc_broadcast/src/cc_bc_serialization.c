#include <zephyr/data/json.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "cc_bc_serialization.h"

LOG_MODULE_DECLARE(cc_broadcast);

static const struct json_obj_descr json_descr[] = {
    JSON_OBJ_DESCR_PRIM(cc_bc_message_parts_t, type, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(cc_bc_message_parts_t, sender_id, JSON_TOK_UINT64),
    JSON_OBJ_DESCR_PRIM(cc_bc_message_parts_t, message, JSON_TOK_STRING),
};


int serialize_cc_broadcast_message(char *buffer, const size_t buffer_size, const cc_bc_message_parts_t *message)
{
    if (message == NULL || buffer == NULL || buffer_size == 0) {
        return -EINVAL;
    }

    const int ret = json_obj_encode_buf(json_descr, ARRAY_SIZE(json_descr), message, buffer, buffer_size);
    if (ret < 0 || (size_t)ret >= buffer_size) {
        return -ENOMEM;
    }

    return 0;
}

int deserialize_cc_broadcast_message(cc_bc_message_parts_t *message, char *json_message)
{
    if (json_message == NULL || message == NULL) {
        return -EINVAL;
    }

    const int64_t ret = json_obj_parse(json_message, strlen(json_message), json_descr, ARRAY_SIZE(json_descr), message);
    if (ret < 0) {
        return -EINVAL;
    }

    return 0;
}