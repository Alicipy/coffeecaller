#include <assert.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/drivers/hwinfo.h>

#include "cc_broadcast/cc_broadcast.h"
#include "cc_bc_serialization.h"
#include "cc_bc_transport.h"


LOG_MODULE_REGISTER(cc_broadcast, CONFIG_LOG_DEFAULT_LEVEL); // CONFIG_CC_BROADCAST_LOG_LEVEL);

typedef struct
{
    cc_broadcast_client_id_t client_id;
    cc_broadcast_type_t type;
    message_received_callback message_callback;
} cc_broadcast_info_t;

K_MUTEX_DEFINE(cc_broadcast_mutex);
static cc_broadcast_info_t CC_BROADCAST_CLIENT_INFO[CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS] = {0};

static int get_first_free_entry(void)
{
    for (int i = 0; i < CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS; i++)
    {
        if (CC_BROADCAST_CLIENT_INFO[i].client_id == 0)
        {
            return i;
        }
    }
    return -1; // No free entry found
}

static cc_broadcast_client_id_t get_valid_client_id(void)
{
    cc_broadcast_client_id_t client_id = sys_rand32_get();
    client_id = (client_id & 0x0FFFFFFF) + 1; // Ensure it's positive and at least 1
    return client_id;
}

static bool is_client_id_unique(const cc_broadcast_client_id_t client_id)
{
    for (int i = 0; i < CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS; i++)
    {
        if (CC_BROADCAST_CLIENT_INFO[i].client_id == client_id)
        {
            return false;
        }
    }
    return true;
}

static int is_same_cc_broadcast_type(cc_broadcast_type_t type1, cc_broadcast_type_t type2)
{
    assert(type1 != NULL);
    assert(type2 != NULL);

    return strncmp(type1, type2, CONFIG_CC_BROADCAST_SUBSYS_MAX_TYPE_LEN) == 0;
}

void dispatch_cc_bc_message_handler(char* json_message)
{
    cc_bc_message_parts_t deserialized_message;
    int ret = deserialize_cc_broadcast_message(&deserialized_message, json_message);
    if (ret)
    {
        LOG_ERR("Failed to deserialize message: %d", ret);
        return;
    }

    if (deserialized_message.type == NULL || deserialized_message.message == NULL)
    {
        LOG_ERR("Deserialized message type or content is NULL");
        return;
    }
    if (strlen(deserialized_message.type) > CONFIG_CC_BROADCAST_SUBSYS_MAX_TYPE_LEN ||
        strlen(deserialized_message.message) > CONFIG_CC_BROADCAST_SUBSYS_MAX_PAYLOAD_LEN)
    {
        LOG_ERR("Message type or content is too long");
        return;
    }


    LOG_INF("Dispatch message handler for type: %s", deserialized_message.type);

    k_mutex_lock(&cc_broadcast_mutex, K_FOREVER);
    for (int i = 0; i < CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS; i++)
    {
        const cc_broadcast_info_t* broadcast_client = CC_BROADCAST_CLIENT_INFO + i;
        if (broadcast_client->client_id && is_same_cc_broadcast_type(broadcast_client->type, deserialized_message.type))
        {
            assert(broadcast_client->message_callback != NULL);

            cc_broadcast_message_t message;
            message.sender_id = deserialized_message.sender_id;
            strncpy(message.payload, deserialized_message.message, CONFIG_CC_BROADCAST_SUBSYS_MAX_PAYLOAD_LEN);
            message.payload[CONFIG_CC_BROADCAST_SUBSYS_MAX_PAYLOAD_LEN] = '\0'; // Ensure null termination

            LOG_INF("Calling message callback for client ID: %d", broadcast_client->client_id);
            LOG_DBG("Message sender ID: %llu", message.sender_id);
            LOG_DBG("Message content: %s", message.payload);
            broadcast_client->message_callback(&message);
        }
    }
    k_mutex_unlock(&cc_broadcast_mutex);
}

static int get_unique_client_id(uint64_t* client_id)
{
    static uint64_t eid_value = 0;

    if (eid_value == 0)
    {
#ifdef CONFIG_ARCH_POSIX
        eid_value = sys_rand64_get();
#else
        uint8_t buffer[8];
        ssize_t ret = hwinfo_get_device_id(buffer, sizeof(buffer));

        if (ret < 0 || ret > sizeof(buffer))
        {
            LOG_ERR("Failed to get device id: %d", ret);
            return -1;
        }

        memcpy(&eid_value, buffer, (size_t)(ret));
#endif

    }

    *client_id = eid_value;
    return 0;
}

