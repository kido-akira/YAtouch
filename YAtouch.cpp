#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
using namespace std;

#include <windows.h>

void ShowHelp(char* program, int status) {
    cerr << "Usage: " << program << " [OPTION]... FILE..." << endl;
    cerr << "" << endl;
    cerr << "Update the access and modification times of each FILE to the current time." << endl;
    cerr << "A FILE argument that does not exist is created empty." << endl;
    cerr << "" << endl;
    cerr << "  -r FILE     use this file's times instead of current time" << endl;
    cerr << "  -t STAMP    use [[CC]YY]MMDDhhmm[.ss] instead of current time" << endl;
    cerr << "  --help      display this help and exit" << endl;
    cerr << "  --version   output version information and exit" << endl;
    cerr << "" << endl;
    exit(status);
}

void ShowVersion() {
    cout << "yet another touch command version 0.1.0" << endl;
    exit(0);
}

void SetTimeStamp(const char* file,
                  FILETIME* cTime,
                  FILETIME* aTime,
                  FILETIME* mTime) {
    HANDLE fileHandle = CreateFile((LPCSTR)file,
                                   GENERIC_READ|GENERIC_WRITE,
                                   0,           //share mode
                                   NULL,        //security
                                   OPEN_ALWAYS, //open or create
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
    if(fileHandle == INVALID_HANDLE_VALUE) {
        cerr << file << ": cannot open" << endl;
        exit(2);
    }
    if(SetFileTime(fileHandle, cTime, aTime, mTime) == 0) {
        cerr << file << ": cannot set timestamp" << endl;
        exit(3);
    }
    if(CloseHandle(fileHandle) == 0) {
        cerr << file << ": cannot close" << endl;
        exit(4);
    }
}

void GetTimeStamp(const char* file,
                  FILETIME* cTime,
                  FILETIME* aTime,
                  FILETIME* mTime) {
    HANDLE fileHandle = CreateFile((LPCSTR)file,
                                   GENERIC_READ,
                                   0,           //share mode
                                   NULL,        //security
                                   OPEN_EXISTING, //open
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
    if(fileHandle == INVALID_HANDLE_VALUE) {
        cerr << file << ": cannot open" << endl;
        exit(2);
    }
    if(GetFileTime(fileHandle, cTime, aTime, mTime) == 0) {
        cerr << file << ": cannot get timestamp" << endl;
        exit(3);
    }
    if(CloseHandle(fileHandle) == 0) {
        cerr << file << ": cannot close" << endl;
        exit(4);
    }
}
                                     
int main(int argc, char* argv[]) {
    if(argc < 2) ShowHelp(argv[0], 1);

    char* file  = argv[1];
    char* ref   = NULL;
    char* stamp = NULL;
    if(strncmp(file, "-r", 2) == 0) {
        if(argc != 4) ShowHelp(argv[0], 1);
        ref   = argv[2];
        file  = argv[3];
    } else
    if(strncmp(file, "-t", 2) == 0) {
        if(argc != 4) ShowHelp(argv[0], 1);
        stamp = argv[2];
        file  = argv[3];
    } else
    if(strncmp(file, "--help", 6) == 0) {
        ShowHelp(argv[0], 0);
    } else
    if(strncmp(file, "--version", 9) == 0) {
        ShowVersion();
    } else
    if(argc > 2) {
        ShowHelp(argv[0], 1);
    }

    if(ref   == NULL &&
       stamp == NULL) {
        FILETIME fileTime;
        GetSystemTimeAsFileTime(&fileTime);
        SetTimeStamp(file, &fileTime, &fileTime, &fileTime);
    } else
    if(ref != NULL) {
        FILETIME cTime;
        FILETIME aTime;
        FILETIME mTime;
        GetTimeStamp(ref,  &cTime, &aTime, &mTime);
        SetTimeStamp(file, &cTime, &aTime, &mTime);
    } else
    if(stamp != NULL) {
        SYSTEMTIME currentTime;
        SYSTEMTIME   stampTime;
        FILETIME      fileTime;
        FILETIME     localTime;
        GetSystemTimeAsFileTime(&fileTime);
        FileTimeToLocalFileTime(&fileTime, &localTime);
        FileTimeToSystemTime(&localTime, &currentTime);

        string         YMDhms = stamp;
        string::size_type pos = YMDhms.find_first_of('.');
        stampTime.wMilliseconds = 0;
        stampTime.wSecond       = 0;
        if(pos != string::npos) {
            stampTime.wSecond = stoi(YMDhms.substr(pos + 1));
            YMDhms = YMDhms.substr(0, pos);
        }
        if(YMDhms.length() == 12) {
            stampTime.wYear   = stoi(YMDhms.substr(0, 4));
            YMDhms = YMDhms.substr(4);
        } else
        if(YMDhms.length() == 10) {
            stampTime.wYear   = stoi(YMDhms.substr(0, 2));
            stampTime.wYear  += currentTime.wYear - currentTime.wYear % 100;
            YMDhms = YMDhms.substr(2);
        } else {
            stampTime.wYear   = currentTime.wYear;
        }
        if(YMDhms.length() ==  8) {
            stampTime.wMonth  = stoi(YMDhms.substr(0, 2));
            stampTime.wDay    = stoi(YMDhms.substr(2, 2));
            stampTime.wHour   = stoi(YMDhms.substr(4, 2));
            stampTime.wMinute = stoi(YMDhms.substr(6, 2));
        } else {
            cerr << stamp << ": illegal format" << endl;
            exit(5);
        }

        SystemTimeToFileTime(&stampTime, &localTime);
        LocalFileTimeToFileTime(&localTime, &fileTime);
        SetTimeStamp(file, &fileTime, &fileTime, &fileTime);
    }

    return 0;
}    
