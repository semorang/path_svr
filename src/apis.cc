#if defined(_WIN32)
#include <windows.h>
#include <wchar.h>
#define _WINDOWS
#else
#include <iconv.h>
#endif

#define USE_LIBJSON
// #define USE_RAPIDJSON
// #define USE_TAOJSON
#define USE_CJSON

#include <node.h>
// #include <nan.h>
// #include <v8.h>

#include <string.h>
#include <time.h>
#if defined(USE_LIBJSON)
#if defined(USE_CJSON)
#include "../ext/libjson/cjson/cJSON.h"
#elif defined(USE_RAPIDJSON)
#define RAPIDJSON_HAS_STDSTRING 1
#include "../ext/libjson/rapidjson/document.h"
#include "../ext/libjson/rapidjson/writer.h"
#include "../ext/libjson/rapidjson/stringbuffer.h"
#include "../ext/libjson/rapidjson/prettywriter.h"
using namespace rapidjson;
#elif defined(USE_TAOJSON)
#include "../ext/libjson/tao/json.hpp"
#else
#include <json/json.h>
#endif
#endif

#include "../ext/utils/UserLog.h"
#include "../ext/cvt/FileManager.h"
#include "../ext/route/RouteManager.h"
#include "../ext/route/DataManager.h"




CFileManager m_pFileMgr;
CDataManager m_pDataMgr;
CRouteManager m_pRouteMgr;

namespace open_route_api {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Array;
using v8::String;
using v8::Value;
using v8::Number;
using v8::Integer;
using v8::Context;
using v8::Boolean;


template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}


// const char* get_result_string(const int code)
// {
//    static string msg;

//    switch(code) {
//    case ROUTE_RESULT_SUCCESS:
//       msg = "성공";
//       break;
//    case ROUTE_RESULT_FAILED:
//       msg = "탐색 실패(내부 오류에 의한 실패)";
//       break;
//    case ROUTE_RESULT_FAILED_SAME_ROUTE:
//       msg = "스마트 재탐색 적용(기존 경로와 동일)";
//    case ROUTE_RESULT_FAILED_WRONG_PARAM:
//       msg = "잘못된 파라미터(필수 파라미터 체크)";
//       break;
//    case ROUTE_RESULT_FAILED_SET_MEMORY:
//       msg = "탐색 확장 관련 메모리 할당 오류(탐색 초기화 관련)";
//       break;
//    case ROUTE_RESULT_FAILED_READ_DATA:
//       msg = "탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패(탐색 초기화 관련)";
//       break;
//    case ROUTE_RESULT_FAILED_SET_START:
//       msg = "출발지가 프로젝션이 안되거나, 잘못된 출발지";
//       break;
//    case ROUTE_RESULT_FAILED_SET_VIA:
//       msg = "경유지가 프로젝션이 안되거나, 잘못된 경유지";
//       break;
//    case ROUTE_RESULT_FAILED_SET_END:
//       msg = "목적지가 프로젝션이 안되거나, 잘못된 목적지";
//       break;
//    case ROUTE_RESULT_FAILED_DIST_OVER:
//       msg = "탐색 가능 거리 초과(직선거리 5km 이내 허용)";
//       break;
//    case ROUTE_RESULT_FAILED_TIME_OVER:
//       msg = "탐색 시간 초과(10초 이상)";
//       break;
//    case ROUTE_RESULT_FAILED_NODE_OVER:
//       msg = "확장 가능 Node 개수 초과";
//       break;
//    case ROUTE_RESULT_FAILED_EXPEND:
//       msg = "확장 실패";
//       break;
//    default:
//       msg = "실패(알수 없는 오류)";
//       break;
//     }


//    return msg.c_str();
// }

// void GetResultString(const FunctionCallbackInfo<Value>& args)
// {
//    Isolate* isolate = args.GetIsolate();
//    Local<Context> context = isolate->GetCurrentContext();
//    Local<Object> obj = Object::New(isolate);
//    v8::MaybeLocal<v8::String> msg;

//    int code = -1;

//    if (args.Length() < 1) {
//       LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

//       msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
//    }
//    else {
//       code = args[0].As<Number>()->Value();

//       msg = String::NewFromUtf8(isolate, get_result_string(code));
//    }

//    obj->Set(context, String::NewFromUtf8(isolate, "code").ToLocalChecked(), Number::New(isolate, code));
//    obj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), msg.ToLocalChecked());

//    args.GetReturnValue().Set(obj);
// }



#if defined(_WIN32)
void MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode)
{
#ifdef _WINDOWS
	int nLen = MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), strUnicode, nLen);
#else
	mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
}

void UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8)
{
#ifdef _WINDOWS
	int nLen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUTF8, nLen, NULL, NULL);
#else
	wcstombs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
}

void MultiByteToUTF8(const char* strMultibyte, char* strUTF8)
{
   wchar_t wszUnicode[MAX_PATH] = {0,};
   
   MultiByteToUnicode(strMultibyte, wszUnicode);

   UnicodeToUTF8(wszUnicode, strUTF8);
}

#else //#if defined(_WIN32)

char * encoding(const char *text_input, char *source, char *target)
{
   iconv_t it;

   int input_len = strlen(text_input) + 1;
   int output_len = input_len*2;

   size_t in_size = input_len;
   size_t out_size = output_len;

   char *output = (char *)malloc(output_len);

   char *output_buf = output;
   char *input_buf = const_cast<char*>(text_input);

   it = iconv_open(target, source); 
   int ret = iconv(it, &input_buf, &in_size, &output_buf, &out_size);


   iconv_close(it);

   return output;
}
#endif //#if defined(_WIN32)


void LogOut(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   // char test[] = "test node-gyp, hello node-gyp";
   // args.GetReturnValue().Set(String::NewFromUtf8(isolate, test).ToLocalChecked());


   String::Utf8Value str(isolate, args[0]);
   std::string strInput (*str);
   LOG_TRACE(LOG_DEBUG, strInput.c_str());
   // arg.GetReturnValue().Set()
}


