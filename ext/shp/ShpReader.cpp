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

inline int BIG2LITTLE_INT( int a ) { return ((a>>24)&0x000000FF) | ((a>>8)&0x0000FF00) | ((a<<8)&0x00FF0000) | ((a<<24)&0xFF000000); }

XSHPReader::XSHPReader()
{
	m_fpSHP = NULL;
	m_fpDBF = NULL;
	m_fpSHX = NULL;

	m_nPreFetchIdx = -2;
	m_ShpObj       = NULL;
	m_DbfRecBuffer = NULL;
	m_DbfFields    = NULL;
	m_DbfFieldCnt  = 0;

	m_lRecordNum	= -1;
	m_bPreFetchSHP = false;
	m_bPreFetchDBF = false;

	m_szFileName[0] = '\0';
}


XSHPReader::~XSHPReader()
{
	Close();
}

void XSHPReader::FreeMem( void )
{
	if( m_DbfFields    ) delete []m_DbfFields;
	m_DbfFields    = NULL;
	m_DbfFieldCnt  = 0;
}

bool XSHPReader::Open(const char *fname )
{
	Close();

#if defined(_WIN32)
	char	cpDrive[ _MAX_DRIVE ], cpPath[ _MAX_DIR ], cpFileName[ _MAX_FNAME ], cpExt[ _MAX_EXT ];
	char    szName[_MAX_PATH];
	_splitpath( fname, cpDrive, cpPath, cpFileName, cpExt );
	sprintf(szName, "%s%s%s.shp", cpDrive, cpPath, cpFileName);
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
#endif
	
	m_fpSHP = fopen(szName, "rb");
	strcpy(m_szFileName, szName);
	
	if (m_fpSHP == NULL)
	{
		// FullPath가 아닐지도 모른다.
		// 현재 폴더에서 읽어보자.
// 		::GetModuleFileName(NULL, szName, _MAX_PATH);
// 		_splitpath(szName, cpDrive, cpPath, NULL, NULL);
// 
// 		sprintf( szName, "%s%s%s", cpDrive, cpPath, fname );
// 		m_fpSHP = fopen( szName, "rb" );
// 		strcpy( m_szFileName, szName );

#if defined(_WIN32)
		char *pStr;
		pStr = strrchr(szName, '.');
		strcpy(pStr, ".dbf");
		m_fpDBF = fopen( szName,  "rb" );

		strcpy(pStr, ".shx");
		m_fpSHX = fopen( szName,  "rb" );
#else
		sprintf(szName, "%s%s.dbf", szPath, szFile);
		m_fpDBF = fopen(szName, "rb");
		
		sprintf(szName, "%s%s.shx", szPath, szFile);
		m_fpSHX = fopen(szName, "rb");
#endif
	}
	else
	{
#if defined(_WIN32)		
		sprintf( szName, "%s%s%s.dbf", cpDrive, cpPath, cpFileName );
		m_fpDBF = fopen( szName,  "rb" );
		
		sprintf( szName, "%s%s%s.shx", cpDrive, cpPath, cpFileName );
		m_fpSHX = fopen( szName,  "rb" );
#else
		sprintf(szName, "%s%s.dbf", szPath, szFile);
		m_fpDBF = fopen(szName, "rb");
		
		sprintf(szName, "%s%s.shx", szPath, szFile);
		m_fpSHX = fopen(szName, "rb");
#endif		
	}


	/*  if( m_fpSHP==NULL || m_fpDBF==NULL || m_fpSHX==NULL )
	{
	Close();
	return false;
	}
	*/
	// By jilee 20031202 : : DBF 파일 없을때 읽게 하기 위해
	if( m_fpSHP==NULL && m_fpDBF==NULL && m_fpSHX==NULL )
	{
		Close();
		return false;
	}

	// .SHP Read Header
	if( m_fpSHP != NULL )
		fread( &m_ShpHeader, sizeof(ARC_FILE_HEADER), 1, m_fpSHP );

	// .DBF Read Header
	if( m_fpDBF != NULL ) {
		fread( &m_DbfHeader, sizeof(TDBF_INFO), 1, m_fpDBF );
		m_DbfFieldCnt = m_DbfHeader.sHeaderBNum / 32 - 1;
		m_DbfFields = new TDBF_FIELDINFO[ m_DbfFieldCnt ];
		//		fread( m_DbfFields, sizeof(TDBF_FIELDINFO), m_DbfFieldCnt, m_fpDBF );

		// { 20041202 by jilee
		int fidx = 0;
		for( int i=0; i<m_DbfFieldCnt; i++ ) {
			if( fread( &m_DbfFields[ fidx ], sizeof(TDBF_FIELDINFO), 1, m_fpDBF )==0 )
				return false;
			if( m_DbfFields[ fidx ].cpName[0] == 0x0D )
				break;
			fidx++;
		}
		// } 20041202 byjilee

		char chEOH;
		fread( &chEOH, 1, 1, m_fpDBF );
		fseek( m_fpDBF, m_DbfHeader.sHeaderBNum, SEEK_SET );

		m_DbfRecBuffer = m_DbfBuffer.GetBuffer( m_DbfHeader.sRecordBNum );		

		m_lRecordNum = m_DbfHeader.lRecordNum;
	}

	m_nPreFetchIdx = -1;

	Fetch( 0 );

	return true;
}

