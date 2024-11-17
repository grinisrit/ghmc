// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <set>
#include <iomanip>
#include <cstring>

#if ! defined( _MSC_VER ) && ! defined( __MINGW32__ ) && ! defined( __MINGW64__ )
   #include <unistd.h>
   #include <sys/utsname.h>
   #include <sys/stat.h>
#endif

#include <noa/3rdparty/tnl-noa/src/TNL/SystemInfo.h>

namespace noa::TNL {

inline String
SystemInfo::getHostname()
{
#if ! defined( _MSC_VER ) && ! defined( __MINGW32__ ) && ! defined( __MINGW64__ )
   char host_name[ 256 ];
   gethostname( host_name, 255 );
   return host_name;
#else
   return "[unknown hostname]";
#endif
}

inline String
SystemInfo::getArchitecture()
{
#if ! defined( _MSC_VER ) && ! defined( __MINGW32__ ) && ! defined( __MINGW64__ )
   utsname uts;
   uname( &uts );
   return uts.machine;
#else
   return "[unknown arch]";
#endif
}

inline String
SystemInfo::getSystemName()
{
#if ! defined( _MSC_VER ) && ! defined( __MINGW32__ ) && ! defined( __MINGW64__ )
   utsname uts;
   uname( &uts );
   return uts.sysname;
#else
   return "[unknown system]";
#endif
}

inline String
SystemInfo::getSystemRelease()
{
#if ! defined( _MSC_VER ) && ! defined( __MINGW32__ ) && ! defined( __MINGW64__ )
   utsname uts;
   uname( &uts );
   return uts.release;
#else
   return "[unknown release]";
#endif
}

inline String
SystemInfo::getCurrentTime( const char* format )
{
   const std::time_t time_since_epoch = std::time( nullptr );
   std::tm* localtime = std::localtime( &time_since_epoch );
   std::stringstream ss;
   ss << std::put_time( localtime, format );
   return ss.str();
}

inline int
SystemInfo::getNumberOfProcessors()
{
   static int numberOfProcessors = 0;
#if ! defined( _WIN32 )
   if( numberOfProcessors == 0 ) {
      CPUInfo info = parseCPUInfo();
      numberOfProcessors = info.numberOfProcessors;
   }
#endif
   return numberOfProcessors;
}

inline String
SystemInfo::getOnlineCPUs()
{
#if ! defined( _WIN32 )
   std::string online = readFile< std::string >( "/sys/devices/system/cpu/online" );
   return online;
#else
   return "";
#endif
}

inline int
SystemInfo::getNumberOfCores( int cpu_id )
{
   static int CPUCores = 0;
#if ! defined( _WIN32 )
   if( CPUCores == 0 ) {
      CPUInfo info = parseCPUInfo();
      CPUCores = info.CPUCores;
   }
#endif
   return CPUCores;
}

inline int
SystemInfo::getNumberOfThreads( int cpu_id )
{
   static int CPUThreads = 0;
#if ! defined( _WIN32 )
   if( CPUThreads == 0 ) {
      CPUInfo info = parseCPUInfo();
      CPUThreads = info.CPUThreads;
   }
#endif
   return CPUThreads;
}

inline String
SystemInfo::getCPUModelName( int cpu_id )
{
   static String CPUModelName;
#if ! defined( _WIN32 )
   if( CPUModelName.empty() ) {
      CPUInfo info = parseCPUInfo();
      CPUModelName = info.CPUModelName;
   }
#endif
   return CPUModelName;
}

inline int
SystemInfo::getCPUMaxFrequency( int cpu_id )
{
#if ! defined( _WIN32 )
   String fileName( "/sys/devices/system/cpu/cpu" );
   fileName += convertToString( cpu_id ) + "/cpufreq/cpuinfo_max_freq";
   return readFile< int >( fileName );
#else
   return 0;
#endif
}

inline CacheSizes
SystemInfo::getCPUCacheSizes( int cpu_id )
{
#if ! defined( _WIN32 )
   String directory( "/sys/devices/system/cpu/cpu" );
   directory += convertToString( cpu_id ) + "/cache";

   CacheSizes sizes;
   for( int i = 0; i <= 3; i++ ) {
      const String cache = directory + "/index" + convertToString( i );

      // check if the directory exists
      struct stat st;
      if( stat( cache.getString(), &st ) != 0 || ! S_ISDIR( st.st_mode ) )
         break;

      const int level = readFile< int >( cache + "/level" );
      const std::string type = readFile< std::string >( cache + "/type" );
      const int size = readFile< int >( cache + "/size" );

      if( level == 1 && type == "Instruction" )
         sizes.L1instruction = size;
      else if( level == 1 && type == "Data" )
         sizes.L1data = size;
      else if( level == 2 )
         sizes.L2 = size;
      else if( level == 3 )
         sizes.L3 = size;
   }
   return sizes;
#else
   return {};
#endif
}

inline size_t
SystemInfo::getFreeMemory()
{
#if ! defined( _MSC_VER ) && ! defined( __MINGW32__ ) && ! defined( __MINGW64__ )
   long pages = sysconf( _SC_PHYS_PAGES );
   long page_size = sysconf( _SC_PAGE_SIZE );
   return pages * page_size;
#else
   return -1;
#endif
}

inline SystemInfo::CPUInfo
SystemInfo::parseCPUInfo()
{
#if ! defined( _WIN32 )
   CPUInfo info;
   std::ifstream file( "/proc/cpuinfo" );
   if( ! file ) {
      std::cerr << "Unable to read information from /proc/cpuinfo." << std::endl;
      return info;
   }

   char line[ 1024 ];
   std::set< int > processors;
   while( ! file.eof() ) {
      int i;
      file.getline( line, 1024 );
      if( strncmp( line, "physical id", strlen( "physical id" ) ) == 0 ) {
         i = strlen( "physical id" );
         while( line[ i ] != ':' && line[ i ] != '\0' )
            i++;
         processors.insert( atoi( &line[ i + 1 ] ) );
         continue;
      }
      // FIXME: the rest does not work on heterogeneous multi-socket systems
      if( strncmp( line, "model name", strlen( "model name" ) ) == 0 ) {
         i = strlen( "model name" );
         while( line[ i ] != ':' && line[ i ] != '\0' )
            i++;
         info.CPUModelName = &line[ i + 1 ];
         continue;
      }
      if( strncmp( line, "cpu cores", strlen( "cpu cores" ) ) == 0 ) {
         i = strlen( "cpu MHz" );
         while( line[ i ] != ':' && line[ i ] != '\0' )
            i++;
         info.CPUCores = atoi( &line[ i + 1 ] );
         continue;
      }
      if( strncmp( line, "siblings", strlen( "siblings" ) ) == 0 ) {
         i = strlen( "siblings" );
         while( line[ i ] != ':' && line[ i ] != '\0' )
            i++;
         info.CPUThreads = atoi( &line[ i + 1 ] );
      }
   }
   info.numberOfProcessors = processors.size();

   return info;
#else
   return {};
#endif
}

}  // namespace noa::TNL
