#include <kernel/user.hpp>

#include <stdio.h>

namespace kernel {
    namespace user {

        int executable::start() const {
            puts( "\nStarting user program.\n" );
            // create user stack

            // map pages

            // jump to user space

            // TODO return value
            return 0;
        }

    } // namespace user
} // namespace kernel
