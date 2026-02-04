#include <stdio.h>
#include "pico/stdlib.h"

#include "web.hpp"
#include "ssi.hpp"
#include "cgi.hpp"

int main() {

    stdio_init_all();

    // Configure webserver, SSI and CGI handler
    web_init();
    printf("Web server initialised\n");
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    while (1) {
        beaconIfRequired();
    };
}
