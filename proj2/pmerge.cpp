/** Implementation of parallel merge sort using hierarchical thread */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <iomanip>
#include "record.h"
#include<mutex>
using namespace std;

// the mutex and condition variable are used for synchronization between threads
pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t con=PTHREAD_COND_INITIALIZER;

// the shared variable that denotes the state of the system
int shared_status = STATUS_INACTIVE;

// name of the attributes
string attributeList[] = {"id", "gender","firstname", "middlename", "lastname", "cityname", "statename", "ssn"};


/*
   function name - compareRecords 
   task - compares employee at index1 with employee at index2 based on attributes in v 
   arguments - 
   index1 - index in the dataset 
   index2 - index in the dataset
   v      - sorting attributes	
   output -
   1 - if employee of index1 is greater than employee of index2 
   0 - otherwise
   */	
int compareRecords( int index1, int index2, vector<int> &v)
{
  /* fill in code here to compare records, make sure to compare the records on attributes in order */
  for (int i : v)
  {
    int ret = 0;
    switch (i)
    {
      case 0:
        ret = dataSet[index1]->eid > dataSet[index2]->eid;
        break;
      case 1:
        ret =  dataSet[index1]->gender > dataSet[index2]->gender;
        break;
      case 2:
        ret =  dataSet[index1]->firstName > dataSet[index2]->firstName;
        break;
      case 3:
        ret =  dataSet[index1]->middleName > dataSet[index2]->middleName;
        break;
      case 4:
        ret =  dataSet[index1]->lastName > dataSet[index2]->lastName;
        break;
      case 5:
        ret =  dataSet[index1]->cityName > dataSet[index2]->cityName;
        break;
      case 6:
        ret =  dataSet[index1]->stateName > dataSet[index2]->stateName;
        break;
      case 7:
        ret =  dataSet[index1]->ssNumber > dataSet[index2]->ssNumber;
        break;
    }
    return ret;
  }
  return 0;
}

/*
   function name - swap
   task - swaps two pointers 
   arguments -
   index1, index2
   output - none
   */
void swap(int index1, int index2)
{
  employeeRecord* t 	= dataSet[index1];
  dataSet[index1]	  	= dataSet[index2];
  dataSet[index2]        	= t;	
}


/*
   function name - getTimeUsed
   task - calculates the time difference between time a and time b
   arguments -
   startTime, endTime - time structure, startTime <= endTime
   output - time difference in microsecs between startTime and endTime
   */
double getTimeUsed(struct timeval &startTime, struct timeval &endTime)
{
  return (endTime.tv_sec - startTime.tv_sec)*1000000 + (endTime.tv_usec - startTime.tv_usec);
}


/*
   function name - writeData
   task - entry function for writerThread, waits for sorting to be finished and then writes the sorted list into the output file
   arguments -
   output file name is passed as argument, return the cpu time used in the thread
   output - none
   */
void* writeData(void * arg)
{
  // wait for the sorting to finish
  pthread_mutex_lock(&mut);
  while(shared_status < STATUS_SORTING_COMPLETED)
    pthread_cond_wait(&con, &mut);
  if(shared_status != STATUS_ABORT)
    shared_status = STATUS_WRITING;
  pthread_mutex_unlock(&mut);

  // exit the thread if abort status is set
  if(shared_status == STATUS_ABORT)
    pthread_exit(NULL);

  // read the argument

  argListforRW* aList = static_cast<argListforRW*> (arg);	
  string fileName = aList->fileName;
  int starting = 0;
  int ending   = dataSet.size()-1;
  retVal* rVal = aList->rVal;
  struct timeval now, later;
  gettimeofday(&now, NULL);

  // open the file to write
  ofstream ofile;
  ofile.open(fileName.c_str());
  if(ofile.is_open()){
    stringstream ss;
    for (int i = starting; i <= ending; i++){
      ss   << setprecision(3) << dataSet[i]->eid         << setw(10) << dataSet[i]->gender 
        << setw(20)        << dataSet[i]->firstName 
        << setw(5)         << dataSet[i]->middleName  << setw(20) << dataSet[i]->lastName  
        << setw(20)        << dataSet[i]->cityName    << setw(20) << dataSet[i]->stateName 	
        << setw(20)        << dataSet[i]->ssNumber  << endl;
    }

    ofile << ss.rdbuf();
    ofile.close();
  }
  else{
    cout << "Failed to open the output file " << fileName << endl;
    pthread_exit(NULL);
  }

  // change the system state to write_completed
  pthread_mutex_lock(&mut);
  shared_status = STATUS_WRITING_COMPLETED;
  pthread_mutex_unlock(&mut);

  gettimeofday(&later, NULL);
  rVal->timeUsed = getTimeUsed(now, later);

  pthread_exit(NULL);
}



