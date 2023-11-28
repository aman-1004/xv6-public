#include "types.h"

#define MAX_HISTORY 16
#define INPUT_BUF 128
#define UP_ARROW 226
#define DOWN_ARROW 227
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
#define C(x)  ((x)-'@')  // Control-x

void eraseCurrentLineOnScreen(void);
void moveToOldBuffer(void);
void eraseContentOnInputBuffer();
void copyBufferToScreen(char *bufToPrintOnScreen, uint length);
void copyBufferToInputBuffer(char *bufToSaveInInput, uint length);
void saveCommandInHistory();
int getCommandFromHistory(char *buffer, int historyId);