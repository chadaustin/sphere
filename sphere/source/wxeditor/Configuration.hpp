// Configuration server
// It will store and retrieve aggregate data and C++ std::strings
// I broke a lot of coding conventions here, because VC++ doesn't deal with templates well
// Quite frankly, this code is awful.  However, it works properly since I spent so much time
//   working around bugs in VC++'s implementation of template functions
//
// Summary of use:
//   Okay, the Configuration namespace is a smart state server which allows you to access
//   data via objects known as keys.  Each key has an identification string, a data type,
//   and a default value associated with it.  A key is an instance of any struct that has
//   a typedef in it named "type" and a static const tchar* named "keyname" along with a
//   default_value field.
//
// If you have a key named "KEY_MY_DATA" which represents a boolean value, you could read
// from the configuration with code like this:
// bool b = Configuration::Get(KEY_MY_DATA);
//
// If you want to store a new value in the configuration database, do this:
// Configuration::Set(KEY_MY_DATA, true);


#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP


#include <wx/gdicmn.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <string>
#include "../common/types.h"


// VC++ 6 is really pathetic
#if defined(_MSC_VER) && _MSC_VER <= 1200
  #define HACK_TYPENAME
#else
  #define HACK_TYPENAME typename
#endif


// begin namespace

// I had to put these in a non-class namespace so VC++ would accept these template functions...
// I can't be a good programmer and put this in an object in case there needs to be more than one configuration.  ;_;
// Down with VC++!  Long live gcc!
namespace Configuration {


    template <typename T>
    struct Type2Type {
        typedef T Type;
    };


    extern std::string ConfigurationFile;


    inline unsigned FromHex(char c) {
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }

        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }

        if (c >= '0' && c <= '9') {
          return c - '0';
        }

        return 0;
    }


    // Retrieves a configuration setting
    // Usage:  var = Configuration::Get(KEY);
    template<typename T>
    typename T::type Get(T key)
    {
        wxFileConfig config("sphere-editor", "", ConfigurationFile.c_str(), "", wxCONFIG_USE_LOCAL_FILE);
        //wxConfigBase *oldconfig = wxConfigBase::Set(&config);
        //char str[8193];
        //GetPrivateProfileString("editor", T::keyname, "", str, 8193, ConfigurationFile.c_str());
        wxString str;
        config.Read(T::keyname, &str, "");

        //wxConfigBase::Set(oldconfig);

        int size = GetTypeSize(HACK_TYPENAME T::type());
        if (strlen(str) < unsigned(size * 2)) {
            return T::default_value;
        }

        uint8* data = new uint8[size];
        for (unsigned i = 0; i < unsigned(size); i++) {
            data[i] = FromHex(str[i * 2]) * 16 + FromHex(str[i * 2 + 1]);
        }

        HACK_TYPENAME T::type t = FromRaw(data, Type2Type<HACK_TYPENAME T::type>());
        delete[] data;
        return t;
    }

    
    inline char ToHex(unsigned i) {
        return (i >= 10 ? ('A' + i - 10) : ('0' + i));
    }


    // Sets a configuration setting
    // Usage:  Configuration::Set(KEY, value);
    template<typename T>
    void Set(T key, typename T::type val)
    {
        uint8* data = (uint8*)ToRaw(val);
        std::string hex;
        for (int i = 0; i < GetTypeSize(HACK_TYPENAME T::type()); i++) {
            hex += ToHex(data[i] >> 4);
            hex += ToHex(data[i] & 0x0F);
        }
        delete[] data;

        wxFileConfig config("sphere-editor", "", ConfigurationFile.c_str(), "", wxCONFIG_USE_LOCAL_FILE);
        config.Write(T::keyname, hex.c_str());
    }
    


    // WARNING! These functions/variables should not be used outside of this namespace


    // *** GetTypeSize() ***

    inline int GetTypeSize(bool b = bool()) { return sizeof(bool); }
    inline int GetTypeSize(int i = int()) { return sizeof(int); }
    inline int GetTypeSize(double d = double()) { return sizeof(double); }
    inline int GetTypeSize(std::string s = std::string()) { return 4096; } // only supports up to 4096-1 characters
    inline int GetTypeSize(wxRect w = wxRect()) { return sizeof(wxRect); }

    
    // *** ToRaw() ***

    inline void* ToRaw(bool val)
    {
        uint8* b = new uint8[sizeof(bool)];
        memcpy(b, &val, sizeof(bool));
        return b;
    }

    inline void* ToRaw(int val)
    {
        uint8* b = new uint8[sizeof(int)];
        memcpy(b, &val, sizeof(int));
        return b;
    }

    inline void* ToRaw(double val)
    {
        uint8* b = new uint8[sizeof(double)];
        memcpy(b, &val, sizeof(double));
        return b;
    }

    inline void* ToRaw(std::string val)
    {
        char* str = new char[GetTypeSize(val)];
        strcpy(str, val.c_str());
        return str;
    }

    inline void* ToRaw(wxRect r)
    {
      uint8* b = new uint8[sizeof(wxRect)];
      memcpy(b, &r, sizeof(wxRect));
      return b;
    }

    // *** FromRaw() ***

    inline bool FromRaw(void* raw, Type2Type<bool>)
    {
        bool t;
        memcpy(&t, raw, sizeof(bool));
        return t;
    }

    inline int FromRaw(void* raw, Type2Type<int>)
    {
        int t;
        memcpy(&t, raw, sizeof(int));
        return t;
    }

    inline double FromRaw(void* raw, Type2Type<double>)
    {
        double t;
        memcpy(&t, raw, sizeof(double));
        return t;
    }

    inline std::string FromRaw(void* raw, Type2Type<std::string>)
    {
        return std::string((char*)raw);
    }

    inline wxRect FromRaw(void* raw, Type2Type<wxRect>)
    {
      wxRect r;
      memcpy(&r, raw, sizeof(wxRect));
      return r;
    }
}

// end namespace


#endif