void Init(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   std::string strLogPath;
   std::string strLogFile;
   std::string strDataPath;
   int nPid = 0;

   if (args.Length() > 0) {
      nPid = args[0].As<Number>()->Value();
      LOG_SET_ID(nPid);
   }
   if (args.Length() > 1) {      
      String::Utf8Value str(isolate, args[1]);
      strDataPath = *str;
   }
   if (args.Length() > 2) {      
      String::Utf8Value str(isolate, args[2]);
      strLogPath = *str;
   }

#if defined(USE_OPTIMAL_POINT_API)
      strLogFile = "log_optimal";
#elif defined(USE_PEDESTRIAN_DATA)
      strLogFile = "log_pedestrian";
#else
      strLogFile = "log_trekking";
#endif

   if (!strLogPath.empty()) {
      LOG_SET_FILEPATH(strLogPath.c_str(), strLogFile.c_str());
      LOG_TRACE(LOG_DEBUG, "Init log : %s, %s", strLogPath.c_str(), strLogFile.c_str());  
   }

   if (strDataPath.empty()) {
#if defined(_WIN32)
#  if defined(USE_OPTIMAL_POINT_API)
      strDataPath = "C:/__Data/optimal";
#  elif defined(USE_PEDESTRIAN_DATA)
      strDataPath = "C:/__Data/walk_route/data";
#  else
      strDataPath = "C:/__Data/trekking";
#  endif
#else
#  if defined(USE_OPTIMAL_POINT_API)
      strDataPath = "/home/ubuntu/optimal_svr/data";
#  elif defined(USE_PEDESTRIAN_DATA)
      strDataPath = "/home/ubuntu/walk_route/data";
#  else
      strDataPath = "/home/ubuntu/trecking/data";
#  endif
#endif
  
      LOG_TRACE(LOG_DEBUG, "User data path not defined, will set default : %s", strDataPath.c_str());
   }
   else {
      LOG_TRACE(LOG_DEBUG, "Init path : %s", strDataPath.c_str());
   }


	m_pDataMgr.Initialize();
	m_pFileMgr.Initialize();
	m_pRouteMgr.Initialize();

   // m_pFileMgr.SetCacheCount(100);
   m_pDataMgr.SetFileMgr(&m_pFileMgr);
	m_pFileMgr.SetDataMgr(&m_pDataMgr);
   m_pFileMgr.LoadData(strDataPath.c_str());
	m_pRouteMgr.SetDataMgr(&m_pDataMgr);
}


void GetVersion(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> mainobj = Object::New(isolate);

   Local<Object> version = Object::New(isolate);

   bool checked = false;

   // get engine version
   const char* pVersion = m_pDataMgr.GetVersionString((uint32_t)-1);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "engine").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
   }


   // get datas version
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_NAME);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "name").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_MESH);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "mesh").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_TREKKING);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "trekking").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_PEDESTRIAN);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "pedestrian").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_VEHICLE);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "vehicle").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_BUILDING);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "building").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }
   pVersion = m_pDataMgr.GetVersionString(TYPE_DATA_COMPLEX);
   if (pVersion != nullptr && strlen(pVersion) > 0) {
      version->Set(context, String::NewFromUtf8(isolate, "complex").ToLocalChecked(), String::NewFromUtf8(isolate, pVersion).ToLocalChecked());
      checked = true;
   }


   if (checked != true) {
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, 1));
      mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "can't find datas").ToLocalChecked());
   } else {
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, 0));
      mainobj->Set(context, String::NewFromUtf8(isolate, "version").ToLocalChecked(), version);
   }


   args.GetReturnValue().Set(mainobj);
}


/*
string GetTypeName(const uint64_t sub_info, const char* strType) {
   string strName;

   if (sub_info > 0 && strType) {
      stLinkBaseInfo* pBaseInfo = (stLinkBaseInfo*)sub_info;

      // 보행자 데이터
      if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
         stLinkPedestrianInfo* pPedInfo = (stLinkPedestrianInfo*)sub_info;

         if (strcmp(strType, "FTYPE") == 0) { // facility_type // 시설물 타입
            switch(pPedInfo->facility_type) {
               case 1:          
               strName =  "토끼굴";
               break;

               case 2:
               strName = "지하보도";
               break;

               case 3:
               strName = "육교";
               break;

               case 4:
               strName = "고가도로";
               break;

               case 5:
               strName = "교량";
               break;

               case 6:
               strName = "지하철역";
               break;

               case 7:
               strName = "철도";
               break;

               case 8:
               strName = "중앙버스정류장";
               break;

               case 9:
               strName = "지하상가";
               break;

               case 10:
               strName = "건물관통도로";
               break;

               case 11:
               strName = "단지도로_공원";
               break;

               case 12:
               strName = "단지도로_주거시설";
               break;

               case 13:
               strName = "단지도로_관광지";
               break;

               case 14:
               strName = "단지도로_기타";
               break;

               default:
               strName = "미정의";
               break;
            }// switch
         } // if (strcmp(strType, "FTYPE") == 0)
         else if (strcmp(strType, "GTYPE") == 0) { // gate_type // 진입로 타입
            switch(pPedInfo->gate_type) {
               case 1:
               strName = "경사로";
               break;

               case 2:
               strName = "계단";
               break;

               case 3:
               strName = "에스컬레이터";
               break;

               case 4:
               strName = "계단/에스컬레이터";
               break;

               case 5:
               strName = "엘리베이터";
               break;

               case 6:
               strName = "단순연결로";
               break;

               case 7:
               strName = "횡단보도";
               break;

               case 8:
               strName = "무빙워크";
               break;

               case 9:
               strName = "징검다리";
               break;

               case 10:
               strName = "의사횡단";
               break;

               default:
               strName = "미정의";
               break;
            }// switch
         } // else if (strcmp(strType, "GTYPE") == 0)
      } //  if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN)
   } // if (sub_info > 0 && strType)

   return strName;
}
*/

