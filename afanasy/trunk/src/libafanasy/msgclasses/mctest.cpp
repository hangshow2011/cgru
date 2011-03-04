#include "mctest.h"

#include <stdio.h>

#include "../msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTest::MCTest( int Number, const std::string & String):
   string( String),
   number( Number),
   numberarray( NULL),
   stringarray( NULL)
{
   construct();
}

MCTest::MCTest( Msg * msg)
{
   read( msg);
}

bool MCTest::construct()
{
   numberarray = new int32_t[ number];
   stringarray = new std::string[ number];
   if((numberarray == NULL) || (stringarray == NULL))
   {
      AFERROR("MCTest::MCTest: Memory allocation failed.");
      number = 0;
      return false;
   }
   return true;
}

MCTest::~MCTest()
{
   if( numberarray != NULL) delete [] numberarray;
   if( stringarray != NULL) delete [] stringarray;
}

void MCTest::readwrite( Msg * msg)
{
   MsgClassUserHost::readwrite( msg);

   rw_int32_t(    number,     msg);
   rw_String(     string,     msg);
   rw_StringList( stringlist, msg);

   if( msg->isReading() )
      if( construct() == false)
         return;

   for( int i = 0; i < number; i++)
   {
      rw_int32_t( numberarray[i], msg);
      rw_String(  stringarray[i], msg);
   }
}

void MCTest::addString( const std::string & String)
{
   numberarray[ stringlist.size()] = stringlist.size();
   stringarray[ stringlist.size()] = String;
   stringlist.push_back( String);
}

void MCTest::stdOut( bool full) const
{
   MsgClassUserHost::stdOut( full);
   printf(": String = \"%s\", Number = %d.\n",
            string.c_str(), number);

   if( full == false ) return;

   if( string[0] == '/') return;

   std::list<std::string>::const_iterator it = stringlist.begin();
   for( int i = 0; i < number; i++, it++)
      printf("a\"%s\" = l\"%s\" : n%d\n",
         stringarray[i].c_str(),
         (*it).c_str(),
         numberarray[i]);
}

