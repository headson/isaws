#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <curl/curl.h>

#include "vzbase/helper/stdafx.h"
#include "vzconn/base/clibevent.h"

#include "ccurleasy.h"
#include "ccurlmulti.h"

int main(int argc, char **argv) {
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#endif

  vzconn::EVT_LOOP c_evt_loop;
  c_evt_loop.Start();


  CCurlMulti g(&c_evt_loop);
  if (g.Start() == false) {
    return -1;
  }

  CCurlEasy *p_curl_easy = new CCurlEasy();
  if (p_curl_easy) {
    p_curl_easy->Start(&g,
                       "http://localhost:8080/vz/index.jsp",
                       "json=helloworlds.");
  }

  /* we don't call any curl_multi_socket*() function yet as we have no handles
     added! */
  c_evt_loop.RunLoop(10000);
  /* this, of course, won't get called since only way to stop this program is
     via ctrl-C, but it is here to show how cleanup /would/ be done. */

  g.c_evt_timer_.Stop();
  //g.p_evt_loop_->Stop();
  c_evt_loop.Stop();
  curl_multi_cleanup(g.p_curl_multi_);
  return 0;
}
