/***********************************************************************
**
**   filser.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Christian Fughe, Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#define _GNU_SOURCE
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include <klocale.h>

#include "../airport.h"
#include "filser.h"

#define MAX_LSTRING    63

/*loger defines*/
#define NODATA         0
#define LSTRING        1 /* to 63 string*/
#define END           64	/* 40 */

#define SHVERSION    127	/* 7f */
#define START        128	/* 80 */
#define ORIGIN       160	/* a0 */

#define POSITION_OK  191	/* bf */
#define POSITION_BAD 195	/* c3 */


#define SECURITY           240	/* f0 */
#define COMPETITION_CLASS  241	/* f1 */
#define SAT_CON            242	/* f2 */
#define DIFFERENTIAL       243	/* f3 */
#define EVENT              244	/* f4 */
#define SECURITY_OLD       245	/* f5 */
#define SER_NR             246	/* f6 */
#define TASK               247	/* f7 */
#define UNITS              248	/* f8 */
#define FIXEXT             249	/* f9 */
#define EXTEND             250	/* fa */
#define DATUM              251	/* fb */
#define FLIGHT_INFO        253	/* fc */
#define EXTEND_INFO        254	/* fd */
#define FIXEXT_INFO        255	/* fe */

#define LOW_SECURITY        0x0d
#define MED_SECURITY        0x0e
#define HIGH_SECURITY       0x0f

/**
 * The device-name of the port.
 */
char* portName = '\0';
int portID;
char c36[] = "0123456789abcdefghijklmnopqrstuvwxyz";

extern int breakTransfer;

/**
 * holds the port-settings at start of the application
 */
struct termios oldTermEnv;

/**
 * is used to change the port-settings
 */
struct termios newTermEnv;

/*
 * Command bytes for communication with the lx device
 */
unsigned char STX = 0x02, /* Command prefix like AT for modems      */
  ACK = 0x06,      /* Response OK, if the crc check is ok           */
  SYN = 0x16,      /* Request for CONNECT                           */
  K = 'K' | 0x80,  /* get_extra_data()   - trailing fix sized block */
  L = 'L' | 0x80,  /* get_mem_sections() - the flight data is       */
                   /*                      retrieved in blocks      */
  M = 'M' | 0x80,  /* read_flight_index()- table of flights           */
  N = 'N' | 0x80,  /* def_mem()          - memory range of one flight */
  Q = 'Q' | 0x80,  /* read_mem_setting()                       */
  f = 'f' | 0x80;  /* get_logger_data()  - first block         */
                   /* f++ get_logger_data()  - next block          */

#define BUFSIZE 1024          /* Genaral buffer size             */

/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the programm. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int /* signal*/)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  //exit(-1);
}

Filser::Filser()
{
  //Set Flightrecorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = 100;             //maximum number of tasks
  _capabilities.maxNrWaypoints = 600;         //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = 10; //maximum number of waypoints per task
  _capabilities.maxNrPilots = 1;            //maximum number of pilots

  //_capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  //_capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  _capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  _capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  //_capabilities.supDlTask = true;          //supports downloading of tasks?
  //_capabilities.supUlTask = true;          //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  //End set capabilities.

  portID = -1;
  serialNo = 0xffffffff;
  flightIndex.setAutoDelete(true);
}

Filser::~Filser()
{
}

