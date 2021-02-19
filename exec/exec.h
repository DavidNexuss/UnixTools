#pragma once
#include <vector>
#include <string>
#include <signal.h>
using namespace std;

//EXECVP
vector<string> split(const string& line);
void execute(const vector<string>& args);
int execute_pipe(int fd,const vector<string>& args);
void send_notification(const string& title,const string& message);

//SIGNALS
void set_signal_handler(int signal,struct sigaction* sa);
