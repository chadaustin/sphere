#ifndef __OPTION_TREE_HPP
#define __OPTION_TREE_HPP



#include <windows.h>
#include <commctrl.h>



enum EDataType { OT_NONE, OT_INTEGER, OT_STRING, OT_FLOATING };


union UData
{
  UData() { }
  UData(int i)         { integer = i; }
  UData(const char* s) { strncpy(str, s, 80); str[80] = 0; }
  UData(double f)      { floating = f; }

  int    integer;
  char   str[81];
  double floating;
};


class COptionTreeDialog;


/*** COptionTree ***/

class COptionTree
{
  friend COptionTreeDialog;

public:
  COptionTree();

  void AddTree(const char* name, COptionTree* tree);
  void AddItem(const char* name, int key, EDataType data_type, UData data, const char* label = NULL);

  EDataType    GetType(int key) const;
  const UData* GetItem(int key) const;

private:
  // linked list
  struct Node
  {
    Node* next;
    char* name;
    bool  is_sub_tree;
    
    // is_sub_tree == true
    COptionTree* sub_tree;
                
    // is_sub_tree == false
    int       key;
    EDataType data_type;
    UData     data;
    char*     label;
  };

private:
  // destructor is private because user can create trees and give them away, but
  // we must destroy them.  This is because we don't want it deleting trees while
  // they are in use.
  ~COptionTree();

  void AddNode(Node* node);

private:
  Node* m_Head;
};



/*** COptionTreeDialog ***/

class COptionTreeDialog
{
public:
  COptionTreeDialog();
  ~COptionTreeDialog();

  void SetTree(COptionTree* tree);
  bool Execute(HINSTANCE instance, HWND parent, const char* caption);

private:
  static BOOL CALLBACK DialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  void OnCreate();
  void OnDestroy();
  void OnSize(int cx, int cy);
  void OnItemClick();
  void OnItemExpand();
  void OnPaint(HDC dc);

  static void GetNodeText(COptionTree::Node* node, char* text);
  void PopulateTreeControl(COptionTree* tree, HTREEITEM parent = NULL);
  static LRESULT CALLBACK NewTreeProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  void ItemDialogBox(COptionTree::Node* node);
  static BOOL CALLBACK ItemDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

private:
  HINSTANCE m_Instance;
  HWND      m_Window;
  HWND      m_TreeControl;

  WNDPROC m_OldTreeProc;

  COptionTree* m_Tree;
};



#endif