/*
   function name - insertion_sort
   task - sorts the employee between start index and end index and returns the frequency of the keyword within that range
   arguments -
   startIndex, endIndex
   keyword - search keyword
   v - sorting attributes
   output - returns number of times the keyword is found in the records between startIndex and endIndex
   */
int insertion_sort(int startIndex, int endIndex, string keyword, vector<int>& v)
{
  // if statrIndex > endIndex return 0
  // otherwise, write code to simultaneously search the keyword and sort the records between startIndex and endIndex using insertion sort	
  int freq = 0;
  if (startIndex <= endIndex)
  {
    int length = endIndex-startIndex; 
    int j,i;

    for (int i = 1; i <= length; i++){
      j = i;
      if (dataSet[j]->stateName == keyword) freq++;
      while (j > 0 && compareRecords(j, j-1, v) == 0) {
        swap(j,j-1);
        j--;
      }
    }
  }
  return freq;		
}

/*
   function name - merge
   task - merges two sorted arrays at index i and at index j into one sorted list, i < j
   arguments -
   i, mid  - start and end index of the first sorted array in the dataset
   bi,  j  - start and end index of the second sorted array in the dataset 
   v - sorting attributes
   output - none
   */ 
void merge(int i, int mid, int bi, int j, vector <int>& v)
{

  int ai = i;
  vector<employeeRecord*> tempList;

  while(ai <= mid && bi <= j) {
    if (compareRecords(ai, bi, v))
      tempList.push_back(dataSet[bi++]);
    else                    
      tempList.push_back(dataSet[ai++]);
  }

  while(ai <= mid) {
    tempList.push_back(dataSet[ai++]);
  }

  while(bi <= j) {
    tempList.push_back(dataSet[bi++]);
  }

  for (ai = 0; ai < (j-i+1) ; ai++){
    dataSet[i+ai] = tempList[ai];
  }

}

/*
   function name - mergesort
   task - entry function for sorterThread and all child threads
   arguments -
   arg - a pointer to the argument structure
   output - none
   */