/**
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Filser::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

QString Filser::getLibName() const {  return "libkfrfil";  }

int Filser::getFlightDir(QList<FRDirEntry>* dirList)
{
  int flightCount;
  int indexByte = 1;
  int rc;
  char *bufP;
  char buf[BUFSIZE + 1];
  struct tm startTime, stopTime;
  time_t startTime_t, stopTime_t;
  struct tm lastDate;
  lastDate.tm_year = 0;
  lastDate.tm_mon = 0;
  lastDate.tm_mday = 1;

  struct flightTable *ft;

  dirList->clear();

  if (!check4Device() || !readMemSetting()) {
    return FR_ERROR;
  }

  _errorinfo = "";

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(M);

  rc = FR_OK;

  while (indexByte) {
    bufP = buf;
    while ((FLIGHT_INDEX_WIDTH + buf - bufP) > 0) {
      bufP = readData(bufP, (FLIGHT_INDEX_WIDTH + buf - bufP));
    }
      
    if ((bufP - buf) != FLIGHT_INDEX_WIDTH) {   
      _errorinfo = i18n("read_flight_index(): Wrong amount of bytes from LX-device");
      rc = FR_ERROR;
      break;
    }
    else if (calcCrcBuf((char *)buf, FLIGHT_INDEX_WIDTH - 1) != (char)buf[FLIGHT_INDEX_WIDTH - 1]) {
      _errorinfo = i18n("read_flight_index(): Bad CRC");
      rc = FR_ERROR;
      break;
    }
    else {
      ft = new struct flightTable;
      memcpy(ft->record, buf, bufP - buf);
      flightIndex.append(ft);

      // remove \0 between date and time
      ft->record[17] = ' ';
      startTime_t = stopTime_t = 0;
      startTime = *gmtime(&startTime_t);
      stopTime = *gmtime(&stopTime_t);
      strptime((char *)ft->record + 9, "%d.%m.%y %T", &startTime);
      strptime((char *)ft->record + 27, "%T", &stopTime);

      startTime_t = mktime(&startTime);
      stopTime_t = mktime(&stopTime);

      FRDirEntry* entry = new FRDirEntry;
 
      if(lastDate.tm_year == startTime.tm_year &&
         lastDate.tm_mon == startTime.tm_mon &&
         lastDate.tm_mday == startTime.tm_mday) {
        flightCount++;
      }
      else {
        flightCount = 1;
      }

      entry->pilotName = (const char *)ft->record + 40;
      entry->gliderID = "n.a."; 

      entry->firstTime = startTime;
      entry->lastTime = stopTime;
      entry->duration = stopTime_t - startTime_t;
      serialNo = (ft->record[91] << 8) + ft->record[92];
      entry->shortFileName.sprintf("%c%c%cf%s%c.igc",
                                   c36[entry->firstTime.tm_year % 10],
                                   c36[entry->firstTime.tm_mon + 1],
                                   c36[entry->firstTime.tm_mday],
                                   wordtoserno(serialNo),
                                   c36[flightCount]);
      entry->longFileName.sprintf("%d-%.2d-%.2d-fil-%s-%.2d.igc",
                                  entry->firstTime.tm_year + 1900,
                                  entry->firstTime.tm_mon + 1,
                                  entry->firstTime.tm_mday,
                                  wordtoserno((ft->record[91] << 8) + ft->record[92]),
                                  flightCount);
      warning(entry->longFileName);
      dirList->append(entry);
      lastDate = startTime;

      indexByte = buf[0];
      if (indexByte != 0 && indexByte != 1) {
        _errorinfo = i18n("read_flight_index(): Wrong index byte");
        rc = FR_ERROR;
        break;
      } 
    }
  }
  return rc;
}

int Filser::downloadFlight(int flightID, int /*secMode*/, const QString& fileName)
{
  int rc;
  struct flightTable *ft;
  char memSection[0x20];  /* Information received from the   */
                          /* logger about the memory         */
                          /* blocks of a specific flight     */
                          /* for download. The table is      */
                          /* 0x20 bytes long. Two bytes for  */
                          /* a block.                        */
  char *memContents = 0; /* buffer to hold igc contents */
  int contentSize; /* length of igc file buffer */
  FILE *f;

  _errorinfo = "";
  
  ft = flightIndex.at(flightID);
  if (!check4Device() || !defMem(ft) || 
      !getMemSection(memSection, sizeof(memSection)) ||
      !getLoggerData(memSection, sizeof(memSection), &memContents, &contentSize)) {
    rc = FR_ERROR;
  }
  else {
    if ((f = fopen(fileName, "w")) != NULL) {
      if (convFil2Igc(f, (unsigned char *)memContents, (unsigned char *)memContents + contentSize)) {
        rc = FR_OK;
      }
      else {
        _errorinfo += i18n("\ncheck igc file for further info");
        rc = FR_ERROR;        
      }
      fclose(f);
    }
    else {
      _errorinfo = i18n("cannot open igc file ") + fileName;
      rc = FR_ERROR;
    }
  }

  delete memContents;

  return rc;
}


QString Filser::getRecorderSerialNo()
{
  if (serialNo > 46655L)
    return "???";
  else
    return QString("%1").arg (serialNo);
}

int Filser::openRecorder(const QString& pName, int baud)
{
  speed_t speed;
  portName = (char *)pName.latin1();

  portID = open(portName, O_RDWR | O_NOCTTY);
  if(portID != -1) {
    //
    // Before we change any port-settings, we must establish a
    // signal-handler, which is used to restore the port-settings
    // after terminating the programm.
    //    Because a SIGKILL-signal removes the programm immediately,
    // the status of the port will be undefined.
    //
    struct sigaction sact;

    sact.sa_handler = releaseTTY;
    sigaction(SIGHUP, &sact, NULL);
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGPIPE, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);

    /*
     * Set the terminal mode of the serial line
     */

    // reading the current port-settings
    tcgetattr(portID, &newTermEnv);

    // storing the port-settings to restore them ...
    oldTermEnv = newTermEnv;

    /*
     * Do some common settup
     */
    newTermEnv.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    newTermEnv.c_oflag &= ~OPOST;
    newTermEnv.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    /*
     * No flow control at all :-(
     */
    newTermEnv.c_cflag &= ~(CSIZE | PARENB | CRTSCTS | IXON | IXOFF);
    newTermEnv.c_cflag |= (CS8 | CLOCAL);

    // control characters
    newTermEnv.c_cc[VMIN] = 0; // don't wait for a character
    newTermEnv.c_cc[VTIME] = 1; // wait at least 1 msec.

    if(baud >= 115200) speed = B115200;
    else if(baud >= 57600) speed = B57600;
    else if(baud >= 38400) speed = B38400;
    else if(baud >= 19200) speed = B19200;
    else if(baud >=  9600) speed = B9600;
    else if(baud >=  4800) speed = B4800;
    else if(baud >=  2400) speed = B2400;
    else if(baud >=  1800) speed = B1800;
    else if(baud >=  1200) speed = B1200;
    else if(baud >=   600) speed = B600;
    else if(baud >=   300) speed = B300;
    else if(baud >=   200) speed = B200;
    else if(baud >=   150) speed = B150;
    else if(baud >=   110) speed = B110;
    else speed = B75;

    cfsetospeed(&newTermEnv, speed);
    cfsetispeed(&newTermEnv, speed);

    // Activating the port-settings
    tcsetattr(portID, TCSANOW, &newTermEnv);

    _isConnected = true;
    return FR_OK;
    }
  else {
    warning(i18n("No logger found!"));
    _isConnected = false;
    return FR_ERROR;
  }
}

