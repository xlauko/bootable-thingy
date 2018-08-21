#include <stdio.h>


int main();

void _start() {
    int exitval = main();
    // cease( exitval );
}

int main() {
//    printf( "Hello from user land.\n" );
    return 10;
}