void * mergesort(void *arg)
{
  // wait for reader to finish and then change the system state, if the abort status is set, exit the thread 
  // if the number of records to be sorted is below the threshold, simply call insertion sort
  // otherwise, 
  // 	we need to create one thread and assign it to sort half the records
  // 	set up thread attributes, create the thread, and pass arguments to the child thread
  //	sort other half by calling insertion sort 
  //	wait for the child thread to finish
  //	merge the results from its child threads

  // update the summary
  // if sorting is finished, notify the writer thread

  // wait for the reading to finish
  pthread_mutex_lock(&mut);
  while(shared_status < STATUS_READING_COMPLETED)
    pthread_cond_wait(&con, &mut);
  if(shared_status != STATUS_ABORT)
    shared_status = STATUS_SORTING;
  pthread_mutex_unlock(&mut);

  // exit the thread if abort status is set
  if(shared_status == STATUS_ABORT)
    pthread_exit(NULL);

  // read the argument

  argList* aList = static_cast<argList*> (arg);	
  int starting = aList->startIndex;
  int ending   = aList->endIndex; 
  int threshold = aList->minSize; 
  int sortlength = ending-starting;
  string kw = aList->keyword; 
  int threadno = aList->threadno; 
  retVal* rVal = aList->rVal;
  struct timeval now, later;
  gettimeofday(&now, NULL);
  rVal->frequency = 1;

  if (sortlength > threshold)
  {
    retVal *rVal2 = new retVal();
    pthread_t sorterThread;
    argList mergeArgList(starting, ending/2, kw, ++threadno, threshold, aList->criteria, rVal2);
    int ret = pthread_create(&sorterThread, NULL, mergesort, (void *)&mergeArgList);
    if(ret){
      cout << " unable to create thread " <<__FUNCTION__ << endl;
    }
    pthread_join(sorterThread, NULL);
    rVal->frequency += insertion_sort(ending/2+1, ending, kw, aList->criteria);  
    merge(starting,ending/2,ending/2+1,ending, aList->criteria);
  }
  rVal->frequency += insertion_sort(starting, ending, kw, aList->criteria);
  // change the system state

  pthread_mutex_lock(&mut);
  shared_status = STATUS_SORTING_COMPLETED;
  pthread_cond_broadcast(&con);
  pthread_mutex_unlock(&mut);

  gettimeofday(&later, NULL);

  // update the summary

  rVal->timeUsed  = getTimeUsed(now, later);
  pthread_exit(NULL);

}


/*
   function name - readData
   task - entry function for readerThread, reads employee records into dataSet and notifies the sorterThread when it is done
   arguments -
   arg - a pointer to the argument structure
   output - none
   */
void * readData(void *arg)
{
  struct timeval now, later;
  gettimeofday(&now, NULL);
  argListforRW* aList = static_cast<argListforRW *>(arg);
  retVal *rVal = aList->rVal;
  string fileName = aList->fileName;

  // change the system state

  pthread_mutex_lock(&mut);
  shared_status = STATUS_READING;
  pthread_mutex_unlock(&mut);

  // read from the file
  ifstream infile;
  infile.open(fileName.c_str());
  string line;
  if(infile.is_open()){
    while(getline(infile, line)){
      stringstream iss(line);
      vector<string> record;
      string str;
      while(getline(iss, str, ',')){
        record.push_back(str);

      }	
      employeeRecord* eR = new employeeRecord(record);
      dataSet.push_back(eR);
    }
    infile.close();
  }
  else{ // set the abort status and notifies all the waiting threads
    cout << "Failed to open the input file " << fileName << endl;
    pthread_mutex_lock(&mut);
    shared_status = STATUS_ABORT;
    pthread_cond_broadcast(&con);
    pthread_mutex_unlock(&mut);
    pthread_exit(NULL);
  }

  // change the system state

  pthread_mutex_lock(&mut);
  shared_status = STATUS_READING_COMPLETED;
  pthread_cond_broadcast(&con);
  pthread_mutex_unlock(&mut);

  gettimeofday(&later, NULL);

  // update the summary

  rVal->frequency = dataSet.size();
  rVal->timeUsed  = getTimeUsed(now, later);
  pthread_exit(NULL);

}
/*
   function name - validateParams
   arguments -
   number of user input and pointer to the user inputs,
   output - 0 if error, 1 otherwise
   */
