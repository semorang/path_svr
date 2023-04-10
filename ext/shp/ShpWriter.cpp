#if defined(_WIN32)
#include "../stdafx.h"
#include <windows.h>
#else
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdio.h>
#include <assert.h>
#include "shpio.h"

// #ifdef _DEBUG
// #define new DEBUG_NEW
// #undef THIS_FILE
// static char THIS_FILE[] = __FILE__;
// #endif

XSHPWriter::XSHPWriter()
{
	m_fpSHP = NULL;
	m_fpDBF = NULL;
	m_fpSHX = NULL;

	m_nRecordCount		= 0;
	m_ShpObj			= NULL;

	m_DbfRecBuffer		= NULL;
	m_DbfFieldCnt		= 0;
	m_DbfFields         = NULL;

	m_ShpObj			= NULL;
	m_nShpWrittenOffset = 50;
	m_nShpTotalSize     = 0;
}

XSHPWriter::~XSHPWriter()
{
	Close();
}

void XSHPWriter::FreeMem( void )
{
	if( m_DbfRecBuffer ) delete []m_DbfRecBuffer;
	m_DbfRecBuffer		= NULL;
	if( m_DbfFields    ) delete []m_DbfFields;
	m_DbfFields         = NULL;

	m_nRecordCount		= 0;
	m_ShpObj			= NULL;

	m_DbfFieldCnt		= 0;

	m_ShpObj			= NULL;
	m_nShpWrittenOffset = 50;
	m_nShpTotalSize     = 0;
}

bool XSHPWriter::Create( const char *fname )
{
	Close();

#if defined(_WIN32)
	char	cpDrive[_MAX_DRIVE];
	char	cpPath[_MAX_DIR];
	char	cpFileName[_MAX_FNAME];
	char	cpExt[_MAX_EXT];

	_splitpath( fname, cpDrive, cpPath, cpFileName, cpExt );

	char    szName[_MAX_PATH];

	sprintf( szName, "%s%s%s.shp", cpDrive, cpPath, cpFileName );
	m_fpSHP = fopen( szName, "wb" );

	sprintf( szName, "%s%s%s.dbf", cpDrive, cpPath, cpFileName );
	m_fpDBF = fopen( szName, "wb" );

	sprintf( szName, "%s%s%s.shx", cpDrive, cpPath, cpFileName );
	m_fpSHX = fopen( szName, "wb" );
#else
	char szPath[MAX_PATH], szFile[MAX_PATH], szName[MAX_PATH];
	char *cpPath, *cpFile, *cpExt;

	strcpy(szPath, fname);
	strcpy(szFile, fname);

	cpPath = dirname(szPath);
	cpFile = basename(szFile);

	if (!cpPath || cpFile) {
		return false;
	}

	cpExt = strchr(cpFile, '.');
	if (cpExt) {
		szFile[cpExt - cpFile] = '\0';
	}
	sprintf(szName, "%s%s.shp", szPath, szFile);
	m_fpSHP = fopen( szName, "wb" );

	sprintf(szName, "%s%s.dbf", szPath, szFile);
	m_fpDBF = fopen( szName, "wb" );

	sprintf(szName, "%s%s.shx", szPath, szFile);
	m_fpSHX = fopen( szName, "wb" );
#endif	

	if( (m_fpSHP == NULL) || (m_fpDBF == NULL) || (m_fpSHX == NULL) ) 
	{
		Close();
		return false;
	}

	m_nRecordCount		= 0;
	m_nShpWrittenOffset	= 50;
	m_nShpTotalSize     = 0;

	return true;
}

inline int BIG2LITTLE_INT( int a ) { return ((a>>24)&0x000000FF) | ((a>>8)&0x0000FF00) | ((a<<8)&0x00FF0000) | ((a<<24)&0xFF000000); }
inline int LITTLE2BIG_INT( int a ) { return ((a>>24)&0x000000FF) | ((a>>8)&0x0000FF00) | ((a<<8)&0x00FF0000) | ((a<<24)&0xFF000000); }

