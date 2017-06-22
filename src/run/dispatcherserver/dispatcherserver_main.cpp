#include "dispatcher/server/dpserver.h"
#include "vzlogging/logging/vzlogging.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
  ShowVzLoggingAlways();
  dp::DpServer server;
  server.StartDpServer();
  server.StopDpServer();
  return 0;
}