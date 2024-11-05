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
#include "../ext/route/TrafficManager.h"


CFileManager m_pFileMgr;
CDataManager m_pDataMgr;
CRouteManager m_pRouteMgr;
CRoutePackage m_pRoutePkg;
CTrafficManager m_pTrafficMgr;

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
#elif defined(USE_FOREST_DATA)
      strLogFile = "log_forest";
#elif defined(USE_PEDESTRIAN_DATA)
      strLogFile = "log_pedestrian";
#elif defined(USE_VEHICLE_DATA)
      strLogFile = "log_vehicle";
#else
      strLogFile = "log_trekking";
#endif

   if (!strLogPath.empty()) {
      LOG_SET_FILEPATH(strLogPath.c_str(), strLogFile.c_str());
      LOG_TRACE(LOG_DEBUG, "Init log : %s, %s", strLogPath.c_str(), strLogFile.c_str());  
   }

   LOG_TRACE(LOG_DEBUG, "Engine version : %d.%d.%d.%d", 
   ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH, ENGINE_VERSION_BUILD);

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
   m_pDataMgr.SetDataPath(strDataPath.c_str());
   m_pFileMgr.SetDataPath(strDataPath.c_str());

	m_pFileMgr.SetDataMgr(&m_pDataMgr);
	m_pRouteMgr.SetDataMgr(&m_pDataMgr);
   m_pRoutePkg.SetDataMgr(&m_pDataMgr);
   m_pTrafficMgr.SetDataMgr(&m_pDataMgr);

   m_pFileMgr.LoadData();
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

      bool useOptimalPoint = false;
      if (cnt >= 3) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }

      int typeLinkMatch = TYPE_LINK_MATCH_NONE;
      if (cnt >= 4) {
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

      LOG_TRACE(LOG_DEBUG, "Set departure lng:%f, lat:%f, type:%d, optimal:%d", lng, lat, typeLinkMatch, useOptimalPoint);

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

      bool useOptimalPoint = false;
      if (cnt >= 3) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }     

      int typeLinkMatch = TYPE_LINK_MATCH_NONE;
      if (cnt >= 4) {
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

      LOG_TRACE(LOG_DEBUG, "Set waypoint lng:%f, lat:%f, type:%d, optimal:%d", lng, lat, typeLinkMatch, useOptimalPoint);

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
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      double lng = args[0].As<Number>()->Value();
      double lat = args[1].As<Number>()->Value();
      // int opt = args[2].As<Number>()->Value();

      bool useOptimalPoint = false;
      if (cnt >= 3) {
         useOptimalPoint = args[2].As<Boolean>()->Value();
      }     

      int typeLinkMatch = TYPE_LINK_MATCH_NONE;
      if (cnt >= 4) {
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

      LOG_TRACE(LOG_DEBUG, "Set destination lng:%f, lat:%f, type:%d, optimal:%d", lng, lat, typeLinkMatch, useOptimalPoint);

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


void SetRouteOption(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   LOG_TRACE(LOG_DEBUG, "Set route option");

   if (args.Length() < 1) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      // int opt = args[0].As<Number>()->Value();
      // int avoid = args[1].As<Number>()->Value();
      int mobility = args[2].As<Number>()->Value();

      // LOG_TRACE(LOG_DEBUG, "Route Opt:%d, Avoid:%d, Mobility:%d", opt, avoid, mobility);

      // m_pRouteMgr.SetRouteOption(opt, avoid, mobility);

      vector<uint32_t>vtRouteOpt = { ROUTE_OPT_RECOMMENDED, ROUTE_OPT_SHORTEST, ROUTE_OPT_COMFORTABLE };
	   vector<uint32_t>vtAvoidOpt = { ROUTE_AVOID_NONE, ROUTE_AVOID_NONE, ROUTE_AVOID_NONE };

	   m_pRouteMgr.SetRouteOption(vtRouteOpt, vtAvoidOpt, mobility);
   }
}


void AddRouteOption(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   LOG_TRACE(LOG_DEBUG, "Set route option");

   if (args.Length() < 3) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      int opt = args[0].As<Number>()->Value();
      int avoid = args[1].As<Number>()->Value();
      int mobility = args[2].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Route Opt:%d, Avoid:%d, Mobility:%d", opt, avoid, mobility);

      m_pRouteMgr.AddRouteOption(opt, avoid, mobility);
   }
}


