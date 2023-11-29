// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"


#define MAX_HISTORY 16
#define INPUT_BUF 128
#define UP_ARROW 226
#define DOWN_ARROW 227
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
#define C(x)  ((x)-'@')  // Control-x

static void consputc(int);


void eraseCurrentLineOnScreen(void);
void moveToCurrentBuffer(void);
void eraseContentOnInputBuffer();
void copyBufferToScreen(char *bufToPrintOnScreen, uint length);
void copyBufferToInputBuffer(char *bufToSaveInInput, uint length);
void addCommandToHistory();
int getCommandFromHistory(char *buffer, int historyId);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

// MODIFICATION
struct
{
  char command[MAX_HISTORY][INPUT_BUF]; // holds the actual command strings,  we will start from 0-
  uint commandLength[MAX_HISTORY];      // this will hold the length of each command string
  uint lastCommandIndex;                // the index of the last command entered to history
  int numOfCommmandsInHistory;          // number of history commands stored
  int displacement;                     // how much moved from the last command index
} historyStored;

char currentBuffer[INPUT_BUF]; // this will hold the details of the command that we were typing before accessing the history
uint currentBufferLength;

//


//PAGEBREAK: 50
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

//PAGEBREAK: 50


// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}



// outputs a character to the console taking care of newline and /n
static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

// outputs a character to both console and serial port
void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}


void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;
  uint indexOfCommandToBeAccessed;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'):  // Kill line.
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case UP_ARROW:
      if (historyStored.displacement < historyStored.numOfCommmandsInHistory - 1)
      { // current history means the oldest possible will be MAX_HISTORY-1
        eraseCurrentLineOnScreen();

        //store the currently entered command (in the terminal) to the currentBuffer
        if (historyStored.displacement == -1)
          moveToCurrentBuffer();

        eraseContentOnInputBuffer();
        historyStored.displacement++;
        indexOfCommandToBeAccessed = (historyStored.lastCommandIndex - historyStored.displacement + MAX_HISTORY) % MAX_HISTORY;
        copyBufferToScreen(historyStored.command[indexOfCommandToBeAccessed], historyStored.commandLength[indexOfCommandToBeAccessed]);
        copyBufferToInputBuffer(historyStored.command[indexOfCommandToBeAccessed], historyStored.commandLength[indexOfCommandToBeAccessed]);
      }
      break;
    case DOWN_ARROW:
      switch (historyStored.displacement)
      {
      case -1:
        break;

      case 0: // get string from old buf
        eraseCurrentLineOnScreen();
        copyBufferToInputBuffer(currentBuffer, currentBufferLength);
        copyBufferToScreen(currentBuffer, currentBufferLength);
        historyStored.displacement--;
        break;

      default:
        eraseCurrentLineOnScreen();
        historyStored.displacement--;
        indexOfCommandToBeAccessed = (historyStored.lastCommandIndex - historyStored.displacement +MAX_HISTORY) % MAX_HISTORY;
        copyBufferToScreen(historyStored.command[indexOfCommandToBeAccessed], historyStored.commandLength[indexOfCommandToBeAccessed]);
        copyBufferToInputBuffer(historyStored.command[indexOfCommandToBeAccessed], historyStored.commandLength[indexOfCommandToBeAccessed]);
        break;
      }
      break;
    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          addCommandToHistory();
          input.w = input.e;
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

// reads from console input buffer into a destination buffer
int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(myproc()->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

// writes buffer to console
int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);

  // MODIFICATION
  historyStored.numOfCommmandsInHistory = 0;
  historyStored.lastCommandIndex = 0;
  //

}

// MODIFICATIONS

void eraseCurrentLineOnScreen(void)
{
  int length = input.e - input.r;
  while (length--)
  {
    consputc(BACKSPACE);
  }
}

/*
  Copy input.buf into currentBuffer to access the command which was not executed
*/
void moveToCurrentBuffer(void)
{
  currentBufferLength = input.e - input.r;
  for (uint i = 0; i < currentBufferLength; i++)
  {
    currentBuffer[i] = input.buf[(input.r + i) % INPUT_BUF];
  }
}

/*
  clear input buffer basically set e = r;
*/
void eraseContentOnInputBuffer()
{
  input.e = input.r;
}

/*
  print bufToPrintOnScreen on-screen
*/
void copyBufferToScreen(char *bufToPrintOnScreen, uint length)
{
  uint i = 0;
  while (length--)
  {
    consputc(bufToPrintOnScreen[i]);
    i++;
  }
}

/*
  Copy bufToSaveInInput to input.buf
*/
void copyBufferToInputBuffer(char *bufToSaveInInput, uint length)
{
  for (uint i = 0; i < length; i++)
  {
    input.buf[(input.r + i) % INPUT_BUF] = bufToSaveInInput[i];
  }

  input.e = input.r + length;
}

/*
  Copy current command in input.buf to historyStored (saved history)
*/

void addCommandToHistory()
{
  uint lengthcommand = input.e - input.r - 1; // -1 to remove the last '\n' character
  if (lengthcommand == 0)
    return; // to avoid blank commands to store in history

  historyStored.displacement = -1; // reseting the users history current viewed

  if (historyStored.numOfCommmandsInHistory < MAX_HISTORY)
  {
    historyStored.numOfCommmandsInHistory++;
    // when we get to MAX_HISTORY commands in memory we keep on inserting to the array in a circular manner
  }
  historyStored.lastCommandIndex = (historyStored.lastCommandIndex + 1) % MAX_HISTORY;
  historyStored.commandLength[historyStored.lastCommandIndex] = lengthcommand;

  // do not want to save in memory the last char '/n'
  for (uint i = 0; i < lengthcommand; i++)
  {
    historyStored.command[historyStored.lastCommandIndex][i] = input.buf[(input.r + i) % INPUT_BUF];
  }
}




int getCommandFromHistory(char *buffer, int historyId)
{
  // historyId != index of command in historyStored.command
  if (historyId < 0 || historyId > MAX_HISTORY - 1)
    return 2;
  if (historyId >= historyStored.numOfCommmandsInHistory)
    return 1;

  // to ensure that every byte in the buffer array is set to the null character.
  memset(buffer, '\0', INPUT_BUF);
  int index_Where_Specified_HistoryId_Stored = (historyStored.lastCommandIndex - historyId + MAX_HISTORY) % MAX_HISTORY;

  for (uint i = 0; i < historyStored.commandLength[index_Where_Specified_HistoryId_Stored]; i++)
  {
    buffer[i % INPUT_BUF] = historyStored.command[index_Where_Specified_HistoryId_Stored][i];
  }
  return 0;
}

//
