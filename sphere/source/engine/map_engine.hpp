#ifndef MAP_ENGINE_HPP
#define MAP_ENGINE_HPP


#include <deque>
#include <map>
#include "engineinterface.hpp"
#include "smap.hpp"
#include "audio.hpp"
#include "input.hpp"


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

public:
  CMapEngine(IEngine* engine, IFileSystem& fs);
  ~CMapEngine();

  bool Execute(const char* filename, int fps);
  bool Run();

  const char* GetErrorMessage();

  // script interface to map engine

  bool ChangeMap(const char* filename);
  bool Exit();
  bool Update();

  bool GetNumLayers(int& layers);
  bool GetLayerWidth(int layer, int& width);
  bool GetLayerHeight(int layer, int& height);
  bool IsLayerVisible(int layer, bool& visible);
  bool SetLayerVisible(int layer, bool visible);
  bool GetNumTiles(int& tiles);
  bool SetTile(int x, int y, int layer, int tile);
  bool GetTile(int x, int y, int layer, int& tile);
  bool GetTileWidth(int& width);
  bool GetTileHeight(int& height);

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

  bool GetPersonList(std::vector<std::string>& list);

  bool CreatePerson(const char* name, const char* spriteset, bool destroy_with_map);
  bool DestroyPerson(const char* name);

  bool SetPersonX(const char* name, int x);
  bool SetPersonY(const char* name, int y);
  bool SetPersonLayer(const char* name, int layer);
  bool SetPersonDirection(const char* name, const char* direction);
  bool SetPersonFrame(const char* name, int frame);
  bool GetPersonX(const char* name, int& x);
  bool GetPersonY(const char* name, int& y);
  bool GetPersonLayer(const char* name, int& layer);
  bool GetPersonDirection(const char* name, std::string& direction);
  bool GetPersonFrame(const char* name, int& frame);
  bool SetPersonScaleAbsolute(const char* name, int width, int height);
  bool SetPersonScaleFactor(const char* name, double scale_x, double scale_y);
  SSPRITESET* GetPersonSpriteset(const char* name);

  bool FollowPerson(const char* name, const char* leader, int pixels);
  bool SetPersonScript(const char* name, int which, const char* script);
  bool CallPersonScript(const char* name, int which);
  bool GetCurrentPerson(std::string& person);
  bool QueuePersonCommand(const char* name, int command, bool immediate);
  bool ClearPersonCommands(const char* name);

  bool IsPersonObstructed(const char* name, int x, int y, bool& result);

  bool SetTalkActivationKey(int key);
  bool SetTalkDistance(int pixels);

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

    int x;
    int y;
    int layer;
    int width;
    int height;
    int base_x1;
    int base_y1;
    int base_x2;
    int base_y2;

    std::string direction;
    int frame;

    int stepping;
    int next_frame_switch;

    IEngine::script script_create;
    IEngine::script script_destroy;
    IEngine::script script_activate_touch;
    IEngine::script script_activate_talk;
    IEngine::script script_command_generator;

    struct Command {
      Command(int c, bool i) : command(c), immediate(i) { }
      int command;
      bool immediate;
    };
    std::deque<Command> commands;

    struct AnimationState {
      int x;
      int y;
      int layer;
      std::string direction;
    };

    int leader;
    std::vector<AnimationState> follow_state_queue;
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
  bool UpdateTriggers();
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

  // miscellaneous
  void ResetNextFrame();
  int FindPerson(const char* name);
  bool IsObstructed(int person, int x, int y, int& obs_person);
  
private:
  // core map engine stuff
  IEngine* m_Engine;
  IFileSystem& m_FileSystem;
  bool m_IsRunning;
  bool m_ShouldExit;

  std::string m_ErrorMessage;

  // rendering state
  bool  m_ThrottleFPS;
  int   m_FrameRate;
  qword m_NextFrame;

  SMAP       m_Map;
  ADR_STREAM m_Music;

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
