#pragma once
#include <string>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <iomanip> // required for std::setw and std::setfill
#include <sstream>
#include <ctime>

std::ostringstream oss;

class Logger {
public:
    Logger(const std::string& path, bool append = false, bool header=false) 
    {
        if (!append) {
            std::remove(path.c_str()); // remove file if not appending
            logFile.open(path, std::ios::out | std::ios::trunc);
        } else {
            logFile.open(path, std::ios::out | std::ios::app);
        }

        std::time_t now = std::time(nullptr);
        std::string timeStr = std::ctime(&now); 

        iline = 0; 
        if(header) log(timeStr);
	    log("[Logger] -----------------------------------------------------------------------");
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, bool show = true) 
    {
        if (logFile.is_open()) {
            logFile << std::setfill('0') << std::setw(4) << iline << " " << message << std::endl;
        }

        if(show) std::cout <<"[Logger] "<< std::setfill('0') << std::setw(4) << iline << " " << message << std::endl;
        logFile.flush();
        iline++; 
    }

    void error(bool condition, const std::string& message) 
    {
        if(condition)
        {
            log(message); 
            
            std::cerr<< "[ERROR] "<< message << std::endl;
            exit(1); 
        }
    }
/*
    void log(std::ostringstream& oss) 
    {
        std::cout << oss.str() << std::endl;
    }
*/
    private:
        std::ofstream logFile;
        int iline; 
};
/*
	logger->log(
		"[UnityDeformableVolumes] nVertices:" + 
        std::to_string( UnityVerts.size() ) +
	);
*/