#include "assert.h"

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "cc_broadcast/cc_broadcast.h"

static const char* TEST_TYPE_1 = "first";
static const char* TEST_TYPE_2 = "second";

static const char* TEST_TYPE_EXACT_MAX = "1234567890";
static const char* TEST_TYPE_TOO_LONG = "1234567890X";

static cc_broadcast_payload_t payload_1 = "FirstMsg"; // length: 8
static cc_broadcast_payload_t payload_2 = "SecondMsg"; // length: 9
static cc_broadcast_payload_t max_payload_length_payload = "12345678901234567890"; // length: 20
static cc_broadcast_payload_t too_long_payload = "12345678901234567890X"; // length: 21

static volatile size_t first_type_len_sum;
static volatile size_t second_type_len_sum;

static void dummy_msg_received_handler(const cc_broadcast_message_t *message)
{
}

static void first_type_msg_received_handler(const cc_broadcast_message_t *message)
{
    first_type_len_sum += strlen(message->payload);
}

static void second_type_msg_received_handler(const cc_broadcast_message_t *message)
{
    second_type_len_sum += strlen(message->payload);
}

static void before_test_setup(void *f)
{
    cc_broadcast_reset();
    first_type_len_sum = 0;
    second_type_len_sum = 0;
}
ZTEST(cc_broadcast_tests, test_register_client__register_multiple__no_error)
{
    cc_broadcast_client_id_t client_1, client_2, client_3;

    int ret = cc_broadcast_register_client(&client_1, TEST_TYPE_1, dummy_msg_received_handler);
    zassert_equal(ret, 0, "cc_broadcast_register_client first call failed");

    ret = cc_broadcast_register_client(&client_2, TEST_TYPE_2, dummy_msg_received_handler);
    zassert_equal(ret, 0, "cc_broadcast_register_client second call failed");

    ret = cc_broadcast_register_client(&client_3, TEST_TYPE_2, dummy_msg_received_handler);
    zassert_equal(ret, 0, "cc_broadcast_register_client third call failed");
}

ZTEST(cc_broadcast_tests, test_register_client__register_too_much__get_error)
{
    cc_broadcast_client_id_t allowed_clients[4];
    for (int i = 0; i < 4; i++)
    {
        int ret = cc_broadcast_register_client(allowed_clients + i, TEST_TYPE_1, dummy_msg_received_handler);
        zassert_equal(ret, 0, "allowed cc_broadcast_register_client failed");
    }

    cc_broadcast_client_id_t client_too_much;
    int ret = cc_broadcast_register_client(&client_too_much, TEST_TYPE_1, dummy_msg_received_handler);

    zassert_equal(ret, -ENOMEM, "invalid client was still accepted");
}

ZTEST(cc_broadcast_tests, test_register_client__input_null__returns_error)
{
    cc_broadcast_client_id_t client_id_1, client_id_2;

    int ret_client_null = cc_broadcast_register_client(NULL, TEST_TYPE_1, dummy_msg_received_handler);
    zassert_equal(ret_client_null, -EINVAL, "register_client accepted NULL as client_id");

    int ret_type_null = cc_broadcast_register_client(&client_id_1, NULL, dummy_msg_received_handler);
    zassert_equal(ret_type_null, -EINVAL, "register_client accepted NULL as type");

    int ret_handler_null = cc_broadcast_register_client(&client_id_2, TEST_TYPE_1, NULL);
    zassert_equal(ret_handler_null, -EINVAL, "register_client accepted NULL as handler");
}

ZTEST(cc_broadcast_tests, test_register_client__exact_max_type_len__no_error)
{
    cc_broadcast_client_id_t client_id;
    int ret = cc_broadcast_register_client(&client_id, TEST_TYPE_EXACT_MAX, dummy_msg_received_handler);
    zassert_equal(ret, 0, "register_client did not accept type that is exactly max long");
}

ZTEST(cc_broadcast_tests, test_register_client__too_long_type__returns_error)
{
    cc_broadcast_client_id_t client_id;
    int ret = cc_broadcast_register_client(&client_id, TEST_TYPE_TOO_LONG, dummy_msg_received_handler);
    zassert_equal(ret, -EINVAL, "register_client accepted too long type");
}

ZTEST(cc_broadcast_tests, test_unregister_client__register_unregister_one_client__no_error)
{
    cc_broadcast_client_id_t client_id;

    int ret_client = cc_broadcast_register_client(&client_id, TEST_TYPE_1, dummy_msg_received_handler);
    zassert_equal(ret_client, 0, "register_client failed suddenly, should be tested already");
    zassert_true(client_id > 0, "client_id is not positive");

    int ret = cc_broadcast_unregister_client(&client_id);
    zassert_equal(ret, 0, "unregister_client of previosly registered client failed");
    zassert_equal(client_id, 0, "client_id is not 0 after unregister");
}