int validateParams(int argc, char** argv, char* inputFile, char* outputFile, string& keyword, int& minSize, vector<int>& criteria)
{
  if(argc < 6){
    strcpy(inputFile, "./input/test.csv");
    keyword = "MI";
    minSize = 10;
    criteria.push_back(2);
    strcpy(outputFile,"./output/output.csv");

  }else{
    strcpy(inputFile, argv[1]);
    strcpy(outputFile,argv[2]);
    keyword = argv[3];
    minSize = atoi(argv[4]);
    for(int i = 5; i < argc ; i++){
      int attr = atoi(argv[i]);
      if(attr < 0 || attr > 7) return 0;
      criteria.push_back(atoi(argv[i]));
    }
  }

  // validate the parameters
  if(minSize <= 0) return 0;
  if(criteria.size() == 0) return 0;


  // print the inputs
  cout << "Input parameters "	<< endl;
  cout << "	input file name: " << inputFile << endl;
  cout << "	output file name: " << outputFile << endl;
  cout << "	keyword: " << keyword << endl;
  cout << "	minsize: " << minSize << endl;
  cout << "	number of attributes: " << criteria.size() << endl;
  cout << "	attributes: " ;
  for(int i = 0; i < criteria.size(); i++)
    cout << attributeList[criteria[i]] << " ";
  cout << endl;
  return 1;
}


/*
   function name - main
   task - takes the user input, creates threads, and waits for the threads to finish the tasks 
   programname input_file_name output_file_name keyword minsize criteria
   0 - id, 1 - gender, 2 - firstname, 3 - middlename, 4 - lastname, 5 - cityname, 6 - statename, 7 - ssn
   */

int main(int argc, char **argv)
{
  pthread_t readerThread, sorterThread, writerThread;
  int numRecords, minSize, ret;
  char inputFile[30];
  char outputFile[30];
  vector<int> criteria;
  string keyword;
  retVal *rVal1, *rVal2, *rVal3;
  struct timeval now, later;

  int success = validateParams(argc, argv, inputFile, outputFile, keyword, minSize, criteria);
  if(!success)
    return 0;


  // add codes to set the attributes and create the main 3 threads - readerThread, sorterThread, writerThread
  rVal1 = new retVal();
  rVal2 = new retVal();
  rVal3 = new retVal();
  argListforRW readArgList(inputFile, rVal1);
  ret = pthread_create(&readerThread, NULL, readData, (void *)&readArgList);
  if(ret){
    cout << " unable to create thread " <<__FUNCTION__ << endl;
  }

  // add codes to print the results, do not print if there is no record in the database

  // add codes to clean the allocated memory
  pthread_join(readerThread, NULL);
  for (int i = 0; i <= dataSet.size()-1; i++){
    cout << setprecision(3) << dataSet[i]->eid         << setw(10) << dataSet[i]->gender
      << setw(20)        << dataSet[i]->firstName
      << setw(5)         << dataSet[i]->middleName  << setw(20) << dataSet[i]->lastName
      << setw(20)        << dataSet[i]->cityName    << setw(20) << dataSet[i]->stateName
      << setw(20)        << dataSet[i]->ssNumber  << endl;
  }
  cout << endl;
  if (rVal1)
  {
    cout << "Read Data: " <<endl << "\t\tFrequency: "<< rVal1->frequency << endl << "\t\tTime Used: "<<rVal1->timeUsed<<endl;
  }
  argList mergeArgList(0, dataSet.size()-1, keyword, 0, minSize, criteria, rVal2);
  ret = pthread_create(&sorterThread, NULL, mergesort, (void *)&mergeArgList);
  if(ret) {
    cout << " unable to create thread " <<__FUNCTION__ << endl;
  }
  pthread_join(sorterThread, NULL);
  if (rVal2)
  {
    cout << "Sort Data: " << endl << "\t\tFrequency: "<< rVal2->frequency << endl << "\t\tTime Used: "<<rVal2->timeUsed<<endl;
  }

  argListforRW writeArgList(outputFile, rVal3);
  ret = pthread_create(&writerThread, NULL, writeData, (void *)&writeArgList);
  if(ret){
    cout << " unable to create thread " <<__FUNCTION__ << endl;
  }
  pthread_join(writerThread, NULL);
  if (rVal3)
  {
    cout << "Write Data: " <<endl << "\t\tFrequency: "<< rVal3->frequency << endl << "\t\tTime Used: "<<rVal3->timeUsed<<endl;
  }

  pthread_cond_destroy(&con);
  pthread_mutex_destroy(&mut);
  pthread_exit(NULL);
}
