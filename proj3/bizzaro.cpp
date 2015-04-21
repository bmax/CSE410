#include <iostream>
#include <fstream>
#include <string.h>
#include <dirent.h> // struct dirent is defined in this header
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

int compileDirAndFiles(string loc, map<string, vector<string>> *files)
{
  DIR *dir;
  struct dirent *entry;
  struct stat buf;


  errno = 0;
  dir = opendir(loc.c_str()); // string to c string conversion
  vector<string> tmpfiles;
  if (errno != 0) {
    cout << "opendir error. " << strerror(errno) << endl;
    return 0;
  }
  else if(dir) {
    errno = 0;
    while((entry = readdir(dir)) != NULL) {
      /* ignore .. and . */
      if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) {
        continue;
      }
      // create new file/dir name
      string newLocation = loc+"/"+entry->d_name;
      if (stat(newLocation.c_str(), &buf) == 0)
      {
        if (S_ISDIR(buf.st_mode))
        {
          // recurse if it's a directoy
          compileDirAndFiles(newLocation, files);
        }
        else if (S_ISREG(buf.st_mode))
        {
          // just a file, add it to list
          tmpfiles.push_back(entry->d_name);
        }
      }
      // error, output it!
      else {
        perror(entry->d_name);
        continue;
      }
    }
    // error, output it!
    if(errno != 0) {
      perror(entry->d_name);
      return 0;
      //cout << "readdir error. " << strerror(errno) << endl;
    }
    cout << endl;
  }
  closedir(dir);
  files->insert(make_pair(loc,tmpfiles));
  return 1;
}
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

int createFileAndReverse(string file, string reverseddir, string normaldir)
{
  // open file
  ifstream infile;
  infile.open(normaldir+"/"+file);
  // grab content from file and reverse it
  string content((istreambuf_iterator<char>(infile)), (istreambuf_iterator<char>()));
  reverse(content.begin(),content.end());

  // open output file (make sure to reverse file name of course!)
  ofstream outfile;
  string revFile = file;
  reverse(revFile.begin(),revFile.end());
  outfile.open(reverseddir+revFile);
  // write content that we reversed already
  outfile << content;
  return 1;
}

int copyAndReverse(map<string, vector<string>> fileSystemData)
{
  map<string, vector<string>> recreatedAsReverse;
  for (auto i : fileSystemData)
  {
    vector<string> seperatedirs = split(i.first, '/');
    // reverse the directory names
    string newDir="";
    for (auto sep : seperatedirs)
    {
      string sepp = sep;
      reverse(sepp.begin(),sepp.end());
      newDir += sepp +"/";
    }
    recreatedAsReverse.insert(make_pair(newDir, i.second));
  }
  // stat for checking if dir exists
  struct stat buf;

  for(auto r_iterator = recreatedAsReverse.begin(), n_iterator = fileSystemData.begin(); r_iterator != recreatedAsReverse.end(); r_iterator++, n_iterator++)
  {
    // create directories
    auto reversedDir = r_iterator->first.c_str();
    auto realDir = n_iterator->first.c_str();
    if (stat(reversedDir, &buf) == -1)
    {
      //since it == -1 that means this doesn't exsist lets create it!
      mkdir(reversedDir, 0700);
    }
    else
    {
      cout << "Directory " << reversedDir << " already exists, bye!"<<endl;
      return 0;
    }
    for (auto file : r_iterator->second)
    {
      // read file and create it into new directory
      createFileAndReverse(file, reversedDir, realDir);
    }
  }
  return 1;
}

int main(int argc, char* argv[])
{
  if ( argc != 2 ) // argc should be 2 for correct execution
    // We print argv[0] assuming it is the program name
    cout<<"usage: "<< argv[0] <<" <directory_name>\n";
  else {
    string loc = argv[1];
    map<string, vector<string>> filesAndDirs;
    // call the compile function to get directories and files in one spot
    if (compileDirAndFiles(loc, &filesAndDirs)) {
      // output to make sure how map is correct
      for (auto i : filesAndDirs) {
        cout << "dir: " <<i.first <<endl;
        for (auto fi : i.second)
          cout << "\t"<<fi<<endl;
      }

      // call new function to copy and reverse everything
      if (copyAndReverse(filesAndDirs))
        cout << "Successful"<<endl;
      else
        cout << "Failure"<<endl;
    }
  }

}
