/*
 * =====================================================================================
 *
 *       Filename: System.h
 *
 *    Description: Abstract layer for system calls
 *
 *         Author: Paul P. Hilscher (2010-), 
 *
 *        License: GPLv3+
 * =====================================================================================
 */

#ifndef __SYSTEM_H
#define __SYSTEM_H

// for portability we export system functions


#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

class System 
{
  public:
    unsigned int static getProcessID() {
             return getpid();
    };


    static void doSleep(int seconds) {
    
        unsigned int s = sleep((unsigned int) seconds);

    };
    
    unsigned int static getTime() {

        return time(0);

    };



};



#endif // __SYSTEM_H


