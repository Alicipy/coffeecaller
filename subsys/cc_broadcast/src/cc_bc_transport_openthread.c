#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>

#include <openthread/coap.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <openthread/ip6.h>

#include "cc_bc_serialization.h"
#include "cc_bc_client_handling.h"

#include <openthread/thread_ftd.h>  // for otThreadGetMeshLocalEid()

LOG_MODULE_DECLARE(cc_broadcast); // CONFIG_CC_BROADCAST_LOG_LEVEL);

static otInstance *ot_inst = NULL;

/**
 * @brief Dispatch incoming CoAP messages based on URI.
 */
static void dispatch_handler(void *context, otMessage *message, const otMessageInfo *messageInfo)
{
    char buffer[CC_BROADCAST_MAX_SERIALIZED_MSG_SIZE+1];
    uint16_t length = otMessageRead(message, otMessageGetOffset(message), buffer, sizeof(buffer) - 1);

    if (length == 0 || length >= sizeof(buffer)) {
        LOG_ERR("Failed to read CoAP message or message too large");
        return;
    }

    buffer[length] = '\0'; // Null-terminate the buffer to make it a valid string
    LOG_DBG("CoAP message payload: %s", buffer);

    dispatch_cc_bc_message_handler(buffer);
}

static int send_coap_message(const char *uri, const void *payload, size_t payload_len, const otIp6Address *peer_addr_str, otCoapType type)
{
    otMessage *message = otCoapNewMessage(ot_inst, NULL);
    otMessageInfo messageInfo;

    if (!message) return -ENOMEM;

    otCoapMessageInit(message, type, OT_COAP_CODE_POST);

    otError err = otCoapMessageAppendUriPathOptions(message, uri);
    if (err != OT_ERROR_NONE) goto fail;

    err = otCoapMessageSetPayloadMarker(message);
    if (err != OT_ERROR_NONE) goto fail;

    err = otMessageAppend(message, payload, payload_len);
    if (err != OT_ERROR_NONE) goto fail;

    memset(&messageInfo, 0, sizeof(messageInfo));
    messageInfo.mPeerAddr = *peer_addr_str;
    messageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;

    err = otCoapSendRequest(ot_inst, message, &messageInfo, NULL, NULL);
    if (err != OT_ERROR_NONE) goto fail;

    return 0;

fail:
    if (message) otMessageFree(message);
    return -EIO;
}

int transport_send_raw(const char *payload)
{
    if (!ot_inst) return -ENODEV;

    const char *uri = "/ot/broadcast";

    size_t payload_len = strlen(payload);

    // Send to multicast address
    otIp6Address multicast_addr;
    otIp6AddressFromString("ff03::1", &multicast_addr);
    int err = send_coap_message(uri, payload, payload_len, &multicast_addr, OT_COAP_TYPE_NON_CONFIRMABLE);
    if (err) {
        LOG_ERR("Failed to send to multicast: %d", err);
        return err;
    }
    LOG_INF("Broadcast message sent to ff03::1");

    // Send to self
    const otIp6Address *self_addr = otThreadGetMeshLocalEid(ot_inst);
    err = send_coap_message(uri, payload, payload_len, self_addr, OT_COAP_TYPE_NON_CONFIRMABLE);
    if (err) {
        LOG_ERR("Failed to send to self: %d", err);
        return err;
    }
    LOG_INF("Also sent message to self");

    return 0;
}

static int cc_broadcast_auto_init()
{

    ot_inst = openthread_get_default_instance();
    if (!ot_inst) {
        LOG_ERR("OpenThread instance not ready");
        return -ENODEV;
    }

    static otCoapResource resource = {
        .mUriPath = "*",
        .mHandler = dispatch_handler,
        .mContext = NULL,
        .mNext = NULL
    };

    const otError err = otCoapStart(ot_inst, OT_DEFAULT_COAP_PORT);
    if (err != OT_ERROR_NONE) {
        LOG_ERR("Failed to start CoAP service: %d", err);
        return -EIO;
    }

    otCoapAddResource(ot_inst, &resource);
    otCoapSetDefaultHandler(ot_inst, dispatch_handler, NULL);

    LOG_INF("OpenThread broadcast subsystem initialized");
    return 0;
}

#ifdef CONFIG_ZTEST
void cc_bc_transport_reset(void) {}
#endif /* CONFIG_ZTEST */

SYS_INIT(cc_broadcast_auto_init, APPLICATION, CC_BROADCAST_INIT_PRIO);
