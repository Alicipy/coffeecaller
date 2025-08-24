#pragma once

#include <stdint.h>

#include <zephyr/sys/dlist.h>
#include <zephyr/sys/mutex.h>

struct participant {
    uint64_t from;
    uint8_t takes_part;
};
typedef struct {
    struct k_mutex participants_mutex;
    sys_dlist_t p_list;
} participant_list_t;

void init_participant_list(participant_list_t *list);
void clear_participant_list(participant_list_t *list);
uint8_t get_number_of_participants(participant_list_t *list);
int set_participant_status(participant_list_t *list, struct participant p);