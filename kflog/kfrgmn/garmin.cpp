//
// garmin.cpp 
// - Garmin GPS Class implementation
//
// 2001, Thomas Nielsen
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "garmin.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

//
// predefined GARMIN commands
//
unsigned char cmd_OK[] = { 0x10,0x06,0x02,0x0A,0x00,0xEE,0x10,0x03 };
unsigned char cmd_PRODUCTRQST[] = { 0x10,0xFE,0x00,0x02,0x10,0x03 };
unsigned char cmd_WPRQST[] = { 0x10,0x0A,0x02,0x07,0x00,0xED,0x10,0x03 };
unsigned char cmd_TRKRQST[] = { 0x10,0x0A,0x02,0x06,0x00,0xEE,0x10,0x03 };
unsigned char cmd_NEXT[] = { 0x10,0x06,0x02,0x22,0x00,0xD6,0x10,0x03 };
unsigned char cmd_FIRSTREC[] = { 0x10,0x06,0x02,0x1B,0x00,0xDD,0x10,0x03 };
unsigned char cmd_NEXTREC[] = { 0x10,0x06,0x02,0x22,0x00,0xD6,0x10,0x03 };
unsigned char cmd_LASTREC[] = { 0x10,0x06,0x02,0x0C,0x00,0xEC,0x10,0x03 };


// Product IDs 

char GarminProductID[][50] = { "", //0
"","","","","","","","","","", // 1-10
"","","","","","","","","","", // 11-20
"","","","","","","","","","", // 21-30
"","","","","","","","","","", // 31-40
"","","","","","","","","","", // 41-50
"","","","","","","","","","", // 51-60
"","","","","","","","","","", // 61-70
"","","","","","","GPS 12/12XL\0","","","", // 71-80
"","","","","","","GPS 12/12XL\0","","","", // 81-90
"","","","","","GPS 12","GPS 48/GPS 12/GPS 12XL\0","","","",  // 91-100
"","","","","GPS 12XL Japanese\0","GPS 12XL Chinese\0","","","",""  //101-110
};

//
// constructor
//
GarminGPS::GarminGPS(void)
{
  this->m_device = -1;
  this->m_model = -1;
  this->m_software_version = -1.0;
  memset(&(this->m_newtio), 0, sizeof(struct termios));  
  memset(&(this->m_oldtio), 0, sizeof(struct termios));
}

//
// destructor
//
GarminGPS::~GarminGPS()
{

}

//////////////////////////////////////////////////////////////////////////////////
//
// Link Layer
//
//////////////////////////////////////////////////////////////////////////////////
int GarminGPS::openComm(char * port)
{
  int fd;

  //
  // O_RDWR   : read and write access
  // O_NOCTTY : don't want to be controlling terminal 
  // O_NDELAY : don't care about state of carrier detect line
  //
  fd = open(port, O_RDWR | O_NDELAY);
  if (fd == -1){
#ifdef _GARMIN_DBG
    printf("::openComm(): Unable to open %s...\n", port);
#endif
    return(fd);
  } else {
#ifdef _GARMIN_DBG
    printf("::openComm(): %s opened...\n", port);
#endif
    this->m_device = fd;
    // F_SETFL = FNDELAY : no blocking on reads!
    fcntl(this->m_device, F_SETFL, FNDELAY);
  }

  // now change the port settings, saving the current setup
  tcgetattr(this->m_device, &(this->m_oldtio));
  tcgetattr(this->m_device, &(this->m_newtio));
  cfsetispeed(&(this->m_newtio), B9600);
  cfsetospeed(&(this->m_newtio), B9600);
  
  // enable reciever and set local mode
  this->m_newtio.c_cflag |= (CLOCAL | CREAD);
  this->m_newtio.c_cflag &= ~PARENB;	// no parity
  this->m_newtio.c_cflag &= ~CSTOPB;	// not 2 stopbits
  this->m_newtio.c_cflag &= ~CSIZE; 	// no bit mask for databits
  this->m_newtio.c_cflag |= CS8; 	// 8N1 mode

  // deselect canonical mode, echoing and ISIG handling
  // echoing will produce a feedback
  this->m_newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  this->m_newtio.c_iflag &= ~(INPCK | ISTRIP);        //disable p-chk and par. stripping
  this->m_newtio.c_iflag |= (IXON | IXOFF | IXANY);   // software flowcontrol
  this->m_newtio.c_oflag &= ~OPOST;  	            // use raw output by masking OPOST bit
  this->m_newtio.c_cc[VTIME] = 0;
  this->m_newtio.c_cc[VMIN] = 0;          // read 1 char
  this->m_newtio.c_cc[VSTART] = 0;        // really DLE;
  this->m_newtio.c_cc[VSTOP] = 0;         // really ETX;
  
  // set new attributes and flush buffers at the same time 
  tcsetattr(this->m_device, TCSAFLUSH, &(this->m_newtio));

  return(this->m_device);
}

