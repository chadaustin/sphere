#pragma warning(disable : 4786)

#include <vector>
#include <string>
#include <string.h>
#include "FileTypes.hpp"
using std::string;
using std::vector;


static const char* Extensions[] = {
  /* maps */         "Map Files:rmp(Sphere Map Files(rmp))",
  /* spriteset */    "Spriteset Files:rss(Sphere Spriteset Files(rss))",
  /* scripts */      "Script Files:js(JavaScript Files(js))",
  /* sounds */       "Sound Files:ogg(MP3 Files(mp3,mp2),Ogg Vorbis Files(ogg),MOD Files(mod,s3m,xm,it),WAV Files(wav),FLAC Files(flac))",
  /* fonts */        "Font Files:rfn(Sphere Font Files(rfn))",
  /* windowstyles */ "Window Style Files:rws(Sphere Window Styles(rws))",
  /* images */       "Image Files:png(JPEG Images(jpeg,jpg,jpe),PNG Images(png),PCX Images(pcx),Windows Bitmap Images(bmp),Truevision Targa(tga))",
  /* animations */   "Animation Files:mng,flic(MNG Animations(mng),FLIC Animations(flic,flc,fli))",
  /* tilesets */     "Tileset Files:rts(Sphere Tileset Files(rts))",
  /* packages */     "Package Files:spk(Sphere Package Files(spk))",
};


CFileTypeLibrary FTL;


////////////////////////////////////////////////////////////////////////////////

CFileTypeLibrary::CFileTypeLibrary()
{
  m_FileTypes.resize(sizeof(Extensions) / sizeof(*Extensions));

  for (int i = 0; i < sizeof(Extensions) / sizeof(*Extensions); i++) {
    const char* s = Extensions[i];
    FileType& ft = m_FileTypes[i];

    // grab the name
    while (*s != ':') {
      ft.name += *s;
      s++;
    }
    s++; // skip ':'

    // grab the default extension
    while (*s != '(') {
      ft.default_extension += *s;
      s++;
    }
    s++;

    // grab all subtypes
    while (*s != ')') {
      SubType st;

      // get label name
      while (*s != '(') {
        st.label += *s;
        s++;
      }
      s++; // skip '('

      // get extensions
      while (*s != ',' && *s != ')') {
        string ext;
        while (*s != ',' && *s != ')') {
          ext += *s;
          s++;
        }
        s++; // skip ',' or ')'

        st.extensions.push_back(ext);
      }
      if (*s == ',') {
        s++;
      }

      ft.sub_types.push_back(st);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

const char*
CFileTypeLibrary::GetFileTypeLabel(int file_type)
{
  return m_FileTypes[file_type].name.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
CFileTypeLibrary::GetFileTypeExtensions(int file_type, vector<string>& extensions)
{
  for (int i = 0; i < m_FileTypes[file_type].sub_types.size(); i++) {
    for (int j = 0; j < m_FileTypes[file_type].sub_types[i].extensions.size(); j++) {
      extensions.push_back(m_FileTypes[file_type].sub_types[i].extensions[j]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

const char*
CFileTypeLibrary::GetDefaultExtension(int file_type)
{
  return m_FileTypes[file_type].default_extension.c_str();
}

////////////////////////////////////////////////////////////////////////////////

int
CFileTypeLibrary::GetNumSubTypes(int file_type)
{
  return m_FileTypes[file_type].sub_types.size();
}

////////////////////////////////////////////////////////////////////////////////

const char*
CFileTypeLibrary::GetSubTypeLabel(int file_type, int sub_type)
{
  return m_FileTypes[file_type].sub_types[sub_type].label.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
CFileTypeLibrary::GetSubTypeExtensions(int file_type, int sub_type, vector<string>& extensions)
{
  extensions = m_FileTypes[file_type].sub_types[sub_type].extensions;
}

////////////////////////////////////////////////////////////////////////////////
