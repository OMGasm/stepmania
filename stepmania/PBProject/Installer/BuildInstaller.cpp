/*
 *  BuildInstaller.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Sun Sep 07 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

using namespace std;

#include "StdString.h"
#include "InstallerFile.h"
#include "Processor.h"
#include <unistd.h>
#include <sys/param.h>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include <map>

void HandleFile(const CString& file, const CString& dir, const CString& archivePath, bool overwrite)
{
    static bool archiveMade = false;
    CString command;

    if (archiveMade)
        command = "tar rfPC '";
    else
    {
        archiveMade = true;
        command = "tar cfPC '";
    }
    command += archivePath + "' '" + dir +"' '" + file + "'";
    
    system(command);
}

const CString GetPath(const CString& ID)
{
    if (ID == "install path")
    {
        char cwd[MAXPATHLEN];
        char path[MAXPATHLEN];
        char *ptr;

        getwd(cwd);
        printf("Enter a path (relative or absolute) to the install files\n"
               "The current working directory is: %s\n"
               "> ", cwd);
        ptr = gets(path);
        ASSERT(ptr);
        while (*ptr != '\000' && (*ptr == ' ' || *ptr == '\t'))
            ++ptr;
        if (*ptr == '/')
            return ptr;
        return CString(cwd) + "/" + CString(ptr);       
    }
    
    fprintf(stderr, "Unknown path command, return `.'\n");
    return ".";
}

void PrintUsage(int err)
{
    printf("usage: BuildInstaller config [dir]\n\n"
           "config:  The configuration file for the installer.\n"
           "dir:     The output directory. It is created if it doesn't exist\n");
    exit(err);
}

int main(int argc, char *argv[])
{
    CString inFile, outDir;

    switch (argc)
    {
        case 1:
            printf("BuildInstaller needs to be run with at least one argument.\n");
            PrintUsage(1);

        case 2:
            inFile = argv[1];
            outDir = ".";
            break;

        case 3:
            inFile = argv[1];
            outDir = argv[2];
            break;

        default:
            printf("BuildInstaller takes at most 2 arguments.\n");
            PrintUsage(2);
    }

    if (inFile == "help" || inFile == "-h" || inFile == "--help")
        PrintUsage(0);

    outDir += "/files";

    InstallerFile config(inFile);
    unsigned nextLine = 0;

    if (!config.ReadFile())
    {
        printf("Couldn't read config file, \"%s\".\n", inFile.c_str());
        return 3;
    }

    /* Create the directory--the lazy man's way */
    CString command = "mkdir -p '" + outDir + "'";
    
    if (system(command))
    {
        printf("The system(\"%s\") call failed.\n", command.c_str());
        return 4;
    }

    CString archivePath = outDir + "/archive.tar";

    Processor p(archivePath, HandleFile, GetPath, NULL, false);

    while (nextLine < config.GetNumLines())
        p.ProcessLine(config.GetLine(nextLine), nextLine);

    printf("Compressing the archive.\n");
    
    /* Compress the archive after it is created */
    command = "gzip '" + archivePath + "'";
    system(command);

    if (!config.WriteFile(outDir + "/config"))
    {
        printf("%s\n", strerror(errno));
        return 6;
    }

    return 0;
}