int GarminGPS::closeComm(void)
{
  int fd;

  fd = this->m_device;

  //reset original attributes
  tcsetattr(fd, TCSAFLUSH, &this->m_oldtio);

  // restore blocking readmode
  fcntl(fd, F_SETFL, 0); 
  close(fd);

#ifdef _GARMIN_DBG
  printf("::closeComm(): closed...\n");
#endif

  return 0;
}

unsigned char * GarminGPS::readPacket(int * bytesread)
{
  unsigned char * bufptr;
  unsigned char buffer[MAX_PACKET_SIZE];
  int done;
  int res;
  int totalread;
  unsigned char * packet;

  if (this->m_device == -1){
#ifdef _GARMIN_DBG
    printf("::readPacket(): ERROR - illegal port\n");
#endif
    return (unsigned char*)NULL; 
  }
  
  bufptr = (unsigned char *)buffer;
  done = 0;
  totalread = 0;
  while (done==0){
    while ((res = read(this->m_device, bufptr, MAX_PACKET_SIZE-totalread-1)) > 0)
    {
      bufptr += res;
      totalread += res;
      //      if (totalread >= 2){ 
        if (bufptr[-2] == DLE && bufptr[-1] == ETX){
          done = 1;
          break;
        }
	//}
    }
  }

  this->stripDoubleDLE((unsigned char*)buffer,&totalread);

  packet = (unsigned char*)malloc(totalread);
  memset(packet, 0, totalread);
  memcpy(packet, buffer, totalread);

  // send back number of bytes read
  *bytesread = totalread-1;

#ifdef _GARMIN_DBG
  printf("::readPacket(): len=%d\n", totalread);
#endif

  return packet;    // return length of packet
}

int GarminGPS::writePacket(unsigned char * cmd, int len)
{
  int ret;

  ret = write(this->m_device, cmd, len);
#ifdef _GARMIN_DBG
  printf("::writePacket(): len = %d \n", ret);
#endif
  return ret;
}

void GarminGPS::dumpPacket(unsigned char * packet, int length)
{
#ifdef _GARMIN_DBG
  printf("::dumpPacket(): ");
  for(int i=0; i <=length; i++){
    printf("<0x%X>", *((unsigned char*)packet+i));
  }
  printf("\n");
#endif
}

//////////////////////////////////////////////////////////////////////////////////
//
// Application Layer
//
//////////////////////////////////////////////////////////////////////////////////
int GarminGPS::getPacketID(unsigned char * packet, int /*length*/)
{
  return (int)*(packet+1);     // return packet ID byte
}

int GarminGPS::getNumberOfRecords(unsigned char * packet, int /*length*/)
{
  // given that this is a valid Pid_Records packet, return the number of records
  int numrecords; 

  if (*(packet+1) != Pid_Records)
    return -1;

  numrecords = 256 * (*(packet+4)) + *(packet+3);  // calc number of packets
  return numrecords;
}

int GarminGPS::getModelNumber(unsigned char * packet, int /*length*/)
{
  int model;

  if (*(packet+1) != Pid_Product_Data)
    return -1;
  
  model = (int) *(packet+3);
  return model;
}

