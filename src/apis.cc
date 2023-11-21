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
#include "../ext/route/RoutePackage.h"




CFileManager m_pFileMgr;
CDataManager m_pDataMgr;
CRouteManager m_pRouteMgr;
CRoutePackage m_pRoutePkg;

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

   LOG_INITIALIZE();

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
   m_pRoutePkg.Initialize();

   // m_pFileMgr.SetCacheCount(100);
   m_pDataMgr.SetFileMgr(&m_pFileMgr);
	m_pFileMgr.SetDataMgr(&m_pDataMgr);
   m_pFileMgr.LoadData(strDataPath.c_str());
	m_pRouteMgr.SetDataMgr(&m_pDataMgr);
   m_pRoutePkg.SetDataMgr(&m_pDataMgr);
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

      int typeLinkMatch = TYPE_LINK_MATCH_NONE;
      if (cnt >= 3) {
         typeLinkMatch = args[3].As<Number>()->Value();
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
      m_pRouteMgr.SetDeparture(lng, lat, typeLinkMatch);
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

      LOG_TRACE(LOG_TEST, "Set waypoint lng:%f, lat:%f", lng, lat);

      bool useOptimalPoint = false;
      if (cnt >= 2) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }     

      int typeLinkMatch = TYPE_LINK_MATCH_NONE;
      if (cnt >= 3) {
         typeLinkMatch = args[3].As<Number>()->Value();
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
      m_pRouteMgr.SetWaypoint(lng, lat, TYPE_LINK_MATCH_CARSTOP_EX);
#else
      m_pRouteMgr.SetWaypoint(lng, lat, typeLinkMatch);
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

      int typeLinkMatch = TYPE_LINK_MATCH_NONE;
      if (cnt >= 3) {
         typeLinkMatch = args[3].As<Number>()->Value();
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
      m_pRouteMgr.SetDestination(lng, lat, typeLinkMatch);
#endif
   }
}