void XSHPReader::Close()
{
	FreeMem();

	if( m_fpSHP ) fclose( m_fpSHP );
	if( m_fpDBF ) fclose( m_fpDBF );
	if( m_fpSHX ) fclose( m_fpSHX );
	m_fpSHP = NULL;
	m_fpDBF = NULL;
	m_fpSHX = NULL;
	m_nPreFetchIdx = -2;
	m_ShpObj       = NULL;
}

SHP_OBJ_TYPE  XSHPReader::GetShpObjectType( void )
{
	if( !m_fpSHP )
		return shpNullShape;

	return (SHP_OBJ_TYPE)m_ShpHeader.ShapeType;
}

void XSHPReader::GetEnvelope( SBox *box )
{
	box->Xmin = m_ShpHeader.Xmin;
	box->Xmax = m_ShpHeader.Xmax;
	box->Ymin = m_ShpHeader.Ymin;
	box->Ymax = m_ShpHeader.Ymax;
}

int  XSHPReader::GetFieldCount( void )
{
	if( !m_fpSHP )
		return 0;
	return m_DbfFieldCnt;
}

bool XSHPReader::GetFieldInfo( int col, DBF_FIELD_INFO &FieldInfo )
{
	if( !m_fpSHP || col<0 || col>=GetFieldCount() )
		return false;

	switch( m_DbfFields[ col ].cType ) {
		case 'C' : FieldInfo.nType = DBF_FIELD_TYPE_STRING;  break;
		case 'D' : FieldInfo.nType = DBF_FIELD_TYPE_DATE;    break;
		case 'L' : FieldInfo.nType = DBF_FIELD_TYPE_LOGICAL; break;
		case 'B' : FieldInfo.nType = DBF_FIELD_TYPE_DOUBLE; break;
		case 'F' : 
		case 'N' : FieldInfo.nType = DBF_FIELD_TYPE_NUMERIC; break;
		case 'I' : FieldInfo.nType = DBF_FIELD_TYPE_INTEGER; break;
		default:
			assert( false );
	}
	strcpy( FieldInfo.szName, m_DbfFields[ col ].cpName );
	FieldInfo.nSize = m_DbfFields[ col ].cLength;
	FieldInfo.nDec  = m_DbfFields[ col ].cDec;

	return true;
}

int	XSHPReader::GetFieldIdx( const char *szFieldName )
{
	if( !m_fpSHP )
		return -1;

	int c;
	for( c=0; c<GetFieldCount(); c++ )
	{
#if defined(_WIN32)
		if( _stricmp( m_DbfFields[ c].cpName, szFieldName )==0 )
#else
		if( my_stricmp( m_DbfFields[ c].cpName, szFieldName )==0 )
#endif		
			return c;
	}
	return -1;
}

bool XSHPReader::RenameField( int col, const char *szFieldName )
{
	strcpy( m_DbfFields[ col ].cpName, szFieldName  );
	return true;
}