int Filser::defMem(struct flightTable *ft)
{
  unsigned char	address_buf[7];
  int flight_start_adr, flight_end_adr;

  /* Flight_table->record[3] is the 4-th and highest address byte,    */
  /* but wb(STX), wb(N) takes only three bytes. Neither this software */
  /* nor the software from Filser is prepared for a value of the 4-th */
  /* address byte different from 0.                                   */
  flight_start_adr = (ft->record[4] << 16) + (ft->record[1] << 8) + (ft->record[2]);

  if(ft->record[3]) {
    _errorinfo = i18n("Invalid memory size in the flight table from the LX-device.");
    return 0;
  }

  /* The discussion about flight_table->record[3] holds as well for   */
  /* flight_table->record[7].                                         */
  flight_end_adr = (ft->record[4+4] << 16) +(ft->record[1+4]<< 8) + (ft->record[2+4]);

  if(ft->record[7]) {
    _errorinfo = i18n("Invalid memory size in the flight table from the LX-device.");
    return 0;
  }

  memcpy(address_buf, &flight_start_adr, 3);
  memcpy(address_buf + 3, &flight_end_adr, 3);
  address_buf[6] = (unsigned char)calcCrcBuf((char *)address_buf, 6);

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(N);
  for(unsigned int i = 0; i < sizeof(address_buf); i++) {
    wb(address_buf[i]);
  }
  if (rb() != ACK) {
    _errorinfo = i18n("Invalid response from LX-device.");
    return 0;
  }
  return 1;
}

int Filser::getMemSection(char *memSection, int size)
{
  int i;

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(L);
  for(i = 0; i < (size + 1); i++) {
    memSection[i] = rb();
  }

  if(calcCrcBuf(memSection, size) != memSection[size]) {
    _errorinfo = i18n("get_mem_sections(): Bad CRC");
    return 0;
  }
  return 1;
}

int Filser::getLoggerData(char *memSection, int sectionSize, char **memContents, int *contentSize)
{
  int i;
  int count;
  char *bufP, *bufP2;
  /*
   * Calculate the size the of the memoty buffer
   */
  *contentSize = 0; 
  for(i = 0; i < (sectionSize / 2); i++) {
    if(!(memSection[2 * i] | memSection[(2 * i) + 1])) {
      break;
    }
    *contentSize += ((unsigned char)memSection[2 * i] << 8) + (unsigned char)memSection[(2 * i) + 1];
  }
  /*
   * plus 0x100 bytes for the extra data of get_extra_data()
   * we ignore this, required only for .fil files
   */
  //fil_file_length+=0x100; 
  /*
   * Allocate fil_file_length bytes
   * plus one byte for the CRC byte.
   */

  *memContents = new char [(*contentSize) + 1]; // for CRC
  bufP = bufP2 = *memContents;

  // read each memory section
  for(i = 0; i < (sectionSize / 2); i++) {
    if(!(memSection[2 * i] | memSection[(2 * i) + 1])) {
      break;
    }
    count = ((unsigned char)memSection[2 * i] << 8) + (unsigned char)memSection[(2 * i) + 1];

    tcflush(portID, TCIOFLUSH);
    wb(STX);
    wb(f + i);
    while ((bufP - bufP2) < (count + 1)) {
      bufP = readData(bufP, (bufP2 + count + 1 - bufP));
    }
    if (calcCrcBuf(bufP2, count) != bufP2[count]) {
      _errorinfo = i18n("get_logger_data(): Bad CRC");
      delete *memContents;
      *memContents = 0;
      *contentSize = 0;
      return 0;
    }
    bufP2 += count;
    bufP = bufP2;
  }

  return 1;  
}

/*
 * This function translates a .fil-file to an .igc-file.
 *
 * The .fil-file is expected in a buffer beginning at *fil_p and ending
 * at *fil_p_last.
 *
 * The resulting .igc-file is written to the open FILE pointer *figc.
 */
