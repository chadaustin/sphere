#ifndef MAP_ENGINE_HPP
#define MAP_ENGINE_HPP

#include <deque>
#include <map>
#include "engineinterface.hpp"
#include "smap.hpp"
#include "audio.hpp"
#include "input.hpp"
#include <string>


class CMapEngine
{
public:
  // person entity commands
  enum {
    COMMAND_WAIT,

    COMMAND_FACE_NORTH,
    COMMAND_FACE_NORTHEAST,
    COMMAND_FACE_EAST,
    COMMAND_FACE_SOUTHEAST,
    COMMAND_FACE_SOUTH,
    COMMAND_FACE_SOUTHWEST,
    COMMAND_FACE_WEST,
    COMMAND_FACE_NORTHWEST,

    COMMAND_MOVE_NORTH,
    COMMAND_MOVE_EAST,
    COMMAND_MOVE_SOUTH,
    COMMAND_MOVE_WEST,
	  
    COMMAND_DO_SCRIPT,

    NUM_COMMANDS
  };

  // person script indices
  enum {
    SCRIPT_ON_CREATE,
    SCRIPT_ON_DESTROY,
    SCRIPT_ON_ACTIVATE_TOUCH,
    SCRIPT_ON_ACTIVATE_TALK,
    SCRIPT_COMMAND_GENERATOR,
  };

  enum {
    SCRIPT_ON_ENTER_MAP,
    SCRIPT_ON_LEAVE_MAP,
    SCRIPT_ON_LEAVE_MAP_NORTH,
    SCRIPT_ON_LEAVE_MAP_EAST,
    SCRIPT_ON_LEAVE_MAP_SOUTH,
    SCRIPT_ON_LEAVE_MAP_WEST,
  };

public:
  CMapEngine(IEngine* engine, IFileSystem& fs);
  ~CMapEngine();

  bool Execute(const char* filename, int fps);
  bool Run();

  const char* GetErrorMessage();

  // script interface to map engine

  bool ChangeMap(const char* filename);
  bool Exit();
  bool IsRunning();
  bool Update();
  bool CallMapScript(int which);

  std::string GetCurrentMap();

  bool GetNumLayers(int& layers);
  bool GetLayerWidth(int layer, int& width);
  bool GetLayerHeight(int layer, int& height);
  bool GetLayerName(int layer, std::string& name);
  bool IsLayerVisible(int layer, bool& visible);
  bool SetLayerVisible(int layer, bool visible);
  bool IsLayerReflective(int layer, bool& reflective);
  bool SetLayerReflective(int layer, bool reflective);

  bool GetNumTiles(int& tiles);
  bool SetTile(int x, int y, int layer, int tile);
  bool GetTile(int x, int y, int layer, int& tile);
  bool GetTileWidth(int& width);
  bool GetTileHeight(int& height);

  bool GetTileImage(int tile, IMAGE& image);
  bool SetTileImage(int tile, IMAGE image);
  bool GetTileDelay(int tile, int& delay);
  bool SetTileDelay(int tile, int delay);
  bool GetNextAnimatedTile(int& tile);
  bool SetNextAnimatedTile(int current_tile, int next_tile);

  bool ReplaceTilesOnLayer(int layer, int old_tile, int new_tile);

  // trigger and zones
  bool IsTriggerAt(int location_x, int location_y, int layer);
  bool AreZonesAt(int location_x, int location_y, int layer);
  bool ExecuteTrigger(int location_x, int location_y, int layer);
  bool ExecuteZones(int location_x, int location_y, int layer);
  bool ExecuteTriggerScript(int trigger_index);
  bool ExecuteZoneScript(int zone_index);
  bool GetNumZones(int& zones);
  bool GetZoneX(int zone, int& x);
  bool GetZoneY(int zone, int& y);
  bool GetZoneWidth(int zone, int& width);
  bool GetZoneHeight(int zone, int& height);

  bool RenderMap();

  bool SetColorMask(RGBA color, int num_frames);
  bool SetDelayScript(int num_frames, const char* script);

  bool BindKey(int key, const char* on_key_down, const char* on_key_up);
  bool UnbindKey(int key);

  bool AttachInput(const char* person);
  bool DetachInput();
  bool IsInputAttached(bool& attached);
  bool GetInputPerson(std::string& person);

  bool SetUpdateScript(const char* script);
  bool SetRenderScript(const char* script);
  bool SetLayerRenderer(int layer, const char* script);

