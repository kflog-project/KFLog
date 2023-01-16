// g++ -I /usr/local/qt/include /usr/local/qt/lib64/libqt-mt.so write_terrain_file.cpp

#include <iostream>
#include <fstream>
#include <math.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qdatetime.h>
#include <vector>
#include <map>

#define KFLOG_FILE_MAGIC        0x404b464c
#define FILE_TYPE_GROUND        0x47
#define FILE_TYPE_TERRAIN       0x54
#define FILE_TYPE_MAP           0x4d
#define FILE_FORMAT_ID_TERRAIN  102
#define FILE_FORMAT_ID_MAP      101
#define ISO_LINE_NUM            50

const int isoLines[] =
{
  0, 10, 25, 50, 75, 100, 150, 200, 250,
  300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750,
  2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000, 4250, 4500,
  4750, 5000, 5250, 5500, 5750, 6000, 6250, 6500, 6750, 7000, 7250,
  7500, 7750, 8000, 8250, 8500, 8750
};

void process_terrain_file (const Q_INT16 saveSecID, const Q_INT8 saveTypeID) {

  // Open input file. This is a GRASS ascii file, generated with
  // v.out.ascii input=<map> output=<file> format=standard
  QString infilename;
  infilename = QString().asprintf("%c_%.5d", saveTypeID, saveSecID);
  QFile infile(infilename);
  infile.open(IO_ReadOnly);
  QTextStream in(&infile);

  // skip input file header
  for (int i=0; i<10; i++) {
    in.readLine();
  }


  // Open output file for binary writing
  QString outfilename;
  outfilename = QString().asprintf("%c_%.5d.kfl", saveTypeID, saveSecID);
  QFile outfile(outfilename);
  outfile.open(IO_WriteOnly);
  QDataStream out(&outfile);
  out.setVersion(6);  // QDataStream::Qt_3_3

  // Write output file header
  Q_UINT32 magic = KFLOG_FILE_MAGIC;
  Q_UINT16 formatID = FILE_FORMAT_ID_TERRAIN;
  QDateTime createDateTime = QDateTime::currentDateTime();
  out << magic;
  out << saveTypeID;
  out << formatID;
  out << saveSecID;
  out << createDateTime;


  // Write isolines
  Q_INT16 elevation = 0;
  Q_INT32 locLength = 0;
  char cdummy;
  double ddummy;
  double lat, lon;
  std::vector<Q_INT32> latlist, lonlist;

  int count = 0;
  while(!in.eof()) {
    latlist.clear();
    lonlist.clear();
    in >> cdummy >> locLength >> cdummy;
    //std::cout << "length = " << locLength << std::endl;
    for (Q_INT32 i=0; i<locLength; i++) {
      in >> lon >> lat >> elevation;
      latlist.push_back( rint(lat*600000) );
      lonlist.push_back( rint(lon*600000) );
      //std::cout << lat << "  " << lon << "  " << elevation << std::endl;
    }
    in >> ddummy >> ddummy;

    if (locLength > 2) {
      out << elevation;
      out << locLength;
      for (Q_INT32 i=locLength-1; i>=0; i--) {
        out << latlist[i];
        out << lonlist[i];
      }
      count++;
      if (! (count%500)) std::cout << count << " isolines done." << std::endl;
    }
  }

  infile.close();
  outfile.close();
  return;
}


void process_binary_file (const Q_INT16 saveSecID, const Q_INT8 saveTypeID) {

  // Open input file. This is a GRASS ascii file, generated with
  // v.out.ascii input=<map> output=<file> format=standard
  QString infilename;
  infilename = QString().asprintf("/home/hoeth/grassdata/%c_%.5d", saveTypeID, saveSecID);
  QFile infile(infilename);
  infile.open(IO_ReadOnly);
  QTextStream in(&infile);

  // skip input file header
  for (int i=0; i<10; i++) {
    in.readLine();
  }


  // Open output file for binary writing
  QString outfilename;
  outfilename = QString().asprintf("%c_%.5d.kfl", saveTypeID, saveSecID);
  QFile outfile(outfilename);
  outfile.open(IO_WriteOnly);
  QDataStream out(&outfile);
  out.setVersion(6);  // QDataStream::Qt_3_3

  // Write output file header
  Q_UINT32 magic = KFLOG_FILE_MAGIC;
  Q_UINT16 formatID = FILE_FORMAT_ID_MAP;
  QDateTime createDateTime = QDateTime::currentDateTime();
  out << magic;
  out << saveTypeID;
  out << formatID;
  out << saveSecID;
  out << createDateTime;


  // Write lake shores (0x31 is the lake type)
  Q_UINT8 type = 0x31;
  Q_INT16 elevation = 0;
  Q_INT8 sort = 0;
  Q_INT32 locLength = 0;
  QString name = "";
  char cdummy;
  double ddummy;
  double lat, lon;
  std::vector<Q_INT32> latlist, lonlist;

  Q_INT32 count = 0;
  while(!in.eof()) {
    count++;
    if (! (count%500)) std::cout << count << " isolines done." << std::endl;
    latlist.clear();
    lonlist.clear();
    in >> cdummy >> locLength >> cdummy;
    //std::cout << "length = " << locLength << std::endl;
    for (Q_INT32 i=0; i<locLength; i++) {
      in >> lon >> lat;
      latlist.push_back( rint(lat*600000) );
      lonlist.push_back( rint(lon*600000) );
      //std::cout << lat << "  " << lon << "  " << std::endl;
    }
    in >> ddummy >> ddummy;

    // the old files contain names of cities, rivers, lakes, ...
    // Unfortunately we'll have to leave that empty.
    if (locLength > 2) {
      out << type;
      out << sort;
      out << name;
      out << locLength;
      for (Q_INT32 i=locLength-1; i>=0; i--) {
        out << latlist[i];
        out << lonlist[i];
      }
    }
  }

  infile.close();
  outfile.close();
  return;
}

int main(int argc, char* argv[]) {
  Q_UINT16 saveSecID = atoi(argv[1]);
  std::cout << "Processing tile T_" << saveSecID << std::endl;
  process_terrain_file (saveSecID, FILE_TYPE_TERRAIN);
  std::cout << "Processing tile G_" << saveSecID << std::endl;
  process_terrain_file (saveSecID, FILE_TYPE_GROUND);
  //std::cout << "Processing tile M_" << saveSecID << std::endl;
  //process_binary_file (saveSecID, FILE_TYPE_MAP);

  return 0;
}