void XSHPWriter::Close()
{
	if( m_fpSHP ) 
	{
		fseek( m_fpSHP, 0, SEEK_END );

		long	lLength = ftell( m_fpSHP )/2;

		m_ShpHeader.FileLength = LITTLE2BIG_INT( lLength );
		fseek( m_fpSHP, 0, SEEK_SET );
		fwrite( &m_ShpHeader, sizeof (ARC_FILE_HEADER), 1, m_fpSHP );
		fclose( m_fpSHP );
	}
	if( m_fpDBF ) 
	{
		char cpEndFile = 0x1A;
		fwrite( &cpEndFile, 1, 1, m_fpDBF );

		fseek( m_fpDBF, 4, SEEK_SET );
		fwrite( &m_nRecordCount, sizeof (long), 1, m_fpDBF );
		fclose( m_fpDBF );
	}
	if( m_fpSHX ) 
	{
		fseek( m_fpSHX, 0, SEEK_END );

		long	lLength = ftell( m_fpSHX )/2;

		m_ShpHeader.FileLength = LITTLE2BIG_INT( lLength );
		fseek( m_fpSHX, 0, SEEK_SET );
		fwrite( &m_ShpHeader, sizeof (ARC_FILE_HEADER), 1, m_fpSHX );
		fclose( m_fpSHX );
	}
	m_fpSHP = NULL;
	m_fpDBF = NULL;
	m_fpSHX = NULL;

	FreeMem();
}


bool XSHPWriter::WriteSHPHeader( SHP_OBJ_TYPE shp_obj_type, const SBox *box )
{
	memset (&m_ShpHeader, 0, sizeof(ARC_FILE_HEADER));

	m_ShpHeader.FileCode     = 9994;
	m_ShpHeader.FileCode     = LITTLE2BIG_INT (m_ShpHeader.FileCode);
	m_ShpHeader.FileLength   = 100;					// default value , this value will be edited later.
	m_ShpHeader.FileLength   = LITTLE2BIG_INT (m_ShpHeader.FileLength);
	m_ShpHeader.Version      = 1000;
	m_ShpHeader.ShapeType    = shp_obj_type;
	m_ShpHeader.Xmin		 = box->Xmin;
	m_ShpHeader.Ymin		 = box->Ymin;
	m_ShpHeader.Xmax		 = box->Xmax;
	m_ShpHeader.Ymax		 = box->Ymax;

	fseek(m_fpSHP, 0, SEEK_SET);
	if (fwrite( &m_ShpHeader, sizeof (ARC_FILE_HEADER), 1, m_fpSHP) == 0) {
		fseek(m_fpSHP, 0, SEEK_END);
		return false;
	}

	fseek(m_fpSHX, 0, SEEK_SET);
	if (fwrite( &m_ShpHeader, sizeof (ARC_FILE_HEADER), 1, m_fpSHX) == 0) {
		fseek(m_fpSHX, 0, SEEK_END);
		return false; 

	}

	fseek(m_fpSHP, 0, SEEK_END);
	fseek(m_fpSHX, 0, SEEK_END);

	return true;
}

