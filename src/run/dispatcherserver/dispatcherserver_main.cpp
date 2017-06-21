#include "dispatcher/server/dpserver.h"

#include "stdafx.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);

  dp::DpServer server;
  server.StartDpServer();
  server.StopDpServer();
  return 0;
}