void SetDeparture(const FunctionCallbackInfo<Value>& args) {
   // Isolate* isolate = args.GetIsolate();
   // double lng = To<double>(info[0]).FromJust();//->NumberValue();
   // double lng = info.GetReturnValue().Set(Number::New(isolate, info[0]));

   if (args.Length() < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      double lng = args[0].As<Number>()->Value();
      double lat = args[1].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Set departure lng:%f, lat:%f", lng, lat);

      bool useOptimalPoint = false;
      if (cnt >= 2) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }     

      if (useOptimalPoint == true) {
         stOptimalPointInfo optInfo;
         if (m_pDataMgr.GetOptimalPointDataByPoint(lng, lat, &optInfo) > 0) {
            lng = optInfo.vtEntryPoint[0].x;
            lat = optInfo.vtEntryPoint[0].y;

            LOG_TRACE(LOG_DEBUG, "Set optimal departure lng:%f, lat:%f", lng, lat);
         }
      }

#if defined(USE_P2P_DATA)
      m_pRouteMgr.SetDeparture(lng, lat, TYPE_LINK_MATCH_FOR_HD);
#else
      m_pRouteMgr.SetDeparture(lng, lat);
#endif
   }
}

void SetWaypoint(const FunctionCallbackInfo<Value>& args) {
   // Isolate* isolate = args.GetIsolate();
   // double lng = To<double>(info[0]).FromJust();//->NumberValue();
   // double lng = info.GetReturnValue().Set(Number::New(isolate, info[0]));

   if (args.Length() < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      double lng = args[0].As<Number>()->Value();
      double lat = args[1].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Set waypoint lng:%f, lat:%f", lng, lat);

      bool useOptimalPoint = false;
      if (cnt >= 2) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }     

      if (useOptimalPoint == true) {
         stOptimalPointInfo optInfo;
         if (m_pDataMgr.GetOptimalPointDataByPoint(lng, lat, &optInfo) > 0) {
            lng = optInfo.vtEntryPoint[0].x;
            lat = optInfo.vtEntryPoint[0].y;

            LOG_TRACE(LOG_DEBUG, "Set optimal waypoint lng:%f, lat:%f", lng, lat);
         }
      }

#if defined(USE_P2P_DATA)
      m_pRouteMgr.SetWaypoint(lng, lat, TYPE_LINK_MATCH_FOR_HD);
#else
      m_pRouteMgr.SetWaypoint(lng, lat);
#endif
   }
}

void SetDestination(const FunctionCallbackInfo<Value>& args) {
   // Isolate* isolate = args.GetIsolate();
   // double lng = To<double>(info[0]).FromJust();//->NumberValue();
   // double lng = info.GetReturnValue().Set(Number::New(isolate, info[0]));

   if (args.Length() < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);
   }
   else {
      int cnt = args.Length();
      LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      double lng = args[0].As<Number>()->Value();
      double lat = args[1].As<Number>()->Value();
      // int opt = args[2].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Set destination lng:%f, lat:%f", lng, lat);

      bool useOptimalPoint = false;
      if (cnt >= 2) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }     

      if (useOptimalPoint == true) {
         stOptimalPointInfo optInfo;
         if (m_pDataMgr.GetOptimalPointDataByPoint(lng, lat, &optInfo) > 0) {
            lng = optInfo.vtEntryPoint[0].x;
            lat = optInfo.vtEntryPoint[0].y;

            LOG_TRACE(LOG_DEBUG, "Set optimal destination lng:%f, lat:%f", lng, lat);
         }
      }

#if defined(USE_P2P_DATA)
      m_pRouteMgr.SetDestination(lng, lat, TYPE_LINK_MATCH_FOR_HD);
#else
      m_pRouteMgr.SetDestination(lng, lat);
#endif
   }
}

void DoRoute(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   LOG_TRACE(LOG_DEBUG, "Start routing.");

   if (args.Length() < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      int opt = args[0].As<Number>()->Value();
      int avoid = args[1].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Route Opt:%d, Avoid:%d", opt, avoid);

      m_pRouteMgr.SetRouteOption(opt, avoid);
      ret = m_pRouteMgr.Route();

      if (ret == 0) {
         LOG_TRACE(LOG_DEBUG, "Success routing.");
         msg = String::NewFromUtf8(isolate, "Success routing");
      }
      else {
         LOG_TRACE(LOG_DEBUG, "Failed routing.");
         msg = String::NewFromUtf8(isolate, "Failed routing");
      }
   }

   // v8::MaybeLocal<v8::String> retVal = Number::New(isolate, ret);
   // Integer intVal = new Integer(ret);
   // Local<Value> retVal2 = Local<Value>::Cast(String::NewFromUtf8(isolate, "0").ToLocalChecked());

   // obj->Set(context, String::NewFromUtf8(isolate, "result"), ret);
   obj->Set(context, String::NewFromUtf8(isolate, "result").ToLocalChecked(), Integer::New(isolate, ret));
   obj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), msg.ToLocalChecked());

   args.GetReturnValue().Set(obj);
}


void ReleaseRoute(const FunctionCallbackInfo<Value>& args) {
   LOG_TRACE(LOG_DEBUG, "Route release");
   
   m_pRouteMgr.Release();
   m_pRouteMgr.Initialize();
}