  bool SetLayerAlpha(int layer, int alpha);
  bool GetLayerAlpha(int layer, int& alpha);

  bool AttachCamera(const char* person);
  bool DetachCamera();
  bool IsCameraAttached(bool& attached);
  bool GetCameraPerson(std::string& person);

  bool SetCameraX(int x);
  bool SetCameraY(int y);
  bool GetCameraX(int& x);
  bool GetCameraY(int& y);

  bool MapToScreenX(int layer, int mx, int& sx);
  bool MapToScreenY(int layer, int my, int& sy);
  bool ScreenToMapX(int layer, int sx, int& mx);
  bool ScreenToMapY(int layer, int sx, int& my);

  bool GetPersonList(std::vector<std::string>& list);

  bool CreatePerson(const char* name, const char* spriteset, bool destroy_with_map);
  bool DestroyPerson(const char* name);

  bool SetPersonX(const char* name, int x);
  bool SetPersonY(const char* name, int y);
  bool SetPersonXYFloat(const char* name, double x, double y);
  bool SetPersonLayer(const char* name, int layer);
  bool SetPersonDirection(const char* name, const char* direction);
  bool SetPersonFrame(const char* name, int frame);
  bool GetPersonX(const char* name, int& x);
  bool GetPersonY(const char* name, int& y);
  bool GetPersonXFloat(const char* name, double& x);
  bool GetPersonYFloat(const char* name, double& y);
  bool GetPersonLayer(const char* name, int& layer);
  bool GetPersonDirection(const char* name, std::string& direction);
  bool GetPersonFrame(const char* name, int& frame);
  bool SetPersonFrameRevert(const char* name, int i);
  bool GetPersonFrameRevert(const char* name, int& i);
  bool SetPersonSpeedXY(const char* name, double x, double y);
  bool GetPersonSpeedX(const char* name, double& x);
  bool GetPersonSpeedY(const char* name, double& y);
  bool SetPersonScaleAbsolute(const char* name, int width, int height);
  bool SetPersonScaleFactor(const char* name, double scale_x, double scale_y);
  bool SetPersonMask(const char* name, RGBA mask);
  bool GetPersonMask(const char* name, RGBA& mask);
  SSPRITESET* GetPersonSpriteset(const char* name);

  bool FollowPerson(const char* name, const char* leader, int pixels);
  bool SetPersonScript(const char* name, int which, const char* script);
  bool CallPersonScript(const char* name, int which);
  bool GetCurrentPerson(std::string& person);
  bool QueuePersonCommand(const char* name, int command, bool immediate);
  bool QueuePersonScript(const char* name, const char* script, bool immediate);
  bool ClearPersonCommands(const char* name);
  bool IsCommandQueueEmpty(const char* name, bool& empty);

  bool IgnorePersonObstructions(const char* name, bool ignore);
  bool IgnoreTileObstructions(const char* name, bool ignore);

  bool IsIgnoringPersonObstructions(const char* name, bool& ignoring);
  bool IsIgnoringTileObstructions(const char* name, bool& ignoring);

  bool IsPersonObstructed(const char* name, int x, int y, bool& result);
  bool GetObstructingTile(const char* name, int x, int y, int& result);
  bool GetObstructingPerson(const char* name, int x, int y, std::string& result);

  bool SetTalkActivationKey(int key);
  bool SetTalkDistance(int pixels);
  int GetTalkActivationKey();
  int GetTalkDistance();

  bool SetMapEngineFrameRate(int fps);
  int GetMapEngineFrameRate();

private:
  struct DelayScript {
    int frames_left;
    IEngine::script script;
  };

  struct Person {
    bool destroy_with_map;

    std::string name;
    std::string description;
    SSPRITESET* spriteset;
    RGBA mask;

    double x;
    double y;
    int layer;
    int width;
    int height;
    int base_x1;
    int base_y1;
    int base_x2;
    int base_y2;

    double speed_x;
    double speed_y;

    std::string direction;
    int frame;

    int stepping;
    int next_frame_switch;
    int stepping_frame_revert;
    int stepping_frame_revert_count;

    IEngine::script script_create;
    IEngine::script script_destroy;
    IEngine::script script_activate_touch;
    IEngine::script script_activate_talk;
    IEngine::script script_command_generator;

