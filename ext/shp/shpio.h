#ifndef _SHP_IO_H
#define _SHP_IO_H

#pragma pack( push )
#pragma pack( 1 )

#if !defined(_WIN32)
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#include <ctype.h>
#endif
//
// DBF File
//
/*
dBASE III DATABASE FILE HEADER:

+---------+-------------------+---------------------------------+
|  BYTE   |     CONTENTS      |          MEANING                |
+---------+-------------------+---------------------------------+
|  0      |  1 byte           | dBASE III version number        |
|         |                   |  (03H without a .DBT file)      |
|         |                   |  (83H with a .DBT file)         |
+---------+-------------------+---------------------------------+
|  1-3    |  3 bytes          | date of last update             |
|         |                   |  (YY MM DD) in binary format    |
+---------+-------------------+---------------------------------+
|  4-7    |  32 bit number    | number of records in data file  |
+---------+-------------------+---------------------------------+
|  8-9    |  16 bit number    | length of header structure      |
+---------+-------------------+---------------------------------+
|  10-11  |  16 bit number    | length of the record            |
+---------+-------------------+---------------------------------+
|  12-31  |  20 bytes         | reserved bytes (version 1.00)   |
+---------+-------------------+---------------------------------+
|  32-n   |  32 bytes each    | field descriptor array          |
|         |                   |  (see below)                    | --+
+---------+-------------------+---------------------------------+   |
|  n+1    |  1 byte           | 0DH as the field terminator     |   |
+---------+-------------------+---------------------------------+   |
|
|
A FIELD DESCRIPTOR:      <------------------------------------------+

+---------+-------------------+---------------------------------+
|  BYTE   |     CONTENTS      |          MEANING                |
+---------+-------------------+---------------------------------+
|  0-10   |  11 bytes         | field name in ASCII zero-filled |
+---------+-------------------+---------------------------------+
|  11     |  1 byte           | field type in ASCII             |
|         |                   |  (C N L D or M)                 |
+---------+-------------------+---------------------------------+
|  12-15  |  32 bit number    | field data address              |
|         |                   |  (address is set in memory)     |
+---------+-------------------+---------------------------------+
|  16     |  1 byte           | field length in binary          |
+---------+-------------------+---------------------------------+
|  17     |  1 byte           | field decimal count in binary   |
+---------+-------------------+---------------------------------+
|  18-31  |  14 bytes         | reserved bytes (version 1.00)   |
+---------+-------------------+---------------------------------+


The data records are layed out as follows:

1. Data records are preceeded by one byte that is a space (20H) if the
record is not deleted and an asterisk (2AH) if it is deleted.

2. Data fields are packed into records with  no  field separators or
record terminators.

3. Data types are stored in ASCII format as follows:

DATA TYPE      DATA RECORD STORAGE
---------      --------------------------------------------
Character      (ASCII characters)
Numeric        - . 0 1 2 3 4 5 6 7 8 9
Logical        ? Y y N n T t F f  (? when not initialized)
Memo           (10 digits representing a .DBT block number)
Date           (8 digits in YYYYMMDD format, such as
19840704 for July 4, 1984)
Floating Point DBase IV
*/

typedef struct tagDBFFieldInfo
{
	char				cpName[11]; // field name size is 10		//0x00
	char				cType;										//0x11
	/*  'C' // Character  'D' :  // Date YYYYMMDD
	'L' // Logical    'N' :  // Numeric
	'M' // Memo using .DBT                        */
	unsigned char		ucpAddr[4]; // n,n,n,n memory addr, n,n,0,0 offset, 0,0,0,0 ignored //0x12
	unsigned char		cLength;	//0x16
	unsigned char		cDec;
	unsigned char		cRes1;
	unsigned char		cRes2;
	unsigned char		cWAID;
	unsigned short		usMultiUserDB;
	unsigned char		ucSetFields;
	unsigned char		cRes3;
	unsigned char		cRes4;
	unsigned char		cRes5;
	unsigned char		cRes6;
	unsigned char		cRes7;
	unsigned char		cRes8;
	unsigned char		cRes9;
	unsigned char		ucMdxIndex;
} TDBF_FIELDINFO;

