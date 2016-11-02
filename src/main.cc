#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <glog/logging.h>

#include "ps_server.h"
#include "ps_options.h"

#include "env.h"

PSServer* ps_server;

void Usage();
void ParseArgs(int argc, char* argv[], PSOptions& options);

static void GlogInit(const PSOptions& options) {
  if (!slash::FileExists(options.log_path)) {
    slash::CreatePath(options.log_path); 
  }

  FLAGS_alsologtostderr = true;

  FLAGS_log_dir = options.log_path;
  FLAGS_minloglevel = 0;
  FLAGS_max_log_size = 1800;
  // TODO rm
  FLAGS_logbufsecs = 0;

  ::google::InitGoogleLogging("ps");
}

static void IntSigHandle(const int sig) {
  LOG(INFO) << "Catch Signal " << sig << ", cleanup...";
  //ps_data_server->server_mutex_.Unlock();
  ps_server->Exit();
}

static void PSSignalSetup() {
  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, &IntSigHandle);
  signal(SIGQUIT, &IntSigHandle);
  signal(SIGTERM, &IntSigHandle);
}

int main(int argc, char** argv) {
  PSOptions options;

  ParseArgs(argc, argv, options);

  //options.Dump();
  GlogInit(options);
  PSSignalSetup();

  ps_server = new PSServer(options);

  ps_server->Start();

  //printf ("Exit\n");
  delete ps_server;

  ::google::ShutdownGoogleLogging();
  return 0;
}

void Usage() {
  printf ("Usage:\n"
          "  ./pgstall --local_ip local_ip --local_port local_port --worker_num 8 --data_path path --log_path path"
          " --file_size 40000000 --load_interval 60 --flush_interval 1800\n"
          "   these Options are optional.\n");
}

void ParseArgs(int argc, char* argv[], PSOptions& options) {
  if (argc < 2) {
    Usage();
    exit(-1);
  }

  const struct option long_options[] = {
    {"local_ip", required_argument, NULL, 'n'},
    {"local_port", required_argument, NULL, 'p'},
    {"worker_num", required_argument, NULL, 'w'},
    {"file_size", required_argument, NULL, 'f'},
    {"load_interval", required_argument, NULL, 'i'},
    {"flush_interval", required_argument, NULL, 's'},
    {"data_path", required_argument, NULL, 'd'},
    {"log_path", required_argument, NULL, 'l'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}, };

  const char* short_options = "n:p:w:f:i:s:d:l:h";

  int ch, longindex;
  while ((ch = getopt_long(argc, argv, short_options, long_options,
                           &longindex)) >= 0) {
    switch (ch) {
      case 'n':
        options.local_ip = optarg;
        break;
      case 'p':
        options.local_port = atoi(optarg);
        break;
      case 'w':
        options.worker_num = atoi(optarg);
        break;
      case 'f':
        options.file_size = atoi(optarg);
        break;
      case 'i':
        options.load_interval = atoi(optarg);
        break;
      case 's':
        options.flush_interval = atoi(optarg);
        break;
      case 'd':
        options.data_path = optarg;
        break;
      case 'l':
        options.log_path = optarg;
        break;
      case 'h':
        Usage();
        exit(0);
      default:
        break;
    }
  }
}