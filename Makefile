PROJECT := sound-level-meter
BUILDIR := build
export DEVICE ?= stm32f1

#NDEBUG := true

HALMCU_ROOT := external/halmcu
include $(HALMCU_ROOT)/projects/sources.mk

DSP_PATH := $(HALMCU_ROOT)/arch/arm/include/CMSIS_5/CMSIS/DSP
DSP_SRC_DIR := $(DSP_PATH)/Source
DSP_SRCS := $(shell find $(DSP_SRC_DIR) -type f -regex ".*\.c")
DSP_INCS := \
	$(DSP_PATH)/Include \
	$(DSP_PATH)/PrivateInclude
DSP_FFT_SRCS := \
	$(DSP_PATH)/Source/CommonTables/arm_const_structs.c \
	$(DSP_PATH)/Source/CommonTables/arm_common_tables.c \
	$(DSP_PATH)/Source/BasicMathFunctions/arm_shift_q15.c \
	$(DSP_PATH)/Source/BasicMathFunctions/arm_abs_q15.c \
	$(DSP_PATH)/Source/TransformFunctions/arm_rfft_init_q15.c \
	$(DSP_PATH)/Source/TransformFunctions/arm_rfft_q15.c \
	$(DSP_PATH)/Source/TransformFunctions/arm_cfft_q15.c \
	$(DSP_PATH)/Source/TransformFunctions/arm_cfft_radix4_q15.c \
	$(DSP_PATH)/Source/TransformFunctions/arm_bitreversal2.c
DSP_DEFS := \
	ARM_DSP_CONFIG_TABLES \
	ARM_FFT_ALLOW_TABLES \
	ARM_TABLE_REALCOEF_Q15 \
	ARM_TABLE_TWIDDLECOEF_Q15_128 \
	ARM_TABLE_BITREVIDX_FXT_128 \

SRCS := src/main.c src/console.c src/mic.c src/tick.c src/printf.c src/fft.c \
	$(HALMCU_SRCS) $(DSP_FFT_SRCS)
INCS := $(HALMCU_INCS) $(DSP_INCS)
DEFS := $(HALMCU_DEFS) $(DSP_DEFS)
OBJS := $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS := $(OBJS:.o=.d)

LD_SCRIPT := ports/stm32f1/stm32f1.ld
LDFLAGS += -specs=nano.specs #-specs=nosys.specs
CFLAGS += -Wno-error=float-equal -Wno-error=sign-conversion # due to printf.c
STACK_LIMIT := 256 # for fft
CFLAGS += \
	  -Wno-error=undef \
	  -Wno-error=cast-qual \
	  -Wno-error=conversion \
	  -Wno-error=missing-prototypes \
	  -Wno-error=switch-default \
	  -Wno-error=unused-parameter \

LIBS = -lm

OUTCOM := $(BUILDIR)/$(PROJECT)
OUTELF := $(OUTCOM).elf
OUTHEX := $(OUTCOM).hex

all: $(OUTHEX)

%.hex : $(OUTELF)
	$(info generating  $@)
	$(OC) -O ihex $< $@

%.elf : $(OBJS) $(LD_SCRIPT)
	$(CC) -o $@ \
		-Wl,-Map,$(OUTCOM).map \
		$(OBJS) \
		$(addprefix -T, $(LD_SCRIPT)) \
		$(CFLAGS) \
		$(LDFLAGS) \
		$(LIBS) \

	$(SZ) -t --common $(sort $(OBJS))

$(BUILDIR)/%.o: %.c $(MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(CC) -o $@ $< -c -MMD \
		$(addprefix -D, $(DEFS)) \
		$(addprefix -I, $(INCS)) \
		$(CFLAGS)

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
-include $(DEPS)
endif
endif

.PHONY: clean
clean:
	$(Q)rm -fr $(BUILDIR)
.PHONY: flash
flash: $(OUTHEX)
	$(Q)pyocd $@ -t $(DEVICE)03c8 $<