double GarminGPS::getSoftwareVersion(unsigned char * packet, int /*length*/)
{
  double d_sw;

  if (*(packet+1) != Pid_Product_Data)
    return -1.0;
  
  d_sw = (double)(256* (*(packet+6)) + *(packet+5)) / 100.0 ;

  return d_sw;
}

char * GarminGPS::getModel(void)
{
  char * model;
  unsigned char * p;
  int m,n;
  double d;

  this->writePacket(cmd_PRODUCTRQST, 8);
  // GPS will reply with an ACK, ignore it...
  p = (unsigned char*)this->readPacket(&n);
  //... and send an ACK to get the product data packet
  this->writePacket(cmd_OK, 8);

  p = (unsigned char*)this->readPacket(&n);
  this->writePacket(cmd_OK, 8);

  if (this->getPacketID(p,n) != Pid_Product_Data){
    printf("::getModel(): unexpected response from GPS\n");
    return NULL;
  }
  else
    printf("::getModel(): GPS replied with Product_Data\n");

  model = (char*)malloc(MAX_PACKET_SIZE);
  memset(model, 0, MAX_PACKET_SIZE);

  m = this->getModelNumber(p,n);
  this->m_model = m; 
  sprintf(model, "%s", GarminProductID[m]);
  printf("::getModel(): Model = Garmin %s\n", model);
 
  d = (double)this->getSoftwareVersion(p,n);
  this->m_software_version = d;
  printf("::getModel(): Software Version = %.2f\n", d);    

  free(p);
  return model;
}

void GarminGPS::getTrackLog(void)
{
  int i,n;
  unsigned char * p;

  n = this->writePacket(cmd_TRKRQST, 8);

  p = (unsigned char*)this->readPacket(&n);

  if (this->getPacketID(p,n) != Pid_Ack_Byte){
#ifdef _GARMIN_DBG
    printf("::getTrackLog(): unexpected response from GPS\n");
#endif
    return;
  }
#ifdef _GARMIN_DBG
  else
    printf("::getTrackLog(): GPS replied OK\n");
#endif
  free(p);

  n = this->writePacket(cmd_FIRSTREC, 8);

  p = (unsigned char*)this->readPacket(&n);
  
  if (this->getPacketID(p,n) != Pid_Records){
#ifdef _GARMIN_DBG
    printf("::getTrackLog(): unexpected response from GPS\n");
#endif
    return;
  }
#ifdef _GARMIN_DBG
  else
    printf("::getTrackLog(): GPS replied with %d records\n", this->getNumberOfRecords(p,n));
#endif

  i = 0;
  do {
    free(p); 
    p = (unsigned char*)this->readPacket(&n);
    this->dumpD300TrkPointTypePacket(p,n);

    if (Pid_Xfer_Cmplt != this->getPacketID(p,n)){
      this->writePacket(cmd_NEXTREC, 8);
      i++;
    }
  } while (Pid_Xfer_Cmplt != this->getPacketID(p,n));
#ifdef _GARMIN_DBG
  printf("::getTrackLog(): read %d packets\n", i);
#endif
  free(p);

  return;
}


void GarminGPS::getWaypoints(void)
{
  int i,n;
  unsigned char * p;
  
  n = this->writePacket(cmd_WPRQST, 8);

  p = (unsigned char*)this->readPacket(&n);

  if (this->getPacketID(p,n) != Pid_Ack_Byte){
#ifdef _GARMIN_DBG
    printf("::getWaypoints(): unexpected response from GPS\n");
#endif
    return;
  }
#ifdef _GARMIN_DBG
  else
    printf("::getWaypoints(): GPS replied OK\n");
#endif
  free(p);

  n = this->writePacket(cmd_FIRSTREC, 8);

  p = (unsigned char*)this->readPacket(&n);
  
  if (this->getPacketID(p,n) != Pid_Records){
#ifdef _GARMIN_DBG
    printf("::getWaypoints(): unexpected response from GPS\n");
#endif
    return;
  }
#ifdef _GARMIN_DBG
  else
    printf("::getWaypoints(): GPS replied with %d records\n", this->getNumberOfRecords(p,n));
#endif

  i = 0;
  do {
    free(p); 
    p = (unsigned char*)this->readPacket(&n);
    //    this->dumpD300TrkPointTypePacket(p,n);
    this->dumpPacket(p,n);
    if (Pid_Xfer_Cmplt != this->getPacketID(p,n)){
      this->writePacket(cmd_NEXTREC, 8);
      i++;
    }
  } while (Pid_Xfer_Cmplt != this->getPacketID(p,n));

#ifdef _GARMIN_DBG
  printf("::getWaypoints(): read %d packets\n", i);
#endif
  free(p);

  return;
}



