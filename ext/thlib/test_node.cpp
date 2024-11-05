#include <iostream>
#include "test_node.h"

TestNode::TestNode()
{

}


TestNode::~TestNode()
{

}

bool TestNode::Initialize(const char* szPath)
{
   strcpy(m_szPath, szPath);

   return true;
}


void TestNode::Release()
{


}


int TestNode::GetSpeed(const unsigned long long ttlid, const int type)
{
   int speed = 0;

   speed = 87;

   return speed;
}

std::string TestNode::Hello()
{
   std::cout << "Hello?" << std::endl;
   return "Hello??";
}

char* TestNode::Echo(char* p)
{
   path = p;

   return path;
}