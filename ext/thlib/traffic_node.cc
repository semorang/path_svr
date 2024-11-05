#include <node.h>
#include <stdio.h>


#include <string.h>
#include <time.h>

#include "ReadTTLSpd.h"
ReadTTLSpd rdSpd;

namespace demo {
 
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Number;
using v8::Integer;
using v8::Context;
using v8::Boolean;
using v8::Uint32;
//using v8::BigInt;


 
void InitFile(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   std::string strDataPath;

   if (args.Length() > 0) {      
      String::Utf8Value str(isolate, args[0]);
      strDataPath = *str;
   }

   char* tmpc;
   strcpy(tmpc, strDataPath.c_str());

   int result = 0;
   result = rdSpd.Init(tmpc);
   if (result < 0)
   {
      result *= -1;
      result += 1000;
   }
   args.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, result));
}

void GetSpd(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   if (args.Length() != 3) {      
      args.GetReturnValue().Set(String::NewFromUtf8(isolate, "need 3 args [ ttlid, time, link length]").ToLocalChecked());
      return;
   }

	unsigned long long ttlId = args[0].As<Number>()->Value();
	int dir = args[1].As<Number>()->Value();
   time_t time = args[2].As<Number>()->Value();
   float linkLength = args[3].As<Number>()->Value();

   std::cout << "ttl ID : " << ttlId << std::endl;
   uint8_t spd = rdSpd.GetSpd(ttlId, time, linkLength);

   args.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, static_cast<uint32_t>(spd)));
}

void ReleaseFile(const FunctionCallbackInfo<Value>& args) {
   Isolate* isolate = args.GetIsolate();

   std::string strDataPath;

   rdSpd.Release();
}

void init(Local<Object> exports) {
   // get ttl spd
   NODE_SET_METHOD(exports, "initFile", InitFile);
   NODE_SET_METHOD(exports, "getSpd", GetSpd);
   NODE_SET_METHOD(exports, "releaseFile", ReleaseFile);

}
 
NODE_MODULE(libtraffic_node, init)
 
}  