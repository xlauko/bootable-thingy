#pragma once

#include <kernel/utils.hpp>
#include <multiboot2.h>
#include <stdio.h>

namespace multiboot {

    Status check( unsigned long magic, unsigned long addr );

    enum class information_type : uint32_t {
        end              = 0,
        command_line     = 1,
        loader_name      = 2,
        modules          = 3,
        basic_memory     = 4,
        boot_device      = 5,
        memory_map       = 6,
        vbe              = 7,
        framebuffer      = 8,
        elf_symbols      = 9,
        apm              = 10,
        efi32            = 11,
        efi64            = 12,
        smbios           = 13,
        acpi_old         = 14,
        acpi_new         = 15,
        network          = 16,
    };

    namespace {
        struct information_item {
            information_type type;
            uint32_t size;
        };

        struct information_list {};
    }

    information_item * next( information_item * item );

    struct memory_info {
        size_t lower;
        size_t upper;
    };

    struct info {
        info( unsigned long addr )
            : list( reinterpret_cast< information_list * >( addr ) )
        {};

        information_item * begin() const;
        information_item * end() const;

        memory_info mem() const;

        void print() const;

        template< typename Fn >
        void yield( Fn fn ) const {
            for ( auto item = begin(); item->type != information_type::end; item = next( item ) )
                fn( item );
        }

        template< typename Fn >
        void yield( information_type type, Fn fn ) const {
            yield( [&] ( const auto & item ) {
                if ( item->type == type )
                    fn( item );
            } );
        }


    private:
        information_list * list;
    };


    struct end_information {
        information_type type;
        uint32_t size;
    };

    struct command_line_information {
        information_type type;
        uint32_t size;

        char command [];
    };

    struct loader_name_information {
        information_type type;
        uint32_t size;

        char name [];
    };

    struct modules_information {
        information_type type;
        uint32_t size;

        uint32_t start;
        uint32_t end;
        char command [];
    };

    struct basic_memory_information {
        information_type type;
        uint32_t size;

        uint32_t lower;
        uint32_t upper;
    };

    struct memory_map_information {
        information_type type;
        uint32_t size;

        uint32_t entry_size;
        uint32_t entry_version;
    };

    struct layout {
        uintptr_t mem_start;
        uintptr_t mem_end;

        static void init();
    };
}