void GetRouteSummary(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> mainobj = Object::New(isolate);
   int cnt = 0;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();

   if (pResult == nullptr) {
      LOG_TRACE(LOG_ERROR, "Error, Route result pointer null");
      mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, Route result pointer null").ToLocalChecked());
   }
   else {
      // info
      Local<Object> routes = Object::New(isolate);
      mainobj->Set(context, String::NewFromUtf8(isolate, "user_id").ToLocalChecked(), Number::New(isolate, pResult->RequestId));
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, pResult->ResultCode));

      // result
      Local<Object> start_coord = Object::New(isolate);
      start_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.x));
      start_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "start").ToLocalChecked(), start_coord);
      Local<Object> end_coord = Object::New(isolate);
      end_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.x));
      end_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "end").ToLocalChecked(), end_coord);
      routes->Set(context, String::NewFromUtf8(isolate, "option").ToLocalChecked(), Integer::New(isolate, pResult->RouteOption));
      // dist
      routes->Set(context, String::NewFromUtf8(isolate, "distance").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkDist));
      // time
      routes->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkTime));
      time_t timer = time(NULL);
      struct tm* tmNow = localtime(&timer);
      string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "now").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      timer += pResult->TotalLinkTime;
      tmNow = localtime(&timer);
      strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "eta").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "summary").ToLocalChecked(), routes);
   }

   args.GetReturnValue().Set(mainobj);
}


void GetRouteResult(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> mainobj = Object::New(isolate);
   int cnt = 0;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();

   if (pResult == nullptr) {
      LOG_TRACE(LOG_ERROR, "Error, route result pointer null");
      mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, route result pointer null").ToLocalChecked());
   }
   else {
      // info
      Local<Object> routes = Object::New(isolate);
      mainobj->Set(context, String::NewFromUtf8(isolate, "user_id").ToLocalChecked(), Number::New(isolate, pResult->RequestId));
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, pResult->ResultCode));

      // result
      Local<Object> start_coord = Object::New(isolate);
      start_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.x));
      start_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "start").ToLocalChecked(), start_coord);
      Local<Object> end_coord = Object::New(isolate);
      end_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.x));
      end_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "end").ToLocalChecked(), end_coord);
      routes->Set(context, String::NewFromUtf8(isolate, "option").ToLocalChecked(), Integer::New(isolate, pResult->RouteOption));
      //dist
      routes->Set(context, String::NewFromUtf8(isolate, "distance").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkDist));
      //time
      routes->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkTime));
      time_t timer = time(NULL);
      struct tm* tmNow = localtime(&timer);
      string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "now").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      timer += pResult->TotalLinkTime;
      tmNow = localtime(&timer);
      strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "eta").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "summary").ToLocalChecked(), routes);


      // link
      Local<Array> link = Array::New(isolate);
      Local<Object> link_list = Object::New(isolate);
      cnt = pResult->LinkInfo.size();
      for(int ii=0; ii<cnt; ii++) {
         Local<Object> idoff = Object::New(isolate);
         idoff->Set(context, String::NewFromUtf8(isolate, "linkid").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].link_id.nid));
         idoff->Set(context, String::NewFromUtf8(isolate, "length").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].length));
         idoff->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].time));
         idoff->Set(context, String::NewFromUtf8(isolate, "angle").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].angle));
         idoff->Set(context, String::NewFromUtf8(isolate, "vertex_offset").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].vtx_off));
         idoff->Set(context, String::NewFromUtf8(isolate, "vertex_count").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].vtx_cnt));
         idoff->Set(context, String::NewFromUtf8(isolate, "remain_distance").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].rlength));
         idoff->Set(context, String::NewFromUtf8(isolate, "remain_time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].rtime));

         // 링크 부가 정보 
         uint64_t sub_info = pResult->LinkInfo[ii].link_info;
         if (sub_info > 0) {
            stLinkBaseInfo* pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);
            
            // 보행자 데이터
            if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
               stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
               idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, pPedInfo->facility_type));
               idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, pPedInfo->gate_type));
            }
            else {
               idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, 9));
               idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, 9));
            }
         }
         link->Set(context, ii, idoff);
      }
      link_list->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, cnt));
      link_list->Set(context, String::NewFromUtf8(isolate, "links").ToLocalChecked(), link);

      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "guide").ToLocalChecked(), link_list);


      // vertex
      Local<Array> coords = Array::New(isolate);
      Local<Object> vertex_list = Object::New(isolate);
      cnt = pResult->LinkVertex.size();
      for(auto ii=0; ii<cnt; ii++) {
         Local<Object> lnglat = Object::New(isolate);
         lnglat->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[ii].x));
         lnglat->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[ii].y));
         coords->Set(context, ii, lnglat);

         // Local<Array> lnglat = Array::New(isolate);
         // lnglat->Set(context, 0, Number::New(isolate, pResult->LinkVertex[ii].x));
         // lnglat->Set(context, 1, Number::New(isolate, pResult->LinkVertex[ii].y));
         // coords->Set(context, ii, lnglat);
      }
      // vertex_list->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), String::NewFromUtf8(isolate, "LineString").ToLocalChecked());
      vertex_list->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, cnt));
      vertex_list->Set(context, String::NewFromUtf8(isolate, "coords").ToLocalChecked(), coords);

      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "routes").ToLocalChecked(), vertex_list);
   }

   args.GetReturnValue().Set(mainobj);
}