bool XSHPWriter::WriteDBFHeader( int nFieldCnt, DBF_FIELD_INFO *pFieldsInfo )
{
	m_DbfFieldCnt = nFieldCnt;
	m_DbfFields   = new TDBF_FIELDINFO[ m_DbfFieldCnt ];

	int i;
	for( i=0; i<m_DbfFieldCnt; i++ ) 
	{
		memset( &m_DbfFields[i], 0x00, sizeof(m_DbfFields[i]) );

		strncpy( m_DbfFields[i].cpName, pFieldsInfo[i].szName, 10 );

		m_DbfFields[i].cpName[10] = '\0';

		switch( pFieldsInfo[i].nType ) 
		{
		case DBF_FIELD_TYPE_STRING :
			m_DbfFields[i].cType        = 'C';
			m_DbfFields[i].cLength      = pFieldsInfo[i].nSize;
			m_DbfFields[i].cDec         = 0;
			break;
		case DBF_FIELD_TYPE_INTEGER :
			m_DbfFields[i].cType        = 'N';
			m_DbfFields[i].cLength      = 16;
			m_DbfFields[i].cDec         = 0;
			break;
		case DBF_FIELD_TYPE_DOUBLE :
			m_DbfFields[i].cType        = 'N';
			m_DbfFields[i].cLength      = 20;
			m_DbfFields[i].cDec         = 5;
			break;
		case DBF_FIELD_TYPE_NUMERIC :
			m_DbfFields[i].cType        = 'N';
			m_DbfFields[i].cLength      = pFieldsInfo[i].nSize;
			if( pFieldsInfo[i].nSize>m_DbfFields[i].cDec )
				m_DbfFields[i].cDec         = pFieldsInfo[i].nDec;
			else
				m_DbfFields[i].cDec         = pFieldsInfo[i].nDec-pFieldsInfo[i].nSize;
			break;
		case DBF_FIELD_TYPE_LOGICAL :
			m_DbfFields[i].cType        = 'L';
			m_DbfFields[i].cLength      = 1;
			m_DbfFields[i].cDec         = 0;
			break;
		case DBF_FIELD_TYPE_DATE     :
			m_DbfFields[i].cType        = 'D';
			m_DbfFields[i].cLength      = 8;
			m_DbfFields[i].cDec         = 0;
			break;
		}
	}

	int	iRecSize = 0;
	for( i=0; i<nFieldCnt; i++ )
		iRecSize += m_DbfFields[i].cLength;

	memset( &m_DbfHeader, 0x00, sizeof(TDBF_INFO) );
	m_DbfHeader.cDBtype      = 0x03;
	m_DbfHeader.cYear	     = 98;		            //01-03 
	m_DbfHeader.cMonth       = 8;
	m_DbfHeader.cDay		 = 15;
	m_DbfHeader.lRecordNum   = 0;					//04 , after written
	m_DbfHeader.sHeaderBNum  = 32 + nFieldCnt*32+1; //08
	m_DbfHeader.sRecordBNum  = iRecSize+1;			//10

	if( fwrite( &m_DbfHeader, 32, 1, m_fpDBF ) == 0 )
		return false;

	m_DbfRecBuffer = new char [m_DbfHeader.sRecordBNum+2];
	memset( m_DbfRecBuffer, 0x20, m_DbfHeader.sRecordBNum );

	if( fwrite( m_DbfFields, sizeof(TDBF_FIELDINFO), nFieldCnt, m_fpDBF) == 0 )
		return false;

	char cpEndField = 0X0D;
	if( fwrite (&cpEndField,1,1,m_fpDBF) == 0 )
		return false;


	return true;
}

int	XSHPWriter::GetFieldIdx( const char *szFieldName )
{
	int c;
	for( c=0; c<m_DbfFieldCnt; c++ )
	{
#if defined(_WIN32)
		if( _stricmp( m_DbfFields[ c].cpName, szFieldName )==0 )
			return c;
#else
		if( my_stricmp( m_DbfFields[ c].cpName, szFieldName )==0 )
			return c;
#endif
	}
	return -1;
}

bool XSHPWriter::SetDataAsString( int col, char *str, int max_len )
{
	if( col<0 || col>=m_DbfFieldCnt )
		return false;

	int off = 1;
	int i=0;
	for( i=0; i<col; i++ ) 
		off += m_DbfFields[i].cLength;

	int flen      = m_DbfFields[col].cLength;
	int slen      = strlen( str );

	if( m_DbfFields[i].cType == 'N' ) {
		double val = atof( str );
		char fmt[ 256+1 ];
		sprintf( fmt, "%%%d.%df", m_DbfFields[i].cLength, m_DbfFields[i].cDec );
		char val_str[ 256+1 ];
		sprintf( val_str, fmt, val );
		int slen      = strlen( val_str );
		int copy_len = (slen < flen) ? slen:flen;
		memcpy( &m_DbfRecBuffer[off], val_str, copy_len );
	}else if( m_DbfFields[i].cType == 'C' ) {
		int copy_len = (slen < flen) ? slen:flen;
		memcpy( &m_DbfRecBuffer[off], str, copy_len );
	}else if( m_DbfFields[i].cType == 'D' ) {
		memcpy( &m_DbfRecBuffer[off], str, flen );
	}else if( m_DbfFields[i].cType == 'L' ) {
		char bool_char[ 10 ] = { "YyTtNnFf?" };
		if( strchr( bool_char, str[0] ) ) m_DbfRecBuffer[off] = str[0];
		else                              m_DbfRecBuffer[off] = 'F';
	}

	return true;	
}

