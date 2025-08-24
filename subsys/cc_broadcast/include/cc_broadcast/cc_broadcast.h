#pragma once

#include <stdint.h>

typedef uint32_t cc_broadcast_client_id_t;
typedef const char *cc_broadcast_type_t;
typedef char* cc_broadcast_payload_t;

typedef struct {
	uint64_t sender_id;
	char payload[CONFIG_CC_BROADCAST_SUBSYS_MAX_PAYLOAD_LEN + 1];
} cc_broadcast_message_t;

typedef void (*message_received_callback)(const cc_broadcast_message_t *message);

int cc_broadcast_register_client(cc_broadcast_client_id_t *client_id,
								 cc_broadcast_type_t type,
								 message_received_callback callback);

int cc_broadcast_send(cc_broadcast_client_id_t client_id, cc_broadcast_payload_t payload);

int cc_broadcast_unregister_client(cc_broadcast_client_id_t* client_id);

#ifdef CONFIG_ZTEST
void cc_broadcast_reset(void);
#endif /* CONFIG_ZTEST */