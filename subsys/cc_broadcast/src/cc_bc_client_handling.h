#pragma once

void dispatch_cc_bc_message_handler(char* json_message);

#ifdef CONFIG_ZTEST
void cc_bc_client_reset(void);
#endif /* CONFIG_ZTEST */