void GarminGPS::dumpD300TrkPointTypePacket(unsigned char * packet, int /*length*/)
{
  D300_Trk_Point_Type TrackRecord;
  double d_lon, d_lat;
  int n_lon, n_lat;
  time_t gpstime, zerotime;
  char s_lon[10], s_lat[10];
  int a,b;

  if (*(packet+1) != Pid_Trk_Data)
    return;

#ifdef _GARMIN_DBG
  printf("::dumpD300TrkPointTypePacket(): ");
#endif

  // cast packet to a D300_Trk_Data_Type
  memset(&TrackRecord, 0, sizeof(TrackRecord));
  memcpy(&TrackRecord, packet+3, sizeof(TrackRecord));

  d_lat = (double)(TrackRecord.posn.lat*(180.0/2147483648.0));
  d_lon = (double)(TrackRecord.posn.lon*(180.0/2147483648.0));
  // positve lat in semicircles = N, negative = S

  if (d_lat >=0.0)
    a = 'N';
  else {
    a = 'S';
    d_lat *= -1; //invert sign
  }
  // positve lon in semicircles = W, negative = E
  if (d_lon >=0.0)
    b = 'E';
  else {
    b = 'W';
    d_lon *= -1;  //invert sign
  }
  // split into integer part and decimal parts
   n_lat = n_lon = 0;
  while (d_lat >= 1.0){
     d_lat -= 1.0;
     n_lat += 1;
  }
  while (d_lon > 1.0){
     d_lon -= 1.0;
     n_lon += 1;
  }
  // convert to secs:
  d_lon *= 60;
  d_lat *= 60;

  zerotime = this->getZerotime();

  gpstime = (time_t)(TrackRecord.time);
  gpstime += zerotime;

  // format and output to screen
  if (d_lon < 10.0)
     sprintf(s_lon, "0%.4f", d_lon);
  else
     sprintf(s_lon, "%.4f", d_lon);
  if (d_lat < 10.0)
     sprintf(s_lat, "0%.4f", d_lat);
  else
     sprintf(s_lat, "%.4f", d_lat);

  printf("T  %c%d %s %c%03d %s %s", a, n_lat, s_lat, b, n_lon, s_lon, ctime(&gpstime));

  return;
}

//////////////////////////////////////////////////////////////////////////////////
//
// Private methods
//
//////////////////////////////////////////////////////////////////////////////////
void GarminGPS::stripDoubleDLE(unsigned char * packet, int * length)
{
  int i, j;

  // safety first
  if (*length < 2) 
    return;

  // search length, but don't include DLE,ETX
  for (i = 0; i < (*length)-2; i++){
    if ((*(packet+i) == DLE) && (*(packet+i+1) == DLE) && (*(packet+i+2) != ETX)){
      j = i;
#ifdef _GARMIN_DBG
      printf("::stripDoubleDLE: double DLEs found, stripping\n");
#endif
      while (j < *length){  
        memcpy(packet+j, packet+j+1, 1);
	j++;
      }
      (*length)--;  // decrease length of packet
    }
  }
}

time_t GarminGPS::getZerotime(void)
{
  struct tm zero;		// Garmin GPS zerodate
  
  bzero(&zero, sizeof(zero));
  zero.tm_sec=0;
  zero.tm_min=0;
  zero.tm_hour=0;
  zero.tm_mday=31;
  zero.tm_mon=11;	//december!
  zero.tm_year=89;
  
  return mktime(&zero);
}
  