void GetMultiRouteResultForiNavi(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();
   Local<Object> mainobj = Object::New(isolate);

   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();
   string strJson;

   int result_code = ROUTE_RESULT_FAILED;
   string str_msg = "";

#if defined(USE_CJSON)

   cJSON* root = cJSON_CreateObject();

   if (pResult == nullptr) {
      result_code = ROUTE_RESULT_FAILED;
      str_msg = "failed";
   } else if (pResult->ResultCode != ROUTE_RESULT_SUCCESS) {
      result_code = pResult->ResultCode;
      str_msg = "failed";
   } else {
      result_code = ROUTE_RESULT_SUCCESS;
      str_msg = "success";
         
      cJSON* routes = cJSON_CreateArray();
      cJSON* data = cJSON_CreateObject();
      
      cJSON_AddItemToObject(data, "option", cJSON_CreateNumber(pResult->RouteOption));
      cJSON_AddItemToObject(data, "spend_tiem", cJSON_CreateNumber(pResult->TotalLinkTime));
      cJSON_AddItemToObject(data, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
      cJSON_AddItemToObject(data, "toll_fee", cJSON_CreateNumber(0));
      cJSON_AddItemToObject(data, "taxiFare", cJSON_CreateNumber(0));
      cJSON_AddItemToObject(data, "isHighWay", cJSON_CreateBool(false));

      // 경로 정보 (Array)
      cJSON* paths = cJSON_CreateArray();
      stLinkInfo* pLink = nullptr;
      stLinkVehicleInfo vehInfo;
      char szBuff[MAX_PATH] = {0,};

      for(const auto& link : pResult->LinkInfo) {
         // 경로 링크 정보
         cJSON* path = cJSON_CreateObject();

         // 경로선 (Array)
         cJSON* coords = cJSON_CreateArray();
         int vtxOffset = link.vtx_off;
         int vtxCount = link.vtx_cnt;

         memcpy(&vehInfo, &link.link_info, sizeof(vehInfo));

         for(int ii=0; ii < vtxCount; ii++) {
            cJSON* coord = cJSON_CreateObject();
            cJSON_AddItemToObject(coord, "x", cJSON_CreateNumber(pResult->LinkVertex[vtxOffset + ii].x));
            cJSON_AddItemToObject(coord, "y", cJSON_CreateNumber(pResult->LinkVertex[vtxOffset + ii].y));

            // add coord to coords
            cJSON_AddItemToArray(coords, coord);
         } // for

         // add coords to path
         cJSON_AddItemToObject(path, "coords", coords);

         // speed
         cJSON_AddNumberToObject(path, "speed", 0);

         // time
         cJSON_AddNumberToObject(path, "spend_time", link.time);

         // distance
         cJSON_AddNumberToObject(path, "distance", link.length);

         // road_code
         cJSON_AddNumberToObject(path, "road_code", vehInfo.road_type);

         // road_name
         if (pLink->name_idx > 0) {
#if defined(_WIN32)
               char szUTF8[MAX_PATH] = {0,};
               MultiByteToUTF8(m_pDataMgr.GetNameDataByIdx(pLink->name_idx), szUTF8);
               cJSON_AddStringToObject(p2p, "road_name", szUTF8);
#else
               cJSON_AddStringToObject(p2p, "road_name", encoding(m_pDataMgr.GetNameDataByIdx(pLink->name_idx), "euc-kr", "utf-8"));
#endif // #if defined(_WIN32)

         // traffic_color
         cJSON_AddStringToObject(path, "traffic_color", "green");

#if defined(USE_P2P_DATA) // P2P HD 매칭을 위한 SD 링크 ID 정보
         // p2p 추가정보
         cJSON* p2p = cJSON_CreateObject();

         pLink = m_pDataMgr.GetVLinkDataById(link.link_id);
         if (pLink != nullptr && link.link_id.llid != NULL_VALUE) {

            // speed 재설정
            cJSON_SetNumberHelper(cJSON_GetObjectItem(path, "speed"), pLink->veh.speed);
            
            // hd matching link id
            // LOG_TRACE(LOG_DEBUG, "tile:%d, id:%d, snode:%d, enode:%d",pLink->link_id.tile_id, pLink->link_id.nid, pLink->snode_id.nid, pLink->enode_id.nid);
            sprintf(szBuff, "%d%06d%06d", pLink->link_id.tile_id, pLink->snode_id.nid, pLink->enode_id.nid);
            // cJSON_AddNumberToObject(p2p, "link_id", (pLink->link_id.tile_id * 1000000000000) + (pLink->snode_id.nid * 1000000) + pLink->enode_id.nid); // 원본 ID 사용, (snode 6자리 + enode 6자리)
            cJSON_AddStringToObject(p2p, "link_id", szBuff);
            
            // dir, 0:정방향, 1:역방향
            cJSON_AddNumberToObject(p2p, "dir", link.dir);
            }
         }

         // add p2p to path
         cJSON_AddItemToObject(path, "p2p_extend", p2p);
#endif // #if 1 defined(USE_P2P_DATA)

         // add path to paths
         cJSON_AddItemToArray(paths, path);

      } // for paths

      // add paths to data
      cJSON_AddItemToObject(data, "paths", paths);
      cJSON_AddItemToArray(routes, data);


      // add header to root
      cJSON_AddNumberToObject(root, "user_id", pResult->RequestId);
      cJSON_AddNumberToObject(root, "result_code", result_code);
      cJSON_AddStringToObject(root, "error_msg", str_msg.c_str());

      cJSON_AddItemToObject(root, "routes", routes);

      strJson = cJSON_Print(root);

      cJSON_Delete(root);
   }

#elif defined(USE_TAOJSON)
   tao::json::value root = json::empty_object;

   tao::json::value header = {
      {"isSuccessful", false},
      {"resultCode", ROUTE_RESULT_FAILED},
      {"resultMessage", "Error, route result pointer null"}
   };

   root.emplace("header", header);

   strJson = tao::json::to_string(root); 

#elif defined(USE_RAPIDJSON)
   Document doc(kObjectType);
   Document::AllocatorType& allocator = doc.GetAllocator();

   // Value header("header", allocator);
   // Value val(72);
   // doc.AddMember(hader, val, allocator);
   
      doc.AddMember("isSuccessful", false, allocator);
      doc.AddMember("resultCode", ROUTE_RESULT_FAILED, allocator);
      doc.AddMember("resultMessage", 0, allocator);

   StringBuffer buffer;
   Writer<StringBuffer> writer(buffer);
   doc.Accept(writer);
   strJson = buffer.GetString();  
#endif

   // add route to root
   if (!strJson.empty()) {
      // mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetMultiRouteResult(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();
   Local<Object> mainobj = Object::New(isolate);

   int target = 0;
   int cnt = 0;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();

 if (args.Length() >= 1) {
      cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      target = args[0].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Route target:%d", target);
   }

   if (pResult == nullptr) {
      LOG_TRACE(LOG_ERROR, "Error, Route result pointer null");

      // header
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, ROUTE_RESULT_FAILED));
      mainobj->Set(context, String::NewFromUtf8(isolate, "error_msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, route result pointer null").ToLocalChecked());
  }
   else {
      // header
      mainobj->Set(context, String::NewFromUtf8(isolate, "user_id").ToLocalChecked(), Number::New(isolate, pResult->RequestId));
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, pResult->ResultCode));


      // routes
      Local<Array> routes = Array::New(isolate);
      
      int cntRoutes = 1;
      for(int ii=0; ii<cntRoutes; ii++) {
        // - summary
         Local<Object> summary = Object::New(isolate);
         summary->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), Integer::New(isolate, pResult->LinkInfo[ii].link_info));
         // - distance
         summary->Set(context, String::NewFromUtf8(isolate, "distance").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkDist));
         // - time
         summary->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkTime));
         // - now
         time_t timer = time(NULL);
         struct tm* tmNow = localtime(&timer);
         string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
         summary->Set(context, String::NewFromUtf8(isolate, "now").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
         // - eta
         timer += pResult->TotalLinkTime;
         tmNow = localtime(&timer);
         strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
         summary->Set(context, String::NewFromUtf8(isolate, "eta").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
         
         // add routes - summary
         Local<Object> route = Object::New(isolate);
         route->Set(context, String::NewFromUtf8(isolate, "summary").ToLocalChecked(), summary);


         // - link_info
         Local<Array> links = Array::New(isolate);
         int cntLinks = pResult->LinkInfo.size();
         for(int jj=0; jj<cntLinks; jj++) {
            Local<Object> idoff = Object::New(isolate);
            idoff->Set(context, String::NewFromUtf8(isolate, "id").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].link_id.nid));
            idoff->Set(context, String::NewFromUtf8(isolate, "length").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].length));
            idoff->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].time));
            idoff->Set(context, String::NewFromUtf8(isolate, "angle").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].angle));
            idoff->Set(context, String::NewFromUtf8(isolate, "vertex_offset").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].vtx_off));
            idoff->Set(context, String::NewFromUtf8(isolate, "vertex_count").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].vtx_cnt));
            idoff->Set(context, String::NewFromUtf8(isolate, "remain_distance").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].rlength));
            idoff->Set(context, String::NewFromUtf8(isolate, "remain_time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[jj].rtime));

            // 링크 부가 정보 
            uint64_t sub_info = pResult->LinkInfo[ii].link_info;
            if (sub_info > 0) {
               stLinkBaseInfo* pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);
               
               idoff->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), Number::New(isolate, pBaseInfo->link_type));

               // 보행자 데이터
               if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
                  stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
                  idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, pPedInfo->facility_type));
                  idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, pPedInfo->gate_type));
               }
               else {
                  idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, 9));
                  idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, 9));
               }
            }
            links->Set(context, jj, idoff);
         }
         // add routes - link_info
         route->Set(context, String::NewFromUtf8(isolate, "link_info").ToLocalChecked(), links);



         // - vertex_info
         Local<Array> vertices = Array::New(isolate);
         int cntVertices = pResult->LinkVertex.size();
         for(auto jj=0; jj<cntVertices; jj++) {
            Local<Object> lnglat = Object::New(isolate);
            lnglat->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[jj].x));
            lnglat->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[jj].y));
            vertices->Set(context, jj, lnglat);

            // Local<Array> lnglat = Array::New(isolate);
            // lnglat->Set(context, 0, Number::New(isolate, pResult->LinkVertex[ii].x));
            // lnglat->Set(context, 1, Number::New(isolate, pResult->LinkVertex[ii].y));
            // coords->Set(context, ii, lnglat);
         }
         // add routes - vertex_info
         route->Set(context, String::NewFromUtf8(isolate, "vertex_info").ToLocalChecked(), vertices);


         if (target == ROUTE_TARGET_KAKAOVX) {
            // junction_info
            Local<Array> junctions = Array::New(isolate);
            vector<RouteProbablePath*> vtRpp;
            int cntRpp = m_pRouteMgr.GetRouteProbablePath(vtRpp);
            for(auto jj=0; jj<cntRpp; jj++) {
               Local<Object> link_info = Object::New(isolate);
               Local<Array> links = Array::New(isolate);
               RouteProbablePath* pRpp = vtRpp[jj];
               for(auto kk=0; kk<pRpp->JctLinks.size(); kk++) {
                  Local<Object> jct_info = Object::New(isolate);
                  Local<Array> link_vtx = Array::New(isolate);
                  stLinkInfo* pLink = pRpp->JctLinks[kk];
                  for (auto ll=0; ll<pLink->getVertexCount()  ; ll++) {
                     Local<Object> lnglat = Object::New(isolate);
                     lnglat->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pLink->getVertexX(ll)));
                     lnglat->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pLink->getVertexY(ll)));
                     link_vtx->Set(context, ll, lnglat);
                  } // for vtx
                  jct_info->Set(context, String::NewFromUtf8(isolate, "id").ToLocalChecked(), Number::New(isolate, pLink->link_id.nid));
                  jct_info->Set(context, String::NewFromUtf8(isolate, "vertices").ToLocalChecked(), link_vtx);
                  links->Set(context, kk, jct_info);  
               } // for links
               link_info->Set(context, String::NewFromUtf8(isolate, "id").ToLocalChecked(), Number::New(isolate, pRpp->LinkId.nid));
               link_info->Set(context, String::NewFromUtf8(isolate, "node_id").ToLocalChecked(), Number::New(isolate, pRpp->NodeId.nid));
               link_info->Set(context, String::NewFromUtf8(isolate, "junction").ToLocalChecked(), links);
               junctions->Set(context, jj, link_info);

               //release
               SAFE_DELETE(pRpp);          
            } // for junctions


            // add routes - junction_info
            route->Set(context, String::NewFromUtf8(isolate, "junction_info").ToLocalChecked(), junctions);
         }


         // increse route
         routes->Set(context, ii, route);
      }

      // add routes
      mainobj->Set(context, String::NewFromUtf8(isolate, "routes").ToLocalChecked(), routes);
   }

   args.GetReturnValue().Set(mainobj);
}


