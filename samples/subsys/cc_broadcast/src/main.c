#include <assert.h>
#include <string.h>

#include "zephyr/kernel.h"

#include <cc_broadcast/cc_broadcast.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(cc_broadcast_sample, LOG_LEVEL_DBG);

size_t first_len_sum = 0;
size_t second_len_sum = 0;

void message_handler_one(const cc_broadcast_message_t *message)
{
    LOG_INF("Received message in one: %s", message->payload);
    first_len_sum += strlen(message->payload);
}

void message_handler_two(const cc_broadcast_message_t *message)
{
    LOG_INF("Received message in two: %s", message->payload);
    second_len_sum += strlen(message->payload);
}

int main(void)
{
    LOG_INF("Starting CC_BROADCAST example");

    cc_broadcast_client_id_t client_id_1;
    cc_broadcast_client_id_t client_id_2;
    cc_broadcast_client_id_t client_id_3;

    LOG_INF("Registering 3 clients...");
    cc_broadcast_register_client(&client_id_1, "one", message_handler_one);
    cc_broadcast_register_client(&client_id_2, "two", message_handler_two);
    cc_broadcast_register_client(&client_id_3, "two", message_handler_two);
    LOG_INF("Registered 3 clients...");

    cc_broadcast_payload_t payload_1 = "Hello First World!";

    cc_broadcast_payload_t payload_2 = "Hello Second World!";

    LOG_INF("Sending payload 1");
    cc_broadcast_send(client_id_1, payload_1);
    LOG_INF("Sending payload 2");
    cc_broadcast_send(client_id_2, payload_2);

    LOG_INF("Wait for payloads to arrive...");
    k_sleep(K_MSEC(100));
    LOG_INF("Waiting done");

    LOG_INF("Sum of first payloads is %d, expected 18", first_len_sum);
    assert(first_len_sum == 18);
    LOG_INF("Sum of second payloads is %d, expected 38", second_len_sum);
    assert(second_len_sum == 38);

    LOG_INF("Unregistering client 3");
    cc_broadcast_unregister_client(&client_id_3);

    LOG_INF("Sending payload 2 again");
    cc_broadcast_send(client_id_2, payload_2);

    LOG_INF("Wait for payloads to arrive...");
    k_sleep(K_MSEC(100));
    LOG_INF("Waiting done");

    LOG_INF("Sum of first payloads is %d, expected 18", first_len_sum);
    assert(first_len_sum == 18);
    LOG_INF("Sum of second payloads is %d, expected 57", second_len_sum);
    assert(second_len_sum == 57);

    LOG_INF("Finished CC_BROADCAST example");
}

