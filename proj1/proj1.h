#include <vector>
#include <iostream>
#include <string>

using namespace std;

// Global variable for the sampling interval, threshold value
int interval = 2;
int threshold = 100; //kB
int previousFreeMem = 0;

/* 
   function name: run_command - executes a command and returns the output of the command
input: cmd - a string containing the unix command to be executed
output: a string containing the output of the command
*/ 
string run_command(string cmd)
{
  FILE *in;       // a FILE pointer for storing the cmd execution outputs
  char buff[512]; // buffer for reading the results from FILE pointer
  string results; // a string to hold all results

  // execute command and store them in the file
  if(!(in = popen(cmd.c_str(), "r"))){
    cerr << "Unable to execute " << cmd << "  command."
      << " Program terminates." << endl;
    exit(EXIT_FAILURE);
  }

  // read the execution results from the file pointer and store them in the result string
  while(fgets(buff, sizeof(buff), in) != NULL){
    results += buff; // here you have the output of your script in buff
  }
  pclose(in); // close the file pointer


  // uncomment next line if you want to have a look at the results
  // cout << results << endl;

  return results;
}

//	function name: find_pid  - parse the multiline output of the 'ps' command, finds the desired program and returns its PID
//  input: name of program
//	output: PID of the program, or -1 if it is not found
pid_t find_pid(string pname)
{
  unsigned int pos = 0; // For splitting the results by line
  int line_number = 0;  // For counting line number
  string pid_str, name_str;
  pid_t tmp_pid = -1;

  string results = run_command("ps uf");

  while(pos != string::npos && results != "")
  {
    // split the results by line
    pos = results.find("\n");
    string a_line = results.substr(0, pos);

    istringstream lineStr(a_line);
    if (line_number != 0)
    {
      lineStr >> pid_str;
      lineStr >> tmp_pid;
      name_str = lineStr.str();
      if(name_str.find(pname) != string::npos)
      {
        return(tmp_pid);
      }	
    }
    line_number += 1;
    results = results.substr(pos+1, results.length());
  }
  return(-1);
}





/*
   function name: check_free_memory() - executes a command to check the memory status,
   parse and process the output using run_command and parse_memfree()
input: none
output: none
*/
void check_free_memory()
{
  string cmd = "grep MemFree /proc/meminfo";
  string results;
  string freeMemString;
  int currentFreeMem;
  int diff;
  pid_t receiver_pid = -1;

  // get results from proc file system
  results = run_command(cmd);

  // process string, put integer value in currentFreeMem
  istringstream resultMem(results);
  resultMem >> freeMemString;
  resultMem >> currentFreeMem;

  cout << "Free Memory: " << currentFreeMem << " kB" ;

  diff = currentFreeMem - previousFreeMem;

  // set the previous value to the current, for next check
  previousFreeMem = currentFreeMem;

  if (diff == 0)
  {
    cout << " (No Change)" << endl;
  }
  else if (diff > 0)
  {
    cout << " (Increase of " << diff << " kB)" << endl;
  }
  else 
  {
    cout << " (Decrease of " << -diff << " kB)" << endl;

    // check if we exceeded the threshold; if so, contact receiver
    if (-diff > threshold)
    {
      // need to find and send signal to the receiver 
      if ((receiver_pid = find_pid("receiver")) > 0)
      {
        kill(receiver_pid, SIGUSR1);
      }
      else
      {
        cout << "Threshold exceeded but cannot signal receiver; not running." << endl;
      }
    }      
  }
}





/*
   function name: kill_receiver() - kill the receiver process if it exists
input: none
output: none
*/
void kill_receiver()
{
  pid_t receiver_pid = -1;
  // if the receiver process exists, kill it with SIGKILL and inform the user. 
  if ((receiver_pid = find_pid("receiver")) > 0)
  {
    cout << "killing pid: " << receiver_pid << endl;
    kill(receiver_pid, SIGKILL);
  }
  // if the receiver process does not exist, inform the user that the kill failed.
  else {
    cout << "Can't find receiver pid"<<endl;  
  }


}