void GetRouteView(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> mainobj = Object::New(isolate);
   int cnt = 0;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();

   if (pResult == nullptr) {
      LOG_TRACE(LOG_ERROR, "Error, Route result pointer null");
      mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, Route result pointer null").ToLocalChecked());
   }
   else {
      // info
      Local<Object> routes = Object::New(isolate);
      mainobj->Set(context, String::NewFromUtf8(isolate, "user_id").ToLocalChecked(), Number::New(isolate, pResult->RequestId));
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, pResult->ResultCode));

      // result
      Local<Object> start_coord = Object::New(isolate);
      start_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.x));
      start_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "start").ToLocalChecked(), start_coord);
      Local<Object> end_coord = Object::New(isolate);
      end_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.x));
      end_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "end").ToLocalChecked(), end_coord);
      routes->Set(context, String::NewFromUtf8(isolate, "option").ToLocalChecked(), Integer::New(isolate, pResult->RouteOption));
      routes->Set(context, String::NewFromUtf8(isolate, "avoid").ToLocalChecked(), Integer::New(isolate, pResult->RouteAvoid));
      //dist
      routes->Set(context, String::NewFromUtf8(isolate, "distance").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkDist));
      string strVal;
      if (pResult->TotalLinkDist >= 1000) {
         strVal = string_format("%.2fkm", pResult->TotalLinkDist / 1000.f);
      }
      else {
         strVal = string_format("%dm", pResult->TotalLinkDist);
      }
      routes->Set(context, String::NewFromUtf8(isolate, "path_distance").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      //time
      routes->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkTime));
      if (pResult->TotalLinkTime >= 60 * 60) {
         strVal = string_format("%dhr %dmin", pResult->TotalLinkTime / (60 * 60), pResult->TotalLinkTime % ((60 * 60) / 60));
      }
      else {
         strVal = string_format("%dhr %dmin", pResult->TotalLinkTime / 60);
      }
      routes->Set(context, String::NewFromUtf8(isolate, "path_time").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      time_t timer = time(NULL);
      struct tm* tmNow = localtime(&timer);
      strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "now").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      timer += pResult->TotalLinkTime;
      tmNow = localtime(&timer);
      strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "eta").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "summary").ToLocalChecked(), routes);


      // link
      Local<Array> link = Array::New(isolate);
      Local<Object> link_list = Object::New(isolate);
      cnt = pResult->LinkInfo.size();
      for(auto ii=0; ii<cnt; ii++) {
         Local<Object> idoff = Object::New(isolate);
         idoff->Set(context, String::NewFromUtf8(isolate, "linkid").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].link_id.nid));
         idoff->Set(context, String::NewFromUtf8(isolate, "length").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].length));
         idoff->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].time));
         idoff->Set(context, String::NewFromUtf8(isolate, "angle").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].angle));
         idoff->Set(context, String::NewFromUtf8(isolate, "vertex_offset").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].vtx_off));
         idoff->Set(context, String::NewFromUtf8(isolate, "vertex_count").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].vtx_cnt));
         idoff->Set(context, String::NewFromUtf8(isolate, "remain_distance").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].rlength));
         idoff->Set(context, String::NewFromUtf8(isolate, "remain_time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].rtime));
        
         // 링크 부가 정보 
         uint64_t sub_info = pResult->LinkInfo[ii].link_info;
         if (sub_info > 0) {
            stLinkBaseInfo* pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);

            // 보행자 데이터
            if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
               stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
               idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, pPedInfo->facility_type));
               idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, pPedInfo->gate_type));
            }
            else {
               idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, 0));
               idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, 0));
            }
         }
         link->Set(context, ii, idoff);
      }
      link_list->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, cnt));
      link_list->Set(context, String::NewFromUtf8(isolate, "links").ToLocalChecked(), link);

      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "guide").ToLocalChecked(), link_list);


      // line
      Local<Array> coords = Array::New(isolate);
      Local<Object> vertex_list = Object::New(isolate);
      cnt = pResult->LinkVertex.size();
      for(auto ii=0; ii<cnt; ii++) {
         Local<Object> lnglat = Object::New(isolate);
         lnglat->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[ii].x));
         lnglat->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[ii].y));
         coords->Set(context, ii, lnglat);
      }
      // vertex_list->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), String::NewFromUtf8(isolate, "LineString").ToLocalChecked());
      vertex_list->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, cnt));
      vertex_list->Set(context, String::NewFromUtf8(isolate, "coords").ToLocalChecked(), coords);

      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "routes").ToLocalChecked(), vertex_list);
   }

   args.GetReturnValue().Set(mainobj);
}