int XSHPReader::GetRecordCount( void )
{
	if( !m_fpSHP ) 
	{
		return 0;
	}

	if( m_lRecordNum >= 0 )
	{
		return m_lRecordNum;
	}

	long lCurPos = ftell( m_fpSHP );

	fseek( m_fpSHP, sizeof(ARC_FILE_HEADER), SEEK_SET );

	ARC_RECORD_HEADER rec_header;

	while( true )
	{
		if( fread( &rec_header, sizeof( ARC_RECORD_HEADER ), 1, m_fpSHP ) == 0 )
		{
			break;
		}
		rec_header.RecordNumber		= BIG2LITTLE_INT( rec_header.RecordNumber );
		rec_header.ContentLength	= BIG2LITTLE_INT( rec_header.ContentLength );

		// Kslee@20050627 - validation 추가 
		if (rec_header.RecordNumber < 0)
		{
			break;
		}
		m_lRecordNum = rec_header.RecordNumber;

		fseek( m_fpSHP, rec_header.ContentLength*2, SEEK_CUR );
	}

	fseek( m_fpSHP, lCurPos, SEEK_SET );

	return m_lRecordNum;
}

bool XSHPReader::ReadRecordDBF( void )
{
	if( m_bPreFetchDBF ) {
		do {
			if( fread( m_DbfRecBuffer, m_DbfHeader.sRecordBNum, 1, m_fpDBF )==0 )
				return false;
		}while( m_DbfRecBuffer[0]==0x2A );
		m_bPreFetchDBF = false;
	}

	return true;
}

SHPGeometry *XSHPReader::ReadRecordSHP( int& nReadObjectSize )
{
	SHPGeometry *pObject = NULL;

	if( !m_bPreFetchSHP ) {
		pObject = (SHPGeometry *) m_ShpBuffer.GetBuffer( 0 );
		nReadObjectSize = m_ShpBuffer.m_nAllocSize;
		return pObject;
	}
	else {

		ARC_RECORD_HEADER rec_header;
		if( fread( &rec_header, sizeof( ARC_RECORD_HEADER ), 1, m_fpSHP )==0 )
			return NULL;
		rec_header.ContentLength = BIG2LITTLE_INT( rec_header.ContentLength );

		pObject = (SHPGeometry *) m_ShpBuffer.GetBuffer( rec_header.ContentLength*2 );
		if( fread( pObject, rec_header.ContentLength*2, 1, m_fpSHP ) == 0 )
			return NULL;
		nReadObjectSize = rec_header.ContentLength*2;
		m_bPreFetchSHP = false;
	}

	return pObject;
}

// 2003년 1월 20일...
// DBF 가 없으면 SHP 못 읽는다..
// 그래서, 수정했당.

bool XSHPReader::Fetch( long off )
{
	m_nShpObjByteSize = 0;

	if( off<0 || off>=GetRecordCount() )
		return false;

	//	if( m_nPreFetchIdx!=(off-1) ) {
	if( m_nPreFetchIdx != off ) {
		if( m_fpSHP ) {
			if( fseek( m_fpSHX, (50+off*4)*2, SEEK_SET ) )
				return false;

			long shp_off, shp_size;
			if( fread( &shp_off,  1, sizeof(long), m_fpSHX ) == 0 )
				return false;
			//			fread( &shp_size, 1, sizeof(long), m_fpSHP );
			if( fread( &shp_size, 1, sizeof(long), m_fpSHX ) == 0 )
				return false;
			shp_off = BIG2LITTLE_INT( shp_off ) * 2;
			if( fseek( m_fpSHP, shp_off, SEEK_SET ) )
				return false;
			m_bPreFetchSHP = true;
		}

		if( m_fpDBF ) {
			long dbf_off = m_DbfHeader.sHeaderBNum + off * m_DbfHeader.sRecordBNum;
			fseek( m_fpDBF, dbf_off, SEEK_SET );
			m_bPreFetchDBF = true;
		}
	}

	bool bRetReadDBF = ReadRecordDBF();

	m_ShpObj = ReadRecordSHP(m_nShpObjByteSize);
	if( m_ShpObj == NULL || bRetReadDBF == false )
	{
		m_nShpObjByteSize = 0;
		return false;
	}

	m_nPreFetchIdx = off;

	return true;
}