    struct Command {
      Command(int c, bool i, std::string s = "") : command(c), immediate(i), script(s) { }
      int command;
      bool immediate;
      std::string script;
    };
    std::deque<Command> commands;

    struct AnimationState {
      double x;
      double y;
      int layer;
      std::string direction;
    };

    int leader;
    std::vector<AnimationState> follow_state_queue;

    bool ignorePersonObstructions;
    bool ignoreTileObstructions;
  };

  struct Trigger {
    IEngine::script script;
    int x;
    int y;
    int layer;
  };

  struct Zone {
    IEngine::script script;
    int x1;
    int y1;
    int x2;
    int y2;
    int layer;
    int reactivate_in_num_steps;
    int current_step;
  };

private:
  void CloseEngine();
  bool OpenMap(const char* filename);
  bool CloseMap();

  bool ExecuteScript(IEngine::script script, std::string& error);
  bool ExecuteScript(const char* script, std::string& error);

  bool CompileEdgeScripts();
  bool LoadMapPersons();
  bool DestroyMapPersons();
  bool DestroyPersonStructure(Person& p);
  bool LoadTriggers();
  void DestroyTriggers();
  bool LoadZones();
  void DestroyZones();
  
  bool Render();
  bool RenderEntities(int layer, bool flipped, int offset_x, int offset_y);
  bool UpdateWorld(bool input_valid);
  bool UpdatePersons();
  bool UpdatePerson(int person_index, bool& activated);
  bool UpdateFollower(int person_index);

  int FindTrigger(int location_x, int location_y, int layer);
  bool UpdateTriggers();

  bool IsPointWithinZone(int location_x, int location_y, int location_layer, int zone_index);
  bool IsPersonInsideZone(int person_index, int zone_index);
  bool UpdateZones();

  bool UpdateColorMasks();
  bool UpdateDelayScripts();
  bool UpdateEdgeScripts();
  bool ProcessInput();
  bool ProcessBoundKeyDown(int key);
  bool ProcessBoundKeyUp(int key);
  void ProcessUnboundKeyDown(int key);
  void ProcessUnboundKeyUp(int key);
  void DestroyBoundKeys();

  // error checking
  bool IsInvalidPersonError(const char* person_name, int& person_index);

  // miscellaneous
  void ResetNextFrame();
  int FindPerson(const char* name);
  bool IsObstructed(int person, int x, int y, int& obs_person);
  int FindObstructingTile(int person, int x, int y);
  int FindObstructingPerson(int person, int x, int y);

private:
  // core map engine stuff
  IEngine* m_Engine;
  IFileSystem& m_FileSystem;
  bool m_IsRunning;
  bool m_ShouldExit;

  std::string m_CurrentMap;
  std::string m_ErrorMessage;

  // rendering state
  bool  m_ThrottleFPS;
  int   m_FrameRate;
  qword m_NextFrame;

  SMAP       m_Map;
  audiere::OutputStreamPtr m_Music;

  // camera
  struct {
    int x;
    int y;
    int layer;
  } m_Camera;

  // input person
  bool m_IsInputAttached;
  int  m_InputPerson;
  bool m_TouchActivationAllowed;
  bool m_TalkActivationAllowed;

  // camera person
  bool m_IsCameraAttached;
  int  m_CameraPerson;

  // edge scripts
  IEngine::script m_NorthScript;
  IEngine::script m_EastScript;
  IEngine::script m_SouthScript;
  IEngine::script m_WestScript;

  // general scripts
  IEngine::script m_UpdateScript;
  IEngine::script m_RenderScript;
  std::vector<IEngine::script> m_LayerRenderers;

  // color masks
  int m_NumFrames;
  int m_FramesLeft;
  RGBA m_CurrentColorMask;
  RGBA m_PreviousColorMask;
  RGBA m_DestinationColorMask;

  // delay scripts
  std::vector<DelayScript> m_DelayScripts;

  // bound keys
  struct KeyScripts { IEngine::script down, up; };
  std::map<int, KeyScripts> m_BoundKeys;


  // ENTITIES

  // triggers
  bool m_OnTrigger;
  int m_LastTrigger;
  std::vector<Trigger> m_Triggers;

  // persons
  std::vector<Person> m_Persons;
  std::string m_CurrentPerson;

  int m_TalkActivationKey;
  int m_TalkActivationDistance;

  // zones
  std::vector<Zone> m_Zones;

  // input handling
  bool m_Keys[MAX_KEY];
};


#endif
