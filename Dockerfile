ARG CI_IMAGE_VERSION=v0.28.4

FROM ghcr.io/zephyrproject-rtos/ci:${CI_IMAGE_VERSION} AS builder

RUN <<EOF
    cd /opt/toolchains/zephyr-sdk-*
    # remove build chains for unused socs
    rm -rf aarch64-* arc64-* microblazeel-* mips-* nios2-* riscv64-* rx-* sparc-* xtensa-*
    # and also remove unusued xilinx pokysdk
    rm -rf sysroots/x86_64-pokysdk-linux/usr/xilinx
EOF

FROM ghcr.io/zephyrproject-rtos/ci-base:${CI_IMAGE_VERSION}

ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr

COPY --from=builder /opt/toolchains/ /opt/toolchains/

RUN <<EOF
    cd /opt/toolchains/zephyr-sdk-*
    ./setup.sh -c
EOF
