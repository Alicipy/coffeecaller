#include <cc_broadcast/cc_broadcast.h>
#include "cc_bc_client_handling.h"
#include "cc_bc_transport.h"

#ifdef CONFIG_ZTEST
void cc_broadcast_reset(void)
{
    cc_bc_client_reset();
    cc_bc_transport_reset();
}
#endif