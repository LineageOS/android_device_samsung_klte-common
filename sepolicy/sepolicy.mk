# Board specific SELinux policy variable definitions
BOARD_SEPOLICY_DIRS += \
    device/samsung/klte-common/sepolicy/common \

BOARD_PLAT_PUBLIC_SEPOLICY_DIR += \
    device/samsung/klte-common/sepolicy/public

BOARD_PLAT_PRIVATE_SEPOLICY_DIR += \
    device/samsung/klte-common/sepolicy/private

-include device/qcom/sepolicy/sepolicy-legacy.mk
-include device/qcom/sepolicy/sepolicy.mk
