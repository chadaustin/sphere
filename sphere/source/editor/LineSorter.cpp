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

/*
bool line_compare_ignore_case(ScintillaLine* a, ScintillaLine* b, bool ignore_case)
{
  if (!a || !b)
    return false;

  for (int i = 0; i < m_lines[a]->size && i < m_lines[b]->size; i++) {
    if (isalpha(a->data[i]) && isalpha(b->data[i])) {
      if (tolower(a->data[i]) <= tolower(b->data[i])) {
        //printf ("1. %d, %s belongs before %d, %s", m_lines[a]->size, m_lines[a]->data, m_lines[b]->size, m_lines[b]->data);
        return true;
      }
    }
    else {
      if (a->data[i] < b->data[i]) {
        //printf ("2. %d, %s belongs before %d, %s", m_lines[a]->size, m_lines[a]->data, m_lines[b]->size, m_lines[b]->data);
        return true;
      }
    }
  }

  return false;
}

bool line_compare(ScintillaLine* a, ScintillaLine* b, bool ignore_case)
{
  if (!a || !b)
    return false;

  for (int i = 0; i < a->size && i < b->size; i++) {
    if (a->data[i] < b->data[i]) {
      //printf ("%c belongs before %c\n", m_lines[a]->data[i], m_lines[b]->data[i]);
      return true;
    }
  }

  return false;
}
*/

class ScintillaLineComparer : public std::binary_function<int, int, bool> 
{
private:
  const std::vector<ScintillaLine*>& m_lines;

public:
  bool m_ignore_case;

public:
  ScintillaLineComparer(const std::vector<ScintillaLine*>& lines) : m_lines(lines) {
    m_ignore_case = false;
  }

  bool operator()(const int a, const int b) const
  {
    if (a < 0) {
      return false;
    }

    if (b < 0) {
      return false;
    }

    if (a >= m_lines.size()) {
      return false;
    }
    
    if (b >= m_lines.size()) {
      return false;
    } 

    if (m_ignore_case) {
      for (int i = 0; i < m_lines[a]->size && i < m_lines[b]->size; i++) {
        int x = m_lines[a]->data[i];
        int y = m_lines[b]->data[i];
        if (isalpha(x) && isalpha(y)) {
          if (tolower(x) != tolower(y)) {
            return (tolower(x) < tolower(y)) ? true : false;
          }
        }
        else {
          if (x != y) {
          return (x < y) ? true : false;
          }
        }
      }
    }
    else {
      for (int i = 0; i < m_lines[a]->size && i < m_lines[b]->size; i++) {
        int x = m_lines[a]->data[i];
        int y = m_lines[b]->data[i];
        if (x != y) {
          return (x < y) ? true : false;
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
  const int selection_start = SendEditor(SCI_GETSELECTIONSTART);
  const int selection_end = SendEditor(SCI_GETSELECTIONEND);

  int start_line = SendEditor(SCI_LINEFROMPOSITION, selection_start);
  int end_line   = SendEditor(SCI_LINEFROMPOSITION, selection_end);

  if (start_line - end_line == 0) {
    SetStatusText("Nothing to sort...");
    return;
  }

  if (!delete_duplicates && !sort_lines && !reverse_lines) {
    SetStatusText("No reason to sort...");
    return;
  }

  SetStatusText("Retrieving lines for sorting...");

  for (unsigned int line_number = start_line; line_number <= end_line; line_number++) 
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
  SendEditor(SCI_SETTARGETSTART, SendEditor(SCI_POSITIONFROMLINE, start_line));
  SendEditor(SCI_SETTARGETEND,   SendEditor(SCI_POSITIONFROMLINE, end_line + 1));
  SendEditor(SCI_REPLACETARGET, 0, (LRESULT) "");

  std::vector</*unsigned*/ int> line_indexes;
  for (/*unsigned*/ int i = 0; i < m_Lines.size(); i++) {
    line_indexes.push_back(i);
  }

  if (0) {
    SendEditor(SCI_ADDTEXT, strlen("Before sort...\n"), (LRESULT)"Before sort...\n");
    for (/*unsigned*/ int i = 0; i < m_Lines.size() && i < line_indexes.size(); i++) {
      /*unsigned*/ int line_index = line_indexes[i];
      if (m_Lines[line_index]->data) {
        SendEditor(SCI_ADDTEXT, m_Lines[line_index]->size, (LRESULT)m_Lines[line_index]->data);
      }
    }
  }

  SetStatusText("Sorting lines...");

  if (0) {
    printf("Before sorting\n");
    std::vector<int>::iterator it;
    int i = 0;
    for (it = line_indexes.begin(); it != line_indexes.end(); it++)
    {
      printf("line_indexes[%d] = %d\n", i, (*it));
       i++;
    }
  }

  ScintillaLineComparer line_comparer(m_Lines);
  line_comparer.m_ignore_case = ignore_case;
  std::stable_sort(line_indexes.begin(), line_indexes.end(), line_comparer);

  if (1) {
    printf("After sorting\n");
    std::vector<int>::iterator it;
    int i = 0;
    for (it = line_indexes.begin(); it != line_indexes.end(); it++)
    {
      printf("line_indexes[%d] = %d\n", i, (*it));
      i++;
    }
  }

  SetStatusText("Lines sorted...");

  if (1) {
    int max_lines = m_Lines.size() - 1;
    int last_index = 0;
    int line_index;

    // SendEditor(SCI_ADDTEXT, strlen("after sort...\n"), (LPARAM)"after sort...\n");
    for (/*unsigned*/ int i = 0; i < m_Lines.size() && i < line_indexes.size(); i++, last_index = line_index) {
      line_index = (reverse_lines ? max_lines - i : i);

      if (sort_lines) {
        line_index = line_indexes[line_index];
      }

      if (m_Lines[line_index]->data) {

        if (i > 0 && delete_duplicates) {

          if (m_Lines[line_index]->size == m_Lines[last_index]->size) {
            if (memcmp(m_Lines[line_index]->data,
                       m_Lines[last_index]->data,
                       m_Lines[line_index]->size) == 0) {
              continue;
            }
          }
        }

        SendEditor(SCI_ADDTEXT, m_Lines[line_index]->size, (LRESULT)m_Lines[line_index]->data);
      }
    }
  }

  SendEditor(SCI_SETSELECTIONSTART, selection_start);
  SendEditor(SCI_SETSELECTIONEND, selection_end);

  SendEditor(SCI_ENDUNDOACTION);

  DestroyLines();

  SetStatusText("");
}

///////////////////////////////////////////////////////////////////////////////