static void send_broadcast_message(cc_broadcast_type_t type, cc_broadcast_payload_t payload)
{
    char json_message[CC_BROADCAST_MAX_SERIALIZED_MSG_SIZE + 1];

    uint64_t sender_id;
    int ret = get_unique_client_id(&sender_id);
    if (ret < 0)
    {
        LOG_ERR("Failed to get sender ID");
        return;
    }


    ret = serialize_cc_broadcast_message(json_message, sizeof(json_message), &(cc_bc_message_parts_t){
                                             .type = type,
                                             .sender_id = sender_id,
                                             .message = payload,
                                         });
    if (ret < 0)
    {
        LOG_ERR("Failed to serialize message: %d", ret);
        return;
    }

    transport_send_raw(json_message);
}

int cc_broadcast_register_client(cc_broadcast_client_id_t* client_id,
                                 cc_broadcast_type_t type,
                                 message_received_callback callback)
{
    if (client_id == NULL || type == NULL || callback == NULL)
    {
        LOG_ERR("Invalid parameters: client_id or type or callback is NULL");
        return -EINVAL;
    }

    if (strlen(type) > CONFIG_CC_BROADCAST_SUBSYS_MAX_TYPE_LEN)
    {
        LOG_ERR("Type is too long");
        return -EINVAL;
    }

    int return_code = 0;

    k_mutex_lock(&cc_broadcast_mutex, K_FOREVER);

    const int free_entry = get_first_free_entry();
    if (free_entry < 0)
    {
        LOG_ERR("No free entry available for new client");
        return_code = -ENOMEM;
        goto end;
    }

    cc_broadcast_client_id_t new_client_id;
    do
    {
        new_client_id = get_valid_client_id();
    }
    while (!is_client_id_unique(new_client_id));
    assert(new_client_id > 0);

    CC_BROADCAST_CLIENT_INFO[free_entry] = (cc_broadcast_info_t){
        .client_id = new_client_id,
        .type = type,
        .message_callback = callback
    };

    *client_id = new_client_id;

end:
    k_mutex_unlock(&cc_broadcast_mutex);
    return return_code;
}

int cc_broadcast_send(cc_broadcast_client_id_t client_id, cc_broadcast_payload_t payload)
{
    if (client_id == 0)
    {
        LOG_ERR("Client not registered");
        return -EINVAL;
    }

    if (payload == NULL)
    {
        LOG_ERR("Invalid payload: payload is NULL");
        return -EINVAL;
    }

    if (strlen(payload) > CONFIG_CC_BROADCAST_SUBSYS_MAX_PAYLOAD_LEN)
    {
        LOG_ERR("Message is too long");
        return -EINVAL;
    }

    int return_code = 0;
    k_mutex_lock(&cc_broadcast_mutex, K_FOREVER);

    for (int i = 0; i < CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS; i++)
    {
        if (CC_BROADCAST_CLIENT_INFO[i].client_id == client_id)
        {
            const cc_broadcast_info_t* client_info = &CC_BROADCAST_CLIENT_INFO[i];
            const char* type = client_info->type;
            send_broadcast_message(type, payload);
            goto end;
        }
    }
    LOG_ERR("Client ID %d not found", client_id);
    return_code = -ENOENT;

end:
    k_mutex_unlock(&cc_broadcast_mutex);
    return return_code;
}

int cc_broadcast_unregister_client(cc_broadcast_client_id_t* client_id)
{
    if (client_id == NULL)
    {
        LOG_ERR("Invalid parameters: client_id is NULL");
        return -EINVAL;
    }
    if (*client_id == 0)
    {
        LOG_ERR("Client not registered");
        return -EINVAL;
    }

    int return_code = 0;
    k_mutex_lock(&cc_broadcast_mutex, K_FOREVER);

    for (int i = 0; i < CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS; i++)
    {
        if (CC_BROADCAST_CLIENT_INFO[i].client_id == *client_id)
        {
            *client_id = 0;
            CC_BROADCAST_CLIENT_INFO[i].client_id = 0;
            CC_BROADCAST_CLIENT_INFO[i].type = NULL;
            CC_BROADCAST_CLIENT_INFO[i].message_callback = NULL;
            goto end;
        }
    }

    LOG_ERR("Client ID %d not found", *client_id);
    return_code = -ENOENT;

end:
    k_mutex_unlock(&cc_broadcast_mutex);
    return return_code;
}

#ifdef CONFIG_ZTEST
void cc_bc_client_reset(void)
{
    k_mutex_lock(&cc_broadcast_mutex, K_FOREVER);
    for (int i = 0; i < CONFIG_CC_BROADCAST_SUBSYS_MAX_CLIENTS; i++)
    {
        CC_BROADCAST_CLIENT_INFO[i].client_id = 0;
        CC_BROADCAST_CLIENT_INFO[i].type = NULL;
        CC_BROADCAST_CLIENT_INFO[i].message_callback = NULL;
    }
    k_mutex_unlock(&cc_broadcast_mutex);
}
#endif /* CONFIG_ZTEST */