void SetRouteSubOption(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   LOG_TRACE(LOG_DEBUG, "Set route sub option");

   if (args.Length() < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      stRouteSubOption subOpt;
      subOpt.mnt.course_type = args[0].As<Number>()->Value();
      subOpt.mnt.course_id = args[1].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "Route Sub Opt:%d, type:%d, course:%d", subOpt.option, subOpt.mnt.course_type, subOpt.mnt.course_id);
      m_pRouteMgr.SetRouteSubOption(subOpt.option);
   }
}


void DoRoute(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;

   LOG_TRACE(LOG_DEBUG, "Start routing.");

   if (args.Length() < 3) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      // int cnt = args.Length();
      // // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      // int opt = args[0].As<Number>()->Value();
      // int avoid = args[1].As<Number>()->Value();
      // int mobility = args[2].As<Number>()->Value();

      // LOG_TRACE(LOG_DEBUG, "Route Opt:%d, Avoid:%d, Mobility:%d", opt, avoid, mobility);

      // m_pRouteMgr.SetRouteOption(opt, avoid, mobility);
      
   #if defined(USE_MOUNTAIN_DATA)
      ret = m_pRouteMgr.Route(3);
   #else
      ret = m_pRouteMgr.Route();
   #endif

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


void DoMultiRoute(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> obj = Object::New(isolate);
   v8::MaybeLocal<v8::String> msg;

   int ret = -1;
   int cntRoute = 0;

   LOG_TRACE(LOG_DEBUG, "Start MultiRouting.");

   if (args.Length() < 1) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      msg = String::NewFromUtf8(isolate, "function call argument too short : " + args.Length());
   }
   else {
      int cnt = args.Length();
      // LOG_TRACE(LOG_DEBUG, "arg length : %d", cnt);

      cntRoute = args[0].As<Number>()->Value();

      LOG_TRACE(LOG_DEBUG, "MultiRoute count : %d", cntRoute);

      // m_pRouteMgr.SetRouteOption(opt, avoid, mobility);
   }

   #if defined(USE_MOUNTAIN_DATA)
   if (cntRoute > 1) {
      ret = m_pRouteMgr.Route(cntRoute);
   } else {
      ret = m_pRouteMgr.Route();
   }
#else
   ret = m_pRouteMgr.Route();
#endif

   if (ret == 0) {
      LOG_TRACE(LOG_DEBUG, "Success MultiRouting.");
      msg = String::NewFromUtf8(isolate, "Success MultiRouting");
   }
   else {
      LOG_TRACE(LOG_DEBUG, "Failed MultiRouting.");
      msg = String::NewFromUtf8(isolate, "Failed MultiRouting");
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

   string strJson;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();
   bool isJunction = false;   

   if (args.Length() >= 1) {
      isJunction = args[0].As<Number>()->Value();
   }
  
   if (pResult == nullptr) {
      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED, strJson);
   } else {
      m_pRoutePkg.GetRouteResult(pResult, isJunction, strJson);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetMultiRouteResult(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   string strJson;
   int cntRoutes = m_pRouteMgr.GetRouteResultsCount();
   bool isJunction = false;

   if (args.Length() >= 1) {
      isJunction = args[0].As<Boolean>()->Value();
   }

   if (cntRoutes <= 0) {
      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED, strJson);
   } else {
#if 1 // use result vector
      const vector<RouteResultInfo>* pResults = m_pRouteMgr.GetMultiRouteResults();
      m_pRoutePkg.GetMultiRouteResult(*pResults, isJunction, strJson);
#else

      const RouteResultInfo* pResult = nullptr;
      for(int ii=0; ii<cntRoutes; ii++) {
            pResult = m_pRouteMgr.GetRouteResults(ii);
            m_pRoutePkg.GetRouteResult(pResult, isJunction, strJson);
      }
#endif
   }
   // add route to root
   if (!strJson.empty()) {
      // mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
   }
   
   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetMapsRouteResult(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   string strJson;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();
  
   if (pResult == nullptr) {
      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED, strJson);
   } else {
      m_pRoutePkg.GetMapsRouteResult(pResult, strJson);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetOptimalPosition(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   LOG_TRACE(LOG_DEBUG, "Start find optimal location.");

   string strJson;

   int cnt = args.Length();

   if (cnt < 2) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);

      m_pRoutePkg.GetErrorResult(OPTIMAL_RESULT_FAILED_WRONG_PARAM, strJson);
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

      // NearRoad Type
      int32_t nOption = 0;
      if (cnt >= 6) {
         nOption = args[5].As<Number>()->Value();
      }

      LOG_TRACE(LOG_DEBUG, "Request, Location lng:%f, lat:%f, ent_type:%d, ret_cnt:%d, expand:%d, option:%d", lng, lat, entType, retCount, isExpand, nOption);

      stReqOptimal reqOpt = {0, };
      stOptimalPointInfo optInfo = {0, };

      reqOpt.x = lng;
      reqOpt.y = lat;
      reqOpt.isExpand = isExpand;
      reqOpt.typeAll = entType;

      uint32_t cntItems = m_pDataMgr.GetOptimalPointDataByPoint(lng, lat, &optInfo, entType, retCount, 1, 0, nOption);

      m_pRoutePkg.GetOptimalPosition(&reqOpt, &optInfo, strJson);
   }

   if (!strJson.empty()) {
      // mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetTable(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   int ret;
   string strJson;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "GetTable arg to short, length : %d", args.Length());

      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, strJson);
   }
   else {
      String::Utf8Value pRequest(isolate, args[0]);
      string strRequest = *pRequest;
      
      // get table
      vector<vector<stDistMatrix>> vtWeightMatrix;
      ret = m_pRouteMgr.GetWeightMatrix(strRequest.c_str(), vtWeightMatrix);

      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, strJson);
      } else {
         m_pRoutePkg.GetWeightMatrixResult(vtWeightMatrix, strJson);
      }
      
      vtWeightMatrix.clear();
      vector<vector<stDistMatrix>>().swap(vtWeightMatrix);
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

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void GetCluster_for_geoyoung(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   int ret;
   string strJson;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "GetCluster arg to short, length : %d", args.Length());

      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, strJson);
   }
   else {
      int cntClusters = args[0].As<Number>()->Value();
      int result_code = ROUTE_RESULT_FAILED;

      // get cluster
      vector<stDistrict> vtClusters;
      vector<SPoint> vtPositionLock;
      ret = m_pRouteMgr.GetCluster_for_geoyoung(cntClusters, vtClusters);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, strJson);
      } else {
         m_pRoutePkg.GetClusteringResult(vtClusters, vtPositionLock, strJson);
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

      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, strJson);
   }
   else {
      String::Utf8Value pRequest(isolate, args[0]);
      string strRequest = *pRequest;

      // get cluster
      vector<stDistrict> vtClusters;
      vector<SPoint> vtPositionLock;
      ret = m_pRouteMgr.GetCluster(strRequest.c_str(), vtClusters, vtPositionLock);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, strJson);
      } else {
         m_pRoutePkg.GetClusteringResult(vtClusters, vtPositionLock, strJson);
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
      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, strJson);
   }
   else {
      int countPois = args[0].As<Number>()->Value();
      Local<Array> arrPois = args[1].As<Array>();

      SPoint ptCenter;
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
      
      int ret = m_pRouteMgr.GetBoundary(vtPois, vtBoundary, ptCenter);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, strJson);
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

   int ret;
   string strJson;

   if (args.Length() < 1) {
      LOG_TRACE(LOG_WARNING, "GetWaypoints arg to short, length : %d", args.Length());

      m_pRoutePkg.GetErrorResult(ROUTE_RESULT_FAILED_WRONG_PARAM, strJson);
   }
   else {
      String::Utf8Value pRequest(isolate, args[0]);
      string strRequest = *pRequest;

      // get bestway
      vector<stWaypoints> vtWaypoints;
      vector<uint32_t> vtBestwaypoints;
      double totDist = 0.f;
      int32_t totTime = 0;
      ret = m_pRouteMgr.GetBestway(strRequest.c_str(), vtWaypoints, vtBestwaypoints, totDist, totTime);
      if (ret != ROUTE_RESULT_SUCCESS) {
         m_pRoutePkg.GetErrorResult(ret, strJson);
      } else {
         m_pRoutePkg.GetBestWaypointResult(vtWaypoints, vtBestwaypoints, totDist, totTime, strJson);
      }

#if 1 // print web view
      // cJSON* root = cJSON_CreateObject();
      // for web route view
      ////////////////////////////////////////////////////////////////////////////////
      // const char *szHost = "localhost";
      // int nPort = 20301;
      const char *szHost = "133.186.153.133";
      int nPort = 8888;

      char szBuff[256] = { 0, };
      string strURL;
      int routOpt = ROUTE_OPT_COMFORTABLE;

      sprintf(szBuff, "http://%s:%d/view/waypoints?id=202302091420&opt=%d", szHost, nPort, routOpt);
      strURL.append(szBuff);

      const int cntPOI = vtBestwaypoints.size();
      for (int ii = 0; ii < cntPOI; ii++) {
         if (ii == 0) { // add start
            sprintf(szBuff, "&start=%.6f,%.6f", vtWaypoints[vtBestwaypoints[ii]].x, vtWaypoints[vtBestwaypoints[ii]].y);
            strURL.append(szBuff);
         } else if (ii == cntPOI - 1) { // add end
            sprintf(szBuff, "&end=%.6f,%.6f", vtWaypoints[vtBestwaypoints[ii]].x, vtWaypoints[vtBestwaypoints[ii]].y);
            strURL.append(szBuff);
         } else { // add via
            sprintf(szBuff, "&vias=%.6f,%.6f", vtWaypoints[vtBestwaypoints[ii]].x, vtWaypoints[vtBestwaypoints[ii]].y);
            strURL.append(szBuff);
         }
      }
      // strURL.append("\n");

      // printf(strURL.c_str());
      // printf("\n");
      // LOG_TRACE(LOG_DEBUG, "%s", strURL.c_str());
      ////////////////////////////////////////////////////////////////////////////////

      // cJSON_AddStringToObject(root, "url", strURL.c_str());
#endif 

      vtBestwaypoints.clear();
      vector<uint32_t>().swap(vtBestwaypoints);

      vtWaypoints.clear();
      vector<stWaypoints>().swap(vtWaypoints);
   }

   args.GetReturnValue().Set(String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
}


