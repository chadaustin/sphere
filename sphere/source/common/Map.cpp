#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <string>
#include <memory>
#include <stdlib.h>
#include "Map.hpp"
#include "Layer.hpp"
#include "Entities.hpp"
#include "Tileset.hpp"
#include "Image32.hpp"
#include "packed.hpp"
#include "types.h"
#include "strcmp_ci.hpp"


////////////////////////////////////////////////////////////////////////////////

sMap::sMap()
: m_StartX(0)
, m_StartY(0)
, m_StartLayer(0)
, m_StartDirection(0)
{
}

////////////////////////////////////////////////////////////////////////////////

sMap::sMap(int width, int height, int layers)
: m_StartX(0)
, m_StartY(0)
, m_StartLayer(0)
, m_StartDirection(0)
{
  m_Layers.resize(layers);
  for (int i = 0; i < layers; i++)
    m_Layers[i].Resize(width, height);
}

////////////////////////////////////////////////////////////////////////////////

sMap::~sMap()
{
}

////////////////////////////////////////////////////////////////////////////////

#define STRUCT_NAME MAP_HEADER
#define STRUCT_BODY                             \
  byte signature[4];                            \
  word version;                                 \
  byte obsolete; /* type */                     \
  byte num_layers;                              \
  byte reserved0;                               \
  word num_entities;                            \
  word startx;                                  \
  word starty;                                  \
  byte startlayer;                              \
  byte startdirection;                          \
  word num_strings;                             \
  word num_zones;                               \
  byte reserved[235];
#include "packed_struct.h"

#define STRUCT_NAME LAYER_HEADER
#define STRUCT_BODY                             \
  word    width;                                \
  word    height;                               \
  word    flags;                                \
  float32 parallax_x;                           \
  float32 parallax_y;                           \
  float32 scrolling_x;                          \
  float32 scrolling_y;                          \
  dword   num_segments;                         \
  byte    reflective;                           \
  byte    reserved[3];
#include "packed_struct.h"

#define STRUCT_NAME ENTITY_HEADER
#define STRUCT_BODY                             \
  word mapx;                                    \
  word mapy;                                    \
  word layer;                                   \
  word type;                                    \
  byte reserved[8];
#include "packed_struct.h"

#define STRUCT_NAME ZONE_HEADER
#define STRUCT_BODY                             \
  word x1;                                      \
  word y1;                                      \
  word x2;                                      \
  word y2;                                      \
  word layer;                                   \
  word reactivate_in_num_steps;                 \
  byte reserved[4];
#include "packed_struct.h"

////////////////////////////////////////////////////////////////////////////////

ASSERT_STRUCT_SIZE(MAP_HEADER,    256)
ASSERT_STRUCT_SIZE(LAYER_HEADER,  30)
ASSERT_STRUCT_SIZE(ENTITY_HEADER, 16)
ASSERT_STRUCT_SIZE(ZONE_HEADER,   16)

////////////////////////////////////////////////////////////////////////////////

inline byte ReadMapByte(IFile* file)
{
  byte b;
  file->Read(&b, 1);
  return b;
}

inline byte ReadMapByte(const std::auto_ptr<IFile>& file)
{
  return ReadMapByte(file.get());
}

////////////////////////////////////////////////////////////////////////////////

inline word ReadMapWord(IFile* file)
{
  word w;
  file->Read(&w, 2);
  return w;
}

inline word ReadMapWord(const std::auto_ptr<IFile>& file)
{
  return ReadMapWord(file.get());
}

////////////////////////////////////////////////////////////////////////////////

inline std::string ReadMapString(IFile* file)
{
  std::string s;
  word length = ReadMapWord(file);
  for (int i = 0; i < length; i++) {
    char c;
    file->Read(&c, 1);
    s += c;
  }
  return s;
}

inline std::string ReadMapString(const std::auto_ptr<IFile>& file)
{
  return ReadMapString(file.get());
}

////////////////////////////////////////////////////////////////////////////////

inline void SkipMapBytes(IFile* file, int bytes)
{
  file->Seek(file->Tell() + bytes);
}

inline void SkipMapBytes(const std::auto_ptr<IFile>& file, int bytes)
{
  SkipMapBytes(file.get(), bytes);
}

////////////////////////////////////////////////////////////////////////////////

