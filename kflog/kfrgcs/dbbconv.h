#ifndef DBBCONV_H
#define DBBCONV_H

#include "vlapityp.h"

class DBB {
//private:
public:
	enum {
		DBBBeg  = 0x0000,
		DBBEnd  = 0x3000,
		FrmBeg  = 0x3000,
		FrmEnd  = 0x4000
	};
 int dbcursor;
 int fdfcursor;
 struct HEADER {
	 int dsanzahl;
	 int dslaenge, keylaenge;
	 unsigned short int dsfirst, dslast;
 };
 HEADER header[8];
public:
 byte block[DBBEnd-DBBBeg];
 byte	fdf[FrmEnd-FrmBeg];
	DBB();
	void open_dbb();
	void close_db(int kennung);
	void add_ds(int kennung,void *quelle);
	void add_fdf(int feldkennung,int feldlaenge, void *quelle);
	int16 fdf_findfield(byte id);
};

#endif
