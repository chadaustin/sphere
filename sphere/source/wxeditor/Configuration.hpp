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


//#include <windows.h>
#include <wx/gdicmn.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <string>
#include <../common/types.h>


// begin namespace

// I had to put these in a non-class namespace so VC++ would accept these template functions...
// I can't be a good programmer and put this in an object in case there needs to be more than one configuration.  ;_;
// Down with VC++!  Long live gcc!
namespace Configuration {


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
    T::type Get(T key)
    {
        wxFileConfig config("sphere-editor", "", ConfigurationFile.c_str(), "", wxCONFIG_USE_LOCAL_FILE);
        //wxConfigBase *oldconfig = wxConfigBase::Set(&config);
        //char str[8193];
        //GetPrivateProfileString("editor", T::keyname, "", str, 8193, ConfigurationFile.c_str());
        wxString str;
        config.Read(T::keyname, &str, "");

        //wxConfigBase::Set(oldconfig);

        int size = GetTypeSize<T::type>(T::type());
        if (strlen(str) < size * 2) {
            return T::default_value;
        }

        uint8* data = new uint8[size];
        for (int i = 0; i < size; i++) {
            data[i] = FromHex(str[i * 2]) * 16 + FromHex(str[i * 2 + 1]);
        }

        T::type t = FromRaw<T::type>(data, T::type());
        delete[] data;
        return t;
    }

    
    inline char ToHex(unsigned i) {
        return (i >= 10 ? ('A' + i - 10) : ('0' + i));
    }


    // Sets a configuration setting
    // Usage:  Configuration::Set(KEY, value);
    template<typename T>
    void Set(T key, T::type val)
    {
        uint8* data = (uint8*)ToRaw(val);
        std::string hex;
        for (int i = 0; i < GetTypeSize<T::type>(T::type()); i++) {
            hex += ToHex(data[i] >> 4);
            hex += ToHex(data[i] & 0x0F);
        }

        wxFileConfig config("sphere-editor", "", ConfigurationFile.c_str(), "", wxCONFIG_USE_LOCAL_FILE);
        config.Write(T::keyname, hex.c_str());
        //WritePrivateProfileString("editor", T::keyname, hex.c_str(), ConfigurationFile.c_str());
    }
    


    // WARNING! These functions/variables should not be used outside of this namespace


    // if I have a default implementation and a specialized implementation, the default implementation is never called
    // therefore, there are specialized implementations for everything...
    // Down with VC++!  Long live gcc!

    // *** GetTypeSize() ***

    template<typename T> GetTypeSize(T t);

    // GRAH, how many of these template problems will I find in one day?
    // these functions now have a worthless parameter like FromRaw does
    inline int GetTypeSize<bool>(bool b = bool()) { return sizeof(bool); }
    inline int GetTypeSize<int>(int i = int()) { return sizeof(int); }
    inline int GetTypeSize<double>(double d = double()) { return sizeof(double); }
    inline int GetTypeSize<std::string>(std::string s = std::string()) { return 4096; } // only supports up to 4096-1 characters
//    inline int GetTypeSize<WINDOWPLACEMENT>(WINDOWPLACEMENT wp = WINDOWPLACEMENT()) { return sizeof(WINDOWPLACEMENT); }
//    inline int GetTypeSize<RECT>(RECT w = RECT()) { return sizeof(RECT); }
    inline int GetTypeSize<wxRect>(wxRect w = wxRect()) { return sizeof(wxRect); }

    
    // *** ToRaw() ***

    template<typename T> void* ToRaw(T val);

    template<>
    inline void* ToRaw<bool>(bool val)
    {
        uint8* b = new uint8[sizeof(bool)];
        memcpy(b, &val, sizeof(bool));
        return b;
    }

    template<>
    inline void* ToRaw<int>(int val)
    {
        uint8* b = new uint8[sizeof(int)];
        memcpy(b, &val, sizeof(int));
        return b;
    }

    template<>
    inline void* ToRaw<double>(double val)
    {
        uint8* b = new uint8[sizeof(double)];
        memcpy(b, &val, sizeof(double));
        return b;
    }

    template<>
    inline void* ToRaw<std::string>(std::string val)
    {
        char* str = new char[GetTypeSize<std::string>(std::string())];
        strcpy(str, val.c_str());
        return str;
    }

/*
    template<>
    inline void* ToRaw<WINDOWPLACEMENT>(WINDOWPLACEMENT wp)
    {
      uint8* b = new uint8[sizeof(WINDOWPLACEMENT)];
      memcpy(b, &wp, sizeof(WINDOWPLACEMENT));
      return b;
    }

    template<>
    inline void* ToRaw<RECT>(RECT r)
    {
      uint8* b = new uint8[sizeof(RECT)];
      memcpy(b, &r, sizeof(RECT));
      return b;
    }
*/
    template<>
    inline void* ToRaw<wxRect>(wxRect r)
    {
      uint8* b = new uint8[sizeof(wxRect)];
      memcpy(b, &r, sizeof(wxRect));
      return b;
    }

    // *** FromRaw() ***

    template<typename T> T FromRaw(void* raw, T t = T());

    // due to yet *another* bug in VC++, I added an extra parameter to these functions so they can be looked up properly
    // Down with VC++!  Long live gcc!

    template<>    
    inline bool FromRaw<bool>(void* raw, bool b)
    {
        bool t;
        memcpy(&t, raw, sizeof(bool));
        return t;
    }

    template<>
    inline int FromRaw<int>(void* raw, int i)
    {
        int t;
        memcpy(&t, raw, sizeof(int));
        return t;
    }

    template<>    
    inline double FromRaw<double>(void* raw, double d)
    {
        double t;
        memcpy(&t, raw, sizeof(double));
        return t;
    }

    template<>    
    inline std::string FromRaw<std::string>(void* raw, std::string s)
    {
        return std::string((char*)raw);
    }
/*
    template<>
    inline WINDOWPLACEMENT FromRaw<WINDOWPLACEMENT>(void* raw, WINDOWPLACEMENT /*wp* /)
    {
      WINDOWPLACEMENT wp;
      memcpy(&wp, raw, sizeof(WINDOWPLACEMENT));
      return wp;
    }

    template<>
    inline RECT FromRaw<RECT>(void* raw, RECT /*r* /)
    {
      RECT r;
      memcpy(&r, raw, sizeof(RECT));
      return r;
    }
*/
    template<>
    inline wxRect FromRaw<wxRect>(void* raw, wxRect /*r*/)
    {
      wxRect r;
      memcpy(&r, raw, sizeof(wxRect));
      return r;
    }
}

// end namespace


#endif
