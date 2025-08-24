#include <assert.h>

#include "zephyr/logging/log.h"

#include "participant_list.h"

LOG_MODULE_REGISTER(cc_coffeecall_subsys_participant_list);


struct participant_list_node
{
    sys_dnode_t node;
    struct participant p;
};

void init_participant_list(participant_list_t* list)
{
    k_mutex_init(&list->participants_mutex);
    sys_dlist_init(&list->p_list);
}

uint8_t get_number_of_participants(participant_list_t* p)
{
    uint8_t count_participants = 0;

    k_mutex_lock(&p->participants_mutex, K_FOREVER);
    struct participant_list_node *cnode, *s_cnode;
    SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&p->p_list, cnode, s_cnode, node)
    {
        if (cnode->p.takes_part)
        {
            count_participants++;
        }
    }
    k_mutex_unlock(&p->participants_mutex);

    return count_participants;
}

void clear_participant_list(participant_list_t* list)
{
    k_mutex_lock(&list->participants_mutex, K_FOREVER);
    struct participant_list_node *cnode, *s_cnode;
    SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&list->p_list, cnode, s_cnode, node)
    {
        sys_dlist_remove(&cnode->node);
        k_free(cnode);
    }
    assert(sys_dlist_is_empty(&list->p_list));
    k_mutex_unlock(&list->participants_mutex);
}

int set_participant_status(participant_list_t* list, struct participant participant)
{
    int ret = 0;
    k_mutex_lock(&list->participants_mutex, K_FOREVER);

    struct participant_list_node *cnode, *s_cnode;

    SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&list->p_list, cnode, s_cnode, node)
    {
        if (cnode->p.from == participant.from)
        {
            if (participant.takes_part != cnode->p.takes_part)
            {
                cnode->p.takes_part = participant.takes_part;
                ret = 1;
            }
            goto end;
        }
    }

    struct participant_list_node* pln = k_malloc(sizeof(struct participant_list_node));
    if (pln != NULL)
    {
        sys_dnode_init(&pln->node);
        pln->p = participant;

        sys_dlist_append(&list->p_list, &pln->node);
        ret = 1;
    }
    else
    {
        LOG_ERR("Can't add user to list, no dynamic memory left!");
        ret = -ENOMEM;
        goto end;
    }

end:
    k_mutex_unlock(&list->participants_mutex);
    return ret;
}
