PROJECT := sound-level-meter
BUILDIR := build
export DEVICE ?= stm32f1

#NDEBUG := true

HALMCU_ROOT := external/halmcu
include $(HALMCU_ROOT)/projects/sources.mk

SRCS := src/main.c src/console.c src/mic.c src/tick.c src/printf.c \
	$(HALMCU_SRCS)
INCS := $(HALMCU_INCS)
DEFS := $(HALMCU_DEFS)
OBJS := $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS := $(OBJS:.o=.d)

LD_SCRIPT := ports/stm32f1/stm32f1.ld
LDFLAGS += -specs=nano.specs #-specs=nosys.specs
CFLAGS += -Wno-error=float-equal -Wno-error=sign-conversion # due to printf.c

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
