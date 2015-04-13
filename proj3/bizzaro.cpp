#include <iostream>
#include <string.h>
#include <dirent.h> // struct dirent is defined in this header
#include <errno.h>
#include <vector>
#include <map>

using namespace std;

int compileDirAndFiles(string loc, map<string, vector<string>> *files)
{
  DIR *dir;
  struct dirent *entry;

  errno = 0;
  dir = opendir(loc.c_str()); // string to c string conversion
      vector<string> tmpfiles;
  if (errno != 0) {
    cout << "opendir error. " << strerror(errno) << endl;
  }
  else if(dir) {
    errno = 0;
    while((entry = readdir(dir)) != NULL) {
      // if it's not . and .. then add it to the files vector
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
      cout << "D_name: " << entry->d_name << " D_type:"<<entry->d_type<<endl;
        tmpfiles.push_back(entry->d_name);
      }
    }
    if(errno != 0) {
      return 0;
      //cout << "readdir error. " << strerror(errno) << endl;
    }
    cout << endl;
  }
  closedir(dir);
  files->insert(make_pair(loc,tmpfiles));
  return 1;
}

int main()
{
  string loc = "..";
  map<string, vector<string>> filesAndDirs;
  compileDirAndFiles(loc, &filesAndDirs);
  for (auto i : filesAndDirs) {
    cout << "dir: " <<i.first <<endl;
    for (auto fi : i.second)
      cout << "\t"<<fi<<endl;
  }
}
