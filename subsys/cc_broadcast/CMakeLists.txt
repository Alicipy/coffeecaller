if(CONFIG_CC_BROADCAST_SUBSYS)
	zephyr_library()
	zephyr_library_sources(
            src/cc_bc_client_handling.c
			src/cc_bc_serialization.c
	)

	zephyr_library_sources_ifdef(CONFIG_TRANSPORT_OPENTHREAD src/cc_bc_transport_openthread.c)
	zephyr_library_sources_ifdef(CONFIG_TRANSPORT_LOCALQUEUE src/cc_bc_transport_localqueue.c)

	zephyr_library_sources_ifdef(CONFIG_ZTEST src/cc_bc_reset.c)

	zephyr_include_directories(include)
endif()