typedef struct tagDBFHeader
{
	unsigned char		cDBtype;	//00
	unsigned char		cYear;		//01-03 
	unsigned char		cMonth;
	unsigned char		cDay;

	unsigned long		lRecordNum;     // Total record count				//04
	unsigned short		sHeaderBNum;    // sheaderBNum/8 -1 = Field Count	//08
	unsigned short		sRecordBNum;						                //10
	unsigned char		cRes1;          // set 0x00					        //12
	unsigned char		cRes2;          // set 0x00					        //13

	unsigned char		cDBase4Flag;    // set 0x00			                //14
	unsigned char		cEncrypted; // normal visible 0x00, encrypted 0x01	//15
	unsigned char		ucpUserEnv[12];				                        //16

	unsigned char		cIndexed;							                //28
	unsigned char		cCodePage;							                //29
	unsigned char		cRes3; // set 0x00					                //30
	unsigned char		cRes4; // set 0x00					                //31

} TDBF_INFO;


//
//  SHP File 
//
typedef struct tagArcFileHeader {
	int    FileCode;		// 9994     [BIG_ENDIAN]
	int    Unused0;			// 0        [BIG_ENDIAN]
	int    Unused1;			// 0        [BIG_ENDIAN]
	int    Unused2;			// 0        [BIG_ENDIAN]
	int    Unused3;			// 0        [BIG_ENDIAN]
	int    Unused4;			// 0        [BIG_ENDIAN]
	int    FileLength;		//          [BIG_ENDIAN]
	int    Version;			// 1000			[LITTLE_ENDIAN]
	int    ShapeType;		// SHP_OBJ_TYPE [LITTLE_ENDIAN]
	double Xmin;			//              [LITTLE_ENDIAN]
	double Ymin;
	double Xmax;
	double Ymax;
	int    Unused5;
	int    Unused6;
	int    Unused7;
	int    Unused8;
	int    Unused9;
	int    Unused10;
	int    Unused11;
	int    Unused12;
}ARC_FILE_HEADER;

typedef struct tagArcRecordHeader {
	int RecordNumber;    // 1..n                [BIG_ENDIAN]
	int ContentLength;	 // ShpObjectByteSize/2 [BIG_ENDIAN]
}ARC_RECORD_HEADER;

enum SHP_OBJ_TYPE {
	shpNullShape   = 0,
	shpPoint       = 1,
	shpPolyLine    = 3,
	shpPolygon     = 5,
	shpMultiPoint  = 8,
	shpPointZ      = 11,
	shpPolyLineZ   = 13,
	shpPolygonZ    = 15,
	shpMultiPointZ = 18,
	shpPointM      = 21,
	shpPolyLineM   = 23,
	shpPolygonM    = 25,
	shpMultiPointM = 28,
	shpMultiPatch  = 31
};

enum DBF_FIELD_TYPE {
	DBF_FIELD_TYPE_STRING,
	DBF_FIELD_TYPE_NUMERIC,
	DBF_FIELD_TYPE_LOGICAL,
	DBF_FIELD_TYPE_DATE,
	DBF_FIELD_TYPE_INTEGER,
	DBF_FIELD_TYPE_DOUBLE
};

typedef struct tagDBF_FIELD_INFO {
	DBF_FIELD_TYPE nType;
	char           szName[ 11 ];
	int            nSize;
	int            nDec;
}DBF_FIELD_INFO;