bool XSHPReader::GetDataAsString( int col, char *str, int max_len )
{
	if( col<0|| col>=GetFieldCount() )
		return false;

	int off = 1;
	int i=0 ;
	for( i=0; i<col; i++ ) 
		off += m_DbfFields[i].cLength;
	int flen      = m_DbfFields[col].cLength + 1;

	int iSize = m_DbfFields[col].cLength;

	switch( m_DbfFields[i].cType ) {
		case 'N' :
			/*			if( flen < max_len ) {
			strncpy( str, &m_DbfRecBuffer[ off ], flen );
			str[ flen ] = '\0';
			}else {
			off += (flen-max_len);
			strncpy( str, &m_DbfRecBuffer[ off ], max_len );
			str[ max_len-1 ] = '\0';
			}
			*/			strncpy (str, &m_DbfRecBuffer[ off ], iSize );
			str [iSize ] = '\0';
			break;
		case 'I' :
			{
				long v = *((long*)(&m_DbfRecBuffer[ off ]));
				sprintf( str,  "%d", v );
			}
			break;
		case 'B' :
			{
				double v = *((double*)(&m_DbfRecBuffer[ off ]));
				sprintf( str,  "%f", v );
			}
			break;
		case 'C' :
		case 'F' :
		default  :
			{
				int copy_len = (flen<max_len)?flen:max_len;
				strncpy( str, &m_DbfRecBuffer[ off ], copy_len );
				str[ copy_len-1 ] = '\0';

				// Remove LastSpace
				for( int idx=copy_len-2; idx>=0; idx-- ) {
					if( str[idx]==' ' ) str[idx] = '\0';
					else                break;
				}
			}
	}

	return true;	
}

//
// DBF Reader
//

XDBFReader::XDBFReader()
{
	m_fpDBF = NULL;

	m_nPreFetchIdx = -2;
	m_DbfRecBuffer = NULL;
	m_DbfFields    = NULL;
	m_DbfFieldCnt  = 0;
}

XDBFReader::~XDBFReader()
{
	Close();
}

void XDBFReader::FreeMem( void )
{
	if( m_DbfFields    ) delete []m_DbfFields;
	m_DbfFields    = NULL;
	m_DbfFieldCnt  = 0;
}

bool XDBFReader::Open( const char *fname )
{
	Close();

#if defined(_WIN32)
	char	cpDrive[ _MAX_DRIVE ], cpPath[ _MAX_DIR ], cpFileName[ _MAX_FNAME ], cpExt[ _MAX_EXT ];
	char    szName[_MAX_PATH];
	_splitpath( fname, cpDrive, cpPath, cpFileName, cpExt );
	sprintf(szName, "%s.dbf", fname);
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
#endif

	m_fpDBF = fopen( szName,  "rb" );
	if( m_fpDBF==NULL ) {
		Close();
		return false;
	}

	// .DBF Read Header
	fread( &m_DbfHeader, sizeof(TDBF_INFO), 1, m_fpDBF );
	m_DbfFieldCnt = m_DbfHeader.sHeaderBNum / 32 - 1;
	m_DbfFields = new TDBF_FIELDINFO[ m_DbfFieldCnt ];
	int fidx = 0;
	for( int i=0; i<m_DbfFieldCnt; i++ ) {
		if( fread( &m_DbfFields[ fidx ], sizeof(TDBF_FIELDINFO), 1, m_fpDBF )==0 )
			return false;
		if( m_DbfFields[ fidx ].cpName[0] == 0x0D )
			break;
		fidx++;
	}
	m_DbfFieldCnt = fidx;
	fseek( m_fpDBF, m_DbfHeader.sHeaderBNum, SEEK_SET );

	m_DbfRecBuffer = m_DbfBuffer.GetBuffer( m_DbfHeader.sRecordBNum );

	m_nPreFetchIdx = -1;

	Fetch( 0 );

	return true;
}

void XDBFReader::Close()
{
	FreeMem();

	if( m_fpDBF ) fclose( m_fpDBF );
	m_fpDBF = NULL;
	m_nPreFetchIdx = -2;
}

int  XDBFReader::GetFieldCount( void )
{
	if( !m_fpDBF )
		return 0;
	return m_DbfFieldCnt;
}
int	XDBFReader::GetFieldIdx( const char *szFieldName )
{
	if( !m_fpDBF )
		return -1;

	int c;
	for( c=0; c<GetFieldCount(); c++ )
	{
#if defined(_WIN32)
		if( _stricmp( m_DbfFields[ c].cpName, szFieldName )==0 )
#else
		if( my_stricmp( m_DbfFields[ c].cpName, szFieldName )==0 )
#endif		
			return c;
	}
	return -1;
}

