.PHONY: kernel clean

CC  = clang
CXX = clang++

PLATFORM_KERNEL = kernel

KERNEL_INCLUDE_DIR = $(INCLUDE_DIR) platform/$(PLATFORM_KERNEL)/includes platform/$(PLATFORM_KERNEL)/internals
KERNEL_SRC = $(SRC) $(wildcard platform/$(PLATFORM_KERNEL)/functions/*/*.c)
OBJ = $(KERNEL_SRC:.c=.o)
KERNEL_OBJ = $(addprefix build/kernel/, $(OBJ))
KERNEL_INCLUDES = $(foreach i, $(KERNEL_INCLUDE_DIR), -I$i)

INCLUDE_DIR = includes internals opt/nothread
SRC = $(wildcard functions/*/*.c) $(wildcard opt/nothread/*.c)

FLAGS = -mno-sse -m32 -ffreestanding -nostdlib -static -fno-stack-protector -fno-PIC -fno-pie -D_PDCLIB_BUILD $(INCLUDES) -g

CFLAGS += $(FLAGS) -std=c11
CXXFLAGS += $(FLAGS) -std=c++17 -fno-rtti -fno-exceptions

kernel: INCLUDES = $(KERNEL_INCLUDES)
kernel: kernel_pdclib.a

kernel_pdclib.a: $(KERNEL_OBJ)
	ar rcs $@ $(KERNEL_OBJ)

build/$(PLATFORM_KERNEL)/%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<
build/$(PLATFORM_KERNEL)/%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<
build/$(PLATFORM_KERNEL)/%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

clean:
	rm -rf build
	rm -f $(KERNEL_OBJ)

BUILD_DIRS = $(dir $(KERNEL_OBJ))
$(shell mkdir -p $(BUILD_DIRS))

