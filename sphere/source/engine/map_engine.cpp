#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <sstream>
#include <math.h>
#include "map_engine.hpp"
#include "render.hpp"
#include "rendersort.hpp"
#include "time.hpp"


static const int c_MaxSkipFrames = 10;


static inline std::string itos(int i)
{
  char s[20];
  sprintf(s, "%d", i);
  return s;
}


////////////////////////////////////////////////////////////////////////////////

CMapEngine::CMapEngine(IEngine* engine, IFileSystem& fs)

: m_Engine(engine)
, m_FileSystem(fs)
, m_IsRunning(false)
, m_ShouldExit(false)

, m_ThrottleFPS(true)

, m_Music(NULL)

, m_IsInputAttached(false)
, m_InputPerson(-1)
, m_TouchActivationAllowed(true)
, m_TalkActivationAllowed(true)
, m_IsCameraAttached(false)
, m_CameraPerson(-1)

, m_NorthScript(NULL)
, m_EastScript(NULL)
, m_SouthScript(NULL)
, m_WestScript(NULL)
, m_UpdateScript(NULL)
, m_RenderScript(NULL)

, m_OnTrigger(false)

, m_TalkActivationKey(KEY_SPACE)
, m_TalkActivationDistance(8)
{
  m_Camera.x     = 0;
  m_Camera.y     = 0;
  m_Camera.layer = 0;

  memset(&m_Keys, 0, sizeof(bool) * MAX_KEY);
}

////////////////////////////////////////////////////////////////////////////////

