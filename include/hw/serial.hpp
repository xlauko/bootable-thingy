#pragma once

#include <hw/ioport.hpp>

namespace hw {

    class Serial {
    public:
        static constexpr uint16_t PORTS[] { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };

        template < uint16_t PORT > static Serial& port() {
            static Serial s{ PORT };
            return s;
        }

        char read();
        void write( char c );

		int received();
		int is_transmit_empty();

        Serial( Serial& ) = delete;
        Serial( Serial&& ) = delete;
        Serial& operator=( Serial& ) = delete;
        Serial operator=( Serial&& ) = delete;

        void init() { init( _port ); };

        static void init( uint16_t port );

        static void eot();
    private:
        Serial( int port );

        char newline = '\r';

        int _port{ PORTS[0] };
    };

} // namespace hw