bool XDBFReader::GetFieldInfo( int col, DBF_FIELD_INFO &FieldInfo )
{
	if( !m_fpDBF || col<0 || col>=GetFieldCount() )
		return false;

	strcpy( FieldInfo.szName, m_DbfFields[ col ].cpName );
	FieldInfo.nSize = m_DbfFields[ col ].cLength;
	FieldInfo.nDec  = m_DbfFields[ col ].cDec;
	switch( m_DbfFields[ col ].cType ) {
		case 'C' : FieldInfo.nType = DBF_FIELD_TYPE_STRING;  break;
		case 'D' : FieldInfo.nType = DBF_FIELD_TYPE_DATE;    break;
		case 'L' : FieldInfo.nType = DBF_FIELD_TYPE_LOGICAL; break;
		case 'F' : 
		case 'N' : FieldInfo.nType = DBF_FIELD_TYPE_NUMERIC; break;
		case 'I' : FieldInfo.nType = DBF_FIELD_TYPE_INTEGER; break;
		case 'B' : FieldInfo.nType = DBF_FIELD_TYPE_DOUBLE;  break;
			break;
		default:
			assert( false );
	}

	return true;
}

bool XDBFReader::RenameField( int col, const char *szFieldName )
{
	strcpy( m_DbfFields[ col ].cpName, szFieldName  );
	return true;
}

int XDBFReader::GetRecordCount( void )
{
	if( !m_fpDBF )
		return 0;
	return m_DbfHeader.lRecordNum;
}

bool XDBFReader::ReadRecordDBF( void )
{
	do {
		if( fread( m_DbfRecBuffer, m_DbfHeader.sRecordBNum, 1, m_fpDBF )==0 )
			return false;
	}while( m_DbfRecBuffer[0]==0x2A );

	return true;
}

bool XDBFReader::Fetch( long off )
{
	if( off<0 || off>=GetRecordCount() )
		return false;

	if( m_nPreFetchIdx!=(off-1) ) {
		long dbf_off = m_DbfHeader.sHeaderBNum + off * m_DbfHeader.sRecordBNum;
		fseek( m_fpDBF, dbf_off, SEEK_SET );
	}

	if( ReadRecordDBF()==false )
		return false;

	m_nPreFetchIdx = off;

	return true;
}

bool XDBFReader::GetDataAsString( int col, char *str, int max_len )
{
	if( col<0|| col>=GetFieldCount() )
		return false;

	int off = 1;
	int i=0 ;
	for( i=0; i<col; i++ ) 
		off += m_DbfFields[i].cLength;

	int flen      = m_DbfFields[col].cLength + 1;
	switch( m_DbfFields[i].cType ) {
		case 'N' :
			/*
			if( flen < max_len ) {
			strncpy( str, &m_DbfRecBuffer[ off ], flen );
			str[ flen-1 ] = '\0';
			}else {
			off += (flen-max_len);
			strncpy( str, &m_DbfRecBuffer[ off ], max_len );
			str[ max_len-1 ] = '\0';
			}
			*/
			{
				int copy_len = (flen<max_len)?flen:max_len;
				strncpy (str, &m_DbfRecBuffer[ off ], copy_len );
				str [copy_len-1 ] = '\0';
			}
			break;
		case 'I' :
			{
				long v = *((long*)(&m_DbfRecBuffer[ off ]));
				sprintf( str,  "%d", v );
			}
			break;
		case 'B' :
			{
				double v = *((double*)(&m_DbfRecBuffer[ off ]));
				sprintf( str,  "%f", v );
			}
			break;
		case 'C' :
		case 'F' :
		default  :
			{
				int copy_len = (flen<max_len)?flen:max_len;
				strncpy( str, &m_DbfRecBuffer[ off ], copy_len );
				str[ copy_len-1 ] = '\0';

				// Remove LastSpace
				for( int idx=copy_len-2; idx>=0; idx-- ) {
					if( str[idx]==' ' ) str[idx] = '\0';
					else                break;
				}
			}
	}

	return true;	
}

