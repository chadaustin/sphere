#ifndef __UNIQUE_HPP
#define __UNIQUE_HPP


class __UNIQUE_INT
{
public:
  __UNIQUE_INT() { value = next++; }
  operator int() const { return value; }

private:
  int value;
  static int next;
}; extern int __unique_id_counter;


#define DECLARE_UNIQUE_ID(name)         \
extern const __UNIQUE_INT name;


#define DEFINE_UNIQUE_ID(name)          \
const __UNIQUE_INT name;


#endif