void GetOptimalPosition(const FunctionCallbackInfo<Value>& args) {

   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> mainobj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   LOG_TRACE(LOG_DEBUG, "Start find optimal location.");

   if (args.Length() < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      double lng = args[0].As<Number>()->Value();
      double lat = args[1].As<Number>()->Value();
      
      int32_t entType = 0;
      // Ent Type
      if (cnt >= 3) {
         entType = args[2].As<Number>()->Value();
      }

      // Result Count
      int32_t retCount = 0;
      if (cnt >= 4) {
         retCount = args[3].As<Number>()->Value();
      }

      // Expand Type
      int32_t isExpand = 0;
      if (cnt >= 5) {
         isExpand = args[4].As<Number>()->Value();
      }

      LOG_TRACE(LOG_DEBUG, "Request, Location lng:%f, lat:%f, ent_type:%d, ret_cnt:%d, expand:%d", lng, lat, entType, retCount, isExpand);

      stOptimalPointInfo optInfo = {0, };

      uint32_t cntItems = m_pDataMgr.GetOptimalPointDataByPoint(lng, lat, &optInfo, entType, retCount);

      if (cntItems <= 0) {
         LOG_TRACE(LOG_ERROR, "Error, Optimal position result null");
         // mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, Can not find optimal location").ToLocalChecked());
         // header
         Local<Object> header = Object::New(isolate);
         header->Set(context, String::NewFromUtf8(isolate, "isSuccessful").ToLocalChecked(), Boolean::New(isolate, false));
         header->Set(context, String::NewFromUtf8(isolate, "resultCode").ToLocalChecked(), Integer::New(isolate, 1));
         header->Set(context, String::NewFromUtf8(isolate, "resultMessage").ToLocalChecked(), String::NewFromUtf8(isolate, m_pDataMgr.GetErrorMessage()).ToLocalChecked());

         // make header only
         mainobj->Set(context, String::NewFromUtf8(isolate, "header").ToLocalChecked(), header);
      }
      else {
         // header
         Local<Object> header = Object::New(isolate);
         header->Set(context, String::NewFromUtf8(isolate, "isSuccessful").ToLocalChecked(), Boolean::New(isolate, true));
         header->Set(context, String::NewFromUtf8(isolate, "resultCode").ToLocalChecked(), Integer::New(isolate, 0));
         header->Set(context, String::NewFromUtf8(isolate, "resultMessage").ToLocalChecked(), String::NewFromUtf8(isolate, "success").ToLocalChecked());

         // data
         Local<Object> data = Object::New(isolate);
         data->Set(context, String::NewFromUtf8(isolate, "result").ToLocalChecked(), Number::New(isolate, 0));
         data->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, optInfo.vtEntryPoint.size()));

         // items
         Local<Array> items = Array::New(isolate);
         for(int ii=0; ii<cntItems; ii++) {
            Local<Object> item = Object::New(isolate);
            item->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, optInfo.vtEntryPoint[ii].x));
            item->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, optInfo.vtEntryPoint[ii].y));
            item->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), Number::New(isolate, optInfo.vtEntryPoint[ii].nAttribute));
            items->Set(context, ii, item);
         }
         // add
         data->Set(context, String::NewFromUtf8(isolate, "entrypoints").ToLocalChecked(), items);


         // make all
         mainobj->Set(context, String::NewFromUtf8(isolate, "header").ToLocalChecked(), header);
         mainobj->Set(context, String::NewFromUtf8(isolate, "data").ToLocalChecked(), data);


         // add more expand
         if (isExpand == true) {
            // expand
            Local<Object> expand = Object::New(isolate);
            
            // request position
            expand->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, optInfo.x));
            expand->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, optInfo.y));

            // type
            expand->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), Integer::New(isolate, optInfo.nType));
            if (!optInfo.name.empty()) {
#if defined(_WIN32)
               char szUTF8[MAX_PATH] = {0,};
               MultiByteToUTF8(optInfo.name.c_str(), szUTF8);
               expand->Set(context, String::NewFromUtf8(isolate, "name").ToLocalChecked(), String::NewFromUtf8(isolate, szUTF8).ToLocalChecked());
#else
               expand->Set(context, String::NewFromUtf8(isolate, "name").ToLocalChecked(), String::NewFromUtf8(isolate, optInfo.name.c_str()).ToLocalChecked());
#endif
               
            }

            // vertices
            Local<Array> vertices = Array::New(isolate);
            for(int ii=optInfo.vtPolygon.size() - 1; ii >= 0; --ii) {
               Local<Array> vertex = Array::New(isolate);
               vertex->Set(context, 0, Number::New(isolate, optInfo.vtPolygon[ii].x));
               vertex->Set(context, 1, Number::New(isolate, optInfo.vtPolygon[ii].y));
               vertices->Set(context, ii, vertex);
            }
            // add
            expand->Set(context, String::NewFromUtf8(isolate, "vertices").ToLocalChecked(), vertices);

            mainobj->Set(context, String::NewFromUtf8(isolate, "expand").ToLocalChecked(), expand);
         }
      }
   } // if-else

   args.GetReturnValue().Set(mainobj);
}


