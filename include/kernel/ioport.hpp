#pragma once

#include <cstdint>

namespace kernel::dev {

    static inline uint8_t inb( int port ) {
        int ret;
        asm volatile ("xorl %eax,%eax");
        asm volatile ("inb %%dx,%%al":"=a" (ret):"d"(port));
        return ret;
    }

    static inline void outb( uint16_t port, uint8_t data ) {
        asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
    }

    static inline uint16_t inw( int port ) {
        int ret;
        asm volatile ("xorl %eax,%eax");
        asm volatile ("inw %%dx,%%ax":"=a" (ret):"d"(port));
        return ret;
    }

} //namespace hw
