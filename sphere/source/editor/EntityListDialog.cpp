#include "EntityListDialog.hpp"
#include "resource.h"
//#include <algorithm>
//#include <fstream>

#include "EntityPersonDialog.hpp"
#include "EntityTriggerDialog.hpp"

#include "NumberDialog.hpp"

////////////////////////////////////////////////////////////////////////////////

sEntity* CloneEntity(const sEntity& entity)
{
  sEntity* clone = NULL;

  switch (entity.GetEntityType()) {
    case sEntity::TRIGGER: {
      clone = new sTriggerEntity;
      if (clone) {
        ((sTriggerEntity*)clone)->script = ((sTriggerEntity&)entity).script;
      }
      break;
    }

    case sEntity::PERSON: {
      clone = new sPersonEntity;
      if (clone) {
        ((sPersonEntity*)clone)->name = ((sPersonEntity&)entity).name;
        ((sPersonEntity*)clone)->script_activate_talk = ((sPersonEntity&)entity).script_activate_talk;
        ((sPersonEntity*)clone)->script_activate_touch = ((sPersonEntity&)entity).script_activate_touch;
        ((sPersonEntity*)clone)->script_create = ((sPersonEntity&)entity).script_create;
        ((sPersonEntity*)clone)->script_destroy = ((sPersonEntity&)entity).script_destroy;
        ((sPersonEntity*)clone)->script_generate_commands = ((sPersonEntity&)entity).script_generate_commands;
        ((sPersonEntity*)clone)->spriteset = ((sPersonEntity&)entity).spriteset;
      }
      break;
    }

    default:
      clone = NULL;
  }

  if (clone) {
    clone->x = entity.x;
    clone->y = entity.y;
    clone->layer = entity.layer;
  }

  return clone;
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEntityListDialog, CDialog)

  ON_COMMAND(IDC_ENTITY_EDIT_ENTITY,     OnEditEntity)
  ON_COMMAND(IDC_ENTITY_MOVE_ENTITIES,   OnMoveEntities)
  ON_COMMAND(IDC_ENTITY_DELETE_ENTITIES, OnDeleteEntities)
  ON_CBN_SELCHANGE(IDC_ENTITY_LISTBOX,   OnEntityChanged)

END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CEntityListDialog::CEntityListDialog(sMap* map)
: CDialog(IDD_ENTITY_LIST)
, m_Map(map)
{
}

////////////////////////////////////////////////////////////////////////////////

