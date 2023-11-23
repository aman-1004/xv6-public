#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

struct stat {
  short type;  // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
};

// MODIFICATION

// const static char* states[] = { "UNUSED", "EMBRYO", "SLEEPING", "RUNNABLE", "RUNNING", "ZOMBIE" };

struct uproc {
  char name[16];
  int pid;
  int ppid;
  struct uproc *parent;
  uint sz; //size of process memory
  uint state;
  /* enum uprocstate state; */
};