bool XSHPWriter::SetGeometry( SHPGeometry *pObj )
{
	m_ShpObj = pObj;

	if( m_ShpHeader.ShapeType==shpNullShape )
		m_ShpHeader.ShapeType = pObj->obj.ShapeType;

	return true;
}

bool XSHPWriter::WriteRecord( void )
{
	//assert( m_ShpObj );
	SHPGeometry SHPObj;

	m_nRecordCount++;

	// DBF Record Write
	fwrite( m_DbfRecBuffer, m_DbfHeader.sRecordBNum, 1, m_fpDBF );
	memset( m_DbfRecBuffer, 0x20, m_DbfHeader.sRecordBNum );

	// SHP Record Write
	int  nShpObjSize   = SHP_GetGeometrySize( m_ShpObj );
	if (nShpObjSize == 0)
	{
		SHPObj.point.ShapeType = 1;
		SHPObj.point.point.x = SHPObj.point.point.y = 0;
		m_ShpObj = &SHPObj;
		nShpObjSize = sizeof(SHPPoint);
	}

	long ContentLength = nShpObjSize / 2;
	long RecordNumber  = m_nRecordCount + 1;
	ContentLength = LITTLE2BIG_INT( ContentLength );
	RecordNumber  = LITTLE2BIG_INT( RecordNumber  );
	if( fwrite( &RecordNumber,  sizeof( long ), 1, m_fpSHP )==0 )
		return false;
	if( fwrite( &ContentLength, sizeof( long ), 1, m_fpSHP )==0 )
		return false;
	if( fwrite( m_ShpObj, nShpObjSize, 1, m_fpSHP )==0 )
		return false;
	assert( m_ShpObj->obj.ShapeType==shpPoint    || 
		m_ShpObj->obj.ShapeType==shpPolyLine ||
		m_ShpObj->obj.ShapeType==shpPolygon );

	// SHX Record Write
	long wOffset = LITTLE2BIG_INT( m_nShpWrittenOffset );
	if( fwrite( &wOffset, sizeof(long), 1, m_fpSHX )==0 )
		return false;
	if( fwrite( &(ContentLength), sizeof(long), 1, m_fpSHX )==0 )
		return false;

	m_nShpWrittenOffset += (nShpObjSize/2+4);
	m_nShpTotalSize     += (nShpObjSize/2);
//	m_nRecordCount++;

	int off = ftell( m_fpSHP );
	//	assert( (off/2)==m_nShpWrittenOffset );

	return true;
}

//
// DBF Writer
//
XDBFWriter::XDBFWriter()
{
	m_fpDBF = NULL;

	m_nRecordCount		= 0;

	m_DbfRecBuffer		= NULL;
	m_DbfFieldCnt		= 0;
	m_DbfFields         = NULL;
}

XDBFWriter::~XDBFWriter()
{
	Close();
}

void XDBFWriter::FreeMem( void )
{
	if( m_DbfRecBuffer ) delete []m_DbfRecBuffer;
	m_DbfRecBuffer		= NULL;
	if( m_DbfFields    ) delete []m_DbfFields;
	m_DbfFields         = NULL;

	m_nRecordCount		= 0;

	m_DbfFieldCnt		= 0;
}

