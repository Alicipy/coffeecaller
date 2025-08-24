#include "state_machine.h"

#include <assert.h>

#include "cc_lib/aldrin.h"
#include "cc_lib/leds.h"
#include "cc_lib/watchdog.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "cc_broadcast/cc_broadcast.h"
#include "cc_leds.h"

LOG_MODULE_REGISTER(cc_coffeecall_subsys_state_machine);

static struct s_object s_obj;

static const struct smf_state cc_states[];

enum cc_state
{
    S_START,
    S_IDLE,
    S_ACTIVE_CALL
};


static void update_leds_with_participants(struct s_object* s)
{
    const uint8_t count_participants = get_number_of_participants(&s->participant_list);
    cc_leds_show_number_of_participants(count_participants);
}

static int send_own_participation_status(const struct s_object* s)
{
    LOG_INF("idle state: sending take part msg");
    const int ret = cc_broadcast_send(s->broadcast_client_id, "1");
    if (ret < 0)
    {
        LOG_ERR("Sending failed with %d", ret);
        return -1;
    }
    else
    {
        LOG_INF("Sending '1' (participation) event successful");
        return 0;
    }
}

static void start_participant_wait_timer(struct s_object* s)
{
    k_timer_start(&s->wait_participants_timer, K_SECONDS(CONFIG_CC_COFFEECALL_CALL_ACTIVE_TIMEOUT),
                  K_FOREVER);
}

static void s_start_run(void* o)
{
    struct s_object* s = o;
    LOG_INF("state change start -> idle");
    cc_leds_all_off();
    smf_set_state(SMF_CTX(s), &cc_states[S_IDLE]);
}

static void s_idle_entry(void* o)
{
    struct s_object* s = o;
    LOG_DBG("idle entry with %d", s->events);

    clear_participant_list(&s->participant_list);
}

static void s_idle_run(void* o)
{
    struct s_object* s = o;
    LOG_DBG("idle run with %d", s->events);

    if (s->events & EVENT_LEFT_BTN_PRESS)
    {
        send_own_participation_status(s);
    }

    if (s->events & EVENT_RECV_PART_MESSAGE)
    {
        LOG_INF("state change idle -> active call");

        const int status = set_participant_status(&s->participant_list, s->new_participant);
        if (status < 0)
        {
            LOG_ERR("Failed to add participant to list");
            return;
        }
        assert(status > 0);

        start_participant_wait_timer(s);
        update_leds_with_participants(s);

        smf_set_state(SMF_CTX(s), &cc_states[S_ACTIVE_CALL]);
    }
}

static void announce_coffee_call_yes()
{
    cc_leds_coffee_call_successful();
    aldrin_buzz_twice();
}

static void announce_coffee_call_no()
{
    cc_leds_coffee_call_failed();
    aldrin_buzz_short_times(3);
}

static void announce_coffee_call_result(struct s_object* s)
{
    const uint8_t count_participants = get_number_of_participants(&s->participant_list);

    if (count_participants > 1)
    {
        LOG_INF("More than one participant, announcing 'yes'");
        announce_coffee_call_yes();
    }
    else
    {
        LOG_INF("Only one participant, announcing 'no'");
        announce_coffee_call_no();
    }
}

static void s_active_call_entry(void* o)
{
    struct s_object* s = o;
    aldrin_buzz_long();
    update_leds_with_participants(s);
}


static void s_active_call_run(void* o)
{
    struct s_object* s = o;

    LOG_INF("active call run with %d", s->events);

    if (s->events & EVENT_LEFT_BTN_PRESS)
    {
        send_own_participation_status(s);
    }

    if (s->events & EVENT_RECV_PART_MESSAGE)
    {
        const int status = set_participant_status(&s->participant_list, s->new_participant);
        if (status < 0)
        {
            LOG_ERR("Failed to add participant to list");
            return;
        }
        if (status == 0)
        {
            LOG_INF("Participant already in list, nothing to do");
        }
        if (status > 0)
        {
            aldrin_buzz_short();
            update_leds_with_participants(s);
        }
    }

    if (s->events & EVENT_WAIT_PARTICIPANT_TIMER_ENDED)
    {
        LOG_INF("state change active call -> idle");
        announce_coffee_call_result(s);
        smf_set_state(SMF_CTX(s), &cc_states[S_IDLE]);
    }
}

static void coffee_call_timer_end(struct k_timer* timer_id)
{
    trigger_event(EVENT_WAIT_PARTICIPANT_TIMER_ENDED, NULL);
}

static const struct smf_state cc_states[] = {
    [S_START] = SMF_CREATE_STATE(NULL, s_start_run, NULL, NULL, NULL),
    [S_IDLE] = SMF_CREATE_STATE(s_idle_entry, s_idle_run, NULL, NULL, NULL),
    [S_ACTIVE_CALL] = SMF_CREATE_STATE(s_active_call_entry, s_active_call_run,
                                       NULL, NULL, NULL),
};

void trigger_event(const uint32_t event_mask, void* event_context)
{
    if (event_mask & EVENT_RECV_PART_MESSAGE)
    {
        const struct participant *participant_info = event_context;
        s_obj.new_participant = *participant_info;
    }

    k_event_post(&s_obj.smf_event, event_mask);
}

static void trigger_participation_response(const uint64_t from, const uint8_t takes_part)
{
    struct participant participant_info = {
        .from = from,
        .takes_part = takes_part,
    };
    trigger_event(EVENT_RECV_PART_MESSAGE, &participant_info);
}

static void recv_cc_bc_message_handler(const cc_broadcast_message_t* message)
{
    const uint64_t sender_id = message->sender_id;
    const char* data = message->payload;

    LOG_INF("Received message from %llx: %s", sender_id, data);

    if (strcmp(data, "1") == 0)
    {
        trigger_participation_response(sender_id, 1);
    }
    else if (strcmp(data, "0") == 0)
    {
        trigger_participation_response(sender_id, 0);
    }
}

void state_machine_init()
{
    init_participant_list(&s_obj.participant_list);

    cc_broadcast_register_client(&s_obj.broadcast_client_id, "cc_coffee_call", recv_cc_bc_message_handler);

    k_timer_init(&s_obj.wait_participants_timer, coffee_call_timer_end, NULL);

    k_event_init(&s_obj.smf_event);

    smf_set_initial(SMF_CTX(&s_obj), &cc_states[S_START]);
    smf_run_state(SMF_CTX(&s_obj));
}

static void process_event(void)
{
    while (true)
    {
        watchdog_feed();
        const uint32_t events = k_event_clear(&s_obj.smf_event, ALL_EVENTS);
        s_obj.events = events;
        LOG_DBG("Got: %d", events);
        if (!events)
        {
            return;
        }
        LOG_DBG("Processing: %d", events);
        const int32_t ret = smf_run_state(SMF_CTX(&s_obj));
        if (ret)
        {
            return;
        }
    }
}

__attribute__((noreturn)) void run_state_machine_forever()
{
    while (1)
    {
        watchdog_feed();
        k_event_wait(&s_obj.smf_event, ALL_EVENTS, true, K_MSEC(500));
        process_event();
    }
}