void SetDataCost(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   int type = 0;
   int devide = 0;
   int count = 0;
   Local<Array> costList;

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   if (args.Length() < 4) {
      LOG_TRACE(LOG_DEBUG, "set data cost argument too short : %d", args.Length());
      msg = String::NewFromUtf8(isolate, "set data cost argument too short : " + args.Length());
   }
   else {
      type = args[0].As<Number>()->Value();
      devide = args[1].As<Number>()->Value();
      count = args[2].As<Number>()->Value();
      costList = args[3].As<Array>();

      DataCost cost;
      if (count >= 128) {
         LOG_TRACE(LOG_DEBUG, "set data cost count too big : %d", count);
         msg = String::NewFromUtf8(isolate, "set data cost count too big : " + count);
      } else {
         for (int ii=0; ii<count; ii++) {
            Local<Value> costValue =  costList->Get(context, ii).ToLocalChecked();
            cost.base[ii] = costValue.As<Number>()->Value();

            if (devide > 0) {
                cost.base[ii] /= devide;
            }
         }

         m_pRouteMgr.SetRouteCost(type, &cost);

         ret = ROUTE_RESULT_SUCCESS;
         msg = String::NewFromUtf8(isolate, "success");
      }
   }

   obj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, ret));
   obj->Set(context, String::NewFromUtf8(isolate, "result_message").ToLocalChecked(), msg.ToLocalChecked());

   args.GetReturnValue().Set(obj);
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

	m_pRoutePkg.GetMultiRouteResultForiNavi(pResult, strJson);

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

   int target = 0;
   int cnt = 0;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();

 if (args.Length() >= 1) {
      cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      target = args[0].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Route target:%d", target);
   }
  
   string strJson;
   
   if (pResult == nullptr) {
      m_pRoutePkg.GetErrorResult(-1, "failed", strJson);
   } else {
      m_pRoutePkg.GetMultiRouteResult(pResult, target, strJson);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
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
         idoff->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].type));

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

   LOG_TRACE(LOG_DEBUG, "Start find optimal location.");

   string strJson;

   int cnt = args.Length();

   if (cnt < 2) {
#if defined(USE_CJSON)
      cJSON* root = cJSON_CreateObject();

      cJSON_AddNumberToObject(root, "result_code", OPTIMAL_RESULT_FAILED_WRONG_PARAM);
      cJSON_AddStringToObject(root, "msg", "input param count not enough");

      strJson = cJSON_Print(root);

      cJSON_Delete(root);
#else

#endif
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);
   } else {      
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

      stReqOptimal reqOpt = {0, };
      stOptimalPointInfo optInfo = {0, };

      reqOpt.x = lng;
      reqOpt.y = lat;
      reqOpt.isExpand = isExpand;
      reqOpt.typeAll = entType;

      uint32_t cntItems = m_pDataMgr.GetOptimalPointDataByPoint(lng, lat, &optInfo, entType, retCount);

      m_pRoutePkg.GetOptimalPosition(&reqOpt, &optInfo, strJson);
   }

   if (!strJson.empty()) {
      // mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


const char* getDistanceString(IN const int32_t dist)
{
   static char szBuff[128];
   if (dist <= 0) {
      return "0 Km";
   } else {
      sprintf(szBuff, "%.1f km", dist / 1000.f);
   }

   return szBuff;
}


const char* getDurationString(IN const int32_t time)
{
   static char szBuff[128];
   if (time <= 0) {
      return "0 mins";
   } else {
      if (time > 60 * 60) {
         sprintf(szBuff, "%d hours %d mins", time / 60 * 60, time / 60);
      } else if (time > 60) {
         sprintf(szBuff, "%d mins", time / 60);
      } else {
         sprintf(szBuff, "1 mins");
      }      
   }

   return szBuff;
}


void GetTable(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();
   Local<Object> mainobj = Object::New(isolate);

   cJSON* root = cJSON_CreateObject();
   string strJson;
   RouteTable** resultTables = nullptr;
   int cntRows = 0;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "arg length : 0");

      cJSON_AddNumberToObject(root, "result_code", ROUTE_RESULT_FAILED_WRONG_PARAM);
      cJSON_AddStringToObject(root, "msg", "input param count not enough");
   }
   else {
      cntRows = args[0].As<Number>()->Value();

      resultTables = new RouteTable*[cntRows];

      for(int ii=0; ii<cntRows; ii++) {
         resultTables[ii] = new RouteTable[cntRows];
      }

      int result_code = ROUTE_RESULT_FAILED;
      string str_msg = "";

      int routOpt = 2;
      int routAvoid = 0;

      m_pRouteMgr.SetRouteOption(routOpt, routAvoid);
      
      int ret = m_pRouteMgr.GetTable(resultTables);
      
#if defined(USE_CJSON)
      if (ret == ROUTE_RESULT_SUCCESS) {
         cJSON* rows = cJSON_CreateArray();

         for(int ii=0; ii<cntRows; ii++) {
            cJSON* elements = cJSON_CreateArray();
            cJSON* cols = cJSON_CreateObject();

            for(int jj=0; jj<cntRows; jj++) {
               cJSON* element = cJSON_CreateObject();
               cJSON* distance = cJSON_CreateObject();
               cJSON* duration = cJSON_CreateObject();
               cJSON* status = cJSON_CreateObject();

               cJSON_AddNumberToObject(distance, "value", resultTables[ii][jj].nTotalDist);
               cJSON_AddStringToObject(distance, "text", getDistanceString(resultTables[ii][jj].nTotalDist));

               cJSON_AddNumberToObject(duration, "value", resultTables[ii][jj].nTotalTime);
               cJSON_AddStringToObject(duration, "text", getDurationString(resultTables[ii][jj].nTotalTime));

               cJSON_AddItemToObject(element, "distance", distance);
               cJSON_AddItemToObject(element, "duration", duration);
               cJSON_AddStringToObject(element, "status", "OK");

               cJSON_AddItemToArray(elements, element);
            } // for

            cJSON_AddItemToObject(cols, "elements", elements);
            cJSON_AddItemToArray(rows, cols);
         } // for

         cJSON_AddItemToObject(root, "rows", rows);
         cJSON_AddStringToObject(root, "status", "OK");

         cJSON_AddNumberToObject(root, "result_code", ret);
         cJSON_AddStringToObject(root, "msg", "success");
      } else {
         cJSON_AddStringToObject(root, "status", "UNKNOWN_ERROR ");
         cJSON_AddNumberToObject(root, "result_code", ret);
         cJSON_AddStringToObject(root, "msg", "failed");
      }

      /*
      https://developers.google.com/maps/documentation/distance-matrix/distance-matrix?hl=ko

      OK indicates the response contains a valid result.
      INVALID_REQUEST indicates that the provided request was invalid.
      MAX_ELEMENTS_EXCEEDED indicates that the product of origins and destinations exceeds the per-query limit.
      MAX_DIMENSIONS_EXCEEDED indicates that the number of origins or destinations exceeds the per-query limit.
      OVER_DAILY_LIMIT indicates any of the following:
      The API key is missing or invalid.
      Billing has not been enabled on your account.
      A self-imposed usage cap has been exceeded.
      The provided method of payment is no longer valid (for example, a credit card has expired).
      OVER_QUERY_LIMIT indicates the service has received too many requests from your application within the allowed time period.
      REQUEST_DENIED indicates that the service denied use of the Distance Matrix service by your application.
      UNKNOWN_ERROR indicates a Distance Matrix request could not be processed due to a server error. The request may succeed if you try again.
      */
         
      strJson = cJSON_Print(root);

      cJSON_Delete(root);

      	// destroyed route table
	if (resultTables) {
		for (int ii = 0; ii < cntRows; ii++) {
			SAFE_DELETE_ARR(resultTables[ii]);
		}
		SAFE_DELETE_ARR(resultTables);
	}
   #endif // #if defined(USE_CJSON)
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetCluster_for_geoyoung(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   int ret;
   string strJson;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "GetCluster arg to short, length : %d", args.Length());

      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, "input param count not enough", strJson);
   }
   else {
      int cntClusters = args[0].As<Number>()->Value();
      int result_code = ROUTE_RESULT_FAILED;

      // get cluster
      vector<stDistrict> vtClusters;
      ret = m_pRouteMgr.GetCluster_for_geoyoung(cntClusters, vtClusters);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, "failed", strJson);
      } else {
         m_pRoutePkg.GetClusteringResult(vtClusters, strJson);
      }
      
      vtClusters.clear();
      vector<stDistrict>().swap(vtClusters);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetCluster(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   int ret;
   string strJson;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "GetCluster arg to short, length : %d", args.Length());

      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, "input param count not enough", strJson);
   }
   else {
      int cntClusters = args[0].As<Number>()->Value();
      int result_code = ROUTE_RESULT_FAILED;
      int nFileMode = 0; // 0:not thing, 1:read, 2:write, 3:read or write
      int cntPois = 0;
      string strFilePath;
      const int32_t dataSize = sizeof(RouteTable::nTotalDist) + sizeof(RouteTable::nTotalTime) + sizeof(RouteTable::dbTotalCost);
      RouteTable** ppResultTables = nullptr;

      if (args.Length() >= 4) {
         cntPois = args[1].As<Number>()->Value();
         String::Utf8Value str(isolate, args[2]);
         strFilePath = *str;
         nFileMode = args[3].As<Number>()->Value();
         bool isRead = false;
	      bool isWrite = false;

         // 지점 개수 만큼의 결과 테이블(n * n) 생성
         // create route table rows
         ppResultTables = new RouteTable*[cntPois];            
         // create route table cols 
         for (int ii = 0; ii < cntPois; ii++) {
            ppResultTables[ii] = new RouteTable[cntPois];
         }

         // 저장된 테이블 데이터를 파일에서 읽어 사용 
         if (nFileMode == 1 || nFileMode == 3) {
            FILE* fp = fopen(strFilePath.c_str(), "rb");
            if (fp) {
               // read count
               int32_t readRows = 0;
               int32_t readDataSize = 0;

               fread(&readRows, 1, sizeof(readRows), fp); // read 4byte
               fread(&readDataSize, 1, sizeof(readDataSize), fp); // read 4byte

               if ((readRows <= 0 || readDataSize <= 0) || (readRows != cntPois) || (readDataSize != dataSize)) {
                  LOG_TRACE(LOG_WARNING, "failed, read weight matrix, rading cnt:%d, dataSize:%d, source cnt:%d, dataSize", readRows, readDataSize, cntPois, dataSize);
               } else {
                  // create route table cols 
                  for (int ii = 0; ii < readRows; ii++) {
                     for (int jj = 0; jj < readRows; jj++) {
                        // read matrix
                        //uint32_t nTotalDist;
                        fread(&ppResultTables[ii][jj].nTotalDist, 1, sizeof(RouteTable::nTotalDist), fp);
                        //uint32_t nTotalTime;
                        fread(&ppResultTables[ii][jj].nTotalTime, 1, sizeof(RouteTable::nTotalTime), fp);
                        //double dbTotalCost;
                        fread(&ppResultTables[ii][jj].dbTotalCost, 1, sizeof(RouteTable::dbTotalCost), fp);
                     } // for
                  } // for

                  isRead = true;
				      ret = ROUTE_RESULT_SUCCESS;
               }
               fclose(fp);
            } // fp
         }
         
         if (!isRead) {
            ret = m_pRouteMgr.GetTable(ppResultTables);
         }

         // 테이블 데이터 미리 읽어 파일에 저장하고 사용하자
         if (ret == ROUTE_RESULT_SUCCESS && (nFileMode == 2 || nFileMode == 3)) {
            FILE* fp = fopen(strFilePath.c_str(), "wb");
            if (fp) {
               // write count
               fwrite(&cntPois, 1, sizeof(cntPois), fp);

               // write data size
               fwrite(&dataSize, 1, sizeof(dataSize), fp);

               // write matrix
               for (int ii = 0; ii < cntPois; ii++) {
                  for (int jj = 0; jj < cntPois; jj++) {
                     //uint32_t nTotalDist;
                     fwrite(&ppResultTables[ii][jj].nTotalDist, 1, sizeof(RouteTable::nTotalDist), fp);
                     //uint32_t nTotalTime;
                     fwrite(&ppResultTables[ii][jj].nTotalTime, 1, sizeof(RouteTable::nTotalTime), fp);
                     //double dbTotalCost;
                     fwrite(&ppResultTables[ii][jj].dbTotalCost, 1, sizeof(RouteTable::dbTotalCost), fp);
                  } // for
               } // for
               isWrite = true;
               fclose(fp);
            } // fp
         }
         else if (ret != ROUTE_RESULT_SUCCESS) {
            LOG_TRACE(LOG_WARNING, "failed, get weight matrix, err:%d", ret);
         }
      }

      // get cluster
      vector<stDistrict> vtClusters;
      ret = m_pRouteMgr.GetCluster(cntClusters, ppResultTables, vtClusters);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, "failed", strJson);
      } else {
         m_pRoutePkg.GetClusteringResult(vtClusters, strJson);
      }


      // release
      if (ppResultTables != nullptr) {
         for (int ii = 0; ii < cntPois; ii++) {
            SAFE_DELETE_ARR(ppResultTables[ii]);
         }
         SAFE_DELETE_ARR(ppResultTables);
      }
      
      vtClusters.clear();
      vector<stDistrict>().swap(vtClusters);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetBoundary(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   string strJson;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "GetBoundary arg to short, length : %d", args.Length());
      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, "input param count not enough", strJson);
   }
   else {
      int countPois = args[0].As<Number>()->Value();
      Local<Array> arrPois = args[1].As<Array>();

      vector<SPoint> vtBoundary;
      vector<SPoint> vtPois;
      SPoint coord;
      for (int ii=0; ii<countPois; ii++) {
         Local<Value> poi = arrPois->Get(context, ii).ToLocalChecked();
         String::Utf8Value strValue(isolate, poi);
         string strCoord = *strValue;
         size_t pos = strCoord.find(',');
         coord.x = atof(strCoord.substr(0, pos).c_str());
         coord.y = atof(strCoord.substr(pos + 1).c_str());

         vtPois.emplace_back(coord);
      }
      
      int ret = m_pRouteMgr.GetBoundary(vtPois, vtBoundary);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, "failed", strJson);
      } else {
         m_pRoutePkg.GetBoundaryResult(vtBoundary, strJson);
      }
      vtPois.clear();
      vector<SPoint>().swap(vtPois);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetWaypoints(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();
   Local<Object> mainobj = Object::New(isolate);

   cJSON* root = cJSON_CreateObject();
   string strJson;
   RouteTable** resultTables = nullptr;
   int cntRows = 0;
   int tspOpt = 0;

   if (args.Length() < 2) {
      LOG_TRACE(LOG_WARNING, "GetWaypoints arg to short, length : %d", args.Length());

      cJSON_AddNumberToObject(root, "result_code", ROUTE_RESULT_FAILED_WRONG_PARAM);
      cJSON_AddStringToObject(root, "msg", "input param count not enough");
   }
   else {
      tspOpt = args[0].As<Number>()->Value();
      cntRows = args[1].As<Number>()->Value();

      // resultTables = new RouteTable*[cntRows];

      // for(int ii=0; ii<cntRows; ii++) {
      //    resultTables[ii] = new RouteTable[cntRows];
      // }

      int result_code = ROUTE_RESULT_FAILED;
      string str_msg = "";

      uint32_t typeAlgorithm = tspOpt; // default tsp value is 0
      uint32_t cntMaxRows = cntRows;
      uint32_t cntMaxPopulation = 100;
      uint32_t cntMaxLoop = 1000;

      TspOptions opt = {
			typeAlgorithm,
			cntMaxRows,
			cntMaxPopulation,
			cntMaxLoop,
		};

      int routOpt = 2;
      int routAvoid = 0;

      m_pRouteMgr.SetRouteOption(routOpt, routAvoid);

      vector<uint32_t> vtBestwaypoints;
      int ret = m_pRouteMgr.Table(&opt, resultTables, vtBestwaypoints);
      
#if defined(USE_CJSON)
      if (ret == ROUTE_RESULT_SUCCESS) {
         cJSON* waypoints = cJSON_CreateArray();
         int cntWaypoints = vtBestwaypoints.size();

         for(int ii=0; ii<cntWaypoints; ii++) {
            cJSON* info = cJSON_CreateObject();

            cJSON_AddNumberToObject(info, "index", vtBestwaypoints[ii]);
            cJSON_AddItemToArray(waypoints, info);
         } // for

         cJSON_AddItemToObject(root, "waypoints", waypoints);

         cJSON_AddStringToObject(root, "status", "OK");
         cJSON_AddNumberToObject(root, "result_code", ret);
         cJSON_AddStringToObject(root, "msg", "success");

#if 1 // print web view
         // for web route view
         ////////////////////////////////////////////////////////////////////////////////
         // const char *szHost = "localhost";
         // int nPort = 20301;
         const char *szHost = "133.186.153.133";
         int nPort = 5555;

         char szBuff[256] = { 0, };
         string strURL;

         sprintf(szBuff, "http://%s:%d/view/waypoints?id=202302091420&opt=%d", szHost, nPort, routOpt);
         strURL.append(szBuff);

         // add start
         sprintf(szBuff, "&start=%.6f,%.6f", m_pRouteMgr.GetDeparture()->x, m_pRouteMgr.GetDeparture()->y);
         strURL.append(szBuff);

         // add end
         sprintf(szBuff, "&end=%.6f,%.6f", m_pRouteMgr.GetDestination()->x, m_pRouteMgr.GetDestination()->y);
         strURL.append(szBuff);

         const int cntVias = m_pRouteMgr.GetWayPointCount();
         for (int ii = 0; ii < cntVias; ii++) {
            sprintf(szBuff, "&vias=%.6f,%.6f", m_pRouteMgr.GetWaypoint(ii)->x, m_pRouteMgr.GetWaypoint(ii)->y);
            strURL.append(szBuff);
         }
         // strURL.append("\n");

         printf(strURL.c_str());
         printf("\n");
         ////////////////////////////////////////////////////////////////////////////////

         cJSON_AddStringToObject(root, "url", strURL.c_str());
#endif 

      } else {
         cJSON_AddStringToObject(root, "status", "UNKNOWN_ERROR ");
         cJSON_AddNumberToObject(root, "result_code", ret);
         cJSON_AddStringToObject(root, "msg", "failed");
      }

      /*
      https://developers.google.com/maps/documentation/distance-matrix/distance-matrix?hl=ko

      OK indicates the response contains a valid result.
      INVALID_REQUEST indicates that the provided request was invalid.
      MAX_ELEMENTS_EXCEEDED indicates that the product of origins and destinations exceeds the per-query limit.
      MAX_DIMENSIONS_EXCEEDED indicates that the number of origins or destinations exceeds the per-query limit.
      OVER_DAILY_LIMIT indicates any of the following:
      The API key is missing or invalid.
      Billing has not been enabled on your account.
      A self-imposed usage cap has been exceeded.
      The provided method of payment is no longer valid (for example, a credit card has expired).
      OVER_QUERY_LIMIT indicates the service has received too many requests from your application within the allowed time period.
      REQUEST_DENIED indicates that the service denied use of the Distance Matrix service by your application.
      UNKNOWN_ERROR indicates a Distance Matrix request could not be processed due to a server error. The request may succeed if you try again.
      */
         
      strJson = cJSON_Print(root);

      cJSON_Delete(root);

      // destroyed route table
      if (resultTables) {
         for (int ii = 0; ii < cntRows; ii++) {
            SAFE_DELETE_ARR(resultTables[ii]);
         }
         SAFE_DELETE_ARR(resultTables);
      }
#endif // #if defined(USE_CJSON)


   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void init(Local<Object> exports) {

#if defined(USE_MULTIPROCESS)
   if (omp_get_max_threads() > PROCESS_NUM) {
	   omp_set_num_threads(PROCESS_NUM);
      printf("Check OpenMP reset threads num to %d\n", omp_get_max_threads());
   } else {
      printf("Check OpenMP threads num : %d\n", omp_get_max_threads());
   }
   printf("Check OpenMP thread ids : ");
#pragma omp parallel
   printf("%d ", omp_get_thread_num());
   printf("\n");
#endif // #if defined(USE_MULTIPROCESS)

   NODE_SET_METHOD(exports, "logout", LogOut);
   NODE_SET_METHOD(exports, "init", Init);
   NODE_SET_METHOD(exports, "getversion", GetVersion);
   NODE_SET_METHOD(exports, "setdeparture", SetDeparture);
   NODE_SET_METHOD(exports, "setdestination", SetDestination);
   NODE_SET_METHOD(exports, "setwaypoint", SetWaypoint);
   NODE_SET_METHOD(exports, "setdatacost", SetDataCost);
   NODE_SET_METHOD(exports, "doroute", DoRoute);
   NODE_SET_METHOD(exports, "releaseroute", ReleaseRoute);
   NODE_SET_METHOD(exports, "getsummary", GetRouteSummary);
   NODE_SET_METHOD(exports, "getroute", GetRouteResult);
   NODE_SET_METHOD(exports, "getmultiroute", GetMultiRouteResult);
   NODE_SET_METHOD(exports, "getmultiroute_for_inavi", GetMultiRouteResultForiNavi);
   NODE_SET_METHOD(exports, "getview", GetRouteView);
   NODE_SET_METHOD(exports, "gettable", GetTable);
   NODE_SET_METHOD(exports, "getcluster", GetCluster);
   NODE_SET_METHOD(exports, "getcluster_for_geoyoung", GetCluster_for_geoyoung);   
   NODE_SET_METHOD(exports, "getboundary", GetBoundary);
   NODE_SET_METHOD(exports, "getwaypoints", GetWaypoints);
// NODE_SET_METHOD(exports, "getresultstring", GetResultString);

   NODE_SET_METHOD(exports, "getoptimalposition", GetOptimalPosition);
}


NODE_MODULE(NODE_GYP_MODULE_NAME, init)

} // namespace open_route_api
