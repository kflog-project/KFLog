//
// Defines for GARMIN PROTOCOL decribed by GARMIN Corporation
//     Spec. version: 001-00063-00 Rev. 3
//

#ifndef _GARMINTYPES_H
#define _GARMINTYPES_H

// Basic Packet IDs
enum {
  Pid_Ack_Byte = 6,
  Pid_Nak_Byte = 21,
  Pid_Protocol_Array = 253,	// may not be implemented in all products
  Pid_Product_Rqst = 254,
  Pid_Product_Data = 255
};

// L001 - Link Protocol 1
enum {
  Pid_Command_Data = 10,
  Pid_Xfer_Cmplt = 12,
  Pid_Data_Time_Data = 14,
  Pid_Position_Data = 17,
  Pid_Prx_Wpt_Data = 19,
  Pid_Records = 27,
  Pid_Rte_Hdr = 29,
  Pid_Rte_Wpt_Data = 30,
  Pid_Almanac_Data = 31,
  Pid_Trk_Data = 34,
  Pid_Wpt_Data = 35,
  Pid_Pvt_Data = 51,
  Pid_Rte_Link_Data = 98,
  Pid_Trk_Hdr = 99
};  

typedef unsigned long longword;
typedef unsigned char boolean;

typedef struct {
   long lat;	//lat in semicircles
   long lon;    //lon in semicircles
} Semicircle_Type;

typedef struct {
   Semicircle_Type	posn;
   longword		time;
   boolean		new_trk;
} D300_Trk_Point_Type;

typedef struct {
  char ident[6];
  Semicircle_Type posn;
  longword unused;
  char cmnt[40];
  unsigned char smbl;
  unsigned char dspl;
} D103_Wpt_Type;

typedef struct {
   int product_ID;
   int software_version;
   char product_description[15];
} Product_Data_Type;

#endif


 
