#ifndef LOAD_FILE_HEADER_FILE_H 
#define LOAD_FILE_HEADER_FILE_H
#include <ctype.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <assert.h>
#include <fstream>

#include <sstream>      // std::stringstream

template<class T>
bool LoadFile(std::string name, std::vector< std::vector<T> >& data, bool show) 
{
  int i = 0; 
  std::ifstream file(name.c_str());
  if (!file.is_open()) {
    std::cerr << "[LoadFile] Error: Cannot open file '" << name << "'\n";
    return false;
  }

  std::string line;
  while(std::getline(file, line))
  {
    std::vector<T> row;
    std::stringstream iss(line);

    std::string val;
    while(std::getline(iss, val, ' '))
    {
      float num_float = std::stof(val); 
      row.push_back(num_float);
    }

    if(show)
    {
      std::cout << i++ << ") ";
      for(auto r : row) std::cout << r << " ";
      std::cout << " \n";
    } 

    data.push_back(row);
  }

  std::cout << "[LoadFile] file:'" << name << "' nlines:" << data.size() << " \n";
  return true; 
}


#endif