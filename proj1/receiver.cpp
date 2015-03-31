#include <signal.h>
#include <unistd.h>
#include <iostream>
//#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
//#include <sys/types.h>

using namespace std;

void usr_handler(int signo)
{
    if(signo == SIGUSR1)
    {
        cout << "WARNING: FREE MEMORY DECREASE HAS EXCEEDED THRESHOLD VALUE!" << endl;
    }
}

int main(int argc, char* argv[])
{
    
    if(signal(SIGUSR1, usr_handler) == SIG_ERR)
    {
        cout << "Failed to register SIGUSR1 to usr_handler. Program Exiting."
             << endl;
        exit(EXIT_FAILURE);
    } 

//Waiting for signal from other process
    while(true)
    {
        sleep(10);
    }
}