int Filser::convFil2Igc(FILE *figc,  unsigned char *fil_p, unsigned char *fil_p_last)
{
  int i, j, l, ftab[16], etab[16], time, time_orig, fix_lat, fix_lat_orig, fix_lon, fix_lon_orig, tp;
  unsigned char flight_no, *fil_p_ev;

  unsigned int ext_dat;                    
  char HFDTE[256], fix_ext_num = 0, ext_num = 0, ev = 0, fix_stat;
  unsigned int flt_id;
  char *flt_pilot, *flt_glider, *flt_reg, *flt_comp, *flt_observer, *flt_gps;
  unsigned char flt_class_id, flt_gps_datum, flt_fix_accuracy;

  struct task {
    int usage;
    int lat;
    int lon;
    char T[256];
  };

  struct task TP[12];

  struct  three_letter_s {
    char *name;
    int count;
  };

  struct three_letter_s   three_letter_tab[16] = {
    { (char *)"FXA", 2 },
    { (char *)"VXA", 2 },
    { (char *)"RPM", 4 },
    { (char *)"GSP", 4 },
    { (char *)"IAS", 4 },
    { (char *)"TAS", 4 },
    { (char *)"HDM", 2 },
    { (char *)"HDT", 2 },
    { (char *)"TRM", 2 },
    { (char *)"TRT", 2 },
    { (char *)"TEN", 4 },
    { (char *)"WDI", 2 },
    { (char *)"WVE", 4 },
    { (char *)"ENL", 2 },
    { (char *)"VAR", 2 },
    { (char *)"XX3", 2 }
  };

  char *competition_class[] = {
    (char *)"STANDARD",
    (char *)"15-METER",
    (char *)"OPEN",
    (char *)"18-METER",
    (char *)"WORLD",
    (char *)"DOUBLE",
    (char *)"MOTOR_GL",
    (char *)""
  };



  char *gps_datum_tab[] = {
    (char *)"ADINDAN        ",
    (char *)"AFGOOYE        ",
    (char *)"AIN EL ABD 1970",
    (char *)"COCOS ISLAND   ",
    (char *)"ARC 1950       ",
    (char *)"ARC 1960       ",
    (char *)"ASCENSION 1958 ",
    (char *)"ASTRO BEACON E ",
    (char *)"AUSTRALIAN 1966",
    (char *)"AUSTRALIAN 1984",
    (char *)"ASTRO DOS 7/14 ",
    (char *)"MARCUS ISLAND  ",
    (char *)"TERN ISLAND    ",
    (char *)"BELLEVUE (IGN) ",
    (char *)"BERMUDA 1957   ",
    (char *)"COLOMBIA       ",
    (char *)"CAMPO INCHAUSPE",
    (char *)"CANTON ASTRO   ",
    (char *)"CAPE CANAVERAL ",
    (char *)"CAPE (AFRICA)  ",
    (char *)"CARTHAGE       ",
    (char *)"CHATHAM 1971   ",
    (char *)"CHUA ASTRO     ",
    (char *)"CORREGO ALEGRE ",
    (char *)"DJAKARTA       ",
    (char *)"DOS 1968       ",
    (char *)"EASTER ISLAND  ",
    (char *)"EUROPEAN 1950  ",
    (char *)"EUROPEAN 1979  ",
    (char *)"FINLAND 1910   ",
    (char *)"GANDAJIKA BASE ",
    (char *)"NEW ZEALAND '49",
    (char *)"OSGB 1936      ",
    (char *)"GUAM 1963      ",
    (char *)"GUX 1 ASTRO    ",
    (char *)"HJOESEY 1955   ",
    (char *)"HONG KONG 1962 ",
    (char *)"INDIAN/NEPAL   ",
    (char *)"INDIAN/VIETNAM ",
    (char *)"IRELAND 1965   ",
    (char *)"DIEGO GARCIA   ",
    (char *)"JOHNSTON 1961  ",
    (char *)"KANDAWALA      ",
    (char *)"KERGUELEN ISL. ",
    (char *)"KERTAU 1948    ",
    (char *)"CAYMAN BRAC    ",
    (char *)"LIBERIA 1964   ",
    (char *)"LUZON/MINDANAO ",
    (char *)"LUZON PHILIPPI.",
    (char *)"MAHE 1971      ",
    (char *)"MARCO ASTRO    ",
    (char *)"MASSAWA        ",
    (char *)"MERCHICH       ",
    (char *)"MIDWAY ASTRO'61",
    (char *)"MINNA (NIGERIA)",
    (char *)"NAD-1927 ALASKA",
    (char *)"NAD-1927 BAHAM.",
    (char *)"NAD-1927 CENTR.",
    (char *)"NAD-1927 CANAL ",
    (char *)"NAD-1927 CANADA",
    (char *)"NAD-1927 CARIB.",
    (char *)"NAD-1927 CONUS ",
    (char *)"NAD-1927 CUBA  ",
    (char *)"NAD-1927 GREEN.",
    (char *)"NAD-1927 MEXICO",
    (char *)"NAD-1927 SALVA.",
    (char *)"NAD-1983       ",
    (char *)"NAPARIMA       ",
    (char *)"MASIRAH ISLAND ",
    (char *)"SAUDI ARABIA   ",
    (char *)"ARAB EMIRATES  ",
    (char *)"OBSERVATORIO'66",
    (char *)"OLD EGYIPTIAN  ",
    (char *)"OLD HAWAIIAN   ",
    (char *)"OMAN           ",
    (char *)"CANARY ISLAND  ",
    (char *)"PICAIRN 1967   ",
    (char *)"PUERTO RICO    ",
    (char *)"QATAR NATIONAL ",
    (char *)"QORNOQ         ",
    (char *)"REUNION        ",
    (char *)"ROME 1940      ",
    (char *)"RT-90 SWEDEN   ",
    (char *)"S.AMERICA  1956",
    (char *)"S.AMERICA  1956",
    (char *)"SOUTH ASIA     ",
    (char *)"CHILEAN 1963   ",
    (char *)"SANTO(DOS)     ",
    (char *)"SAO BRAZ       ",
    (char *)"SAPPER HILL    ",
    (char *)"SCHWARZECK     ",
    (char *)"SOUTHEAST BASE ",
    (char *)"FAIAL          ",
    (char *)"TIMBALI 1948   ",
    (char *)"TOKYO          ",
    (char *)"TRISTAN ASTRO  ",
    (char *)"RESERVED       ",
    (char *)"VITI LEVU 1916 ",
    (char *)"WAKE-ENIWETOK  ",
    (char *)"WGS-1972       ",
    (char *)"WGS-1984       ",
    (char *)"ZANDERIJ       ",
    (char *)"CH-1903        "
  };

  while(fil_p < fil_p_last) {
    if(!fil_p[0]) {
      i = 0;
      while(fil_p[0] == 0) {
        i++;
        fil_p++;
        if(fil_p > fil_p_last) {
          _errorinfo = i18n("unexpected end of '.fil'-file");
          return 0;
        }
      }
      fprintf(figc, "LFILEMPTY%d\r\n", i);
    }
    else if((fil_p[0] <= MAX_LSTRING) && (fil_p[0] > 0)) {
      fprintf(figc, "%.*s\r\n", fil_p[0], fil_p + 1);         
      fil_p += fil_p[0] + 1;
    }
    /*
     * Identify the records of a .fil-file and translate them
     * to the .igc-file format.
     */
    else switch (fil_p[0]) {
    case START: 
      /* 80h                         */
      /* 8   byte: "STReRAZ\0"       */
      /* 1   byte: flight of the day */
      fil_p++;
      flight_no = fil_p[8];  
      fil_p += 9;
      break;
  
    case DATUM: 
      /* fbh                         */
      /* 1   byte: day               */
      /* 1   byte: month             */
      /* 2   byte: year              */
      fil_p++;
      sprintf(HFDTE, "%02d%02d%02d", fil_p[0], fil_p[1], (fil_p[2] << 8) + fil_p[3]);
      fil_p += 4;
      break;
  
    case SER_NR:
      /* f6h                         */
      /* 8   byte, "FIL_S/N\0"       */
      /* 1   byte, flight of the day */
      fil_p++;
      fprintf(figc, "A%s", fil_p);         
      fil_p += 9;
      fprintf(figc, "FLIGHT:%d\r\n", flight_no);         
      fprintf(figc, "HFDTE%s\r\n", HFDTE);         
      break;
  
    case FLIGHT_INFO:
      /* fch                         */
      /* 2   byte, id                */
      /* 19  byte, pilot             */
      /* 12  byte, glider            */
      /* 8   byte, registration      */
      /* 4   byte, competition       */
      /* 1   byte, class id          */
      /* 10  byte, observer          */
      /* 1   byte, gps datum         */
      /* 1   byte, fix accuracy      */
      /* 60  byte, gps               */
      fil_p++;
      flt_id = (fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      flt_pilot = (char *)fil_p;
      fil_p += 19;
      flt_glider = (char *)fil_p;
      fil_p += 12;
      flt_reg = (char *)fil_p;
      fil_p += 8;
      flt_comp = (char *)fil_p;
      fil_p += 4;
      flt_class_id = fil_p[0];
      fil_p++;
      flt_observer = (char *)fil_p;
      fil_p += 10;
      flt_gps_datum = fil_p[0];
      fil_p++;
      flt_fix_accuracy = fil_p[0];
      fil_p++;
      flt_gps = (char *)fil_p;
      fil_p += 60;
      if(flt_class_id == 7) {
        break;
      }
      fprintf(figc, "HFFXA%03d\r\n", flt_fix_accuracy);
      fprintf(figc, "HFPLTPILOT:%s\r\n", flt_pilot);
      fprintf(figc, "HFGTYGLIDERTYPE:%s\r\n", flt_glider);
      fprintf(figc, "HFGIDGLIDERID:%s\r\n", flt_reg);
      if(flt_gps_datum != 255) {
        fprintf(figc, "HFDTM%03dGPSDATUM:%s\r\n", flt_gps_datum, 
                gps_datum_tab[flt_gps_datum]);
      }
      else {
        fprintf(figc, "HFDTM%03dGPSDATUM:UNKNOWN\r\n", flt_gps_datum);
      }
      fprintf(figc, "HFCIDCOMPETITIONID:%s\r\n", flt_comp);
      fprintf(figc, "HFCCLCOMPETITIONCLASS:%s\r\n", competition_class[flt_class_id]);
      fprintf(figc, "HFGPSGPS:%s\r\n", flt_gps);
      break;
  
    case COMPETITION_CLASS:
      /* f1h                         */
      /* 9   byte, class id          */
      fil_p++;
      if(flt_class_id != 7) {
        break;
      }
      fprintf(figc, "HFFXA%03d\r\n", flt_fix_accuracy);
      fprintf(figc, "HFPLTPILOT:%s\r\n", flt_pilot);
      fprintf(figc, "HFGTYGLIDERTYPE:%s\r\n", flt_glider);
      fprintf(figc, "HFGIDGLIDERID:%s\r\n", flt_reg);
      if(flt_gps_datum != 255) {
        fprintf(figc, "HFDTM%03dGPSDATUM:%s\r\n", flt_gps_datum, gps_datum_tab[flt_gps_datum]);
      }
      else {
        fprintf(figc, "HFDTM%03dGPSDATUM:UNKNOWN\r\n", flt_gps_datum);
      }
      fprintf(figc, "HFCIDCOMPETITIONID:%s\r\n", flt_comp);
      fprintf(figc, "HFCCLCOMPETITIONCLASS:%s\r\n", fil_p);
      fil_p += 9;
      fprintf(figc, "HFGPSGPS:%s\r\n", flt_gps);
      break;
  
    case SHVERSION:
      /* 7fh                         */
      /* 1   byte, hardware          */
      /* 1   byte, software          */
      fil_p++;
      fprintf(figc, "HFRFWFIRMWAREVERSION:%3.1f\r\n", fil_p[1]/10.);         
      fprintf(figc, "HFRHWHARDWAREVERSION:%3.1f\r\n", fil_p[0]/10.);         
      fil_p += 2;
      break;
  
    case FIXEXT_INFO:
      /* feh                         */
      /* 2   byte, time              */
      /* 2   byte, dat               */
      fil_p++;
      fil_p += 2;
      ext_dat = (fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      fix_ext_num = 0;
      for (l = 1; l < 65535; l *= 2) {
        if (ext_dat & l) {
          fix_ext_num++;
        }
      }
      if (fix_ext_num > 0) {
        fprintf(figc, "I%02d", fix_ext_num);
      }
      else {
        break;
      }

      i = 36;
      j = 0;
      for (l = 0; l < 16; l++) {
        if(ext_dat & (1 << l)) {
          fprintf(figc, "%02d%02d%s", i, i+three_letter_tab[l].count, three_letter_tab[l].name);
          i += three_letter_tab[l].count + 1;
          ftab[j] = three_letter_tab[l].count + 1;
          j++;
        }
      }
      fprintf(figc, "\r\n");
      break;
  
    case EXTEND_INFO:
      /* fdh                         */
      /* 2   byte, time              */
      /* 2   byte, dat               */
      fil_p++;
      fil_p += 2;
      ext_dat = (fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      ext_num = 0;
      for (l = 1; l < 65535; l *= 2) {
        if (ext_dat & l) {
          ext_num++;
        }
      }
      if(ext_num > 0) {
        fprintf(figc, "J%02d", ext_num);
      }
      else {
        break;
      }

      i = 8;
      j = 0;
      for (l = 0; l < 16; l++) {
        if(ext_dat & (1 << l)) {
          fprintf(figc, "%02d%02d%s", i, three_letter_tab[l].count+i, three_letter_tab[l].name);
          i += three_letter_tab[l].count + 1;
          etab[j] = three_letter_tab[l].count + 1;
          j++;
        }
      }
      fprintf(figc, "\r\n");
      break;
  
    case FIXEXT:
      /* f9h                         */
      fil_p++;
      for(i = 0;i < fix_ext_num; i++) {
        fprintf(figc, "%0*u", ftab[i], (fil_p[0] << 8) + fil_p[1]);
        fil_p += 2;
      }  
      fprintf(figc, "\r\n");
      break;
  
    case EXTEND:
      /* fah                         */
      fil_p++;
      l = fil_p[0] + time;
      fil_p++;
      fprintf(figc, "K%02d%02d%02d", l / 3600, l % 3600 / 60, l % 60 );
      for(i = 0; i < ext_num; i++) {
        fprintf(figc, "%0*u", etab[i], (fil_p[0] << 8) + fil_p[1]);
        fil_p += 2;
      }  
      fprintf(figc, "\r\n");
      break;
  
    case TASK:
      /* f7h                         */
      /* 4   byte, time              */
      /* 1   byte, day               */
      /* 1   byte, month             */
      /* 1   byte, year              */
      /* 1   byte, day   (user)      */
      /* 1   byte, month (user)      */
      /* 1   byte, year  (user)      */
      /* 2   byte, taskid            */
      /* 1   byte, num_of_tp         */
      /* 12x1 byte, usage    (12 tp) */
      /* 12x4 byte, long     (12 tp) */
      /* 12x4 byte, lat      (12 tp) */
      /* 12x9 byte, name     (12 tp) */
      fil_p++;
      time = (fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
      fil_p += 4;
      fprintf(figc, "C%02d%02d%02d%02d%02d%02d", fil_p[0], fil_p[1], fil_p[2], time / 3600, time % 3600 / 60, time % 60 );
      fil_p += 3;
      fprintf(figc, "%02d%02d%02d", fil_p[0], fil_p[1], fil_p[2]);
      fil_p += 3;
      fprintf(figc, "%04d%02d\r\n", (fil_p[0] << 8) + fil_p[1], fil_p[2]);
      fil_p += 3;

      // get usage
      for(tp = 0; tp < 12; tp++) {
        TP[tp].usage = fil_p[0];
        fil_p++;
      }
      // get lon
      for(tp = 0; tp < 12; tp++) {
        TP[tp].lon = ((char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
        fil_p += 4;
      }
      // get lat
      for(tp = 0; tp < 12; tp++) {
        TP[tp].lat = ((char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
        fil_p += 4;
      }
      // get name
      for(tp = 0; tp < 12; tp++) {
        sprintf(TP[tp].T, "%s", fil_p);
        /*
         * the TP-names are 8-bytes long plus one '\0'
         * the space of shorter names is filled with '\0'
         */
        fil_p += 9;
      }
      for(tp = 0; tp < 12; tp++) {
        if(TP[tp].usage) {
          fprintf(figc, "C%02d%05d%c", TP[tp].lat / 60000, TP[tp].lat % 60000, TP[tp].lat >= 0 ?  'N' : 'S');
          fprintf(figc, "%03d%05d%c", TP[tp].lon / 60000, TP[tp].lon % 60000, TP[tp].lon >= 0 ? 'E' : 'W');
          fprintf(figc,  "%s\r\n", TP[tp].T);
        }
      }
      break;
  
    case EVENT:
      fil_p++;
      fil_p_ev = fil_p;
      fil_p += 9;
      ev = 1;
      break;
  
    case POSITION_OK:
    case POSITION_BAD:
      /* bfh                         */
      /* 2   byte, time              */
      /* 2   byte, lat               */
      /* 2   byte, lon               */
      /* 2   byte, aalt              */
      /* 2   byte, galt              */
      fix_stat = fil_p[0] == POSITION_OK ? 'A' : 'V';
      fil_p++;
      time = time_orig + ((char) fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      fix_lat = fix_lat_orig + ((char) fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      fix_lon = fix_lon_orig + ((char) fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
  
      if(ev) {
        fprintf(figc,"E%02d%02d%02d%s\r\n", time / 3600, time % 3600 / 60, time % 60, fil_p_ev);
        ev = 0;
      }
  
      if (time < 0) {
        ev=0;
      }
      fprintf(figc, "B%02d%02d%02d", time / 3600, time % 3600 / 60, time % 60);         
      fprintf(figc, "%02d%05d%c", abs( fix_lat / 60000), abs(fix_lat % 60000), fix_lat >= 0 ? 'N' : 'S');
      fprintf(figc, "%03d%05d%c", abs(fix_lon / 60000), abs(fix_lon % 60000), fix_lon >= 0 ? 'E' : 'W'); fprintf(figc, "%c", fix_stat);
      fprintf(figc, "%05d", (fil_p[0] << 8) + fil_p[1]);
      fil_p += 2;
      fprintf(figc, "%05d", (fil_p[0] << 8) + fil_p[1]);
      fil_p += 2;
      if (!fix_ext_num) {
        fprintf(figc,"\r\n");
      }
      break;                
  
    case ORIGIN:
      /* a0h                         */
      /* 4   byte, time              */
      /* 4   byte, lat               */
      /* 4   byte, lon               */
      fil_p++;
      time = (fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
      fil_p += 4;
      fprintf(figc, "LFILORIGIN%02d%02d%02d", time / 3600, time % 3600 / 60, time % 60 );         

      fix_lat= ((char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] <<8 ) + fil_p[3];
      fil_p += 4;
      fprintf(figc, "%02d%05d%c", abs(fix_lat / 60000), abs(fix_lat % 60000), fix_lat >= 0 ? 'N' : 'S');

      fix_lon = ((char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) +fil_p[3];
      fil_p += 4;
      fprintf(figc, "%03d%05d%c\r\n", abs(fix_lon / 60000), abs(fix_lon % 60000), fix_lon >= 0 ? 'E' : 'W');
      time_orig = time;
      fix_lat_orig = fix_lat;
      fix_lon_orig = fix_lon;
      break;
  
    case END:
      /* 40h                         */
      fil_p = fil_p_last;   /* end of while loop */
      break;
  
    case SECURITY_OLD:
      /* f5h                         */
      fil_p++;
      fprintf(figc, "G%22.22s\r\n", fil_p);          
      fil_p += 22;
      break;                
    case SECURITY:
      /* f0h                         */
      /* 1   byte, len               */
      /* 1   byte, type              */
      /* 64  byte, char              */
      fil_p++;
  
      switch(fil_p[1]) {
      case HIGH_SECURITY: 
        fprintf(figc, "G2"); 
        break;
      case MED_SECURITY:  
        fprintf(figc, "G1"); 
        break;
      case LOW_SECURITY:  
        fprintf(figc, "G0"); 
        break;
      }
  
      for(i = 0; i < fil_p[0]; i++) {
        fprintf(figc, "%02X", fil_p[i + 2]);         
      }
      fprintf(figc, "\r\n");         
      fil_p += 66;
      break;                
    default:        /* ???? */
      fprintf(figc, "LFILUNKNOWN%#x\r\n", fil_p[0]);         
      fil_p++;
      _errorinfo = i18n("unexpected record id in '.fil'-file");
      return 0;
      break;                
    }
  }

  return 1;
}

/**
 * Calculate the check sum
 */
char Filser::calcCrc(char d, char crc)
{
  char tmp, crcpoly = 0x69; /* Static value for the calculation of the checksum. */

  for(int count = 8; --count >= 0; d <<= 1) {
    tmp = crc ^ d;
    crc <<= 1;
    if(tmp < 0) {
      crc ^= crcpoly;
    }
  }
  return crc;
}

/**
 * Calculate the check sum on a buffer of bytes
 */
char Filser::calcCrcBuf(char *buf, unsigned int count)
{
  unsigned int i;
  char crc = (char)0xff;
  for(i = 0; i < count; i++) {
    crc = calcCrc(buf[i], crc);
  }
  return crc;
}

/*
 * Read the memory setup from the lx device.
 *
 * The lx devices do have a memory setup described with three bytes.
 * In the future this function might be necessary to disconnect from lx
 * devices with more memory. The CRC check will tell.
 */
char *Filser::readData(char *bufP, int count)
{
  int rc;
  switch (rc = read(portID, bufP, count)) {
  case -1:
    warning("read_data(): ERROR");
    break;
  default:
    bufP += rc;
    break;
  }
  return bufP;  
}

int Filser::readMemSetting()
{
  char *bufP;
  char buf[BUFSIZE + 1];

  memset(buf, '\0', sizeof(buf));

  if (!check4Device()) {
    return 0;
  }

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(Q);

  bufP = buf;
  while ((LX_MEM_RET + buf - bufP) > 0) {
    bufP = readData(bufP, LX_MEM_RET + buf - bufP);
  }

  if(calcCrcBuf(buf, 6) != buf[6]) {
    warning("read_mem_setting(): Bad CRC");
    return 0;
  }  

  /*
   * Byte description and the values from my logger
   *
   *     (buf[0] == 0x0)       position on the start page, MS byte
   *     (buf[1] == 0x0)       position on the start page, LS byte
   *     (buf[2] == 0x6)       start page
   *     (buf[3] == 0x80)      position on the stop page, MS byte
   *     (buf[4] == 0x0)       position on the stop page, LS byte
   *     (buf[5] == 0x0b)      stop page
   *     (buf[6] == 0x41)      CRC value
   */
  warning("read_mem_setting(): all fine!!");

  return 1;
}

/**
 * Check presence of LX-device and make CONNECT
 *
 * Necessary wakeup before a command
 * if the LX-device is in TIMEOUT countdown
 * mode waiting for CONNECT.
 */
bool Filser::check4Device()
{
  bool rc = false;
  int ret;
  time_t t1;
  _errorinfo = "";

  t1 = time(NULL);
  tcflush(portID, TCIOFLUSH);
  while (!breakTransfer) {
    wb(SYN);
    ret = rb();
    if (ret == ACK) {
      warning("connected");
      rc = true;
      break;
    }
    else {
      // waiting 5 secs. for response
      if (time(NULL) - t1 > 10) {
        _errorinfo = i18n("No response from recorder within 10 seconds!\nDid you press READ/WRITE?");
        rc = false;
        break;
      }      
    }
  }
  return rc;
}

/*
 * write byte
 */
int Filser::wb(unsigned char c)
{
  if (write(portID, &c, 1) != 1) {
    return -1;
  }
  return 1;
}


/*
 * read byte
 */
int Filser::rb()
{
  char buf;

  if (read(portID, &buf, 1) != 1) {
    return -1;
  }
  return buf;
}   

char *Filser::wordtoserno(unsigned int Binaer)
{
//  char SerNStr[4];
  static char Seriennummer[4];
  // limitation
  if (Binaer > 46655L) {
    Binaer = 46655L;
  }

  Seriennummer[0]=c36[Binaer / 36 / 36];
  Seriennummer[1]=c36[Binaer / 36 % 36];
  Seriennummer[2]=c36[Binaer % 36];
  Seriennummer[3] = '\0';

  return Seriennummer;
}

int Filser::closeRecorder()
{
  if (portID != -1) {
    tcsetattr(portID, TCSANOW, &oldTermEnv);
    close(portID);
    _isConnected = false;
    return FR_OK;
  }
  else {
    return FR_ERROR;
  }
}

/** NOT IMLEMENTED
    ============================================*/

int Filser::writeDeclaration(FRTaskDeclaration* /*taskDecl*/, QList<Waypoint> * /*taskPoints*/ )
{
  return FR_NOTSUPPORTED;
}

int Filser::readDatabase()
{
  return FR_NOTSUPPORTED;
}

int Filser::readTasks(QList<FlightTask> * /*tasks*/)
{
  return FR_NOTSUPPORTED;
}

int Filser::writeTasks(QList<FlightTask> * /*tasks*/)
{
  return FR_NOTSUPPORTED;
}

int Filser::readWaypoints(QList<Waypoint> * /*waypoints*/)
{
  return FR_NOTSUPPORTED;
}

int Filser::writeWaypoints(QList<Waypoint> * /*waypoints*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Opens the recorder for other communication.
 */
int Filser::openRecorder(const QString& /*URL*/)
{
  return FR_NOTSUPPORTED;
}
