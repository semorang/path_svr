#pragma once

class TestNode {
public:
   TestNode();
   virtual ~TestNode();

   bool Initialize(const char* szPath);
   void Release();

   int GetSpeed(const unsigned long long ttlid, int type);

   std::string Hello();

   char* Echo(char* p);

private:
   char m_szPath[1024];   

   char* path;
};
