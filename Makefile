PROJECT := sound-level-meter
BUILDIR := build
export DEVICE ?= stm32f1

#NDEBUG := true

LIBABOV_ROOT := external/libabov
include $(LIBABOV_ROOT)/projects/sources.mk

SRCS := src/main.c src/console.c src/mic.c src/tick.c src/printf.c \
	$(LIBABOV_SRCS)
INCS := $(LIBABOV_INCS)
DEFS := $(LIBABOV_DEFS)
OBJS := $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS := $(OBJS:.o=.d)

LD_SCRIPT := ports/stm32f1/stm32f1.ld
LDFLAGS += -specs=nano.specs #-specs=nosys.specs
CFLAGS += -Wno-error=float-equal -Wno-error=sign-conversion # due to printf.c

LIBS = -lm

OUTCOM := $(BUILDIR)/$(PROJECT)
OUTELF := $(OUTCOM).elf

all: $(OUTELF)

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