ZTEST(cc_broadcast_tests, test_unregister_client__unregister_already_unregistered_client__get_error)
{
    cc_broadcast_client_id_t client_id;

    int ret_client = cc_broadcast_register_client(&client_id, TEST_TYPE_1, dummy_msg_received_handler);
    zassert_equal(ret_client, 0, "register_client failed suddenly, should be tested already");
    zassert_true(client_id > 0, "client_id is not positive");

    int ret = cc_broadcast_unregister_client(&client_id);
    zassert_equal(ret, 0, "unregister_client of previosly registered client failed, tested in other case");
    ret = cc_broadcast_unregister_client(&client_id);

    zassert_equal(ret, -EINVAL, "unregister_client of already un-registered client succeeded but should fail");
}


ZTEST(cc_broadcast_tests, test_unregister_client__unregister_non_registered_client__get_error)
{
    cc_broadcast_client_id_t client_id = 123456789;

    int ret = cc_broadcast_unregister_client(&client_id);
    zassert_equal(ret, -ENOENT, "unregister_client of non-registered client works should fail");
}


ZTEST(cc_broadcast_tests, test_unregister_client__unregister_null__get_error)
{

    int ret = cc_broadcast_unregister_client(NULL);
    zassert_equal(ret, -EINVAL, "unregister_client of non-registered client works should fail");
}

ZTEST(cc_broadcast_tests, test_send_message__registered_clients_send_message__correctly_received)
{
    cc_broadcast_client_id_t client_1, client_2, client_3;
    cc_broadcast_register_client(&client_1, TEST_TYPE_1, first_type_msg_received_handler);
    cc_broadcast_register_client(&client_2, TEST_TYPE_2, second_type_msg_received_handler);
    cc_broadcast_register_client(&client_3, TEST_TYPE_2, second_type_msg_received_handler);

    int ret = cc_broadcast_send(client_1, payload_1);
    zassert_equal(ret, 0, "cc_broadcast_send 1 failed");
    k_sleep(K_MSEC(10));
    zassert_equal(first_type_len_sum, 8, "message 1 was not received properly");
    zassert_equal(second_type_len_sum, 0, "message 1 changed second counter");

    ret = cc_broadcast_send(client_2, payload_2);
    zassert_equal(ret, 0, "cc_broadcast_send 2 failed");
    k_sleep(K_MSEC(10));
    zassert_equal(first_type_len_sum, 8, "message 2 changed first counter");
    zassert_equal(second_type_len_sum, 18, "message 2 was not received properly");

    cc_broadcast_unregister_client(&client_3);
    cc_broadcast_send(client_2, payload_2);
    k_sleep(K_MSEC(10));
    zassert_equal(second_type_len_sum, 27, "message 2/3 was not received properly");
}

ZTEST(cc_broadcast_tests, test_send_message__send_message_null__get_error)
{
    cc_broadcast_client_id_t client_id;
    cc_broadcast_register_client(&client_id, TEST_TYPE_1, dummy_msg_received_handler);

    int ret = cc_broadcast_send(client_id, NULL);
    zassert_equal(ret, -EINVAL, "cc_broadcast_send with NULL message succeeded");
}

ZTEST(cc_broadcast_tests, test_send_message__send_message_by_unregistered_client__get_error)
{
    cc_broadcast_client_id_t client_id_1 = 0;
    cc_broadcast_client_id_t client_id_2 = 123;

    int ret = cc_broadcast_send(client_id_1, payload_1);
    zassert_equal(ret, -EINVAL, "cc_broadcast_send by unregistered client succeeded");

    ret = cc_broadcast_send(client_id_2, payload_1);
    zassert_equal(ret, -ENOENT, "cc_broadcast_send by unregistered client succeeded");
}

ZTEST(cc_broadcast_tests, test_send_message__max_payload_length__no_error_works_propertly)
{
    cc_broadcast_client_id_t client_id;
    cc_broadcast_register_client(&client_id, TEST_TYPE_EXACT_MAX, first_type_msg_received_handler);

    int ret = cc_broadcast_send(client_id, max_payload_length_payload);
    zassert_equal(ret, 0, "cc_broadcast_send with max message length failed");
    k_sleep(K_MSEC(100));
    zassert_equal(first_type_len_sum, 20, "message with max length was not received properly");
}

ZTEST(cc_broadcast_tests, test_send_message__too_long_payload__get_error)
{
    cc_broadcast_client_id_t client_id;
    cc_broadcast_register_client(&client_id, TEST_TYPE_EXACT_MAX, first_type_msg_received_handler);

    int ret = cc_broadcast_send(client_id, too_long_payload);
    zassert_equal(ret, -EINVAL, "cc_broadcast_send with too long message succeeded");
    k_sleep(K_MSEC(10));
    zassert_equal(first_type_len_sum, 0, "message with max length was accidentally received");
}


ZTEST_SUITE(cc_broadcast_tests, NULL, NULL, before_test_setup, NULL, NULL);
