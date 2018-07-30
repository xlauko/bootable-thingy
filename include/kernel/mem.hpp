#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define ALIGNED __attribute__((aligned(4096)))

namespace kernel::mem {

    namespace paging {

        struct page_entry {
            union alignas( 4 ) {
                uint32_t raw;
                struct {
					uint32_t present    : 1;   // Page present in memory
					uint32_t rw         : 1;   // Read-only if clear, readwrite if set
					uint32_t user       : 1;   // Supervisor level only if clear
					uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
					uint32_t dirty      : 1;   // Has the page been written to since last refresh?
					uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
					uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
				};
            };
        };

		struct page_table {
			static constexpr size_t size = 1024;
			page_entry pages[ size ];

			static page_table * create();
			static page_table * empty();
		};

		using address_t = uint32_t;

		struct page_directory {
			static constexpr size_t size = 1024;
			page_table * tables[ size ];

			static page_directory * create();
		};
    } // namespace paging

    namespace frame {
        using paging::page_entry;

        void alloc( page_entry * page, bool kernel, bool writable );
    } // namespace frame

    void init( size_t mem_size );

} // namespace kernel::mem