bool XDBFWriter::Create( const char *fname )
{
	Close();

#if defined(_WIN32)
	char	cpDrive[ _MAX_DRIVE ], cpPath[ _MAX_DIR ], cpFileName[ _MAX_FNAME ], cpExt[ _MAX_EXT ];
	_splitpath( fname, cpDrive, cpPath, cpFileName, cpExt );
	char    szName[ _MAX_PATH ];

	sprintf( szName, "%s%s%s.dbf", cpDrive, cpPath, cpFileName );
#else
	char szPath[MAX_PATH], szFile[MAX_PATH], szName[MAX_PATH];
	char *cpPath, *cpFile, *cpExt;

	strcpy(szPath, fname);
	strcpy(szFile, fname);

	cpPath = dirname(szPath);
	cpFile = basename(szFile);

	if (!cpPath || cpFile) {
		return false;
	}

	cpExt = strchr(cpFile, '.');
	if (cpExt) {
		szFile[cpExt - cpFile] = '\0';
	}
	sprintf(szName, "%s%s.dbf", szPath, szFile);
#endif

	m_fpDBF = fopen( szName, "wb" );
	if( m_fpDBF==NULL ) {
		Close();
		return false;
	}

	m_nRecordCount		= 0;

	return true;
}

void XDBFWriter::Close()
{
	if( m_fpDBF ) {
		char cpEndFile = 0x1A;
		fwrite( &cpEndFile, 1, 1, m_fpDBF );

		fseek( m_fpDBF, 4, SEEK_SET );
		fwrite( &m_nRecordCount, sizeof (long), 1, m_fpDBF );
		fclose( m_fpDBF );
	}
	m_fpDBF = NULL;

	FreeMem();
}

bool XDBFWriter::WriteDBFHeader( int nFieldCnt, DBF_FIELD_INFO *pFieldsInfo )
{
	m_DbfFieldCnt = nFieldCnt;
	m_DbfFields   = new TDBF_FIELDINFO[ m_DbfFieldCnt ];
	int i;
	for( i=0; i<m_DbfFieldCnt; i++ ) {
		memset( &m_DbfFields[i], 0x00, sizeof(m_DbfFields[i]) );
		strncpy( m_DbfFields[i].cpName, pFieldsInfo[i].szName, 11 );
		m_DbfFields[i].cpName[ 10 ] = '\0';
		switch( pFieldsInfo[i].nType ) {
			case DBF_FIELD_TYPE_STRING :
				m_DbfFields[i].cType        = 'C';
				m_DbfFields[i].cLength      = pFieldsInfo[i].nSize;
				m_DbfFields[i].cDec         = 0;
				break;
			case DBF_FIELD_TYPE_INTEGER :
				m_DbfFields[i].cType        = 'N';
				m_DbfFields[i].cLength      = 16;
				m_DbfFields[i].cDec         = 0;
				break;
			case DBF_FIELD_TYPE_DOUBLE  :
				m_DbfFields[i].cType        = 'N';
				m_DbfFields[i].cLength      = 20;
				m_DbfFields[i].cDec         = 5;
				break;
			case DBF_FIELD_TYPE_NUMERIC :
				m_DbfFields[i].cType        = 'N';
				m_DbfFields[i].cLength      = pFieldsInfo[i].nSize;
				if( pFieldsInfo[i].nSize>m_DbfFields[i].cDec )
					m_DbfFields[i].cDec         = pFieldsInfo[i].nDec;
				else
					m_DbfFields[i].cDec         = pFieldsInfo[i].nDec-pFieldsInfo[i].nSize;
				break;
			case DBF_FIELD_TYPE_LOGICAL :
				m_DbfFields[i].cType        = 'L';
				m_DbfFields[i].cLength      = 1;
				m_DbfFields[i].cDec         = 0;
				break;
			case DBF_FIELD_TYPE_DATE     :
				m_DbfFields[i].cType        = 'D';
				m_DbfFields[i].cLength      = 8;
				m_DbfFields[i].cDec         = 0;
				break;
		}
	}

	int	iRecSize = 0;
	for( i=0; i<nFieldCnt; i++ )
		iRecSize += m_DbfFields[i].cLength;

	memset( &m_DbfHeader, 0x00, sizeof(TDBF_INFO) );
	m_DbfHeader.cDBtype      = 0x03;
	m_DbfHeader.cYear	     = 01;		            //01-03 
	m_DbfHeader.cMonth       = 8;
	m_DbfHeader.cDay		 = 15;
	m_DbfHeader.lRecordNum   = 0;					//04 , after written
	m_DbfHeader.sHeaderBNum  = 32 + nFieldCnt*32+1; //08
	m_DbfHeader.sRecordBNum  = iRecSize+1;			//10

	if( fwrite( &m_DbfHeader, 32, 1, m_fpDBF ) == 0 )
		return false;

	if( fwrite( m_DbfFields, sizeof(TDBF_FIELDINFO), nFieldCnt, m_fpDBF) == 0 )
		return false;

	char cpEndField = 0X0D;
	if( fwrite (&cpEndField,1,1,m_fpDBF) == 0 )
		return false;

	m_DbfRecBuffer = new char [m_DbfHeader.sRecordBNum+2];
	memset( m_DbfRecBuffer, 0x20, m_DbfHeader.sRecordBNum );
	return true;
}