CEntityListDialog::~CEntityListDialog()
{
  unsigned int i;
  for (i = 0; i < m_Entities.size(); i++)
  {
    DeleteEntity(m_Entities[i]);
	}

  m_Entities.clear();
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::UpdateEntityDetails(char string[2048], int entity_index, sMap* map)
{
  sEntity* entity = NULL;
  if (entity_index >= 0 && entity_index < m_Entities.size()) {
    entity = m_Entities[entity_index];
  }

  if (!entity) {
    sprintf (string, "ID: %5d", entity_index);
    return;
  }

  sprintf (string, "ID: %3d X: %3d Y: %3d - Layer: %3d", entity_index,
              (entity->x / map->GetTileset().GetTileWidth()),
              (entity->y / map->GetTileset().GetTileHeight()), entity->layer);

  if (entity->layer >= 0 && entity->layer < map->GetNumLayers()) {
    if (strlen(map->GetLayer(entity->layer).GetName()) < 512) {
      sprintf (string + strlen(string), " - '%s'", map->GetLayer(entity->layer).GetName());
    }
  }

  const char* type = NULL;
  switch (entity->GetEntityType()) {
    case sEntity::TRIGGER:
      type = "trigger";
    break;

    case sEntity::PERSON:
      type = "person";
    break;
  }

  if (type != NULL) {
    sprintf (string + strlen(string), " - Type: %s", type);
  }

  switch (entity->GetEntityType()) {
    case sEntity::TRIGGER:
      if (((sTriggerEntity*)entity)->script.size() < 512) {
        sprintf (string + strlen(string), " - Script: '%s'", ((sTriggerEntity*)entity)->script.c_str());
      }
    break;

    case sEntity::PERSON:
      if (((sPersonEntity*)entity)->name.size() < 512) {
        sprintf (string + strlen(string), " - Name: '%s'", ((sPersonEntity*)entity)->name.c_str());
      }
      if (((sPersonEntity*)entity)->spriteset.size() < 512) {
        sprintf (string + strlen(string), " - Spriteset: '%s'", ((sPersonEntity*)entity)->spriteset.c_str());
      }
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CEntityListDialog::OnInitDialog()
{
  unsigned int i;
  m_Entities.resize(m_Map->GetNumEntities());
  for (i = 0; i < m_Entities.size(); i++)
  {
    const sEntity& entity = m_Map->GetEntity(i);
    m_Entities[i] = CloneEntity(entity);
  }

  UpdateButtons();

  for (i = 0; i < m_Map->GetNumEntities(); i++) {
    char string[2048];
    UpdateEntityDetails(string, i, m_Map);
    SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_ADDSTRING, 0, (LPARAM)string);
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::OnOK()
{
  int i;
  for (i = m_Map->GetNumEntities() - 1; i >= 0; i--) {
    m_Map->DeleteEntity(i);
  }

  for (i = 0; i < int(m_Entities.size()); i++)
  {
    if (m_Entities[i]) {
      m_Map->AddEntity(m_Entities[i]);
      m_Entities[i] = NULL; // m_Map owns m_Entities[i] now
    }
  }

  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::OnEntityChanged()
{
  UpdateButtons();
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::UpdateButtons()
{
  BOOL enable_edit = FALSE;
  BOOL enable_move_and_delete = FALSE;

  int num_selected = SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_GETSELCOUNT);
  if (num_selected == 1) {
    enable_edit = TRUE;
  }

  if (num_selected >= 1) {
    enable_move_and_delete = TRUE;
  }

  GetDlgItem(IDC_ENTITY_EDIT_ENTITY)->EnableWindow(enable_edit);
  GetDlgItem(IDC_ENTITY_DELETE_ENTITIES)->EnableWindow(enable_move_and_delete);
  GetDlgItem(IDC_ENTITY_MOVE_ENTITIES)->EnableWindow(enable_move_and_delete);
}

////////////////////////////////////////////////////////////////////////////////

std::vector<int>
CEntityListDialog::GetSelectedEntities()
{
  std::vector<int> list;
  unsigned int i;

  for (i = 0; i < SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_GETCOUNT, 0, 0); i++) {
    int selected = SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_GETSEL, (WPARAM) i, 0);
    if (selected > 0) {
      list.push_back(i);
    }
  }

  return list;
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::OnMoveEntities(int dx, int dy, int layer)
{
  std::vector<int> list = GetSelectedEntities();
  if (!(list.size() > 0))
    return;

  for (int i = int(list.size() - 1); i >= 0; i--)
  {
    if (list[i] >= 0 && list[i] < m_Entities.size()) {
      if (m_Entities[list[i]] != NULL)
      {
        sEntity* entity = m_Entities[list[i]];
        const int tile_width  = m_Map->GetTileset().GetTileWidth();
        const int tile_height = m_Map->GetTileset().GetTileHeight();

        bool changed = false;
        if (layer != -1) {
          if (entity->layer != layer) {
            entity->layer = layer;
            changed = true;
          }
        }
        else
        if (entity->layer >= 0 && entity->layer < m_Map->GetNumLayers()) {
          const int layer_width  =  m_Map->GetLayer(entity->layer).GetWidth() * tile_width;
          const int layer_height = m_Map->GetLayer(entity->layer).GetHeight() * tile_height;

          int x = entity->x + (dx * tile_width);
          int y = entity->y + (dy * tile_height);

          // handle wrap around
          if (x < 0) x = layer_width + x;
          else if (x >= layer_width) x = x - layer_width;

          if (y < 0) y = layer_height + y;
          else if (y >= layer_height) y = y - layer_height;

          entity->x = x;
          entity->y = y;
        }

        if (changed) {
          char string[2048];
          UpdateEntityDetails(string, list[i], m_Map);
          SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_DELETESTRING, list[i], 0);
          SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_INSERTSTRING, list[i], (LPARAM)string);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::OnMoveEntities()
{
  std::vector<int> list = GetSelectedEntities();
  if (!(list.size() > 0))
    return;

  // find out the biggest width and height of layers
  int width = 0;
  int height = 0;
  
  for (int i = 0; i < m_Map->GetNumLayers(); i++) {
    if (m_Map->GetLayer(i).GetWidth() > width)
      width = m_Map->GetLayer(i).GetWidth();
    if (m_Map->GetLayer(i).GetHeight() > height)
      height = m_Map->GetLayer(i).GetHeight();
  }

  int dx;
  int dy;
  int layer;

  CNumberDialog dld("Move Layer", "Layer Index", -1, -1, m_Map->GetNumLayers());
  if (dld.DoModal() == IDOK) {
    layer = dld.GetValue();
    if (layer != -1) {
      OnMoveEntities(0, 0, layer);
    }
    else
    if (layer == -1) {
      CNumberDialog dxd("Slide Horizontally", "Value", 0, -width, width); 
      if (dxd.DoModal() == IDOK)
      {
        dx = dxd.GetValue();
        CNumberDialog dyd("Slide Vertically", "Value", 0, -height, height); 
        if (dyd.DoModal() == IDOK)
        {
          dy = dyd.GetValue();
          OnMoveEntities(dx, dy, -1);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::OnDeleteEntities()
{
  std::vector<int> list = GetSelectedEntities();
  if (list.size() > 0) {

    int result = MessageBox("Delete selected entities?", "Entity List", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
    if (result == IDNO) {
      return;
    }

    for (int i = int(list.size() - 1); i >= 0; i--) {

      if (list[i] >= 0 && list[i] < m_Entities.size()) {
        DeleteEntity(m_Entities[list[i]]);
        m_Entities.erase(m_Entities.begin() + list[i]);
        SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_DELETESTRING, (WPARAM) i, 0);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CEntityListDialog::OnEditEntity()
{
  int current_entity = -1;

  if (SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_GETSELCOUNT) == 1) {
    std::vector<int> list = GetSelectedEntities();
    if (list.size() == 1) {
      if (list[0] >= 0 && list[0] < m_Entities.size()) {
        if (m_Entities[list[0]] != NULL) {
          current_entity = list[0];
        }
      }
    }
  }

  if (current_entity != -1)
  {
    bool changed = false;
    sEntity* entity = m_Entities[current_entity];

    switch (entity->GetEntityType())
    {
      case sEntity::TRIGGER: {
        CEntityTriggerDialog dialog((sTriggerEntity&)*entity, m_Map);
        if (dialog.DoModal() == IDOK) {
          changed = true;
        }
        break;
      }

      case sEntity::PERSON: {
        CEntityPersonDialog dialog((sPersonEntity&)*entity, m_Map);
        if (dialog.DoModal() == IDOK) {
          changed = true;
        }
        break;
      }
    }

    if (changed) {
      char string[2048];
      UpdateEntityDetails(string, current_entity, m_Map);
      SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_DELETESTRING, current_entity, 0);
      SendDlgItemMessage(IDC_ENTITY_LISTBOX, LB_INSERTSTRING, current_entity, (LPARAM)string);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

