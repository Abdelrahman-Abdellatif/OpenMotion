SUMMARY = "OpenMotion embedded Linux image for STM32MP257"

# start from the same base as core-image-minimal
require recipes-core/images/core-image-minimal.bb

# inherit ST's partition splitting class
# this is what populates bootfs with kernel + dtb
inherit st-partitions-image

# tell ST's class which extlinux boot config to use
EXTLINUX_CONSOLE = "ttySTM0,115200"