bool XDBFWriter::SetDataAsString( int col, char *str, int max_len )
{
	if( col<0 || col>=m_DbfFieldCnt )
		return false;

	int off = 1;
	int i=0;
	for( i=0; i<col; i++ ) 
		off += m_DbfFields[i].cLength;

	int flen      = m_DbfFields[col].cLength;
	int slen      = strlen( str );

	if( m_DbfFields[i].cType == 'N' ) {
		if( m_DbfFields[i].cDec == 0 )
		{
			if( m_DbfFields[i].cLength > 7 )
			{
				char temp[ 256+1 ];
#if defined(_WIN32)				
				__int64 val = _atoi64( str );
				_i64toa( val, temp, 10 );
#else
				// int64_t val = std::stoull( str );
				int64_t val = strtoll(str, NULL, 10);
				sprintf(temp, "%lld", val);
#endif

				char val_str[ 256+1 ];
				int j;
				for( j=0; j<255; j++ )
					val_str[j] = ' ';
				strcpy( &val_str[ m_DbfFields[i].cLength - strlen( temp ) ], temp );
				val_str[ m_DbfFields[i].cLength ] = '\0';
				int slen      = strlen( val_str );
				int copy_len = (slen < flen) ? slen:flen;
				memcpy( &m_DbfRecBuffer[off], val_str, copy_len );
			}
			else
			{
				int val = atoi( str );
				char fmt[ 256+1 ];
				sprintf( fmt, "%%%dd", m_DbfFields[i].cLength );
				char val_str[ 256+1 ];
				sprintf( val_str, fmt, val );
				int slen      = strlen( val_str );
				int copy_len = (slen < flen) ? slen:flen;
				memcpy( &m_DbfRecBuffer[off], val_str, copy_len );
			}

		}
		else
		{
			double val = atof( str );
			char fmt[ 256+1 ];
			sprintf( fmt, "%%%d.%df", m_DbfFields[i].cLength, m_DbfFields[i].cDec );
			char val_str[ 256+1 ];
			sprintf( val_str, fmt, val );
			int slen      = strlen( val_str );
			int copy_len = (slen < flen) ? slen:flen;
			memcpy( &m_DbfRecBuffer[off], val_str, copy_len );
		}
	}else if( m_DbfFields[i].cType == 'C' ) {
		int copy_len = (slen < flen) ? slen:flen;
		memcpy( &m_DbfRecBuffer[off], str, copy_len );
	}else if( m_DbfFields[i].cType == 'D' ) {
		memcpy( &m_DbfRecBuffer[off], str, flen );
	}else if( m_DbfFields[i].cType == 'L' ) {
		char bool_char[ 10 ] = { "YyTtNnFf?" };
		if( strchr( bool_char, str[0] ) ) m_DbfRecBuffer[off] = str[0];
		else                              m_DbfRecBuffer[off] = 'F';
	}

	return true;	
}

bool XDBFWriter::WriteRecord( void )
{
	// DBF Record Write
	fwrite( m_DbfRecBuffer, m_DbfHeader.sRecordBNum, 1, m_fpDBF );
	memset( m_DbfRecBuffer, 0x20, m_DbfHeader.sRecordBNum );

	m_nRecordCount++;

	return true;
}

