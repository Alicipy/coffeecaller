#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#include "cc_bc_serialization.h"
#include "cc_bc_client_handling.h"

LOG_MODULE_DECLARE(cc_broadcast);

#define QUEUE_SIZE 10
#define STACK_SIZE 1024
#define THREAD_PRIORITY 5

struct recv_message_frame_t {
    char data[CC_BROADCAST_MAX_SERIALIZED_MSG_SIZE+1];
};

// Message queue for simulating RX (receiving from chip)
K_MSGQ_DEFINE(sim_msg_queue, sizeof(struct recv_message_frame_t), QUEUE_SIZE, 4);

// Thread definition
K_THREAD_STACK_DEFINE(recv_thread_stack, STACK_SIZE);
static struct k_thread recv_msg_thread;


// Simulated receiver thread
static void recv_new_message(void *arg1, void *arg2, void *arg3)
{
    struct recv_message_frame_t frame;

    while (1) {
        int ret = k_msgq_get(&sim_msg_queue, &frame, K_FOREVER);
        if (ret == 0) {
            LOG_INF("Simulated receive: Dispatching message: '%s'", frame.data);
            dispatch_cc_bc_message_handler(frame.data);
        }
    }
}

int transport_send_raw(const char *payload)
{
    if (payload == NULL) {
        return -EINVAL;
    }

    size_t payload_len = strlen(payload);
    if(payload_len > CC_BROADCAST_MAX_SERIALIZED_MSG_SIZE) {
      LOG_ERR("Payload too large to send");
      return -EINVAL;
    }

    struct recv_message_frame_t frame;
    memcpy(frame.data, payload, payload_len);
    frame.data[payload_len] = '\0';

    int ret = k_msgq_put(&sim_msg_queue, &frame, K_NO_WAIT);
    if (ret != 0) {
        LOG_WRN("Simulated send queue full, dropping frame");
        return -ENOBUFS;
    }

    LOG_INF("Simulated write: Queued frame: %s", payload);
    return 0;
}

// Init function to launch simulated receiver thread
static int cc_broadcast_auto_init()
{
    k_thread_create(&recv_msg_thread, recv_thread_stack,
                    K_THREAD_STACK_SIZEOF(recv_thread_stack),
                    recv_new_message,
                    NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);

    LOG_INF("cc_broadcast transport simulation initialized");
    return 0;
}

#ifdef CONFIG_ZTEST
void cc_bc_transport_reset(void)
{
    // empty message queue
    char buffer[sizeof(struct recv_message_frame_t)];
    while (k_msgq_get(&sim_msg_queue, &buffer, K_NO_WAIT) == 0)
    {
    }
}
#endif /* CONFIG_ZTEST */

SYS_INIT(cc_broadcast_auto_init, APPLICATION, CC_BROADCAST_INIT_PRIO);