void init(Local<Object> exports) {
   NODE_SET_METHOD(exports, "logout", LogOut);
   NODE_SET_METHOD(exports, "init", Init);
   NODE_SET_METHOD(exports, "getversion", GetVersion);
   NODE_SET_METHOD(exports, "setdeparture", SetDeparture);
   NODE_SET_METHOD(exports, "setdestination", SetDestination);
   NODE_SET_METHOD(exports, "setwaypoint", SetWaypoint);
   NODE_SET_METHOD(exports, "doroute", DoRoute);
   NODE_SET_METHOD(exports, "releaseroute", ReleaseRoute);
   NODE_SET_METHOD(exports, "getsummary", GetRouteSummary);
   NODE_SET_METHOD(exports, "getroute", GetRouteResult);
   NODE_SET_METHOD(exports, "getmultiroute", GetMultiRouteResult);
   NODE_SET_METHOD(exports, "getmultiroute_for_inavi", GetMultiRouteResultForiNavi);
   NODE_SET_METHOD(exports, "getview", GetRouteView);
// NODE_SET_METHOD(exports, "getresultstring", GetResultString);

   NODE_SET_METHOD(exports, "getoptimalposition", GetOptimalPosition);
}


NODE_MODULE(NODE_GYP_MODULE_NAME, init)

} // namespace open_route_api
