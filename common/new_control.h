#ifndef __NEW_CONTROL_H
#define __NEW_CONTROL_H


#define IF(exp) if (exp) {
#define ELSE    } else {
#define ENDIF   }

#define WHILE(exp) while (exp) {
#define ENDWHILE   }

#define DO           do {
#define DOWHILE(exp) } while (exp);
#define DOUNTIL(exp) DOWHILE(!exp)

#define FOR(exp) for (exp) {
#define ENDFOR   }

#define SWITCH(exp) switch (exp) {
#define CASE(exp)   case exp:
#define ENDSWITCH   }


#endif