CMapEngine::~CMapEngine()
{
  DestroyBoundKeys();

  // destroy update script
  if (m_UpdateScript) {
    m_Engine->DestroyScript(m_UpdateScript);
    m_UpdateScript = NULL;
  }

  // destroy render script
  if (m_RenderScript) {
    m_Engine->DestroyScript(m_RenderScript);
    m_RenderScript = NULL;
  }

}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::Execute(const char* filename, int fps)
{
  if (m_IsRunning) {
    m_ErrorMessage = "Map engine is already running!";
    return false;
  }

  if (fps <= 0) {
    m_ErrorMessage = "fps must be greater than zero!";
    return false;
  }

  m_IsRunning = true;

  m_FrameRate = fps;

  m_OnTrigger = false;
  m_ErrorMessage = "";
  m_Music = 0;

  m_NumFrames = 0;
  m_FramesLeft = 0;
  m_CurrentColorMask     = CreateRGBA(0, 0, 0, 0);
  m_PreviousColorMask    = CreateRGBA(0, 0, 0, 0);
  m_DestinationColorMask = CreateRGBA(0, 0, 0, 0);

  // change map to 'filename'
  if (!OpenMap(filename)) {
    m_IsRunning = false;
    return false;
  }

  m_CurrentMap = filename;

  if (!Run()) {
    m_IsRunning = false;
    return false;
  }

  // change map to nothing
  if (!CloseMap()) {
    m_IsRunning = false;
    return false;
  }


  m_IsRunning = false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

const char*
CMapEngine::GetErrorMessage()
{
  return m_ErrorMessage.c_str();
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ChangeMap(const char* filename)
{
  if (m_IsRunning) {

    // close previous map
    if (!CloseMap()) {
      return false;
    }

    // clear all entity queues
    for (unsigned int i = 0; i < m_Persons.size(); ++i) {
      m_Persons[i].commands.clear();
    }

    // open new one
    if (!OpenMap(filename)) {
      return false;
    }

    // reset map engine timing
    m_NextFrame = (qword)GetTime() * m_FrameRate;  // update next rendering time

    m_CurrentMap = filename;
    return true;

  } else {

    m_ErrorMessage = "ChangeMap() called while map engine was not running";
    return false;

  }

}

////////////////////////////////////////////////////////////////////////////////

std::string
CMapEngine::GetCurrentMap()
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetCurrentMap() called while map engine was not running";
    return "";
  }
  else {
    return m_CurrentMap;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::Exit()
{
  if (m_IsRunning) {

    m_ShouldExit = true;
    
    return true;

  } else {
    // shouldn't this be ExitMapEngine()?
    m_ErrorMessage = "Exit() called while map engine was not running";
    return false;

  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsRunning() {
  return m_IsRunning;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetMapEngineFrameRate(int fps) {
  if (!m_IsRunning) {
    m_ShouldExit = true;
    m_ErrorMessage = "SetMapEngineFrameRate() called while map engine was not running";
    return false;
  }
  if (fps <= 0) {
    m_ShouldExit = true;
    m_ErrorMessage = "fps must be greater than zero!";
    return false;
  }

  m_FrameRate = fps;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapEngine::GetMapEngineFrameRate() {
  return m_FrameRate;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::Update()
{
  if (!m_IsRunning) {
    m_ErrorMessage = "UpdateMapEngine() called while map engine was not running";
    return false;
  }

  if (!UpdateWorld(false)) {
    return false;
  }

  return true;
}
 
////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetNumLayers(int& layers)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetNumLayers() called while map engine was not running";
    return false;
  }

  layers = m_Map.GetMap().GetNumLayers();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetLayerWidth(int layer, int& width)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetLayerWidth() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  width = m_Map.GetMap().GetLayer(layer).GetWidth();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetLayerHeight(int layer, int& height)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetLayerHeight() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  height = m_Map.GetMap().GetLayer(layer).GetHeight();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetLayerName(int layer, std::string& name)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetLayerName() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  name = m_Map.GetMap().GetLayer(layer).GetName();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsLayerVisible(int layer, bool& visible)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "IsLayerHeight() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  visible = m_Map.GetMap().GetLayer(layer).IsVisible();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetLayerVisible(int layer, bool visible)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "SetLayerVisible() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  m_Map.GetMap().GetLayer(layer).SetVisible(visible);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetNumTiles(int& tiles)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetNumTiles() called while map engine was not running";
    return false;
  }

  tiles = m_Map.GetMap().GetTileset().GetNumTiles();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetTile(int x, int y, int layer, int tile)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "SetTile() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  sLayer& l = m_Map.GetMap().GetLayer(layer);

  // make sure x and y are valid
  if (x < 0 || y < 0 || x >= l.GetWidth() || y >= l.GetHeight()) {
    m_ErrorMessage = "Invalid x or y";
    return false;
  }

  l.SetTile(x, y, tile);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetTile(int x, int y, int layer, int& tile)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetTile() called while map engine was not running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer";
    return false;
  }

  sLayer& l = m_Map.GetMap().GetLayer(layer);

  // make sure x and y are valid
  if (x < 0 || y < 0 || x >= l.GetWidth() || y >= l.GetHeight()) {
    m_ErrorMessage = "Invalid x or y";
    return false;
  }

  tile = l.GetTile(x, y);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetTileWidth(int& width)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetTileWidth() called while map engine was not running";
    return false;
  }

  width = m_Map.GetMap().GetTileset().GetTileWidth();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetTileHeight(int& height)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetTileHeight() called while map engine was not running";
    return false;
  }

  height = m_Map.GetMap().GetTileset().GetTileHeight();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetTileImage(int tile, IMAGE& image) {

  if (!m_IsRunning) {
    m_ErrorMessage = "GetTileImage() called while map engine was not running";
    return false;
  }

  if (tile < 0 || tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }

  image = CreateImage(m_Map.GetMap().GetTileset().GetTileWidth(),
                      m_Map.GetMap().GetTileset().GetTileHeight(),
                      m_Map.GetMap().GetTileset().GetTile(tile).GetPixels());

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetTileImage(int tile, IMAGE image) {

  if (!m_IsRunning) {
    m_ErrorMessage = "SetTileImage() called while map engine was not running";
    return false;
  }

  if (tile < 0 || tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }
  
  if ( GetImageWidth(image) != m_Map.GetMap().GetTileset().GetTileWidth() ) {
    m_ErrorMessage = "Image used in SetTileImage call doesn't match the tile width";
    return false;
  }

  if ( GetImageHeight(image) != m_Map.GetMap().GetTileset().GetTileHeight() ) {
    m_ErrorMessage = "Image used in SetTileImage call doesn't match the tile height";
    return false;
  }

  RGBA* pixels = LockImage(image);
  for (int x = 0; x < m_Map.GetMap().GetTileset().GetTileWidth(); x++) {
    for (int y = 0; y < m_Map.GetMap().GetTileset().GetTileHeight(); y++) {
       m_Map.GetMap().GetTileset().GetTile(tile).SetPixel(x, y, pixels[y * GetImageWidth(image) + x]);
    }
  }
  UnlockImage(image);

  m_Map.UpdateTile(tile);
  m_Map.UpdateSolidTile(tile);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetTileDelay(int tile, int& delay)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetTileDelay() called while map engine was not running";
    return false;
  }

  if (tile < 0 || tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }

  delay = m_Map.GetMap().GetTileset().GetTile(tile).GetDelay();
  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetTileDelay(int tile, int delay)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "SetTileDelay() called while map engine was not running";
    return false;
  }

  if (tile < 0 || tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }

  if (delay < 0) {
    m_ErrorMessage = "Delay must be greater than zero";
    return false;
  }

  m_Map.GetMap().GetTileset().GetTile(tile).SetDelay(delay);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetNextAnimatedTile(int& tile)
{
  if (tile < 0 || tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }

  if (m_Map.GetMap().GetTileset().GetTile(tile).GetDelay() > 0)
    tile = m_Map.GetMap().GetTileset().GetTile(tile).GetNextTile();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetNextAnimatedTile(int current_tile, int next_tile)
{
  if (current_tile < 0 || current_tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }

  if (next_tile < 0 || next_tile >= m_Map.GetMap().GetTileset().GetNumTiles()) {
    m_ErrorMessage = "Tile index does not exist";
    return false;
  }

  m_Map.GetMap().GetTileset().GetTile(current_tile).SetNextTile(next_tile);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ReplaceTilesOnLayer(int layer, int old_tile, int new_tile) {
  if (!m_IsRunning) {
    m_ErrorMessage = "ReplaceTilesOnLayer() called while map engine was not running";
    return false;
  }

  sMap& map = m_Map.GetMap();
  if (layer < 0 || layer >= map.GetNumLayers()) {
    m_ErrorMessage = "Invalid layer index used in ReplaceTilesOnLayer";
    return false;
  }

  sLayer& l = map.GetLayer(layer);
  sTileset& tileset = map.GetTileset();
  if (old_tile < 0 || old_tile >= tileset.GetNumTiles()) {
    m_ErrorMessage = "Invalid old tile index in ReplaceTilesOnLayer";
    return false;
  }
  if (new_tile < 0 || new_tile >= tileset.GetNumTiles()) {
    m_ErrorMessage = "Invalid new tile index in ReplaceTilesOnLayer";
    return false;
  }

  l.Replace(old_tile, new_tile);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ExecuteTrigger(int location_x, int location_y, int layer)
{
  // check to see which trigger we're looking at on
  int trigger_index = FindTrigger(location_x, location_y, layer);
  return ExecuteTriggerScript(trigger_index);
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::AreZonesAt(int location_x, int location_y, int layer) {
  for (int i = 0; i < m_Map.GetMap().GetNumZones(); i++) {
    if (m_Map.GetMap().GetZone(i).x1 >= location_x
     && m_Map.GetMap().GetZone(i).y1 >= location_y
     && m_Map.GetMap().GetZone(i).layer == layer
     && m_Map.GetMap().GetZone(i).x2 - m_Map.GetMap().GetZone(i).x1 < location_x
     && m_Map.GetMap().GetZone(i).y2 - m_Map.GetMap().GetZone(i).y1 < location_y) {
       return true;
     }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ExecuteZones(int location_x, int location_y, int layer) {
  for (int i = 0; i < m_Map.GetMap().GetNumZones(); i++) {
    if (IsPointWithinZone(location_x, location_y, layer, i)) {
      if ( !ExecuteZoneScript(i) ) {
        return false;
      }
    }
  }
  return true; 
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetNumZones(int& zones) {
  if (!m_IsRunning) {
    m_ErrorMessage = "GetNumZones() called while map engine was not running";
    return false;
  }

  zones = m_Map.GetMap().GetNumZones();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetZoneX(int zone, int& x) {
  if (!m_IsRunning) {
    m_ErrorMessage = "GetZoneX() called while map engine was not running";
    return false;
  }

  if (zone < 0 || zone > m_Map.GetMap().GetNumZones()) {
    m_ErrorMessage = "Invalid zone index";
    return false;
  }

  x = m_Map.GetMap().GetZone(zone).x1;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetZoneY(int zone, int& y) {
  if (!m_IsRunning) {
    m_ErrorMessage = "GetZoneY() called while map engine was not running";
    return false;
  }

  if (zone < 0 || zone > m_Map.GetMap().GetNumZones()) {
    m_ErrorMessage = "Invalid zone index";
    return false;
  }

  y = m_Map.GetMap().GetZone(zone).y1;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetZoneWidth(int zone, int& width) {
  if (!m_IsRunning) {
    m_ErrorMessage = "GetZoneWidth() called while map engine was not running";
    return false;
  }

  if (zone < 0 || zone > m_Map.GetMap().GetNumZones()) {
    m_ErrorMessage = "Invalid zone index";
    return false;
  }

  width = m_Map.GetMap().GetZone(zone).x2 - m_Map.GetMap().GetZone(zone).x1;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetZoneHeight(int zone, int& height) {
  if (!m_IsRunning) {
    m_ErrorMessage = "GetZoneHeight() called while map engine was not running";
    return false;
  }

  if (zone < 0 || zone > m_Map.GetMap().GetNumZones()) {
    m_ErrorMessage = "Invalid zone index";
    return false;
  }

  height = m_Map.GetMap().GetZone(zone).y2 - m_Map.GetMap().GetZone(zone).y1;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::RenderMap()
{
  if (m_IsRunning) {

    Render();
    return true;

  } else {

    m_ErrorMessage = "RenderMap() called while map engine was not running";
    return false;

  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetColorMask(RGBA color, int num_frames)
{
  if (num_frames == 0) {
    m_CurrentColorMask     = color;
    m_PreviousColorMask    = color;
    m_DestinationColorMask = color;
  }

  if (m_CurrentColorMask.alpha == 0) {
    m_CurrentColorMask = color;
    m_CurrentColorMask.alpha = 0;
  }

  m_NumFrames = num_frames;
  m_FramesLeft = num_frames;
  m_PreviousColorMask = m_CurrentColorMask;
  m_DestinationColorMask = color;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetDelayScript(int num_frames, const char* script)
{
  if (m_IsRunning) {

    // compile the script
    DelayScript ds;
    ds.frames_left = num_frames;
    std::string error;
    ds.script = m_Engine->CompileScript(script, error);
    if (ds.script == NULL) {
      m_ErrorMessage = "Could not compile delay script\n" + error;
      return false;
    }

    m_DelayScripts.push_back(ds);
    return true;

  } else {

    m_ErrorMessage = "SetDelayScript() called while map engine was not running";
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::BindKey(int key, const char* on_key_down, const char* on_key_up)
{
  // unbind previous binding (if it exists)
  if (m_BoundKeys.count(key) > 0) {
    UnbindKey(key);
  }

  // compile the two scripts
  std::string error;
  KeyScripts ks;

  ks.down = m_Engine->CompileScript(on_key_down, error);
  if (ks.down == NULL) {
    m_ErrorMessage = "OnKeyDown script compile failed in BindKey()\n" + error;
    return false;
  }

  ks.up = m_Engine->CompileScript(on_key_up, error);
  if (ks.up == NULL) {
    m_Engine->DestroyScript(ks.down);
    m_ErrorMessage = "OnKeyUp script compile failed in BindKey()\n" + error;
    return false;
  }

  m_BoundKeys[key] = ks;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UnbindKey(int key)
{
  if (m_BoundKeys.count(key) > 0) {
    
    m_Engine->DestroyScript(m_BoundKeys[key].down);
    m_Engine->DestroyScript(m_BoundKeys[key].up);
    m_BoundKeys.erase(key);
    return true;

  } else {

    m_ErrorMessage = "UnbindKey() called on an unbound key";
    return false;

  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsInvalidPersonError(const char* person_name, int& person_index)
{
  person_index = FindPerson(person_name);

  if (person_index == -1) {
    m_ErrorMessage = "Person '" + std::string(person_name) + "' doesn't exist";
    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::AttachInput(const char* name)
{
  // make sure the person entity exists
  m_InputPerson = -1;
  if ( IsInvalidPersonError(name, m_InputPerson) ) {
    return false;
  }

  m_IsInputAttached = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::DetachInput()
{
  m_IsInputAttached = false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsInputAttached(bool& attached)
{
  attached = m_IsInputAttached;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetInputPerson(std::string& person)
{
  if (m_IsInputAttached) {
    person = m_Persons[m_InputPerson].name;
    return true;
  } else {
    m_ErrorMessage = "Input not attached!";
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetUpdateScript(const char* script)
{
  // destroy the previous script
  if (m_UpdateScript) {
    m_Engine->DestroyScript(m_UpdateScript);
    m_UpdateScript = NULL;
  }

  // try to compile the script
  std::string error;
  IEngine::script s = m_Engine->CompileScript(script, error);
  if (s == NULL) {
    m_ErrorMessage = "Could not compile update script";
    return false;
  }

  m_UpdateScript = s;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetRenderScript(const char* script)
{
  // destroy the previous script
  if (m_RenderScript) {
    m_Engine->DestroyScript(m_RenderScript);
    m_RenderScript = NULL;
  }

  // try to compile the script
  std::string error;
  IEngine::script s = m_Engine->CompileScript(script, error);
  if (s == NULL) {
    m_ErrorMessage = "Could not compile render script";
    return false;
  }

  m_RenderScript = s;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetLayerRenderer(int layer, const char* script)
{
  // make sure the engine is running
  if (!m_IsRunning) {
    m_ErrorMessage = "SetLayerRenderer() called while map engine isn't running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= int(m_LayerRenderers.size())) {
    m_ErrorMessage = "Invalid layer value";
    return false;
  }

  // destroy old layer renderer
  if (m_LayerRenderers[layer]) {
    m_Engine->DestroyScript(m_LayerRenderers[layer]);
    m_LayerRenderers[layer] = NULL;
  }

  // compile the new one
  std::string error;
  IEngine::script s = m_Engine->CompileScript(script, error);
  if (s == NULL) {
    m_ErrorMessage = "Could not compile layer renderer";
    return false;
  }

  m_LayerRenderers[layer] = s;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetLayerAlpha(int layer, int alpha)
{
  // make sure the engine is running
  if (!m_IsRunning) {
    m_ErrorMessage = "SetLayerRenderer() called while map engine isn't running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer value";
    return false;
  }

  // valid alpha
  if (alpha < 0) {
    alpha = 0;
  } else if (alpha > 255) {
    alpha = 255;
  }

  m_Map.SetLayerAlpha(layer, alpha);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetLayerAlpha(int layer, int& alpha)
{
  // make sure the engine is running
  if (!m_IsRunning) {
    m_ErrorMessage = "SetLayerRenderer() called while map engine isn't running";
    return false;
  }

  // make sure layer is valid
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer value";
    return false;
  }

  alpha = m_Map.GetLayerAlpha(layer);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::AttachCamera(const char* name)
{
  // make sure the person entity exists
  m_CameraPerson = -1;
  if ( IsInvalidPersonError(name, m_CameraPerson) ) {
    return false;
  }

  m_IsCameraAttached = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::DetachCamera()
{
  m_IsCameraAttached = false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsCameraAttached(bool& attached)
{
  attached = m_IsCameraAttached;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetCameraPerson(std::string& person)
{
  if (m_IsCameraAttached) {
    person = m_Persons[m_CameraPerson].name;
    return true;
  } else {
    m_ErrorMessage = "Camera not attached!";
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetCameraX(int x)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "SetCameraX() called while map engine was not running";
    return false;
  }

  m_Camera.x = x;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetCameraY(int y)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "SetCameraY() called while map engine was not running";
    return false;
  }

  m_Camera.y = y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetCameraX(int& x)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetCameraX() called while map engine was not running";
    return false;
  }

  x = m_Camera.x;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetCameraY(int& y)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "GetCameraY() called while map engine was not running";
    return false;
  }

  y = m_Camera.y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::MapToScreenX(int layer, int mx, int& sx)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "MapToScreenX() called while map engine was not running";
    return false;
  }

  // validate layers
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer index";
    return false;
  }

  sx = m_Map.MapToScreenX(layer, m_Camera.x, mx);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::MapToScreenY(int layer, int my, int& sy)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "MapToScreenY() called while map engine was not running";
    return false;
  }

  // validate layers
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer index";
    return false;
  }

  sy = m_Map.MapToScreenY(layer, m_Camera.y, my);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ScreenToMapX(int layer, int sx, int& mx)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "ScreenToMapX() called while map engine was not running";
    return false;
  }

  // validate layers
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer index";
    return false;
  }

  mx = m_Map.ScreenToMapX(layer, m_Camera.x, sx);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ScreenToMapY(int layer, int sy, int& my)
{
  if (!m_IsRunning) {
    m_ErrorMessage = "ScreenToMapY() called while map engine was not running";
    return false;
  }

  // validate layers
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Invalid layer index";
    return false;
  }

  my = m_Map.ScreenToMapY(layer, m_Camera.y, sy);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonList(std::vector<std::string>& list)
{
  list.resize(m_Persons.size());
  for (unsigned i = 0; i < m_Persons.size(); i++) {
    list[i] = m_Persons[i].name;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::CreatePerson(const char* name, const char* spriteset, bool destroy_with_map)
{
  Person p;
  p.destroy_with_map = destroy_with_map;

  p.name = name;
  p.description = std::string("name=[") + name + "]";
  
  // load spriteset
  p.spriteset = m_Engine->LoadSpriteset(spriteset);
  if (p.spriteset == NULL) {
    m_ErrorMessage = "Could not load spriteset\nPerson: " + p.description;
    return false;
  }

  // LoadSpriteset already calls AddRef()
//  p.spriteset->AddRef();
  p.mask = CreateRGBA(255, 255, 255, 255);

  // put them in the starting position by default
  if (m_IsRunning) {
    p.x     = m_Map.GetMap().GetStartX();
    p.y     = m_Map.GetMap().GetStartY();
    p.layer = m_Map.GetMap().GetStartLayer();
  } else {
    p.x     = 0;
    p.y     = 0;
    p.layer = 0;
  }

  p.speed_x = 1;
  p.speed_y = 1;

  p.stepping_frame_revert = 0;
  p.stepping_frame_revert_count = 0;

  p.ignorePersonObstructions = false;
  p.ignoreTileObstructions = false;

  p.spriteset->GetSpriteset().GetBase(p.base_x1, p.base_y1, p.base_x2, p.base_y2);
  p.width = p.spriteset->GetSpriteset().GetFrameWidth();
  p.height = p.spriteset->GetSpriteset().GetFrameHeight();

  p.direction = p.spriteset->GetSpriteset().GetDirectionName(0);
  p.stepping = 0;
  p.frame = p.spriteset->GetSpriteset().GetFrameIndex(p.direction, p.stepping);
  p.next_frame_switch = p.spriteset->GetSpriteset().GetFrameDelay(p.direction, p.stepping);

  p.script_create            = NULL;
  p.script_destroy           = NULL;
  p.script_activate_touch    = NULL;
  p.script_activate_talk     = NULL;
  p.script_command_generator = NULL;

  p.leader = -1;

  m_Persons.push_back(p);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::DestroyPerson(const char* name)
{
  // make sure the person entity exists
  for (int i = 0; i < int(m_Persons.size()); i++) {
    if (m_Persons[i].name == name) {

      // detach input if necessary
      if (i == m_InputPerson) {
        m_IsInputAttached = false;
      } else if (m_InputPerson > i) {
        m_InputPerson--;
      }
      
      // detach camera if necessary
      if (i == m_CameraPerson) {
        m_IsCameraAttached = false;
      } else if (m_CameraPerson > i) {
        m_CameraPerson--;
      }

      // update all leader indices
      for (int j = 0; j < int(m_Persons.size()); j++) {
        if (i != j) {
          if (m_Persons[j].leader > i) {

            m_Persons[j].leader--;

          } else if (m_Persons[j].leader == i) {

            m_Persons[j].leader = -1;
            m_Persons[j].follow_state_queue.resize(0);

          }
        }
      }

      // destroy the person entity
      if (!DestroyPersonStructure(m_Persons[i])) {
        return false;
      }

      m_Persons.erase(m_Persons.begin() + i);
      return true;

    }
  }

  m_ErrorMessage = "Person '" + std::string(name) + "' doesn't exist";
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonX(const char* name, int x)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // !!!! verify x

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  m_Persons[person].x = x;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonY(const char* name, int y)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // !!!! verify y

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  m_Persons[person].y = y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonXYFloat(const char* name, double x, double y)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // !!!! verify x and y

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  m_Persons[person].x = x;
  m_Persons[person].y = y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonLayer(const char* name, int layer)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // verify layer
  if (layer < 0 || layer >= m_Map.GetMap().GetNumLayers()) {
    m_ErrorMessage = "Layer " + itos(layer) + " doesn't exist";
    return false;
  }

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  m_Persons[person].layer = layer;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonDirection(const char* name, const char* direction)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  Person& p = m_Persons[person];

  // if person has a leader, ignore the command
  if (p.leader != -1) {
    return true;
  }

  if (p.spriteset->GetSpriteset().GetDirectionNum(direction) == -1) {
    m_ErrorMessage = "Person '" + std::string(name) + "' direction '" + std::string(direction) + "' doesn't exist";
    return false;
  }

  p.direction = direction;
  
  // make sure 'stepping' is valid
  p.stepping %= p.spriteset->GetSpriteset().GetNumFrames(p.direction);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonFrame(const char* name, int frame)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  Person& p = m_Persons[person];

  // if person has a leader, ignore the command
  if (p.leader != -1) {
    return true;
  }

  if (frame < 0) {
    frame = 0;
  } else {
    frame %= p.spriteset->GetSpriteset().GetNumFrames(p.direction);
  }

  m_Persons[person].frame = frame;
  m_Persons[person].stepping = frame;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonX(const char* name, int& x)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  x = int(m_Persons[person].x);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonY(const char* name, int& y)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  y = int(m_Persons[person].y);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonXFloat(const char* name, double& x)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  x = m_Persons[person].x;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonYFloat(const char* name, double& y)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  y = m_Persons[person].y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonLayer(const char* name, int& layer)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  layer = m_Persons[person].layer;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonDirection(const char* name, std::string& direction)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  direction = m_Persons[person].direction;
//  direction = m_Persons[person].spriteset->GetSpriteset().GetDirectionNum(name);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IgnorePersonObstructions(const char* name, bool ignoring)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  m_Persons[person].ignorePersonObstructions = ignoring;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsIgnoringPersonObstructions(const char* name, bool& ignoring)
{
  int person = -1;
  ignoring = false;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  ignoring = m_Persons[person].ignorePersonObstructions;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IgnoreTileObstructions(const char* name, bool ignoring)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  m_Persons[person].ignoreTileObstructions = ignoring;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsIgnoringTileObstructions(const char* name, bool& ignoring)
{
  int person = -1;
  ignoring = false;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  ignoring = m_Persons[person].ignoreTileObstructions;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonFrame(const char* name, int& frame)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  frame = m_Persons[person].frame;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonFrameRevert(const char* name, int i)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  m_Persons[person].stepping_frame_revert = i;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonFrameRevert(const char* name, int& i)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  i = m_Persons[person].stepping_frame_revert;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonSpeedXY(const char* name, double x, double y)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // !!!! verify x

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  m_Persons[person].speed_x = x;
  m_Persons[person].speed_y = y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonSpeedX(const char* name, double& x)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  x = m_Persons[person].speed_x;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonSpeedY(const char* name, double& y)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  y = m_Persons[person].speed_y;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonScaleAbsolute(const char* name, int width, int height)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // convert to float. I can't figure out how to calculate the base in 
  // absolute mode...
  const Person& s = m_Persons[person];
  double old_w = s.spriteset->GetSpriteset().GetFrameWidth();
  double old_h = s.spriteset->GetSpriteset().GetFrameHeight();
  double new_w = width;
  double new_h = height;

  double scale_w = new_w / old_w;
  double scale_h = new_h / old_h;

  return SetPersonScaleFactor(name, scale_w, scale_h);
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonScaleFactor(const char* name, double scale_w, double scale_h)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  Person& p = m_Persons[person];

  // convert to integer ;)
  int base_x1;
  int base_y1; 
  int base_x2; 
  int base_y2;
  double width = p.spriteset->GetSpriteset().GetFrameWidth();
  double height = p.spriteset->GetSpriteset().GetFrameHeight();
  p.spriteset->GetSpriteset().GetBase(base_x1, base_y1, base_x2, base_y2);

  p.width = (int)(scale_w * width);
  p.height = (int)(scale_h * height);
  p.base_x1 = (int)(scale_w * (double)base_x1);
  p.base_y1 = (int)(scale_h * (double)base_y1);
  p.base_x2 = (int)(scale_w * (double)base_x2);
  p.base_y2 = (int)(scale_h * (double)base_y2);
  

  // oopsies on scaling problems? ;)
  if (p.width < 1) p.width = 1;
  if (p.height < 1) p.height = 1;
/*
  if (p.base_x1 < 0 && p.width > 1 && base_x1 != 0) p.base_x1 = 1;
  if (p.base_x2 < 0 && p.width > 1 && base_x2 != 0) p.base_x2 = 1;
  if (p.base_y1 < 0 && p.height > 1 && base_y1 != 0) p.base_y1 = 1;
  if (p.base_y2 < 0 && p.height > 1 && base_y2 != 0) p.base_y2 = 1;
*/

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonMask(const char* name, RGBA mask)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  m_Persons[person].mask = mask;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetPersonMask(const char* name, RGBA& mask)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  mask = m_Persons[person].mask;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

SSPRITESET*
CMapEngine::GetPersonSpriteset(const char* name)
{
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return 0;
  }

  return m_Persons[person].spriteset;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::FollowPerson(const char* follower, const char* leader, int pixels)
{
  // get follower index
  int follower_index = -1;
  if ( IsInvalidPersonError(follower, follower_index) ) {
    m_ErrorMessage = "Follower person '" + std::string(follower) + "' not found";
    return false;
  }

  // remove any queued up commands
  m_Persons[follower_index].commands.clear();

  // if we want to remove the leader...
  if (strcmp(leader, "") == 0) {
    m_Persons[follower_index].leader = -1;
    return true;
  }

  // get leader index
  int leader_index = -1;
  if ( IsInvalidPersonError(leader, leader_index) ) {
    m_ErrorMessage = "Leader person '" + std::string(leader) + "' not found";
    return false;
  }

  // verify following distance
  if (pixels < 1) {
    m_ErrorMessage = "Invalid following distance";
    return false;
  }

  // make sure the leader doesn't ever point back to the follower (circular list)
  int current = leader_index;
  while (current != -1) {         // go until we've reached the end

    if (current == follower_index) {
      m_ErrorMessage = "Circular reference in following chain";
      return false;
    }

    // go up in the chain
    current = m_Persons[current].leader;
  }

  // give the follower a follow queue
  m_Persons[follower_index].leader = leader_index;
  m_Persons[follower_index].follow_state_queue.resize(pixels);

  // initialize the follow queue
  std::vector<Person::AnimationState>& vas = m_Persons[follower_index].follow_state_queue;
  for (int i = 0; i < pixels; i++) {
    vas[i].x         = m_Persons[leader_index].x;
    vas[i].y         = m_Persons[leader_index].y;
    vas[i].layer     = m_Persons[leader_index].layer;
    vas[i].direction = m_Persons[leader_index].direction;
  }

  m_Persons[follower_index].x         = m_Persons[leader_index].x;
  m_Persons[follower_index].y         = m_Persons[leader_index].y;
  m_Persons[follower_index].layer     = m_Persons[leader_index].layer;
  m_Persons[follower_index].direction = m_Persons[leader_index].direction;
  m_Persons[follower_index].frame     = 0;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetPersonScript(const char* name, int which, const char* script)
{
  // verify the script constant
  if (which < 0 || which >= 5) {
    m_ErrorMessage = "SetPersonScript() - script does not exist";
    return false;
  }

  // find the person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // try to compile the script
  std::string error;
  IEngine::script s = m_Engine->CompileScript(script, error);
  if (s == NULL) {
    m_ErrorMessage = "Could not compile script\n" + error;
    return false;
  }

  // find out which script we're changing
  IEngine::script* ps = NULL;
  switch (which) {
    case 0: ps = &m_Persons[person].script_create;            break;
    case 1: ps = &m_Persons[person].script_destroy;           break;
    case 2: ps = &m_Persons[person].script_activate_touch;    break;
    case 3: ps = &m_Persons[person].script_activate_talk;     break;
    case 4: ps = &m_Persons[person].script_command_generator; break;
  }

  // now replace the script
  if (*ps) {
    m_Engine->DestroyScript(*ps);
  }
  *ps = s;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::CallPersonScript(const char* name, int which)
{
  // make sure 'which' is valid
  if (which < 0 || which >= 5) {
    m_ErrorMessage = "SetPersonScript() - script does not exist";
    return false;
  }

  // find the person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // set the current person
  std::string old_person = m_CurrentPerson;
  m_CurrentPerson = m_Persons[person].name;

  // find out which script we're dealing with
  IEngine::script* ps = NULL;
  bool reset_time = false;
  switch (which) {
    case 0: ps = &m_Persons[person].script_create;            break;
    case 1: ps = &m_Persons[person].script_destroy;           break;
    case 2: ps = &m_Persons[person].script_activate_touch;    reset_time = true; break;
    case 3: ps = &m_Persons[person].script_activate_talk;     reset_time = true; break;
    case 4: ps = &m_Persons[person].script_command_generator; break;
  }

  if (*ps) {

    std::string error;
    if (ExecuteScript(*ps, error)) {
      m_ErrorMessage = "Could not execute person script\n" + error;
      return false;
    }
    if (reset_time) {
      ResetNextFrame();
    }

  }

  m_CurrentPerson = old_person;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetCurrentPerson(std::string& person)
{
  if (m_CurrentPerson.empty()) {
    m_ErrorMessage = "GetCurrentPerson() called outside of person script";
    return false;
  }

  person = m_CurrentPerson;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::QueuePersonCommand(const char* name, int command, bool immediate)
{
  // make sure command is valid
  if (command < 0 || command >= NUM_COMMANDS) {
    m_ErrorMessage = "Invalid command";
    return false;    
  }

  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  // add person to queue
  m_Persons[person].commands.push_back(Person::Command(command, immediate));
  return true;
}

///////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::QueuePersonScript(const char* name, const char* script, bool immediate)
{
  // Make sure script is not null
  if ((script == "") || (script == NULL)) {
    m_ErrorMessage = "Null script.";
    return false;
  }
  
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }
  
  // add person to queue
  m_Persons[person].commands.push_back(Person::Command(COMMAND_DO_SCRIPT, immediate, std::string(script)));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ClearPersonCommands(const char* name)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  // if person has a leader, ignore the command
  if (m_Persons[person].leader != -1) {
    return true;
  }

  // clear queue
  m_Persons[person].commands.clear();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsCommandQueueEmpty(const char* name, bool& empty)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  empty = m_Persons[person].commands.empty();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsPersonObstructed(const char* name, int x, int y, bool& result)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    return false;
  }

  int obs_person;
  result = IsObstructed(person, x, y, obs_person);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetTalkActivationKey(int key)
{
  m_TalkActivationKey = key;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapEngine::GetTalkActivationKey()
{
  return m_TalkActivationKey;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::SetTalkDistance(int pixels)
{
  m_TalkActivationDistance = pixels;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapEngine::GetTalkDistance()
{
  return m_TalkActivationDistance;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::OpenMap(const char* filename)
{
  // load the map
  std::string path = "maps/";
  path += filename;
  if (!m_Map.Load(path.c_str(), m_FileSystem)) {
    m_ErrorMessage = "Could not load map '";
    m_ErrorMessage += filename;
    m_ErrorMessage += "'";
    return false;
  }

  // if a person entity is here, it's not map-specific
  // so put it in the starting position!
  for (unsigned i = 0; i < m_Persons.size(); i++) {
    m_Persons[i].x     = m_Map.GetMap().GetStartX(); 
    m_Persons[i].y     = m_Map.GetMap().GetStartY();
    m_Persons[i].layer = m_Map.GetMap().GetStartLayer();

    // update follow queues
    if (m_Persons[i].leader != -1) {
      for (unsigned j = 0; j < m_Persons[i].follow_state_queue.size(); j++) {
        m_Persons[i].follow_state_queue[j].x         = m_Persons[i].x;
        m_Persons[i].follow_state_queue[j].y         = m_Persons[i].y;
        m_Persons[i].follow_state_queue[j].layer     = m_Persons[i].layer;
        m_Persons[i].follow_state_queue[j].direction = m_Persons[i].direction;
      }
    }
  }

  if (!CompileEdgeScripts()) { return false; }
  if (!LoadTriggers())       { return false; }
  if (!LoadZones())          { return false; }
  if (!LoadMapPersons())     { return false; }

  // load the background music (if there is any)
  std::string music = m_Map.GetMap().GetMusicFile();
  if (music.length()) {
    m_Music = m_Engine->LoadSound(music.c_str(), true);
    if (!m_Music) {
      m_ErrorMessage = "Could not load background music '" + music + "'";
      return false;
    }
  }

  // start background music
  if (m_Music) {
    m_Music->setRepeat(true);
    m_Music->play();
  }
  
  // initialize camera
  m_Camera.x     = m_Map.GetMap().GetStartX();
  m_Camera.y     = m_Map.GetMap().GetStartY();
  m_Camera.layer = m_Map.GetMap().GetStartLayer();

  // initialize the layer script array
  m_LayerRenderers.resize(m_Map.GetMap().GetNumLayers());
  for (unsigned i = 0; i < m_LayerRenderers.size(); i++) {
    m_LayerRenderers[i] = NULL;
  }

  // execute entry script
  std::string error;
  if (!ExecuteScript(m_Map.GetMap().GetEntryScript(), error)) {
    m_ErrorMessage = "Entry Script Error:\n" + error;

    // stop background music
    m_Music = 0;

    // destroy edge scripts
    m_Engine->DestroyScript(m_NorthScript);
    m_Engine->DestroyScript(m_EastScript);
    m_Engine->DestroyScript(m_SouthScript);
    m_Engine->DestroyScript(m_WestScript);

    return false;
  }
  ResetNextFrame();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::CloseMap()
{
  // stop background music
  m_Music = 0;

  if (!DestroyMapPersons()) {
    return false;
  }

  DestroyTriggers();
  DestroyZones();

  // destroy any remaining delay scripts
  for (unsigned i = 0; i < m_DelayScripts.size(); i++) {
    m_Engine->DestroyScript(m_DelayScripts[i].script);
  }
  m_DelayScripts.clear();

  // destroy layer scripts
  for (unsigned i = 0; i < m_LayerRenderers.size(); i++) {
    if (m_LayerRenderers[i]) {
      m_Engine->DestroyScript(m_LayerRenderers[i]);
    }
  }
  m_LayerRenderers.resize(0);

  // destroy edge scripts
  m_Engine->DestroyScript(m_NorthScript);
  m_Engine->DestroyScript(m_EastScript);
  m_Engine->DestroyScript(m_SouthScript);
  m_Engine->DestroyScript(m_WestScript);

  // execute exit script
  std::string error;
  if (!ExecuteScript(m_Map.GetMap().GetExitScript(), error)) {
    m_ErrorMessage = "Exit Script Error:\n" + error;
    return false;
  }
  ResetNextFrame();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::Run()
{
  // initialize renderer
  m_NextFrame = (qword)GetTime() * m_FrameRate;
  int frames_skipped = 0;

  while (!m_ShouldExit) {


    // RENDER STEP
    
    if (m_ThrottleFPS) {  // throttle

      // if we're ahead of schedule, do the render
      qword actual_time = (qword)GetTime() * m_FrameRate;
      if (actual_time < m_NextFrame || frames_skipped >= c_MaxSkipFrames) {

        frames_skipped = 0;
        
        if (!Render()) {
          return false;
        }
        FlipScreen();

        // wait for a while (make sure we don't go faster than fps)
        while (actual_time < m_NextFrame) {
          actual_time = (qword)GetTime() * m_FrameRate;
        }
      } else {
        frames_skipped++;
      }

      // update ideal rendering time
      m_NextFrame += 1000;

    } else {              // don't throttle

      if (!Render()) {
        return false;
      }
      FlipScreen();

    }


    // UPDATE STEP

    if (!UpdateWorld(true)) {
      return false;
    }

    if (!ProcessInput()) {
      return false;
    }

  } // end map engine loop
  m_ShouldExit = false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ExecuteScript(IEngine::script script, std::string& error)
{
  bool should_exit;
  bool result = m_Engine->ExecuteScript(script, should_exit, error);

  if (should_exit) {
    m_ShouldExit = true;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ExecuteScript(const char* script, std::string& error)
{
  IEngine::script s = m_Engine->CompileScript(script, error);
  if (s == NULL) {
    return false;
  }

  bool result = ExecuteScript(s, error);
  m_Engine->DestroyScript(s);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::CompileEdgeScripts()
{
  std::string error;

  // NORTH
  m_NorthScript = m_Engine->CompileScript(m_Map.GetMap().GetEdgeScript(sMap::NORTH), error);
  if (m_NorthScript == NULL) {

    m_ErrorMessage = "Could not compile north script\n" + error;
    return false;
  }

  // EAST
  m_EastScript = m_Engine->CompileScript(m_Map.GetMap().GetEdgeScript(sMap::EAST), error);
  if (m_EastScript == NULL) {

    m_Engine->DestroyScript(m_NorthScript);
    m_NorthScript = NULL;

    m_ErrorMessage = "Could not compile east script\n" + error;
    return false;
  }

  // SOUTH
  m_SouthScript = m_Engine->CompileScript(m_Map.GetMap().GetEdgeScript(sMap::SOUTH), error);
  if (m_SouthScript == NULL) {

    m_Engine->DestroyScript(m_NorthScript);
    m_NorthScript = NULL;

    m_Engine->DestroyScript(m_EastScript);
    m_EastScript = NULL;

    m_ErrorMessage = "Could not compile south script\n" + error;
    return false;
  }

  // WEST
  m_WestScript = m_Engine->CompileScript(m_Map.GetMap().GetEdgeScript(sMap::WEST), error);
  if (m_WestScript == NULL) {

    m_Engine->DestroyScript(m_NorthScript);
    m_NorthScript = NULL;

    m_Engine->DestroyScript(m_EastScript);
    m_EastScript = NULL;

    m_Engine->DestroyScript(m_SouthScript);
    m_SouthScript = NULL;

    m_ErrorMessage = "Could not compile west script\n" + error;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::LoadMapPersons()
{
  const int tile_width  = m_Map.GetMap().GetTileset().GetTileWidth();
  const int tile_height = m_Map.GetMap().GetTileset().GetTileHeight();

  std::string old_person;  // used down where script_create is called

  // load the persons, evaluate their startup scripts, and compile the other scripts
  for (int i = 0; i < m_Map.GetMap().GetNumEntities(); i++) {
    if (m_Map.GetMap().GetEntity(i).GetEntityType() == sEntity::PERSON) {
      sPersonEntity& person = (sPersonEntity&)m_Map.GetMap().GetEntity(i);

      // generate the person string for error messages
      std::string person_string = "name=[" + person.name + "], x=[" +
                                             itos(person.x / tile_width) + "], y=[" +
                                             itos(person.y / tile_height) + "]";
      
      std::string error;  // have to define locals before gotos

      Person p;
      p.destroy_with_map = true;

      p.name = person.name;
      p.description = person_string;

      p.x     = person.x;
      p.y     = person.y;
      p.layer = person.layer;

      p.speed_x = 1;
      p.speed_y = 1;

	  p.stepping_frame_revert = 0;
	  p.stepping_frame_revert_count = 0;

    p.ignorePersonObstructions = false;
    p.ignoreTileObstructions = false;

      // load spriteset
      p.spriteset = m_Engine->LoadSpriteset(person.spriteset.c_str());
      if (p.spriteset == NULL) {
        m_ErrorMessage = "Could not load spriteset\nPerson: " + person_string;
        goto spriteset_error;
      }

      p.mask = CreateRGBA(255, 255, 255, 255);

      p.width = p.spriteset->GetSpriteset().GetFrameWidth();
      p.height = p.spriteset->GetSpriteset().GetFrameHeight();
      p.spriteset->GetSpriteset().GetBase(p.base_x1, p.base_y1, p.base_x2, p.base_y2);

      // whatever direction
      p.direction = p.spriteset->GetSpriteset().GetDirectionName(0);
      p.stepping = 0;
      p.frame = p.spriteset->GetSpriteset().GetFrameIndex(p.direction, p.stepping);
      p.next_frame_switch = p.spriteset->GetSpriteset().GetFrameDelay(p.direction, p.stepping);
      
      // compile script_create
      p.script_create = m_Engine->CompileScript(person.script_create.c_str(), error);
      if (p.script_create == NULL) {
        m_Engine->DestroySpriteset(p.spriteset);
        m_ErrorMessage = "Could not compile OnCreate script\nPerson:" + person_string + "\n" + error;
        goto spriteset_error;
      }

      // compile script_destroy
      p.script_destroy = m_Engine->CompileScript(person.script_destroy.c_str(), error);
      if (p.script_destroy == NULL) {
        m_Engine->DestroySpriteset(p.spriteset);
        m_Engine->DestroyScript(p.script_create);
        m_ErrorMessage = "Could not compile OnDestroy script\nPerson:" + person_string + "\n" + error;
        goto spriteset_error;
      }
      
      // compile script_activate_touch
      p.script_activate_touch = m_Engine->CompileScript(person.script_activate_touch.c_str(), error);
      if (p.script_activate_touch == NULL) {
        m_Engine->DestroySpriteset(p.spriteset);
        m_Engine->DestroyScript(p.script_create);
        m_Engine->DestroyScript(p.script_destroy);
        m_ErrorMessage = "Could not compile OnActivate (touch) script\nPerson:" + person_string + "\n" + error;
        goto spriteset_error;
      }

      // compile script_activate_talk
      p.script_activate_talk = m_Engine->CompileScript(person.script_activate_talk.c_str(), error);
      if (p.script_activate_talk == NULL) {
        m_Engine->DestroySpriteset(p.spriteset);
        m_Engine->DestroyScript(p.script_create);
        m_Engine->DestroyScript(p.script_destroy);
        m_Engine->DestroyScript(p.script_activate_touch);
        m_ErrorMessage = "Could not compile OnActivate (talk) script\nPerson:" + person_string + "\n" + error;
        goto spriteset_error;
      }

      // compile script_command_generator
      p.script_command_generator = m_Engine->CompileScript(person.script_generate_commands.c_str(), error);
      if (p.script_command_generator == NULL) {
        m_Engine->DestroySpriteset(p.spriteset);
        m_Engine->DestroyScript(p.script_create);
        m_Engine->DestroyScript(p.script_destroy);
        m_Engine->DestroyScript(p.script_activate_touch);
        m_Engine->DestroyScript(p.script_activate_talk);
        m_ErrorMessage = "Could not compile OnGenerateCommands script\nPerson:" + person_string + "\n" + error;
        goto spriteset_error;
      }

      p.leader = -1;

      // old_person defined earlier (before goto)
      /*std::string*/ old_person = m_CurrentPerson;
      m_CurrentPerson = p.name;

      // execute script_create
      if (!ExecuteScript(p.script_create, error)) {
        m_Engine->DestroySpriteset(p.spriteset);
        m_Engine->DestroyScript(p.script_create);
        m_Engine->DestroyScript(p.script_destroy);
        m_Engine->DestroyScript(p.script_activate_touch);
        m_Engine->DestroyScript(p.script_activate_talk);
        m_Engine->DestroyScript(p.script_command_generator);
        m_ErrorMessage = "Could not execute OnCreate script\nPerson:" + person_string + "\n" + error;
        goto spriteset_error;
      }

      m_CurrentPerson = old_person;

      // add it to the list
      m_Persons.push_back(p);

      continue;

spriteset_error:
      DestroyMapPersons();

      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::DestroyMapPersons()
{
  // destroy map person entities
  for (int i = 0; i < int(m_Persons.size()); i++) {

    if (m_Persons[i].destroy_with_map) {

      // detach input if necessary
      if (i == m_InputPerson) {
        m_IsInputAttached = false;
      } else if (m_InputPerson > i) {
        m_InputPerson--;
      }
      
      // detach camera if necessary
      if (i == m_CameraPerson) {
        m_IsCameraAttached = false;
      } else if (m_CameraPerson > i) {
        m_CameraPerson--;
      }

      // update all leader indices
      for (int j = 0; j < int(m_Persons.size()); j++) {
        if (i != j) {
          if (m_Persons[j].leader > i) {

            m_Persons[j].leader--;

          } else if (m_Persons[j].leader == i) {

            m_Persons[j].leader = -1;
            m_Persons[j].follow_state_queue.resize(0);

          }
        }
      }

      // destroy the entity scripts/spriteset/etc.
      if (!DestroyPersonStructure(m_Persons[i])) {
        return false;
      }

      // remove the entity from the list
      m_Persons.erase(m_Persons.begin() + i);
      i--;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::DestroyPersonStructure(Person& p)
{
  // execute OnDestroy scripts (if it exists)
  if (p.script_destroy) {

    // set current person
    std::string old_person = m_CurrentPerson;
    m_CurrentPerson = p.name;

    std::string error;
    if (!ExecuteScript(p.script_destroy, error)) {
      m_ErrorMessage = "Could not execute OnDestroy script\nPerson:";
      m_ErrorMessage += p.description + "\n" + error;
      return false;
    }

    // restore current person
    m_CurrentPerson = old_person;
  }


  //m_Engine->DestroySpriteset(p.spriteset);
  p.spriteset->Release();

  if (p.script_create) {
    m_Engine->DestroyScript(p.script_create);
  }

  if (p.script_destroy) {
    m_Engine->DestroyScript(p.script_destroy);
  }
  
  if (p.script_activate_touch) {
    m_Engine->DestroyScript(p.script_activate_touch);
  }

  if (p.script_activate_talk) {
    m_Engine->DestroyScript(p.script_activate_talk);
  }

  if (p.script_command_generator) {
    m_Engine->DestroyScript(p.script_command_generator);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::LoadTriggers()
{
  const int tile_width  = m_Map.GetMap().GetTileset().GetTileWidth();
  const int tile_height = m_Map.GetMap().GetTileset().GetTileHeight();

  // load the triggers and compile the trigger scripts
  for (int i = 0; i < m_Map.GetMap().GetNumEntities(); i++) {
    if (m_Map.GetMap().GetEntity(i).GetEntityType() == sEntity::TRIGGER) {
      sTriggerEntity& trigger = (sTriggerEntity&)m_Map.GetMap().GetEntity(i);

      Trigger t;
      t.x      = trigger.x;
      t.y      = trigger.y;
      t.layer  = trigger.layer;
      std::string error;
      t.script = m_Engine->CompileScript(trigger.script.c_str(), error);

      if (t.script == NULL) {
        // destroy scripts that have been created so far
        for (unsigned j = 0; j < m_Triggers.size(); j++) {
          m_Engine->DestroyScript(m_Triggers[i].script);
        }

        // build error message and return
        std::ostringstream os;
        os << "Could not compile trigger ("
           << t.x / tile_width
           << ", "
           << t.y / tile_height
           << ")\n";
        m_ErrorMessage = os.str() + error;
        return false;
      }

      m_Triggers.push_back(t);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapEngine::DestroyTriggers()
{
  // destroy trigger scripts
  for (unsigned i = 0; i < m_Triggers.size(); i++) {
    m_Engine->DestroyScript(m_Triggers[i].script);
  }
  m_Triggers.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::LoadZones()
{
  for (int i = 0; i < m_Map.GetMap().GetNumZones(); i++) {
    sMap::sZone& zone = m_Map.GetMap().GetZone(i);
    std::string error;
    Zone z;

    z.x1 = zone.x1;
    z.y1 = zone.y1;
    z.x2 = zone.x2;
    z.y2 = zone.y2;
    z.layer = zone.layer;
    z.reactivate_in_num_steps = zone.reactivate_in_num_steps;

    z.current_step = 0;
    z.script = m_Engine->CompileScript(zone.script.c_str(), error);
    if (z.script == NULL) {
      // destroy scripts that have been created so far
      for (unsigned j = 0; j < m_Zones.size(); j++) {
        m_Engine->DestroyScript(m_Zones[i].script);
      }

      // build error message and return
      std::ostringstream os;
      os << "Could not compile zone ("
         << z.x1
         << ", "
         << z.y1
         << ") -> ("
         << z.x2
         << ", "
         << z.y2
         << ")\n";
      m_ErrorMessage = os.str() + error;
      return false;
    }

    m_Zones.push_back(z);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapEngine::DestroyZones()
{
  for (unsigned i = 0; i < m_Zones.size(); i++) {
    m_Engine->DestroyScript(m_Zones[i].script);
  }
  m_Zones.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::Render()
{
//  const int cx = GetScreenWidth()  / 2;
//  const int cy = GetScreenHeight() / 2;

  // for doing camera correction (with sprites and doodads and whatnot)
  int offset_x;
  int offset_y;

  // render all layers
  for (int i = 0; i < m_Map.GetMap().GetNumLayers(); i++) {

    // IF REFLECTIVE
    if (m_Map.GetMap().GetLayer(i).IsReflective()) {

      // solid render
      m_Map.RenderLayer(i, true, m_Camera.x, m_Camera.y, offset_x, offset_y);

      // render upside-down sprites
      if (!RenderEntities(i, true, offset_x, offset_y)) {
        return false;
      }

      // do normal render
      m_Map.RenderLayer(i, false, m_Camera.x, m_Camera.y, offset_x, offset_y);

      // draw person entities
      if (!RenderEntities(i, false, offset_x, offset_y)) {
        return false;
      }

    } else {  // IF NOT REFLECTIVE

      m_Map.RenderLayer(i, false, m_Camera.x, m_Camera.y, offset_x, offset_y);

      // draw person entities
      if (!RenderEntities(i, false, offset_x, offset_y)) {
        return false;
      }

    } // end if reflective


    // execute layer renderer
    if (m_LayerRenderers[i]) {
      std::string error;
      if (!ExecuteScript(m_LayerRenderers[i], error)) {
        m_ErrorMessage = "Could not execute layer renderer\n" + error;
        return false;
      }
    }

  } // end for all layers


  ApplyColorMask(m_CurrentColorMask);

  // render script
  if (m_RenderScript) {
    std::string error;
    if (!ExecuteScript(m_RenderScript, error)) {
      m_ErrorMessage = "Could not execute update script\n" + error;
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::RenderEntities(int layer, bool flipped, int offset_x, int offset_y)
{
  CRenderSort rs;

  // add non-map-specific person entities
  for (unsigned i = 0; i < m_Persons.size(); i++) {
    if (m_Persons[i].layer == layer) {

      Person& p = m_Persons[i];
      const sSpriteset& ss = p.spriteset->GetSpriteset();

      IMAGE image = (flipped
        ? p.spriteset->GetFlipImage(ss.GetFrameIndex(p.direction, p.stepping))
        : p.spriteset->GetImage(ss.GetFrameIndex(p.direction, p.stepping))
      );


      // calculate distance from upper-left corner of image to center of base
      int base_x = (p.base_x1 + p.base_x2) / 2;
      int base_y = (p.base_y1 + p.base_y2) / 2;

      int draw_x = int(p.x - base_x - m_Camera.x - offset_x + GetScreenWidth()  / 2);
      int draw_y = int(p.y - base_y - m_Camera.y - offset_y + GetScreenHeight() / 2);
      int sort_y = int(p.y);

      if (flipped) {
        draw_y += base_y;
      }

      rs.AddObject(draw_x, draw_y, sort_y, p.width, p.height, image, p.mask);

    }
  }

  rs.DrawObjects();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateWorld(bool input_valid)
{
  m_Map.UpdateMap();

  if (!UpdatePersons()) {
    return false;
  }

  if (input_valid) {
    if (!UpdateTriggers()) {
      return false;
    }
  }

  if (!UpdateColorMasks()) {
    return false;
  }

  if (!UpdateDelayScripts()) {
    return false;
  }

  if (input_valid) {
    if (!UpdateEdgeScripts()) {
      return false;
    }
  }

  // update the camera
  if (m_IsCameraAttached) {
    m_Camera.x     = int(m_Persons[m_CameraPerson].x);
    m_Camera.y     = int(m_Persons[m_CameraPerson].y);
    m_Camera.layer = m_Persons[m_CameraPerson].layer;
  }

  // call update script
  if (m_UpdateScript) {
    std::string error;
    if (!ExecuteScript(m_UpdateScript, error)) {
      m_ErrorMessage = "Could not execute update script\n" + error;
      return false;
    }

    // if we took more than a second to run the update script, reset the timer
    if (qword(GetTime()) * m_FrameRate > m_NextFrame) {
      ResetNextFrame();
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdatePersons()
{
  // if any of the persons are activated, disable talk activation

  bool anything_activated = false;

  // for each person...
  for (unsigned i = 0; i < m_Persons.size(); i++) {
    bool activated;
    if (!UpdatePerson(i, activated)) {
      return false;
    }

    anything_activated |= activated;
  }

  m_TalkActivationAllowed = !anything_activated;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdatePerson(int person_index, bool& activated)
{
  Person& p = m_Persons[person_index];

  // if this person has a leader, skip it
  if (p.leader != -1) {
    /*revert back to the first frame if reversion has been set and enough updates have passed*/
    if(p.stepping_frame_revert > 0) {
      if(p.stepping_frame_revert_count++ >= p.stepping_frame_revert) {
        if(p.stepping != 0) {
          p.stepping = 0;
          p.frame = p.spriteset->GetSpriteset().GetFrameIndex(p.direction, p.stepping);
          p.next_frame_switch = p.spriteset->GetSpriteset().GetFrameDelay(p.direction, p.stepping);
        }
      }
    }
    return true;
  }

  
  // store current position
  double x = p.x;
  double y = p.y;

  // we haven't called an activation function yet
  bool activation_called = false;

  bool processing = true;
  bool force_stop = false;
  while (processing && !force_stop) {

    // if this entity has no commands, execute generator
    if (p.commands.empty()) {

      if (p.script_command_generator) {

        // set current person
        std::string old_person = m_CurrentPerson;
        m_CurrentPerson = p.name;
  
        std::string error;
        if (!ExecuteScript(p.script_command_generator, error)) {
          m_ErrorMessage = "Error executing person command generator\nPerson:"
            + p.description +
            "\nError:" + error;
          return false;
        }

        m_CurrentPerson = old_person;
      }

      // if all of the commands are immediate, force a stop after this command
      force_stop = true;
      std::deque<Person::Command>::iterator k;
      for (k = p.commands.begin(); k != p.commands.end(); k++) {
        if (k->immediate == false) {
          force_stop = false;
          break;
        }
      }

      // if there are no commands, stop
      if (p.commands.size() < 1) {
        break;
      }

    } // end (if command queue is empty)
    
    
    // read the top command
    Person::Command c = p.commands.front();
    p.commands.pop_front();

    // store position in case the obstruction check needs to put it back
    double old_x = p.x;
    double old_y = p.y;

    
    std::string error;
    switch (c.command) {
      
      case COMMAND_WAIT: break;
      case COMMAND_FACE_NORTH:     p.direction = "north";     break;
      case COMMAND_FACE_NORTHEAST: p.direction = "northeast"; break;
      case COMMAND_FACE_EAST:      p.direction = "east";      break;
      case COMMAND_FACE_SOUTHEAST: p.direction = "southeast"; break;
      case COMMAND_FACE_SOUTH:     p.direction = "south";     break;
      case COMMAND_FACE_SOUTHWEST: p.direction = "southwest"; break;
      case COMMAND_FACE_WEST:      p.direction = "west";      break;
      case COMMAND_FACE_NORTHWEST: p.direction = "northwest"; break;
      case COMMAND_MOVE_NORTH:     p.y-=p.speed_y; break;
      case COMMAND_MOVE_EAST:      p.x+=p.speed_x; break;
      case COMMAND_MOVE_SOUTH:     p.y+=p.speed_y; break;
      case COMMAND_MOVE_WEST:      p.x-=p.speed_x; break;
      case COMMAND_DO_SCRIPT:        
        if (!ExecuteScript(c.script.c_str(), error)) {
	  m_ErrorMessage = "Could not execute queued script\nPerson:" + p.description +
	    "\nError:" + error;
	}
	break;
    }

    // make sure 'stepping' is valid
    p.stepping %= p.spriteset->GetSpriteset().GetNumFrames(p.direction);


    // check for obstructions
    int obs_person;
    if (IsObstructed(person_index, int(p.x), int(p.y), obs_person)) {
      p.x = old_x;
      p.y = old_y;
    }

    // CHECK FOR ENTITY ACTIVATION

    // if we're processing the input target
    if (person_index == m_InputPerson) {
      // and if a person caused the obstruction
      // and if the activation function has not already 
      if (obs_person != -1) {
        if (m_TouchActivationAllowed) {

          IEngine::script script = m_Persons[obs_person].script_activate_touch;
        
          // execute the script!
          if (script) {

            std::string old_person = m_CurrentPerson;
            m_CurrentPerson = m_Persons[obs_person].name;

            std::string error;
            if (!ExecuteScript(script, error)) {
              m_ErrorMessage = "Error executing person activation (touch) script\n"
                "Person:"
                + p.description +
                "\nError:" + error;
              return false;
            }

            m_CurrentPerson = old_person;

            ResetNextFrame();
          }
        }

        activation_called = true;

      }                
    }

    processing = c.immediate;
  }


  // if an activation function was called, activation should not be allowed
  if (person_index == m_InputPerson) {
    m_TouchActivationAllowed = !activation_called;
  }


  // if position has changed, update frame index and state of followers
  if (x != p.x || y != p.y) {
    p.stepping_frame_revert_count = 0;

    // frame index
    if (--p.next_frame_switch <= 0) {
      p.stepping = (p.stepping + 1) % p.spriteset->GetSpriteset().GetNumFrames(p.direction);
      p.frame = p.spriteset->GetSpriteset().GetFrameIndex(p.direction, p.stepping);
      p.next_frame_switch = p.spriteset->GetSpriteset().GetFrameDelay(p.direction, p.stepping);
    }

    // followers
    for (int j = 0; j < int(m_Persons.size()); j++) {
      if (person_index != j) {
        if (m_Persons[j].leader == person_index) {  // if the current entity is a leader...
          UpdateFollower(j);
        }
      }
    }

  } else {
    /*revert back to the first frame if reversion has been set and enough updates have passed*/
    if(p.stepping_frame_revert > 0) {
      if(p.stepping_frame_revert_count++ >= p.stepping_frame_revert) {
        if(p.stepping != 0) {
          p.stepping = 0;
          p.frame = p.spriteset->GetSpriteset().GetFrameIndex(p.direction, p.stepping);
          p.next_frame_switch = p.spriteset->GetSpriteset().GetFrameDelay(p.direction, p.stepping);
        }
      }
    }
  }

  activated = false;

  // test if talk activation script should be called
  if (m_InputPerson == person_index) {
    // if the activation key is pressed
    if (m_Keys[m_TalkActivationKey]) {
      
      int talk_x = int(m_Persons[m_InputPerson].x);
      int talk_y = int(m_Persons[m_InputPerson].y);
      int tad = m_TalkActivationDistance;

      // god this is slow...
      if (m_Persons[m_InputPerson].direction == "north") {

        talk_y -= tad;

      } else if (m_Persons[m_InputPerson].direction == "northeast") {

        talk_x += tad;
        talk_y -= tad;

      } else if (m_Persons[m_InputPerson].direction == "east") {

        talk_x += tad;

      } else if (m_Persons[m_InputPerson].direction == "southeast") {

        talk_x += tad;
        talk_y += tad;

      } else if (m_Persons[m_InputPerson].direction == "south") {

        talk_y += tad;

      } else if (m_Persons[m_InputPerson].direction == "southwest") {

        talk_x -= tad;
        talk_y += tad;

      } else if (m_Persons[m_InputPerson].direction == "west") {

        talk_x -= tad;

      } else if (m_Persons[m_InputPerson].direction == "northwest") {

        talk_x -= tad;
        talk_y -= tad;

      }

      // if a person obstructs that spot, call his activation script
      int obs_person;
      if (IsObstructed(person_index, talk_x, talk_y, obs_person)) {
        if (obs_person != -1) {

          activated = true;
          if (m_TalkActivationAllowed) {

            IEngine::script s = m_Persons[obs_person].script_activate_talk;
            if (s) {

              std::string old_person = m_CurrentPerson;
              m_CurrentPerson = m_Persons[obs_person].name;

              std::string error;
              if (!ExecuteScript(s, error)) {
                m_ErrorMessage = "Error executing person activation (talk) script\n"
                  "Person:" + p.description +
                  "\nError:" + error;
                return false;
              }

              m_CurrentPerson = old_person;
            }

            ResetNextFrame();
          }
        }
      }

    }
  }

  if (m_InputPerson == person_index) {
    int px = int(fabs(x - p.x));
    int py = int(fabs(y - p.y));
    int s;

    if (px > py) s = px;
    else s = py;

    while (s-- > 0)
      UpdateZones();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateFollower(int person_index)
{
  Person& p = m_Persons[person_index];

  p.x         = p.follow_state_queue[0].x;
  p.y         = p.follow_state_queue[0].y;
  p.layer     = p.follow_state_queue[0].layer;
  p.direction = p.follow_state_queue[0].direction;

  // make sure 'stepping' is valid
  p.stepping %= p.spriteset->GetSpriteset().GetNumFrames(p.direction);

  p.stepping_frame_revert_count = 0;

  // update the follow state
  for (int i = 0; i < (int)p.follow_state_queue.size() - 1; i++) {
    p.follow_state_queue[i] = p.follow_state_queue[i + 1];
  }

  int last = p.follow_state_queue.size() - 1;
  p.follow_state_queue[last].x         = m_Persons[p.leader].x;
  p.follow_state_queue[last].y         = m_Persons[p.leader].y;
  p.follow_state_queue[last].layer     = m_Persons[p.leader].layer;
  p.follow_state_queue[last].direction = m_Persons[p.leader].direction;

  // frame index
  if (--p.next_frame_switch <= 0) {
    p.stepping = (p.stepping + 1) % p.spriteset->GetSpriteset().GetNumFrames(p.direction);
    p.frame = p.spriteset->GetSpriteset().GetFrameIndex(p.direction, p.stepping);
    p.next_frame_switch = p.spriteset->GetSpriteset().GetFrameDelay(p.direction, p.stepping);
  }

  // now update any followers of this one
  for (int i = 0; i < int(m_Persons.size()); i++) {
    if (i != person_index) {
      if (m_Persons[i].leader == person_index) {
        UpdateFollower(i);
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapEngine::FindTrigger(int location_x, int location_y, int layer)
{
  sTileset& tileset = m_Map.GetMap().GetTileset();
  const int tile_width  = tileset.GetTileWidth();
  const int tile_height = tileset.GetTileHeight();

  // this doesn't seem to use layers, but I'll include the layer parameter just incase

  int trigger_index = -1;
  for (unsigned i = 0; i < m_Triggers.size(); i++) {
    int dx = m_Triggers[i].x - location_x;
    int dy = m_Triggers[i].y - location_y;
    if (dx <= tile_width / 2 && dx > -tile_width / 2 &&
        dy <= tile_height / 2 && dy > -tile_height / 2) {
      trigger_index = i;
      break;
    }
  }
  return trigger_index;
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsTriggerAt(int location_x, int location_y, int layer)
{
  int trigger_index = FindTrigger(location_x, location_y, layer);
  return (trigger_index != -1);
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ExecuteTriggerScript(int trigger_index)
{
  if (trigger_index < 0 || trigger_index >= int(m_Triggers.size())) {
    std::ostringstream os;
    os << "Invalid trigger index\n";
    m_ErrorMessage = os.str();
    return false;
  }

  // execute the trigger code
  IEngine::script script = m_Triggers[trigger_index].script;
  std::string error;
  if (!ExecuteScript(script, error)) {
    sTileset& tileset = m_Map.GetMap().GetTileset();
    const int tile_width  = tileset.GetTileWidth();
    const int tile_height = tileset.GetTileHeight();

    std::ostringstream os;
    os << "Could not execute trigger ("
       << m_Triggers[trigger_index].x / tile_width
       << ", "
       << m_Triggers[trigger_index].y / tile_height
       << ")\n";
    m_ErrorMessage = os.str() + error;
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateTriggers()
{
  // can't process triggers if we don't have an input target
  if (!m_IsInputAttached) {
    return true;
  }

  // convenience
  int location_x = int(m_Persons[m_InputPerson].x);
  int location_y = int(m_Persons[m_InputPerson].y);
  int location_layer = m_Persons[m_InputPerson].layer;

  // check to see which trigger we're on
  int trigger_index = FindTrigger(location_x, location_y, location_layer);

  if (m_OnTrigger) {

    if (trigger_index == -1) {
      m_OnTrigger = false;
    }

  } else {

    if (trigger_index != -1) {

      if (!ExecuteTriggerScript(trigger_index)) {
        return false;
      }

      ResetNextFrame();
      m_OnTrigger = true;

    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsPointWithinZone(int location_x, int location_y, int location_layer, int zone_index) {
  if (zone_index < 0 || zone_index >= int(m_Zones.size()))
    return false;

  Zone& z = m_Zones[zone_index];

  return (location_x >= z.x1 && 
          location_y >= z.y1 &&
          location_x <= z.x2 && 
          location_y <= z.y2 &&
          location_layer == z.layer);
}

bool
CMapEngine::IsPersonInsideZone(int person_index, int zone_index)
{
  if (person_index < 0 || person_index >= int(m_Persons.size()))
    return false;

  // convenience
  int location_x = int(m_Persons[person_index].x);
  int location_y = int(m_Persons[person_index].y);
  int location_l = m_Persons[person_index].layer;

  return IsPointWithinZone(location_x, location_y, location_l, zone_index);
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ExecuteZoneScript(int zone_index)
{
  if (zone_index < 0 || zone_index >= int(m_Zones.size())) {
    std::ostringstream os;
    os << "Invalid zone index\n";
    m_ErrorMessage = os.str();
    return false;
  }

  Zone& z = m_Zones[zone_index];

  // execute the trigger code
  IEngine::script script = z.script;
  std::string error;
  if (!ExecuteScript(script, error)) {
    std::ostringstream os;
    os << "Could not compile zone ("
       << z.x1
       << ", "
       << z.y1
       << ") -> ("
       << z.x2
       << ", "
       << z.y2
       << ")\n";
    m_ErrorMessage = os.str() + error;
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateZones()
{
  // can't process triggers if we don't have an input target
  if (!m_IsInputAttached) {
    return true;
  }

  for (unsigned i = 0; i < m_Zones.size(); i++) {

    // check if the person is inside the zone
    if (IsPersonInsideZone(m_InputPerson, i)) {

      Zone& z = m_Zones[i];
      z.current_step--;
      if (z.current_step < 0) {
        z.current_step = z.reactivate_in_num_steps - 1;

        if (!ExecuteZoneScript(i))
          return false;

        ResetNextFrame();
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateColorMasks()
{
  // update color masks
  if (m_NumFrames != 0 && m_FramesLeft > 0) {

    m_CurrentColorMask.red   = (m_DestinationColorMask.red   * (m_NumFrames - m_FramesLeft) + m_PreviousColorMask.red   * m_FramesLeft) / m_NumFrames;
    m_CurrentColorMask.green = (m_DestinationColorMask.green * (m_NumFrames - m_FramesLeft) + m_PreviousColorMask.green * m_FramesLeft) / m_NumFrames;
    m_CurrentColorMask.blue  = (m_DestinationColorMask.blue  * (m_NumFrames - m_FramesLeft) + m_PreviousColorMask.blue  * m_FramesLeft) / m_NumFrames;
    m_CurrentColorMask.alpha = (m_DestinationColorMask.alpha * (m_NumFrames - m_FramesLeft) + m_PreviousColorMask.alpha * m_FramesLeft) / m_NumFrames;

    m_FramesLeft--;

  } else {

    m_CurrentColorMask = m_DestinationColorMask;

  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateDelayScripts()
{
  // update delay scripts
  for (unsigned i = 0; i < m_DelayScripts.size(); i++) {
    if (--m_DelayScripts[i].frames_left < 0) {

      IEngine::script script = m_DelayScripts[i].script;
      
      // the script may cause a CloseMap call, so remove the script from the array now
      m_DelayScripts.erase(m_DelayScripts.begin() + i);
      i--;

      std::string error;
      if (!ExecuteScript(script, error)) {
        m_ErrorMessage = "Could not execute delay script\n" + error;
        return false;
      }

      m_Engine->DestroyScript(script);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::UpdateEdgeScripts()
{
  const sMap& map = m_Map.GetMap();
  const int tile_width   = map.GetTileset().GetTileWidth();
  const int tile_height  = map.GetTileset().GetTileHeight();
  const int layer_width  = map.GetLayer(m_Camera.layer).GetWidth();
  const int layer_height = map.GetLayer(m_Camera.layer).GetHeight();

  if (m_Camera.x < 0) {                                 // west
    
    std::string error;
    if (!ExecuteScript(m_WestScript, error)) {
      m_ErrorMessage = "Could not execute west script\n" + error;
      return false;
    }    

  } else if (m_Camera.x > tile_width * layer_width) {   // east

    std::string error;
    if (!ExecuteScript(m_EastScript, error)) {
      m_ErrorMessage = "Could not execute east script\n" + error;
      return false;
    }    

  }

  if (m_Camera.y < 0) {                                 // north
    
    std::string error;
    if (!ExecuteScript(m_NorthScript, error)) {
      m_ErrorMessage = "Could not execute north script\n" + error;
      return false;
    }    

  } else if (m_Camera.y > tile_height * layer_height) { // south
    
    std::string error;
    if (!ExecuteScript(m_SouthScript, error)) {
      m_ErrorMessage = "Could not execute south script\n" + error;
      return false;
    }    

  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ProcessInput()
{
  GetKeyStates(m_Keys);
  RefreshInput();
  bool new_keys[MAX_KEY];
  GetKeyStates(new_keys);

  // clear the key queue
  while (AreKeysLeft()) {
    GetKey();
  }

  // check to see if key state has changed
  for (int i = 0; i < MAX_KEY; i++) {
    if (new_keys[i] != m_Keys[i]) {

      if (new_keys[i]) {                // event: key down

        if (m_BoundKeys.count(i) > 0) {

          // bound
          if (!ProcessBoundKeyDown(i)) {
            return false;
          }

        } else {

          // unbound
          ProcessUnboundKeyDown(i);

        }

      } else {                          // event: key up

        if (m_BoundKeys.count(i) > 0) {

          // bound
          if (!ProcessBoundKeyUp(i)) {
            return false;
          }

        } else {

          // unbound
          ProcessUnboundKeyUp(i);

        }

      }
    }
  }


  // process default input bindings
  if (m_IsInputAttached) {

    int dx = 0;
    int dy = 0;

    if (new_keys[KEY_UP])    dy--; 
    if (new_keys[KEY_RIGHT]) dx++;
    if (new_keys[KEY_DOWN])  dy++;
    if (new_keys[KEY_LEFT])  dx--;

    if (dy < 0) m_Persons[m_InputPerson].commands.push_back(Person::Command(COMMAND_MOVE_NORTH, true));
    if (dx > 0) m_Persons[m_InputPerson].commands.push_back(Person::Command(COMMAND_MOVE_EAST,  true));
    if (dy > 0) m_Persons[m_InputPerson].commands.push_back(Person::Command(COMMAND_MOVE_SOUTH, true));
    if (dx < 0) m_Persons[m_InputPerson].commands.push_back(Person::Command(COMMAND_MOVE_WEST,  true));

    // set the direction
    int command = -1;
    if (dx < 0) {
      if (dy < 0) {
        command = COMMAND_FACE_NORTHWEST;
      } else if (dy > 0) {
        command = COMMAND_FACE_SOUTHWEST;
      } else {
        command = COMMAND_FACE_WEST;
      }
    } else if (dx > 0) {
      if (dy < 0) {
        command = COMMAND_FACE_NORTHEAST;
      } else if (dy > 0) {
        command = COMMAND_FACE_SOUTHEAST;
      } else {
        command = COMMAND_FACE_EAST;
      }
    } else {
      if (dy < 0) {
        command = COMMAND_FACE_NORTH;
      } else if (dy > 0) {
        command = COMMAND_FACE_SOUTH;
      }
    }

    m_Persons[m_InputPerson].commands.push_back(Person::Command(command, false));

  }  


  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ProcessBoundKeyDown(int key)
{
  KeyScripts& a = m_BoundKeys[key];

  std::string error;
  if (!ExecuteScript(a.down, error)) {
    m_ErrorMessage = "Could not execute key down script\n" + error;
    return false;
  }

  ResetNextFrame();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::ProcessBoundKeyUp(int key)
{
  KeyScripts& a = m_BoundKeys[key];

  std::string error;
  if (!ExecuteScript(a.up, error)) {
    m_ErrorMessage = "Could not execute key up script\n" + error;
    return false;
  }

  ResetNextFrame();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CMapEngine::ProcessUnboundKeyDown(int key)
{
  switch (key) {

    case KEY_ESCAPE: {
      m_ShouldExit = true;
    } break;


    case KEY_F1: {
      m_ThrottleFPS = !m_ThrottleFPS;
      m_NextFrame = (qword)GetTime() * m_FrameRate;  // update next rendering time
    } break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMapEngine::ProcessUnboundKeyUp(int key)
{
}

////////////////////////////////////////////////////////////////////////////////

void
CMapEngine::DestroyBoundKeys()
{
  std::map<int, KeyScripts>::iterator i;
  for (i = m_BoundKeys.begin(); i != m_BoundKeys.end(); i++) {
    m_Engine->DestroyScript(i->second.down);
    m_Engine->DestroyScript(i->second.up);
  }

  m_BoundKeys.clear();
}

////////////////////////////////////////////////////////////////////////////////

void
CMapEngine::ResetNextFrame()
{
  m_NextFrame = (qword)GetTime() * m_FrameRate;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapEngine::FindPerson(const char* name)
{
  for (unsigned i = 0; i < m_Persons.size(); i++) {
    if (m_Persons[i].name == name) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetObstructingTile(const char* name, int x, int y, int& result)
{
  // find person
  int person = -1;
  if ( IsInvalidPersonError(name, person) ) {
    result = -1;
    return false;
  }

  result = FindObstructingTile(person, x, y);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

int
CMapEngine::FindObstructingTile(int person, int x, int y)
{
  const Person& p = m_Persons[person];
  const sLayer& layer = m_Map.GetMap().GetLayer(p.layer);
  const int tile_width  = m_Map.GetMap().GetTileset().GetTileWidth();
  const int tile_height = m_Map.GetMap().GetTileset().GetTileHeight();

  int bx = (p.base_x1 + p.base_x2) / 2;
  int by = (p.base_y1 + p.base_y2) / 2;
  
  int x1 = x - bx + p.base_x1;
  int y1 = y - by + p.base_y1;
  int x2 = x - bx + p.base_x2;
  int y2 = y - by + p.base_y2;

  // test per-tile obstructions
  int min_x = (x1 < x2 ? x1 : x2);
  int max_x = (x1 > x2 ? x1 : x2);
  int min_y = (y1 < y2 ? y1 : y2);
  int max_y = (y1 > y2 ? y1 : y2);

  int min_tx = min_x / tile_width;
  int max_tx = max_x / tile_width;
  int min_ty = min_y / tile_height;
  int max_ty = max_y / tile_height;
  
  for (int ty = min_ty; ty <= max_ty; ty++) {
    for (int tx = min_tx; tx <= max_tx; tx++) {

        // if the tile is on the map
      if (tx < 0 || ty < 0 || tx >= layer.GetWidth() || ty >= layer.GetHeight()) {
        continue;
      }
      
        // get the tile object
      int t = m_Map.GetAnimationMap()[layer.GetTile(tx, ty)].current;
      sTile& tile = m_Map.GetMap().GetTileset().GetTile(t);

      int tbx = tx * tile_width;
      int tby = ty * tile_height;

      if (tile.GetObstructionMap().TestRectangle(x1 - tbx, y1 - tby, x2 - tbx, y2 - tby)) {
        return t;
      }
    }
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::GetObstructingPerson(const char* name, int x, int y, std::string& result)
{
  // find person
  int person = -1;
  int found = -1;
  if ( IsInvalidPersonError(name, person) ) {
    result = "";
    return false;
  }

  found = FindObstructingPerson(person, x, y);
  if (found != -1) {
    result = m_Persons[found].name;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

int
CMapEngine::FindObstructingPerson(int person, int x, int y)
{
  const Person& p = m_Persons[person];
//  const sLayer& layer = m_Map.GetMap().GetLayer(p.layer);
//  const int tile_width  = m_Map.GetMap().GetTileset().GetTileWidth();
//  const int tile_height = m_Map.GetMap().GetTileset().GetTileHeight();

  int bx = (p.base_x1 + p.base_x2) / 2;
  int by = (p.base_y1 + p.base_y2) / 2;
  
  int x1 = x - bx + p.base_x1;
  int y1 = y - by + p.base_y1;
  int x2 = x - bx + p.base_x2;
  int y2 = y - by + p.base_y2;

  int min_x = (x1 < x2 ? x1 : x2);
  int max_x = (x1 > x2 ? x1 : x2);
  int min_y = (y1 < y2 ? y1 : y2);
  int max_y = (y1 > y2 ? y1 : y2);

  // check obstructions against other entities
  for (int i = 0; i < int(m_Persons.size()); i++) {

    // don't check current person
    if (i == person) {
      continue;
    }

    // if people aren't on the same layer, skip
    if (m_Persons[i].layer != m_Persons[person].layer) {
      continue;
    }

    // if this entity is a follower of the current entity, don't check it
    int j = m_Persons[i].leader;
    while (j != -1) {
      if (j == person) {
        goto skip_this_guy;
      }
      j = m_Persons[j].leader;
    }

    goto dont_skip;
skip_this_guy:
    continue;
dont_skip:

    // now do a simple bounding rectangle test
    const Person& q = m_Persons[i];

    int j_bx = (q.base_x1 + q.base_x2) / 2;
    int j_by = (q.base_y1 + q.base_y2) / 2;

    int j_x1 = int(m_Persons[i].x - j_bx + q.base_x1);
    int j_y1 = int(m_Persons[i].y - j_by + q.base_y1);
    int j_x2 = int(m_Persons[i].x - j_bx + q.base_x2);
    int j_y2 = int(m_Persons[i].y - j_by + q.base_y2);

    int min_jx = (j_x1 < j_x2 ? j_x1 : j_x2);
    int max_jx = (j_x1 > j_x2 ? j_x1 : j_x2);
    int min_jy = (j_y1 < j_y2 ? j_y1 : j_y2);
    int max_jy = (j_y1 > j_y2 ? j_y1 : j_y2);

    if (
      // if a corner is within the rectangle
      (j_x1 >= min_x && j_x1 <= max_x && j_y1 >= min_y && j_y1 <= max_y) ||
      (j_x1 >= min_x && j_x1 <= max_x && j_y2 >= min_y && j_y2 <= max_y) ||
      (j_x2 >= min_x && j_x2 <= max_x && j_y1 >= min_y && j_y1 <= max_y) ||
      (j_x2 >= min_x && j_x2 <= max_x && j_y2 >= min_y && j_y2 <= max_y) ||

      // if the other rectangle has a corner in this one
      (x1 >= min_jx && x1 <= max_jx && y1 >= min_jy && y1 <= max_jy) ||
      (x1 >= min_jx && x1 <= max_jx && y2 >= min_jy && y2 <= max_jy) ||
      (x2 >= min_jx && x2 <= max_jx && y1 >= min_jy && y1 <= max_jy) ||
      (x2 >= min_jx && x2 <= max_jx && y2 >= min_jy && y2 <= max_jy)

    ) {

      return i;

    }
  
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////

bool
CMapEngine::IsObstructed(int person, int x, int y, int& obs_person)
{
  // get useful elements
  const Person& p = m_Persons[person];
  //  const sSpriteset& s = p.spriteset->GetSpriteset();
  const sObstructionMap& obs_map = m_Map.GetMap().GetLayer(p.layer).GetObstructionMap();
//  const int tile_width  = m_Map.GetMap().GetTileset().GetTileWidth();
//  const int tile_height = m_Map.GetMap().GetTileset().GetTileHeight();
//  const sLayer& layer = m_Map.GetMap().GetLayer(p.layer);

  // test obstruction map
  int bx = (p.base_x1 + p.base_x2) / 2;
  int by = (p.base_y1 + p.base_y2) / 2;

  int x1 = x - bx + p.base_x1;
  int y1 = y - by + p.base_y1;
  int x2 = x - bx + p.base_x2;
  int y2 = y - by + p.base_y2;

  obs_person = -1;

  if (obs_map.TestRectangle(x1, y1, x2, y2)) {
    obs_person = -1;
    return true;
  }

  if (!m_Persons[person].ignoreTileObstructions)
  {
    if (FindObstructingTile(person, x, y) != -1) {
      obs_person = -1;
      return true;
    }
  }

  // don't check other entity obstructions if this spriteset ignores them
  if(m_Persons[person].ignorePersonObstructions) {
    obs_person = -1;
    return false;
  }

  obs_person = FindObstructingPerson(person, x, y);

  if (obs_person != -1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
