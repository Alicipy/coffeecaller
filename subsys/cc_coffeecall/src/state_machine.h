#pragma once

#include <zephyr/kernel.h>
#include <zephyr/smf.h>

#include "participant_list.h"

#include "cc_broadcast/cc_broadcast.h"

/* List of events */
#define EVENT_LEFT_BTN_PRESS               BIT(0)
#define EVENT_RECV_PART_MESSAGE            BIT(2)
#define EVENT_WAIT_PARTICIPANT_TIMER_ENDED BIT(3)

#define ALL_EVENTS                                                                                 \
	(EVENT_LEFT_BTN_PRESS | EVENT_RECV_PART_MESSAGE | EVENT_WAIT_PARTICIPANT_TIMER_ENDED)


// context object
struct s_object {
	struct smf_ctx ctx;

	participant_list_t participant_list;

	cc_broadcast_client_id_t broadcast_client_id;

	struct k_timer wait_participants_timer;

	// event notifications
	struct k_event smf_event;
	uint32_t events;
	// new participant info
	// only valid for RECV_PART_MESSAGE
	struct participant new_participant;
};

void trigger_event(uint32_t event_mask, void* event_context);

void state_machine_init();

void run_state_machine_forever();
