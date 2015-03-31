#include <signal.h>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#include "proj1.h"
using namespace std;

/*
   function name: int_handler - handles  interrupt signal and take necessary action
input: signal number
output: none
*/

void int_handler(int signo)
{
  if(signo == SIGINT)
  {
    alarm(0); // Stop SIGALRM
    string choice;

    cout << "\nSelect one of the following: \nQ to Quit \nI to change free memory checking Interval \nT to change the Threshold  \nK to kill the receiver process\nC to Continue \nWaiting for user input:\n";
    cin >> choice;

    // Ask for inputs until the user enters a valid response
    while(true)
    {
      if(choice[0] == 'Q' || choice[0] == 'q')
      {
        // handle quit case
        exit(EXIT_FAILURE);
      }
      else if(choice[0] == 'I' || choice[0] == 'i')
      {
        // get new interval and continue
        cout << "Enter new interval (in seconds): ";
        int newInterval = 2;
        cin >> newInterval;
        if (newInterval >= 0)
        {
          interval = newInterval;
          cout << "Interval changed to: " << interval << " seconds."<<endl;
        }
        break;
      }
      else if(choice[0] == 'T' || choice[0] == 't')
      {
        // get new threshold and continue
        cout << "Enter new threshold (in kb): ";
        int newThreshold = 100;
        cin >> newThreshold;
        if (newThreshold >= 0)
        {
          threshold = newThreshold;
          cout << "Threshold changed to: " << threshold << " kb."<<endl;
        }
        break;
      }
      else if(choice[0] == 'K' || choice[0] == 'k')
      {
        // find the receiver program (if it is running) and terminate it.
        kill_receiver();
        break;

      }
      else if(choice[0] == 'C' || choice[0] == 'c')
      {
        // continue sampling of free memory
        break;
      }
      else
      {
        cout << "\nSelect one of the following: \nQ to Quit \nI to change free memory checking Interval \nT to change the Threshold  \nK to kill the receiver program\nC to Continue \nWaiting for user input:\n";
        cin >> choice;
      }		
    }

    // if we break then we want to continue with alarm
    cout << "Program Continues"<<endl;
    alarm(interval);
  }
}


/*
   function name: alarm_handler - handles alarm signal
input: signal no
output: none
*/	
void alarm_handler(int signo)
{
  // report free memory status with check_free_memory() and restart alarm
  if (signo == SIGALRM)
  {
    cout << "Checking Free Memory every " << interval << " seconds"<<endl;
    check_free_memory();
    alarm(interval);
  }
}

/* 
   main function
   */
int main(int argc, char* argv[])
{

  cout << "Monitoring Free Memory." << endl;
  cout << " Default frequency is " << interval << " seconds." << endl;
  cout << " Default threshold is " << threshold << " kB." << endl << endl;


  // register the signal handlers (SIGINT, SIGALARM)
  // register int handler
  if (signal(SIGINT, int_handler) == SIG_ERR)
  {
    cout << "Failed to register int handler." <<endl;
    exit(EXIT_FAILURE);
  }
  //register alarm handler
  if (signal(SIGALRM, alarm_handler) == SIG_ERR)
  {
    cout << "Failed to register alarm handler." <<endl;
    exit(EXIT_FAILURE);
  }

  // set the first alarm 
  alarm(interval);
  while(true)
  {
    sleep(10);
  }
}