bool
sMap::Load(const char* filename, IFileSystem& fs)
{
  // open the file
  std::auto_ptr<IFile> file(fs.Open(filename, IFileSystem::read));
  if (!file.get()) {
    return false;
  }

  // read the header
  MAP_HEADER header;
  file->Read(&header, sizeof(header));

  // make sure it's valid
  if (memcmp(header.signature, ".rmp", 4) != 0 ||
      header.version != 1 ||
      (header.num_strings != 3 && header.num_strings != 5 && header.num_strings != 9))
  {
    return false;
  }

  m_StartX         = header.startx;
  m_StartY         = header.starty;
  m_StartLayer     = header.startlayer;
  m_StartDirection = header.startdirection;

  // read the strings (tileset, music, script)
  std::string tileset_file = ReadMapString(file); // OBSOLETE
  m_MusicFile   = ReadMapString(file);
  ReadMapString(file);  // script file
  if (header.num_strings == 3) {
    m_EntryScript = "";
    m_ExitScript  = "";
  } else {
    m_EntryScript = ReadMapString(file);
    m_ExitScript  = ReadMapString(file);
  }
  if (header.num_strings > 5) {
    m_EdgeScripts[0] = ReadMapString(file);
    m_EdgeScripts[1] = ReadMapString(file);
    m_EdgeScripts[2] = ReadMapString(file);
    m_EdgeScripts[3] = ReadMapString(file);
  }
  
  // delete the old layer array and allocate a new one
  m_Layers.clear();
  m_Layers.resize(header.num_layers);

  // read the layers
  for (int i = 0; i < header.num_layers; i++)
  {
    // read the layer header
    LAYER_HEADER lh;
    file->Read(&lh, sizeof(lh));

    // read the layer name
    std::string name = ReadMapString(file);

    // set all layer attributes
    m_Layers[i].SetName(name.c_str());
    m_Layers[i].Resize(lh.width, lh.height);
    m_Layers[i].SetXParallax(lh.parallax_x);
    m_Layers[i].SetYParallax(lh.parallax_y);
    m_Layers[i].SetXScrolling(lh.scrolling_x);
    m_Layers[i].SetYScrolling(lh.scrolling_y);
    m_Layers[i].SetVisible((lh.flags & 1) == 0);
    m_Layers[i].EnableParallax((lh.flags & 2) != 0);
    m_Layers[i].SetReflective(lh.reflective != 0);

    // read the layer data
    for (int iy = 0; iy < lh.height; iy++) {
      for (int ix = 0; ix < lh.width; ix++) {
        word tile;
        file->Read(&tile, sizeof(tile));
        m_Layers[i].SetTile(ix, iy, tile);
      }
    }

    // load the obstruction map

    for (unsigned j = 0; j < lh.num_segments; j++) {

      dword x1; file->Read(&x1, sizeof(dword));
      dword y1; file->Read(&y1, sizeof(dword));
      dword x2; file->Read(&x2, sizeof(dword));
      dword y2; file->Read(&y2, sizeof(dword));

      m_Layers[i].GetObstructionMap().AddSegment(x1, y1, x2, y2);
    }

  } // end for layer

  // delete the old entities
  m_Entities.clear();

  // read entities
  for (int i = 0; i < header.num_entities; i++)
  {
    ENTITY_HEADER eh;
    file->Read(&eh, sizeof(eh));

    sEntity* entity;
    switch (eh.type)
    {
      // PERSON
      case 1:
      {
        sPersonEntity* person = new sPersonEntity;

        // read the person data
        person->name      = ReadMapString(file);
        person->spriteset = ReadMapString(file);

        word num_strings = ReadMapWord(file);

        // strings
        if (num_strings >= 1) person->script_create            = ReadMapString(file);
        if (num_strings >= 2) person->script_destroy           = ReadMapString(file);
        if (num_strings >= 3) person->script_activate_touch    = ReadMapString(file);
        if (num_strings >= 4) person->script_activate_talk     = ReadMapString(file);
        if (num_strings >= 5) person->script_generate_commands = ReadMapString(file);

        // reserved
        for (int i = 0; i < 16; i++)
          ReadMapByte(file);

        entity = person;
        break;
      }

      // TRIGGER
      case 2:
      {
        sTriggerEntity* trigger = new sTriggerEntity;

        // read/set the trigger data
        trigger->script = ReadMapString(file);

        entity = trigger;
        break;
      }

      default:  // unknown
        continue;

    } // end switch

    entity->x = eh.mapx;
    entity->y = eh.mapy;
    entity->layer = eh.layer;

    AddEntity(entity);
  }

  // clear the zones
  m_Zones.clear();

  // load the zones
  for (int i = 0; i < header.num_zones; i++) 
  {
    ZONE_HEADER zh;
    sZone zone;

    file->Read(&zh, sizeof(zh));
    
    if (zh.x1 > zh.x2) {
      std::swap(zh.x1, zh.x2);
    }
    if (zh.y1 > zh.y2) {
      std::swap(zh.y1, zh.y2);
    }
    zone.x1 = zh.x1;
    zone.y1 = zh.y1;
    zone.x2 = zh.x2;
    zone.y2 = zh.y2;
    zone.layer = zh.layer;
    zone.reactivate_in_num_steps = zh.reactivate_in_num_steps;

    zone.script = ReadMapString(file);
    m_Zones.push_back(zone);
  }


  // if no tileset file was specified, it is appended to the map file
  if (tileset_file.length() == 0)
  {
    if (!m_Tileset.LoadFromFile(file.get()))
    {
      return false;
    }
  }
  else
    m_Tileset.Clear();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

inline void WriteMapByte(IFile* file, byte b)
{
  file->Write(&b, 1);
}

inline void WriteMapByte(const std::auto_ptr<IFile>& file, byte b)
{
  WriteMapByte(file.get(), b);
}

////////////////////////////////////////////////////////////////////////////////

inline void WriteMapWord(IFile* file, word w)
{
  file->Write(&w, 2);
}

inline void WriteMapWord(const std::auto_ptr<IFile>& file, word w)
{
  WriteMapWord(file.get(), w);
}

////////////////////////////////////////////////////////////////////////////////

inline void WriteMapString(IFile* file, const char* string)
{
  word len = strlen(string);
  WriteMapWord(file, len);
  file->Write(string, len);
}

inline void WriteMapString(const std::auto_ptr<IFile>& file, const char* string)
{
  WriteMapString(file.get(), string);
}

////////////////////////////////////////////////////////////////////////////////

bool
sMap::Save(const char* filename, IFileSystem& fs)
{
  // do some preliminary checking...
  // the start layer should not have parallax
  m_Layers[m_StartLayer].EnableParallax(false);

  std::auto_ptr<IFile> file(fs.Open(filename, IFileSystem::write));
  if (!file.get())
    return false;

  // write the map header
  MAP_HEADER header;
  memset(&header, 0, sizeof(header));
  memcpy(header.signature, ".rmp", 4);
  header.version        = 1;
  header.num_layers     = m_Layers.size();
  header.num_entities   = m_Entities.size();
  header.startx         = m_StartX;
  header.starty         = m_StartY;
  header.startlayer     = m_StartLayer;
  header.startdirection = m_StartDirection;
  header.num_strings    = 9;
  header.num_zones      = m_Zones.size();
  file->Write(&header, sizeof(header));

  // write the strings
  WriteMapString(file, "");  // OBSOLETE
  WriteMapString(file, m_MusicFile.c_str());
  WriteMapString(file, "");  // OBSOLETE
  WriteMapString(file, m_EntryScript.c_str());
  WriteMapString(file, m_ExitScript.c_str());
  WriteMapString(file, m_EdgeScripts[0].c_str());
  WriteMapString(file, m_EdgeScripts[1].c_str());
  WriteMapString(file, m_EdgeScripts[2].c_str());
  WriteMapString(file, m_EdgeScripts[3].c_str());
 
  // write layers
  for (unsigned i = 0; i < m_Layers.size(); i++)
  {
    const sLayer& layer = m_Layers[i];
    const sObstructionMap& obstructions = layer.GetObstructionMap();

    // write the header
    LAYER_HEADER lh;
    memset(&lh, 0, sizeof(lh));
    lh.width        = m_Layers[i].GetWidth();
    lh.height       = m_Layers[i].GetHeight();
    lh.flags        = (m_Layers[i].IsVisible() ? 0 : 1) | (m_Layers[i].HasParallax() ? 2 : 0);
    lh.parallax_x   = m_Layers[i].GetXParallax();
    lh.parallax_y   = m_Layers[i].GetYParallax();
    lh.scrolling_x  = m_Layers[i].GetXScrolling();
    lh.scrolling_y  = m_Layers[i].GetYScrolling();
    lh.num_segments = obstructions.GetNumSegments();
    lh.reflective   = (m_Layers[i].IsReflective() ? 1 : 0);
    file->Write(&lh, sizeof(lh));

    // write the layer name
    WriteMapString(file, m_Layers[i].GetName());

    // write the layer data
    for (int iy = 0; iy < m_Layers[i].GetHeight(); iy++)
      for (int ix = 0; ix < m_Layers[i].GetWidth(); ix++)
      {
        word w = m_Layers[i].GetTile(ix, iy);
        file->Write(&w, 2);
      }

    // write the obstruction map
    for (int i = 0; i < obstructions.GetNumSegments(); i++) {
      const sObstructionMap::Segment& s = obstructions.GetSegment(i);

      dword x1 = s.x1;
      dword y1 = s.y1;
      dword x2 = s.x2;
      dword y2 = s.y2;

      file->Write(&x1, sizeof(dword));
      file->Write(&y1, sizeof(dword));
      file->Write(&x2, sizeof(dword));
      file->Write(&y2, sizeof(dword));
    }

  } // end for layer

  // write entities
  for (unsigned i = 0; i < m_Entities.size(); i++)
  {
    // write the header
    ENTITY_HEADER eh;
    memset(&eh, 0, sizeof(eh));
    eh.mapx  = m_Entities[i]->x;
    eh.mapy  = m_Entities[i]->y;
    eh.layer = m_Entities[i]->layer;
    switch (m_Entities[i]->GetEntityType())
    {
      case sEntity::PERSON:  eh.type = 1; break;
      case sEntity::TRIGGER: eh.type = 2; break;
      default: break; // prevent a gcc warning (unhandled enum)
    }
    file->Write(&eh, sizeof(eh));

    // write the entity data
    switch (m_Entities[i]->GetEntityType())
    {
      case sEntity::PERSON:
      {
        sPersonEntity* person = (sPersonEntity*)m_Entities[i];

        WriteMapString(file, person->name.c_str());
        WriteMapString(file, person->spriteset.c_str());

        WriteMapWord(file, 5);  // four scripts
        
        // scripts
        WriteMapString(file, person->script_create.c_str());
        WriteMapString(file, person->script_destroy.c_str());
        WriteMapString(file, person->script_activate_touch.c_str());
        WriteMapString(file, person->script_activate_talk.c_str());
        WriteMapString(file, person->script_generate_commands.c_str());

        // reserved
        for (int i = 0; i < 16; i++) {
          WriteMapByte(file, 0);
        }

        break;
      }

      case sEntity::TRIGGER:
      {
        sTriggerEntity* trigger = (sTriggerEntity*)m_Entities[i];
        WriteMapString(file, trigger->script.c_str());
        break;
      }

      default:  // fix gcc3 "unhandled enum" warning
        break;

    } // end switch entity type
  } // end for entity

  // write the zones
  for (unsigned i = 0; i < m_Zones.size(); i++)
  {
    ZONE_HEADER zh;

    memset(&zh, 0, sizeof(zh));
    zh.x1 = m_Zones[i].x1;
    zh.y1 = m_Zones[i].y1;
    zh.x2 = m_Zones[i].x2;
    zh.y2 = m_Zones[i].y2;
    zh.layer = m_Zones[i].layer;
    zh.reactivate_in_num_steps = m_Zones[i].reactivate_in_num_steps;

    file->Write(&zh, sizeof(zh));
    WriteMapString(file, m_Zones[i].script.c_str());
  } // end for zones

  // save the tileset
  return m_Tileset.SaveToFile(file.get());
}

////////////////////////////////////////////////////////////////////////////////

bool
sMap::Create(int width, int height, int layers)
{
  // clear out the old map
  m_MusicFile   = "";
  m_EntryScript = "";
  m_ExitScript  = "";

  m_Layers.clear();
  m_Entities.clear();

  // allocate the new map
  m_Layers.resize(layers);
  for (int i = 0; i < layers; i++)
    m_Layers[i].Resize(width, height);

  // put a default tile in the tileset
  m_Tileset.Create(1);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
sMap::BuildFromImage(CImage32& i, int tile_width, int tile_height, bool allow_duplicates)
{
  int num_tiles_x = (i.GetWidth()  + tile_width - 1)  / tile_width;
  int num_tiles_y = (i.GetHeight() + tile_height - 1) / tile_height;

  CImage32 image = i;
  image.Resize(num_tiles_x * tile_width, num_tiles_y * tile_height);

  if (!m_Tileset.BuildFromImage(image, tile_width, tile_height, allow_duplicates))
    return false;

  // clear out the old map
  m_MusicFile   = "";
  m_EntryScript = "";
  m_ExitScript  = "";

  m_Layers.clear();
  m_Entities.clear();

  // create the single layer
  m_Layers.resize(1);
  m_Layers[0].Resize(num_tiles_x, num_tiles_y);

  // fill it in
  for (int ty = 0; ty < num_tiles_y; ty++)
    for (int tx = 0; tx < num_tiles_x; tx++)
    {
      int src_x = tx * tile_width;
      int src_y = ty * tile_height;

      // grab a tile
      RGBA* tile = new RGBA[tile_width * tile_height];
      for (int iy = 0; iy < tile_height; iy++)
      {
        memcpy(tile + iy * tile_width,
               image.GetPixels() + (src_y + iy) * image.GetWidth() + src_x,
               tile_width * sizeof(RGBA));
      }

      // find which tile it is in the tileset
      int t = 0;
      for (int i = 0; i < m_Tileset.GetNumTiles(); i++)
        if (memcmp(tile, m_Tileset.GetTile(i).GetPixels(), tile_width * tile_height * sizeof(RGBA)) == 0)
        {
          t = i;
          break;
        }

      m_Layers[0].SetTile(tx, ty, t);

      delete[] tile;
    }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::PruneTileset(std::set<int>* allowed_tiles)
{
  // FIXME
  // this code has an absolutely hideous running time.  it's something
  // like O(t^2 * l * w * h).  It could easily be O(t * l * w * h) using
  // some dynamic programming.

  for (int it = 0; it < m_Tileset.GetNumTiles(); it++) {
    
    bool in_use = false;

    // check to see if the tile is in an animation
    if (m_Tileset.GetTile(it).GetDelay() > 0) {
      for (int k = 0; k < m_Tileset.GetNumTiles(); k++) {
        if (m_Tileset.GetTile(it).GetNextTile() == k) {
          in_use = true;
          goto done;
        }
      }
    }

    for (unsigned il = 0; il < m_Layers.size(); il++) {
      for (int iy = 0; iy < m_Layers[il].GetHeight(); iy++) {
        for (int ix = 0; ix < m_Layers[il].GetWidth(); ix++) {
          if (m_Layers[il].GetTile(ix, iy) == it) {
            in_use = true;
            goto done;  // break out
          }
        }
      }
    }

done:
    if (!in_use && (allowed_tiles == NULL || allowed_tiles->count(it))) {
      m_Tileset.DeleteTiles(it, 1);

      // update tile animations
      for (int k = it; k < m_Tileset.GetNumTiles(); k++)
        m_Tileset.GetTile(k).SetNextTile(m_Tileset.GetTile(k).GetNextTile() - 1);
 
      // now update all of the layers
      for (unsigned il = 0; il < m_Layers.size(); il++) {
        for (int iy = 0; iy < m_Layers[il].GetHeight(); iy++) {
          for (int ix = 0; ix < m_Layers[il].GetWidth(); ix++) {
        
            int tile = m_Layers[il].GetTile(ix, iy);
            if (tile > it) {
              m_Layers[il].SetTile(ix, iy, tile - 1);
            }

          }
        }
      }

      it--; // process this tile index again

    } // end if (!in_use)

  }
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::ValidateTileIndices() {
  for (unsigned l = 0; l < m_Layers.size(); ++l) {
    sLayer& layer = m_Layers[l];
    for (int y = 0; y < layer.GetHeight(); ++y) {
      for (int x = 0; x < layer.GetWidth(); ++x) {
        if (layer.GetTile(x, y) >= m_Tileset.GetNumTiles()) {
          layer.SetTile(x, y, 0);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetTileSize(int width, int height, bool rescale)
{
  int old_width  = m_Tileset.GetTileWidth();
  int old_height = m_Tileset.GetTileHeight();
  m_Tileset.SetTileSize(width, height, rescale);

  if (old_width)  { m_StartX = m_StartX * width  / old_width;  }
  if (old_height) { m_StartY = m_StartY * height / old_height; }

  for (int i = 0; i < GetNumEntities(); ++i) {
    sEntity& e = GetEntity(i);
    if (old_width)  { e.x = e.x * width  / old_width;  }
    if (old_height) { e.y = e.y * height / old_height; }
  }

  for (int i = 0; i < GetNumZones(); ++i) {
    sZone& z = GetZone(i);
    if (old_width)  { z.x1 = z.x1 * width  / old_width;  }
    if (old_height) { z.y1 = z.y1 * height / old_height; }
    if (old_width)  { z.x2 = z.x2 * width  / old_width;  }
    if (old_height) { z.y2 = z.y2 * height / old_height; }
  }
}

////////////////////////////////////////////////////////////////////////////////

#define STRUCT_NAME V1MAP_HEADER
#define STRUCT_BODY                             \
  byte version;                                 \
  byte vsp_fname[13];                           \
  byte music_fname[13];                         \
  byte parallax_mode;                           \
  byte parallax_multiplier;                     \
  byte parallax_divisor;                        \
  byte level_name[30];                          \
  byte level_showname;                          \
  byte level_saveable;                          \
  word x_start;                                 \
  word y_start;                                 \
  byte level_hideable;                          \
  byte level_warpable;                          \
  word layer_size_x;                            \
  word layer_size_y;                            \
  byte reservedC[28];
#include "packed_struct.h"

#define STRUCT_NAME V2MAP_HEADER
#define STRUCT_BODY                             \
  byte signature[6];                            \
  dword empty;                                  \
  byte  vsp_name[60];                           \
  byte  music_name[60];                         \
  byte  renderstring[20];                       \
  word  x_start;                                \
  word  y_start;                                \
  byte  reserved[51];                           \
  byte  num_layers;
#include "packed_struct.h"

#define STRUCT_NAME V2MAP_LAYERINFO
#define STRUCT_BODY                             \
  byte  multx, pdivx;                           \
  byte  multy, pdivy;                           \
  word  size_x, size_y;                         \
  byte  transparent, hline;
#include "packed_struct.h"

////////////////////////////////////////////////////////////////////////////////

ASSERT_STRUCT_SIZE(V1MAP_HEADER, 100)
ASSERT_STRUCT_SIZE(V2MAP_HEADER, 206)

////////////////////////////////////////////////////////////////////////////////

bool
sMap::Import_VergeMAP(const char* filename, const char* tilesetFilename, IFileSystem& fs)
{
  m_MusicFile   = "";
  m_EntryScript = "";
  m_ExitScript  = "";
  m_Layers.clear();
  m_Entities.clear();

  std::auto_ptr<IFile> file(fs.Open(filename, IFileSystem::read));
  if (!file.get()) 
    return false;

  // check for v1 maps (ver 4) and v2 maps (ver 5)
  char signature[6];
  file->Read(signature, 6);
  file->Seek(0);

  bool success = false;

  if (signature[0] == 4)
  {
    V1MAP_HEADER header;
    file->Read(&header, sizeof(header));

    word* layer_background = new word[header.layer_size_x * header.layer_size_y];
    word* layer_foreground = new word[header.layer_size_x * header.layer_size_y];
    file->Read(layer_background, header.layer_size_x * header.layer_size_y * sizeof(word));
    file->Read(layer_foreground, header.layer_size_x * header.layer_size_y * sizeof(word));

    sTileset tileset;
    if (strcmp_ci(tilesetFilename + strlen(tilesetFilename) - 4, ".vsp") == 0)
      success = tileset.Import_VSP(tilesetFilename, fs);
    else
      success = tileset.Load(tilesetFilename, fs);

    sLayer layer[2];

    if (success)
    {
      // process map and see if the map has tiles that are out of range
      int highestTileIndex = 0;
      for (int j=0; j<header.layer_size_y; j++)
        for (int i=0; i<header.layer_size_x; i++)
          if (layer_background[j * header.layer_size_x + i] >= highestTileIndex)
            highestTileIndex = layer_background[j * header.layer_size_x + i];
          else if (layer_foreground[j * header.layer_size_x + i] >= highestTileIndex)
            highestTileIndex = layer_foreground[j * header.layer_size_x + i];

      if (highestTileIndex >= tileset.GetNumTiles())
        success = false;

      // transfer data across into the sMap now...
      if (success)
      {
        layer[0].SetName("Background");
        layer[1].SetName("Foreground");
        layer[0].Resize(header.layer_size_x, header.layer_size_y);
        layer[1].Resize(header.layer_size_x, header.layer_size_y);

        for (int j=0; j<header.layer_size_y; j++)
          for (int i=0; i<header.layer_size_x; i++)
          {
            layer[0].SetTile(i,j, layer_background[j * header.layer_size_x + i]);

            if (layer_foreground[j * header.layer_size_x + i])
            layer[1].SetTile(i,j, layer_foreground[j * header.layer_size_x + i]);
            else
              layer[1].SetTile(i,j, tileset.GetNumTiles());
          }

        tileset.AppendTiles(1);
        memset(tileset.GetTile(tileset.GetNumTiles() - 1).GetPixels(), 0, 256 * sizeof(RGBA));
        m_Tileset = tileset;
        AppendLayer(layer[0]);
        AppendLayer(layer[1]);
        SetMusicFile((char*)header.music_fname);
        SetStartX(header.x_start);
        SetStartX(header.y_start);
        SetStartDirection(4);

        // calculate the parallax mode
        for (int i=0; i<2; i++)
        {
          // FIXME (set parallax properly)
//          GetLayer(i).SetParallaxX(1, 1);
//          GetLayer(i).SetParallaxY(1, 1);
//          GetLayer(i).SetScrollingX(1, 1);
//          GetLayer(i).SetScrollingX(1, 1);
        }

        switch(header.parallax_mode)
        {
          case 0:
            SetStartLayer(0);
            break;

          case 1:
            SetStartLayer(1);
            break;

          case 2:
            // FIXME (set parallax properly)
            SetStartLayer(1);
//            GetLayer(0).SetParallaxX(header.parallax_multiplier, header.parallax_divisor);
//            GetLayer(0).SetParallaxY(header.parallax_multiplier, header.parallax_divisor);
            break;

          case 3:
            // FIXME (set parallax properly)
            SetStartLayer(0);
//            GetLayer(1).SetParallaxX(header.parallax_multiplier, header.parallax_divisor);
//            GetLayer(1).SetParallaxY(header.parallax_multiplier, header.parallax_divisor);
            break;
        }
      }
    }

    // cleanup
    delete[] layer_background;
    delete[] layer_foreground;
  }
  else if (strcmp(signature, "MAPù5") == 0)
  {
    V2MAP_HEADER header;
    V2MAP_LAYERINFO LayerInfo[7];
    sTileset tileset;
    word *mapLayer[7];
    int i,j,k;
    int highestTileIndex = 0;

    file->Read(&header, sizeof(header));
    for (i=0; i<header.num_layers; i++)
    {
      file->Read(LayerInfo + i, sizeof(V2MAP_LAYERINFO));
      //bug for v2's map: two bytes are added for no reason
      word w;
      file->Read(&w, 2);
    }

    // get info about map and uncompress it
    for (i=0; i<header.num_layers; i++)
      mapLayer[i] = new word[LayerInfo[i].size_x * LayerInfo[i].size_y];

    for (i=0; i<header.num_layers; i++)
    {
      // god, this is so dumb. It's supposed to be the buffersize, but do I look like I need it?
      file->Read(&j, 4);
      for (j=0; j<LayerInfo[i].size_x*LayerInfo[i].size_y; j++)
      {
        word value;
        byte run;
        
        file->Read(&value, sizeof(word));

        if ((value & 0xFF00) == 0xFF00)
        {
          run = (byte)value & 0x00FF;
          file->Read(&value, sizeof(word));
          
          mapLayer[i][j] = value;
          for (k=1; k<run; k++)
          {
            j++;
            mapLayer[i][j] = value;
          }
        }
        else
        {
          mapLayer[i][j]  = value;
        }
      }
    }

    if (strcmp_ci(tilesetFilename + strlen(tilesetFilename) - 4, ".vsp") == 0)
      success = tileset.Import_VSP(tilesetFilename);
    else
      success = tileset.Load(tilesetFilename);


    // transfer map array into the class
    if (success)
    {
      highestTileIndex = 0;
      // check for any tile index larger than the tilset's index
      for (i=0; i<header.num_layers; i++)
        for (j=0; j<LayerInfo[i].size_x*LayerInfo[i].size_y; j++)
          if (mapLayer[i][j] >= highestTileIndex)
            highestTileIndex = mapLayer[i][j];

      if (highestTileIndex >= tileset.GetNumTiles())
        success = false;

    if (success)
      {
        sLayer *layer;
        layer = new sLayer[header.num_layers];

        for (i=0; i<header.num_layers; i++)
        {
          char Name[7];
          memcpy(Name, "Layer A", 8);
          Name[6] += i;

          layer[i].SetName(Name);
          layer[i].Resize(LayerInfo[i].size_x, LayerInfo[i].size_y);
        }

        for (i=0; i<header.num_layers; i++)
        {
          for (j=0; j<LayerInfo[i].size_y; j++)
            for (k=0; k<LayerInfo[i].size_x; k++)
              layer[i].SetTile(k, j, mapLayer[i][(j * LayerInfo[i].size_x) + k]);

          // FIXME: set parallax properly
//          layer[i].SetParallaxX(LayerInfo[i].multx, LayerInfo[i].pdivx);
//          layer[i].SetParallaxY(LayerInfo[i].multy, LayerInfo[i].pdivy);
//          layer[i].SetScrollingX(1,1);
//          layer[i].SetScrollingY(1,1);
        }

        for (i=0; i<(int)strlen((char*)header.renderstring); i++)
          switch(header.renderstring[i])
          {
          case '1': AppendLayer(layer[0]); j = 0; break;
          case '2': AppendLayer(layer[1]); j = 1; break;
          case '3': AppendLayer(layer[2]); j = 2; break;
          case '4': AppendLayer(layer[3]); j = 3; break;
          case '5': AppendLayer(layer[4]); j = 4; break;
          case '6': AppendLayer(layer[5]); j = 5; break;
          case 'E': SetStartLayer(j); break;
          }

        SetMusicFile((char*)header.music_name);
        SetStartX(header.x_start);
        SetStartY(header.y_start);
        m_Tileset = tileset;

        delete[] layer;
      } 
    }

    for (i=0; i<header.num_layers; i++)
      delete mapLayer[i];
  }

  return success;
} 

////////////////////////////////////////////////////////////////////////////////

int
sMap::GetStartX() const
{
  return m_StartX;
}

////////////////////////////////////////////////////////////////////////////////

int
sMap::GetStartY() const
{
  return m_StartY;
}

////////////////////////////////////////////////////////////////////////////////

int
sMap::GetStartLayer() const
{
  return m_StartLayer;
}

////////////////////////////////////////////////////////////////////////////////

int
sMap::GetStartDirection() const
{
  return m_StartDirection;
}

////////////////////////////////////////////////////////////////////////////////

const char*
sMap::GetMusicFile() const
{
  return m_MusicFile.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
sMap::GetEntryScript() const
{
  return m_EntryScript.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
sMap::GetExitScript() const
{
  return m_ExitScript.c_str();
}

////////////////////////////////////////////////////////////////////////////////

const char*
sMap::GetEdgeScript(int edge)
{
  return m_EdgeScripts[edge].c_str();
}

////////////////////////////////////////////////////////////////////////////////

int
sMap::GetNumEntities() const
{
  return m_Entities.size();
}

////////////////////////////////////////////////////////////////////////////////

sEntity&
sMap::GetEntity(int i)
{
  return *m_Entities[i];
}

////////////////////////////////////////////////////////////////////////////////

const sEntity&
sMap::GetEntity(int i) const
{
  return *m_Entities[i];
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetStartX(int x)
{
  m_StartX = x;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetStartY(int y)
{
  m_StartY = y;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetStartLayer(int layer)
{
  m_StartLayer = layer;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetStartDirection(int direction)
{
  m_StartDirection = direction;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::InsertLayer(int where, const sLayer& layer)
{
  // resize the layer array
  m_Layers.insert(m_Layers.begin() + where, layer);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::InsertLayerFromMap(int where, const sMap& map, int layer)
{
  const sLayer& old_l = map.GetLayer(layer);
  sLayer new_l(old_l.GetWidth(), old_l.GetHeight());

  // merge the two tilesets, updating the indices in layer 'l'
  sTileset& dst_ts = m_Tileset;
  const sTileset& src_ts = map.GetTileset();

  // add each tile to the tileset
  for (int i = src_ts.GetNumTiles() - 1; i >= 0; i--) {

    // if it's already in the old tileset, we don't need to add it
    bool duplication = false;
    for (int j = 0; j < dst_ts.GetNumTiles(); j++) {
      if (src_ts.GetTile(i) == dst_ts.GetTile(j)) {

        // put 'j' where 'i' was in old layer
        for (int iy = 0; iy < old_l.GetHeight(); iy++) {
          for (int ix = 0; ix < old_l.GetWidth(); ix++) {
            if (old_l.GetTile(ix, iy) == i) {
              new_l.SetTile(ix, iy, j);
            }
          }
        }

        duplication = true;
        break;
      }
    }

    // add tile to end of tileset
    if (!duplication) {
      dst_ts.AppendTiles(1);
      int j = dst_ts.GetNumTiles() - 1;
      dst_ts.GetTile(j) = src_ts.GetTile(i);

      // put 'j' where 'i' was in old layer
      for (int iy = 0; iy < old_l.GetHeight(); iy++) {
        for (int ix = 0; ix < old_l.GetWidth(); ix++) {
          if (old_l.GetTile(ix, iy) == i) {
            new_l.SetTile(ix, iy, j);
          }
        }
      }
    }
  }

  InsertLayer(where, new_l);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::AppendLayer(const sLayer& layer)
{
  m_Layers.push_back(layer);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::DeleteLayer(int where, bool delete_tiles)
{
  std::set<int> tiles;  // list of tiles to consider for deletion

  // build list of tiles for deletion
  if (delete_tiles) {
    sLayer& l = m_Layers[where];
    for (int iy = 0; iy < l.GetHeight(); iy++) {
      for (int ix = 0; ix < l.GetWidth(); ix++) {
        tiles.insert(l.GetTile(ix, iy));
      }
    }
  }

  m_Layers.erase(m_Layers.begin() + where);

  if (delete_tiles) {
    // now prune tiles in set
    PruneTileset(&tiles);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SwapLayers(int layer1, int layer2)
{
  // swap the start position too
  if (layer1 == m_StartLayer) {
    m_StartLayer = layer2;
  } else if (layer2 == m_StartLayer) {
    m_StartLayer = layer1;
  }

  std::swap(m_Layers[layer1], m_Layers[layer2]);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::DuplicateLayer(int layer)
{
  sLayer l = m_Layers[layer];
  InsertLayer(layer, l);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::AddEntity(sEntity* entity)
{
  m_Entities.push_back(entity);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::DeleteEntity(int index)
{
  m_Entities.erase(m_Entities.begin() + index);
}

////////////////////////////////////////////////////////////////////////////////

int
sMap::FindEntity(int x, int y, int layer)
{
  for (unsigned i = 0; i < m_Entities.size(); i++)
    if (m_Entities[i]->x == x &&
        m_Entities[i]->y == y &&
        m_Entities[i]->layer == layer)
      return i;
  return -1;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::AddZone(sZone& zone)
{
  if (zone.x1 > zone.x2) {
    std::swap(zone.x1, zone.x2);
  }
  if (zone.y1 > zone.y2) {
    std::swap(zone.y1, zone.y2);
  }
  m_Zones.push_back(zone);
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::DeleteZone(int index)
{
  m_Zones.erase(m_Zones.begin() + index);
}

////////////////////////////////////////////////////////////////////////////////

int
sMap::FindZone(int x, int y, int layer)
{
  for (unsigned i = 0; i < m_Zones.size(); i++)
    if (x >= m_Zones[i].x1 &&
        y >= m_Zones[i].y1 &&
        x <= m_Zones[i].x2 &&
        y <= m_Zones[i].y2 &&
        m_Zones[i].layer == layer)
      return i;
  return -1;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::UpdateZone(int index, int x1, int y1, int x2, int y2)
{
  if (x1 > x2) {
    std::swap(x1, x2);
  }
  if (y1 > y2) {
    std::swap(y1, y2);
  }
  m_Zones[index].x1 = x1;
  m_Zones[index].y1 = y1;
  m_Zones[index].x2 = x2;
  m_Zones[index].y2 = y2;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetMusicFile(const char* music)
{
  m_MusicFile = music;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetEntryScript(const char* script)
{
  m_EntryScript = script;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetExitScript(const char* script)
{
  m_ExitScript = script;
}

////////////////////////////////////////////////////////////////////////////////

void
sMap::SetEdgeScript(int edge, const char* script)
{
  m_EdgeScripts[edge] = script;
}

////////////////////////////////////////////////////////////////////////////////

// when layer_to_translate = -1, translate all layers
void 
sMap::Translate(int dx, int dy, int layer_to_translate)
{
  bool persons = true;
  bool triggers = true;
  bool zones = true;
  bool startpoint = true;

  if (layer_to_translate == -1) {
    for (unsigned j = 0; j < m_Layers.size(); j++) {
      m_Layers[j].Translate(dx, dy);
    }
  }
  else {
    m_Layers[layer_to_translate].Translate(dx, dy);
  }

  int tile_width = GetTileset().GetTileWidth();
  int tile_height = GetTileset().GetTileHeight();

  for (int i = 0; i < GetNumEntities(); i++) {

    int type = GetEntity(i).GetEntityType();

    if (type == sEntity::PERSON  && persons  == true
     || type == sEntity::TRIGGER && triggers == true) {

      int layer = GetEntity(i).layer;
      int layer_width = GetLayer(layer).GetWidth() * tile_width;
      int layer_height = GetLayer(layer).GetHeight() * tile_height;

      int x = GetEntity(i).x + (dx * tile_width);
      int y = GetEntity(i).y + (dy * tile_height);

      // handle wrap around
      if (x < 0) x = layer_width + x;
      else if (x >= layer_width) x = x - layer_width;

      if (y < 0) y = layer_height + y;
      else if (y >= layer_height) y = y - layer_height;

      if (layer_to_translate == -1 || layer == layer_to_translate) {
        GetEntity(i).x = x;
        GetEntity(i).y = y;
      }
    }
  }

  if (zones) {
    for (int i = 0; i < GetNumZones(); i++) {

      int layer = GetZone(i).layer;
      int x = GetZone(i).x1 + (dx * tile_width);
      int y = GetZone(i).y1 + (dy * tile_height);
      int width = GetZone(i).x2 - GetZone(i).x1;
      int height = GetZone(i).y2 - GetZone(i).y1;

      int layer_width =  GetLayer(layer).GetWidth() * tile_width;
      int layer_height = GetLayer(layer).GetHeight() * tile_height;

      // handle wrap around
      if (x < 0) x = layer_width + x;
      else if (x >= layer_width) x = x - layer_width;

      if (y < 0) y = layer_height + y;
      else if (y >= layer_height) y = y - layer_height;

      if (layer_to_translate == -1 || layer == layer_to_translate) {
        GetZone(i).x1 = x;
        GetZone(i).y1 = y;
        GetZone(i).x2 = x + width;
        GetZone(i).y2 = y + height;
      }
    }
  }

  if (startpoint) {
    int x = GetStartX() + (dx * tile_width);
    int y = GetStartY() + (dy * tile_height);

    int layer = GetStartLayer();
    int layer_width =  GetLayer(layer).GetWidth() * tile_width;
    int layer_height = GetLayer(layer).GetHeight() * tile_height;

    // handle wrap around
    if (x < 0) x = layer_width + x;
    else if (x >= layer_width) x = x - layer_width;

    if (y < 0) y = layer_height + y;
    else if (y >= layer_height) y = y - layer_height;

    if (layer_to_translate == -1 || layer == layer_to_translate) {
      SetStartX(x);
      SetStartY(y);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
