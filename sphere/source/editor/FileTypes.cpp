#pragma warning(disable : 4786)

#include <vector>
#include <string>
#include <string.h>
#include "FileTypes.hpp"
using std::string;
using std::vector;


 // format DefaultFileTypeText:DefaultFileTypeExtension(Text1(FileType1Extenstion),FileType2(FileType2Extenstion)
static const char* Extensions[] = {
  /* maps */         "Map Files:rmp(Sphere Map Files(rmp))",
  /* spriteset */    "Spriteset Files:rss(Sphere Spriteset Files(rss))",
  /* scripts */      "Script Files:js(JavaScript Files(js))",
  /* sounds */       "Sound Files:ogg(MP3 Files(mp3,mp2),Ogg Vorbis Files(ogg),MOD Files(mod,s3m,xm,it),WAV Files(wav),FLAC Files(flac))",
  /* fonts */        "Font Files:rfn(Sphere Font Files(rfn))",
  /* windowstyles */ "Window Style Files:rws(Sphere Window Styles(rws))",
  /* images */       "Image Files:png(JPEG Images(jpeg,jpg,jpe),PNG Images(png),PCX Images(pcx),Windows Bitmap Images(bmp),Truevision Targa(tga),Gif(gif))",
  /* animations */   "Animation Files:mng,flic(MNG Animations(mng),FLIC Animations(flic,flc,fli))",
  /* tilesets */     "Tileset Files:rts(Sphere Tileset Files(rts))",
  /* packages */     "Package Files:spk(Sphere Package Files(spk))",
};


CFileTypeLibrary FTL;


////////////////////////////////////////////////////////////////////////////////

CFileTypeLibrary::CFileTypeLibrary()
{
  const int num_extensions = sizeof(Extensions) / sizeof(*Extensions);
  m_FileTypes.resize(num_extensions);

  for (int i = 0; i < num_extensions; i++) {
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
CFileTypeLibrary::GetFileTypeExtensions(int file_type, bool save, vector<string>& extensions)
{
  if (save) {
    if (file_type == 6) {
      for (int i = 0; i < m_FileTypes[file_type].sub_types.size(); i++) {
        for (int j = 0; j < m_FileTypes[file_type].sub_types[i].extensions.size(); j++) {
          std::string ext = m_FileTypes[file_type].sub_types[i].extensions[j];
          if (ext == "png" || ext == "tga")
            extensions.push_back(ext);
        }
      }
    }
    else
      extensions.push_back(FTL.GetDefaultExtension(file_type));
  }
  else {
    for (int i = 0; i < m_FileTypes[file_type].sub_types.size(); i++) {
      for (int j = 0; j < m_FileTypes[file_type].sub_types[i].extensions.size(); j++) {
        extensions.push_back(m_FileTypes[file_type].sub_types[i].extensions[j]);
      }
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
CFileTypeLibrary::GetSubTypeExtensions(int file_type, int sub_type, bool save, vector<string>& extensions)
{
  if (save && file_type == 6) {
    for (int j = 0; j < m_FileTypes[file_type].sub_types[sub_type].extensions.size(); j++) {
      std::string ext = m_FileTypes[file_type].sub_types[sub_type].extensions[j];
      if (ext == "png" || ext == "tga")
        extensions.push_back(ext);
    }
  }
  else {
    extensions = m_FileTypes[file_type].sub_types[sub_type].extensions;
  }
}

////////////////////////////////////////////////////////////////////////////////
