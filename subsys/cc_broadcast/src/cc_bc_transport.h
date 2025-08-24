#pragma once

void transport_send_raw(const char *payload);

#ifdef CONFIG_ZTEST
void cc_bc_transport_reset(void);
#endif /* CONFIG_ZTEST */