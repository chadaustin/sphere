#pragma warning(disable: 4786)

#include <Scintilla.h>
#include <SciLexer.h>

#include <algorithm>
#include <functional>
#include <vector>
#include <ctype.h>

#include "LineSorter.hpp"

#include "Editor.hpp"

///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

class ScintillaLineComparer : public std::binary_function<unsigned int, unsigned int, bool> 
{
private:
  const std::vector<ScintillaLine*>	&m_lines;

public:
  bool m_ignore_case;

public:
  ScintillaLineComparer(const std::vector<ScintillaLine*>& lines) : m_lines(lines) { }

  bool operator()(unsigned int a, unsigned int b) const
  {
    if (a >= m_lines.size() || b >= m_lines.size()) {
      return false;
    } 

    if (m_ignore_case) {
      for (int i = 0; i < m_lines[a]->size && i < m_lines[b]->size; i++) {
        if (isalpha(m_lines[a]->data[i]) && isalpha(m_lines[b]->data[i])) {
          if (tolower(m_lines[a]->data[i]) <= tolower(m_lines[b]->data[i])) {
            return true;
          }
        }
        else {
          if (m_lines[a]->data[i] < m_lines[b]->data[i]) {
            return true;
          }
        }
      }
    }
    else {
      for (int i = 0; i < m_lines[a]->size && i < m_lines[b]->size; i++) {
        if (m_lines[a]->data[i] < m_lines[b]->data[i]) {
          return true;
        }
      }
    }

    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////

CLineSorter::CLineSorter()
{
  delete_duplicates = false;
  sort_lines = true;
  reverse_lines = false;
  ignore_case = false;
}

///////////////////////////////////////////////////////////////////////////////

CLineSorter::~CLineSorter()
{
  DestroyLines();
}

///////////////////////////////////////////////////////////////////////////////

void
CLineSorter::DestroyLines()
{
  for (unsigned int i = 0; i < m_Lines.size(); i++) {
    if (m_Lines[i]->data) {
      delete[] m_Lines[i]->data;
      m_Lines[i]->data = NULL;
      m_Lines[i]->size = 0;
    }

    delete m_Lines[i];
    m_Lines[i] = NULL;
  }

  m_Lines.clear();
}

///////////////////////////////////////////////////////////////////////////////

LRESULT
CLineSorter::SendEditor(UINT msg, WPARAM wparam, LPARAM lparam) {
  return ::SendMessage(m_Editor, msg, wparam, lparam);
}

///////////////////////////////////////////////////////////////////////////////

void
CLineSorter::SetStatusText(const char* string)
{
  GetStatusBar()->SetWindowText(string);
}

///////////////////////////////////////////////////////////////////////////////

void
CLineSorter::Sort()
{
  int selection_start = SendEditor(SCI_LINEFROMPOSITION, SendEditor(SCI_GETSELECTIONSTART));
  int selection_end   = SendEditor(SCI_LINEFROMPOSITION, SendEditor(SCI_GETSELECTIONEND));

  if (selection_start - selection_end == 0) {
    SetStatusText("Nothing to sort...");
    return;
  }

  if (!delete_duplicates && !sort_lines && !reverse_lines) {
    SetStatusText("No reason to sort...");
    return;
  }

  SetStatusText("Retrieving lines for sorting...");

  for (unsigned int line_number = selection_start; line_number <= selection_end; line_number++) 
  {
    unsigned int line_index = m_Lines.size();
    ScintillaLine* line_ptr = new ScintillaLine;
    if (!line_ptr) {
      //SetStatusText("fail error 1...");
      return;
    }

    m_Lines.push_back(line_ptr);
    if (line_index + 1 != m_Lines.size()) {
      delete line_ptr;
      line_ptr = NULL;
      //SetStatusText("fail error 2...");
      return;
    }

    m_Lines[line_index]->data = NULL;
    m_Lines[line_index]->size = 0;

    int line_length = SendEditor(SCI_LINELENGTH, line_number);
    if (line_length >= 0) {
      m_Lines[line_index]->data = new char[line_length + 1];
      if (m_Lines[line_index]->data != NULL) {
        m_Lines[line_index]->size = line_length;
        SendEditor(SCI_GETLINE, line_number, (LRESULT)m_Lines[line_index]->data);
        m_Lines[line_index]->data[line_length] = '\0';
      } else {
        //SetStatusText("fail error 3...");
        return;
      }
    }
    else {
      //SetStatusText("fail error 4...");
      return;
    }
  }

  //SetStatusText("Removing old lines...");

  SendEditor(SCI_BEGINUNDOACTION);

  // remove the old selection
  SendEditor(SCI_SETTARGETSTART, SendEditor(SCI_POSITIONFROMLINE, selection_start));
  SendEditor(SCI_SETTARGETEND,   SendEditor(SCI_POSITIONFROMLINE, selection_end + 1));
  SendEditor(SCI_REPLACETARGET, 0, (LRESULT) "");

  std::vector<unsigned int> line_indexes;
  for (unsigned int i = 0; i < m_Lines.size(); i++) {
    line_indexes.push_back(i);
  }

  if (0) {
    SendEditor(SCI_ADDTEXT, strlen("Before sort...\n"), (LRESULT)"Before sort...\n");
    for (unsigned int i = 0; i < m_Lines.size() && i < line_indexes.size(); i++) {
      unsigned int line_index = line_indexes[i];
      if (m_Lines[line_index]->data) {
        SendEditor(SCI_ADDTEXT, m_Lines[line_index]->size, (LRESULT)m_Lines[line_index]->data);
      }
    }
  }

  SetStatusText("Sorting lines...");

  ScintillaLineComparer line_comparer(m_Lines);
  line_comparer.m_ignore_case = ignore_case;
  std::sort(line_indexes.begin(), line_indexes.end(), line_comparer);

  SetStatusText("Lines sorted...");

  if (1) {
    // SendEditor(SCI_ADDTEXT, strlen("after sort...\n"), (LPARAM)"after sort...\n");
    for (unsigned int i = 0; i < m_Lines.size() && i < line_indexes.size(); i++) {
      unsigned int line_index = sort_lines ? line_indexes[(reverse_lines ? (m_Lines.size() - 1) - i : i)] : (reverse_lines ? (m_Lines.size() - 1) - i : i);

      if (m_Lines[line_index]->data) {

        if (i > 0 && delete_duplicates) {
          unsigned int last_index = sort_lines ? line_indexes[(reverse_lines ? (m_Lines.size() - 1) - i + 1 : i - 1)] : (reverse_lines ? (m_Lines.size() - 1) - i + 1 : i - 1);

          if (m_Lines[line_index]->size == m_Lines[last_index]->size) {
            if (memcmp(m_Lines[line_index]->data,
                       m_Lines[last_index]->data,
                       m_Lines[line_index]->size) == 0)
              continue;
          }
        }

        SendEditor(SCI_ADDTEXT, m_Lines[line_index]->size, (LRESULT)m_Lines[line_index]->data);
      }
    }
  }

  SendEditor(SCI_ENDUNDOACTION);

  DestroyLines();

  SetStatusText("");
}

///////////////////////////////////////////////////////////////////////////////
