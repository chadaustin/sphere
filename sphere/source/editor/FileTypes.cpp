#pragma warning(disable : 4786)

#include <vector>
#include <string>
#include <string.h>
#include "FileTypes.hpp"
using std::string;
using std::vector;

#include <corona.h>

 // format DefaultFileTypeText:DefaultFileTypeExtension(Text1(FileType1Extenstion),FileType2(FileType2Extenstion)
static const char* Extensions[] = {
  /* maps */         "Map Files:rmp(Sphere Map Files(rmp))",
  /* spriteset */    "Spriteset Files:rss(Sphere Spriteset Files(rss))",
  /* scripts */      "Script Files:js(JavaScript Files(js))",
  /* sounds */       "Sound Files:ogg(MP3 Files(mp3,mp2),Ogg Vorbis Files(ogg),MOD Files(mod,s3m,xm,it),WAV Files(wav),FLAC Files(flac))",
  /* fonts */        "Font Files:rfn(Sphere Font Files(rfn))",
  /* windowstyles */ "Window Style Files:rws(Sphere Window Styles(rws))",
  /* images */       "", //Image Files:png(JPEG Images(jpeg,jpg,jpe),PNG Images(png),PCX Images(pcx),Windows Bitmap Images(bmp),Truevision Targa(tga),Gif(gif),Portable Bitmap(pbm),Portable Graymap(pgm),Portable Pixel Map(ppm))",
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
CFileTypeLibrary::GetFileTypeLabel(int file_type, bool save)
{
  if (file_type == 6) {
    return "Image Files";
  }

  return m_FileTypes[file_type].name.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
CFileTypeLibrary::GetFileTypeExtensions(int file_type, bool save, vector<string>& extensions)
{
  if (file_type == 6) {
    corona::FileFormatDesc** formats =
      save ? corona::GetSupportedWriteFormats() : corona::GetSupportedReadFormats();

    for (size_t i = 0; formats[i]; ++i) {
      for (size_t j = 0; j < 1 && formats[i]->getExtensionCount(); ++j) {
        extensions.push_back(formats[i]->getExtension(j));
      }
    }
      /*
      for (int i = 0; i < m_FileTypes[file_type].sub_types.size(); i++) {
        for (int j = 0; j < m_FileTypes[file_type].sub_types[i].extensions.size(); j++) {
          std::string ext = m_FileTypes[file_type].sub_types[i].extensions[j];
          if (ext == "png" || ext == "tga" || ext == "pbm" || ext == "pgm" || ext == "ppm" || ext == "pcx")
            extensions.push_back(ext);
        }
      }
      */
  }
  if (file_type == 7 && save) {
    extensions.push_back(m_FileTypes[file_type].sub_types[0].extensions[0]);
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
CFileTypeLibrary::GetDefaultExtension(int file_type, bool save)
{
  if (file_type == 6) {
    corona::FileFormatDesc** formats =
      save ? corona::GetSupportedWriteFormats() : corona::GetSupportedReadFormats();
    return formats[0]->getExtension(0);
  }

  return m_FileTypes[file_type].default_extension.c_str();
}

////////////////////////////////////////////////////////////////////////////////

int
CFileTypeLibrary::GetNumSubTypes(int file_type, bool save)
{
  if (file_type == 6) {
    corona::FileFormatDesc** formats =
      save ? corona::GetSupportedWriteFormats() : corona::GetSupportedReadFormats();

    int num_sub_types = 0;
    for (size_t i = 0; formats[i]; ++i) {
      num_sub_types += 1;
    }

    return num_sub_types;
  }

  if (file_type == 7 && save)
    return 1;

  return m_FileTypes[file_type].sub_types.size();
}

////////////////////////////////////////////////////////////////////////////////

const char* GetImageSubTypeLabel(const char* ext) {
  if (strcmp(ext, "png") == 0)
    return "PNG Images";
  if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "jpe") == 0)
    return "JPEG Images";
  if (strcmp(ext, "pcx") == 0)
    return "PCX Images";
  if (strcmp(ext, "bmp") == 0)
    return "Windows Bitmap Images";
  if (strcmp(ext, "tga") == 0)
    return "Truevision Targa";
  if (strcmp(ext, "gif") == 0)
    return "Gif Images";
  if (strcmp(ext, "pbm") == 0)
    return "Portable Bitmap Images";
  if (strcmp(ext, "pgm") == 0)
    return "Portable Graymap Images";
  if (strcmp(ext, "ppm") == 0)
    return "Portable Pixelmap Images";
  return "*.*";
}

////////////////////////////////////////////////////////////////////////////////

const char*
CFileTypeLibrary::GetSubTypeLabel(int file_type, int sub_type, bool save)
{
  if (file_type == 6) {
    corona::FileFormatDesc** formats =
      save ? corona::GetSupportedWriteFormats() : corona::GetSupportedReadFormats();

    for (size_t i = 0; formats[i]; ++i) {
      if (sub_type == i) {
        return GetImageSubTypeLabel(formats[i]->getExtension(0)); // should be getExtensionDescription
      }
    }

    return "";
  }

  return m_FileTypes[file_type].sub_types[sub_type].label.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
CFileTypeLibrary::GetSubTypeExtensions(int file_type, int sub_type, bool save, vector<string>& extensions)
{
  if (file_type == 6) {
    corona::FileFormatDesc** formats =
      save ? corona::GetSupportedWriteFormats() : corona::GetSupportedReadFormats();

    for (size_t i = 0; formats[i]; ++i) {
      if (sub_type == i) {
        for (size_t j = 0; j < formats[i]->getExtensionCount(); ++j) {
          extensions.push_back(formats[i]->getExtension(j));
        }
      }
    }
  }
  else {
    extensions = m_FileTypes[file_type].sub_types[sub_type].extensions;
  }
}

////////////////////////////////////////////////////////////////////////////////