void UpdateTraffic(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   // LOG_TRACE(LOG_DEBUG, "Start update traffic");

   int cnt = args.Length();

   if (cnt < 3) {
      LOG_TRACE(LOG_DEBUG, "function call argument too short : %s", args);
   } else {      
      String::Utf8Value strName(isolate, args[0]);
      String::Utf8Value strPath(isolate, args[1]);
      string strFileName = *strName;
      string strFilePath = *strPath;
      uint32_t timeNow = args[2].As<Number>()->Value();;
      uint32_t update_imestamp = 0;

      char szTrafiicFilePath[MAX_PATH] = {0,};
      sprintf(szTrafiicFilePath, "%s/%s", strFilePath.c_str(), strFileName.c_str());
      if (update_imestamp = m_pTrafficMgr.Update(szTrafiicFilePath, TYPE_TRAFFIC_TTL, timeNow)) {
			LOG_TRACE(LOG_DEBUG, "traffic file loaded: %s", szTrafiicFilePath);
		} else {
			LOG_TRACE(LOG_WARNING, "failed, can't load traffic file : %s", szTrafiicFilePath);
		}
   }
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
   NODE_SET_METHOD(exports, "setrouteoption", SetRouteOption);
   NODE_SET_METHOD(exports, "addrouteoption", AddRouteOption);
   NODE_SET_METHOD(exports, "setroutesuboption", SetRouteSubOption);
   NODE_SET_METHOD(exports, "doroute", DoRoute);
   NODE_SET_METHOD(exports, "domultiroute", DoMultiRoute);
   NODE_SET_METHOD(exports, "releaseroute", ReleaseRoute);
   NODE_SET_METHOD(exports, "getsummary", GetRouteSummary);
   NODE_SET_METHOD(exports, "getroute", GetRouteResult);
   NODE_SET_METHOD(exports, "getmultiroute", GetMultiRouteResult);
   NODE_SET_METHOD(exports, "getmapsroute", GetMapsRouteResult);
   NODE_SET_METHOD(exports, "gettable", GetTable);
   NODE_SET_METHOD(exports, "getcluster", GetCluster);
   NODE_SET_METHOD(exports, "getcluster_for_geoyoung", GetCluster_for_geoyoung);   
   NODE_SET_METHOD(exports, "getboundary", GetBoundary);
   NODE_SET_METHOD(exports, "getwaypoints", GetWaypoints);
// NODE_SET_METHOD(exports, "getresultstring", GetResultString);
   NODE_SET_METHOD(exports, "getoptimalposition", GetOptimalPosition);
   NODE_SET_METHOD(exports, "updatetraffic", UpdateTraffic);
}


NODE_MODULE(NODE_GYP_MODULE_NAME, init)

} // namespace open_route_api
