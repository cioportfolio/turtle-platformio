#include "web.hpp"
#include <string.h>

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"status", "wifi", "ssid"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen, u16_t cPart, u16_t *nPart) {
  size_t printed;
  printf("SSI content limit: %d\n", iInsertLen);
  switch (iIndex) {
  case 0: // status
    {
      printed = snprintf(pcInsert, iInsertLen, "Working");
    }
    break;
  case 1: // wifi
    {
//      if (ssid[0]=='\0') {
        switch (cPart) {
          case 0:
                  printed = snprintf(pcInsert, iInsertLen,"<p>Connected via %s</p>\
          <form method=\"GET\" enctype=\"text/plain\" action=\"/wifi.cgi\">", ssid[0]=='\0'?"Turtle Access Point":ssid);
          *nPart=1;
          break;
          case 1:
                  printed = snprintf(pcInsert, iInsertLen,"<label for=\"ssid\">Wifi name:</label>\
          <input type=\"text\" id=\"ssid\" name=\"ssid\" size=\"32\" maxlength=\"32\">");
          *nPart=2;
          break;
          case 2:
                  printed = snprintf(pcInsert, iInsertLen,"<label for=\"pwd\">Wifi key:</label>\
          <input type=\"text\" id=\"pwd\" name=\"pwd\" size=\"32\" maxlength=\"32\">");
          *nPart=3;
          break;
          case 3:
                  printed = snprintf(pcInsert, iInsertLen,"<input type=\"submit\" value=\"Connect\">\
          <input type=\"reset\" value=\"Clear\">\
          </form>");
          break;
        }
/*      } else {
        printed=snprintf(pcInsert, iInsertLen, "<p>Connected to: %s</p>", ssid);
      } */
    }
    break;
  case 2: //ssid
    printed=snprintf(pcInsert, iInsertLen, ssid);
    break;
  default:
    printed = 0;
    break;
  }

  return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init() {
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}
