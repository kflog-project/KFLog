//
// GARMIN.H
// - GARMIN GPS Class header file
//
// 2001, Thomas Nielsen
//

#ifndef _GARMIN_H
#define _GARMIN_H
//#define _POSIX_SOURCE 1	  	// posix compliant source

#include "garmintypes.h"        // Garmin specific types, structs and ready commands
#include <termios.h>
#include <time.h>

//#define _GARMIN_DBG             // use this flag if you want debug text output

#define BAUDRATE B9600
#define FALSE 0
#define TRUE 1
#define MAX_PACKET_SIZE 255
#define DLE 0x10			// ASCII DLE char
#define ETX 0x03			// ASCII ETX char
#define TRACK 0
#define WAYPOINT 1
#define DOWNLOAD 0
#define UPLOAD 1

class GarminGPS
{
  public:
    GarminGPS(void);
    ~GarminGPS();
    //
    // link layer
    //
    int openComm(char *);
    int closeComm();
    unsigned char * readPacket(int *);
    int writePacket(unsigned char *, int);
    void dumpPacket(unsigned char *, int);
    //
    // application layer
    //
    int getPacketID(unsigned char *, int);    
    int getNumberOfRecords(unsigned char *, int);
    int getModelNumber(unsigned char *, int);
    double getSoftwareVersion(unsigned char *, int);
    // high-level methods
    char * getModel();
    void getTrackLog();
    void getWaypoints();

    void dumpD300TrkPointTypePacket(unsigned char *, int);

  private:
    //
    // private methods 
    //
    void stripDoubleDLE(unsigned char *, int *);
    time_t getZerotime();

    //
    // member variables
    //
    int m_device;              // handle to opened comm device
    struct termios m_newtio;   // holds GARMIN comm settings
    struct termios m_oldtio;   // holds startup comm settings
    int m_model;               // holds the GARMIN model ID of the GPS
    double m_software_version; // holds the GARMIN sw verion of the GPS
};

#endif