//
// Shape Object
//
typedef struct _tagSPoint {
	double x;
	double y;

	_tagSPoint& operator=(const _tagSPoint& rhs) {
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	bool operator==(const _tagSPoint& rhs) const {
		return ((x == rhs.x) && (y == rhs.y)) ? true : false;
	}

	bool operator!=(const _tagSPoint& rhs) const {
		return ((x != rhs.x) || (y != rhs.y)) ? true : false;
	}

	bool equal(const _tagSPoint& rhs) const {
		return (((int)(x * 1000000) == (int)(rhs.x * 1000000)) && ((int)(y * 1000000) == (int)(rhs.y * 1000000))) ? true : false;
	}
	
	bool has(void) const {
		return (x != 0.f && y != 0.f) ? true : false;
	}
}SPoint;

typedef struct _tagSBox {
	double    Xmin;
	double    Ymin;
	double    Xmax;
	double    Ymax;

	_tagSBox& operator=(const _tagSBox& rhs) {
		Xmin = rhs.Xmin; Ymin = rhs.Ymin; Xmax = rhs.Xmax; Ymax = rhs.Ymax;
		return *this;
	}
}SBox;

#pragma warning(disable:4200)			// ignore zero-sized array in struct/union

typedef struct {
	int         ShapeType;
}SHPNullObj;

typedef struct {
	int         ShapeType;
	SPoint      point;
}SHPPoint;

typedef struct {
	int         ShapeType;
	SBox        Box;
	int         NumPoints;
	SPoint      Points[0];
}SHPMultiPoint;

typedef struct {
	int         ShapeType;
	SBox        Box;
	int         NumParts;
	int         NumPoints;
	int         Parts[ 0 ];	
	//	SPoint     *Points;
}SHPPolyLine;

typedef struct {
	int         ShapeType;
	SBox        Box;
	int         NumParts;
	int         NumPoints;
	int         Parts[ 0 ];
	//SPoint     *Points;
}SHPPolygon;

typedef struct {
	int         ShapeType;
	SBox        Box;
	int         NumParts;
	int         NumPoints;
	int         Parts[ 0 ];
	//SPoint     *Points;
}SHPMultiPatch;

typedef struct {
	union {
		SHPNullObj      obj;
		SHPPoint        point;
		SHPMultiPoint   mpoint;
		SHPPolyLine     polyline;
		SHPPolygon      polygon;
		SHPMultiPatch   mpatch;
	};
} SHPGeometry;


#pragma pack( pop )


#if !defined(_WIN32)
int my_stricmp (const char *s1, const char *s2);
#endif

int				SHP_GetGeometrySize( const SHPGeometry *pObj );
int				SHP_GetNumPartPoints( const SHPGeometry *pObj, int nPart );
SPoint		   *SHP_GetPartPoints( const SHPGeometry *pObj, int nPart );
bool			SHP_IsOuterRing( const SHPGeometry *pObj, int nPart );

SHPGeometry	   *SHP_CreatePoint( const SPoint &point );
SHPGeometry	   *SHP_CreateMultiPoint( int numPoint, const SPoint *points );
SHPGeometry	   *SHP_CreatePolyline( const SPoint *lpPoints, const int *lpVtxCounts, int nCount );
SHPGeometry	   *SHP_CreatePolygon(  const SPoint *lpPoints, const int *lpVtxCounts, int nCount );
void            SHP_DestroyGeometry( SHPGeometry *pObj );

class XExtBuffer {
public:
	int   m_nAllocSize;
	char *m_pBuffer;

public:
	XExtBuffer()
	{
		m_nAllocSize = 0;
		m_pBuffer    = NULL;
	}
	virtual ~XExtBuffer()
	{
		if( m_pBuffer ) delete []m_pBuffer;
	}

	char *GetBuffer( int nSize ) 
	{
		if( m_nAllocSize < nSize ) {
			if( m_pBuffer ) delete []m_pBuffer;
			m_pBuffer = new char [nSize];
			m_nAllocSize = nSize;
		}
		return m_pBuffer;
	}
};

class XSHPReader {
	FILE			*m_fpSHP;
	FILE			*m_fpDBF;
	FILE			*m_fpSHX;

	int				m_nPreFetchIdx;
	bool			m_bPreFetchDBF;
	bool			m_bPreFetchSHP;

	int				m_nShpObjByteSize;
	SHPGeometry		*m_ShpObj;
	XExtBuffer       m_ShpBuffer;
	char			*m_DbfRecBuffer;
	XExtBuffer       m_DbfBuffer;

	TDBF_INFO		 m_DbfHeader;
	int              m_DbfFieldCnt;
	TDBF_FIELDINFO  *m_DbfFields;
	ARC_FILE_HEADER	 m_ShpHeader;

	void             FreeMem( void );
	bool             ReadRecordDBF( void );
	SHPGeometry     *ReadRecordSHP( int& nReadObjectSize );

	long				m_lRecordNum;
public:
	char                m_szFileName[ 256 ];

	XSHPReader();
	~XSHPReader();

	bool			Open( const char *fname );
	void			Close();

	SHP_OBJ_TYPE	GetShpObjectType( void );
	void			GetEnvelope( SBox *box );
	int				GetFieldCount( void );
	bool			GetFieldInfo( int col, DBF_FIELD_INFO &FieldInfo );
	bool			RenameField( int col, const char *szFieldName );
	int				GetFieldIdx( const char *szFieldName );

	int				GetRecordCount( void );

	bool			Fetch( long off );
	bool   			GetDataAsString( int col, char *str, int max_len );
	SHPGeometry    *GetGeometry( void ) { return m_ShpObj; };
	int				GetGeometryByteSize( void ) { return m_nShpObjByteSize; };
};


class XSHPWriter {
public:
	FILE	        *m_fpSHP;
	FILE	        *m_fpDBF;
	FILE		    *m_fpSHX;

	int			     m_nRecordCount;

	char			*m_DbfRecBuffer;
	TDBF_INFO		 m_DbfHeader;
	int              m_DbfFieldCnt;
	TDBF_FIELDINFO  *m_DbfFields;

	SHPGeometry 	*m_ShpObj;
	ARC_FILE_HEADER	 m_ShpHeader;
	int				 m_nShpWrittenOffset;
	int              m_nShpTotalSize;

	void             FreeMem( void );
public:
	XSHPWriter();
	~XSHPWriter();

	bool			Create( const char *fname );
	bool			WriteSHPHeader( SHP_OBJ_TYPE shp_obj_type, const SBox *box );
	bool			WriteDBFHeader( int nFieldCnt, DBF_FIELD_INFO *pFieldsInfo );
	void			Close();

	int				GetFieldIdx( const char *szFieldName );
	bool            SetGeometry( SHPGeometry *pObj );
	SHPGeometry    *GetGeometry( void ) { return m_ShpObj; };
	SHP_OBJ_TYPE	GetShpObjectType( void )
	{
		if( !m_fpSHP )
			return shpNullShape;
		return (SHP_OBJ_TYPE)m_ShpHeader.ShapeType;
	}

	bool   	        SetDataAsString( int col, char *str, int max_len );
	bool            WriteRecord( void );
};

class XDBFWriter {
	FILE	        *m_fpDBF;

	int			     m_nRecordCount;

	char			*m_DbfRecBuffer;
	TDBF_INFO		 m_DbfHeader;
	int              m_DbfFieldCnt;
	TDBF_FIELDINFO  *m_DbfFields;

	void             FreeMem( void );
public:
	XDBFWriter();
	~XDBFWriter();

	bool			Create( const char *fname );
	bool			WriteDBFHeader( int nFieldCnt, DBF_FIELD_INFO *pFieldsInfo );
	void			Close();

	bool   	        SetDataAsString( int col, char *str, int max_len );
	bool            WriteRecord( void );
};

class XDBFReader {
	FILE			*m_fpDBF;

	int				m_nPreFetchIdx;

	char			*m_DbfRecBuffer;
	XExtBuffer       m_DbfBuffer;

	TDBF_INFO		 m_DbfHeader;
	int              m_DbfFieldCnt;
	TDBF_FIELDINFO  *m_DbfFields;

	void             FreeMem( void );
	bool             ReadRecordDBF( void );

public:
	XDBFReader();
	~XDBFReader();

	bool			Open( const char *fname );
	void			Close();

	int				GetFieldCount( void );
	bool			GetFieldInfo( int col, DBF_FIELD_INFO &FieldInfo );
	int				GetFieldIdx( const char *szFieldName );

	bool			RenameField( int col, const char *szFieldName );
	int				GetRecordCount( void );

	bool			Fetch( long off );
	bool   			GetDataAsString( int col, char *str, int max_len );
};

#endif
