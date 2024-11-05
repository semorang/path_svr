#pragma once
class stiMapFileDef {
public:
	enum { TR_TYPE_IDIO = 0, TR_TYPE_REAL, TR_TYPE_TPEG, TR_TYPE_TCON };
public:
	/// index file
	//static const char* F_TR_ID_DARC0;
	//static const char* F_TR_ID_DARC1;
	//static const char* F_TR_INDEX_DARC0;
	//static const char* F_TR_INDEX_DARC1;
	//static const char* F_TR_INDEX_DARC2;

	//static const char* F_TR_ID_ROTIS0;
	//static const char* F_TR_ID_ROTIS1;
	//static const char* F_TR_INDEX_ROTIS0;
	//static const char* F_TR_INDEX_ROTIS1;
	//static const char* F_TR_INDEX_ROTIS2;

	//static const char* F_TR_ID_TPEG0;
	//static const char* F_TR_ID_TPEG1;
	//static const char* F_TR_INDEX_TPEG0;
	//static const char* F_TR_INDEX_TPEG1;
	static const char* F_TR_INDEX_TPEG2;

	/// id file
	//static const char* F_TR_ID_DARC2;
	//static const char* F_TR_ID_ROTIS2;
	//static const char* F_TR_ID_TPEG2;
};





class ttlMapFileDef {
public:
	// index file
	//static const char* F_TR_ID_TTL0;
	//static const char* F_TR_ID_TTL1;
	//static const char* F_TR_INDEX_TTL0;
	//static const char* F_TR_INDEX_TTL1;
	static const char* F_TR_INDEX_TTL2;

	/// id file
	//static const char* F_TR_ID_TTL2;
};
