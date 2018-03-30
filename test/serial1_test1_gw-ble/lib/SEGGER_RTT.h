
extern "C" {

#ifndef SEGGER_RTT_H_
#define SEGGER_RTT_H_

/*********************************************************************
*              SEGGER MICROCONTROLLER SYSTEME GmbH                   *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996-2014 SEGGER Microcontroller Systeme GmbH           *
*                                                                    *
* Internet: www.segger.com Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT.h
Date    : 17 Dec 2014
Purpose : Implementation of SEGGER real-time terminal which allows
          real-time terminal communication on targets which support
          debugger memory accesses while the CPU is running.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SEGGER_RTT_MODE_MASK                  (3 << 0)

#define SEGGER_RTT_MODE_NO_BLOCK_SKIP         (0)
#define SEGGER_RTT_MODE_NO_BLOCK_TRIM         (1 << 0)
#define SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL    (1 << 1)

#define RTT_CTRL_RESET                "[0m"

#define RTT_CTRL_CLEAR                "[2J"

#define RTT_CTRL_TEXT_BLACK           "[2;30m"
#define RTT_CTRL_TEXT_RED             "[2;31m"
#define RTT_CTRL_TEXT_GREEN           "[2;32m"
#define RTT_CTRL_TEXT_YELLOW          "[2;33m"
#define RTT_CTRL_TEXT_BLUE            "[2;34m"
#define RTT_CTRL_TEXT_MAGENTA         "[2;35m"
#define RTT_CTRL_TEXT_CYAN            "[2;36m"
#define RTT_CTRL_TEXT_WHITE           "[2;37m"

#define RTT_CTRL_TEXT_BRIGHT_BLACK    "[1;30m"
#define RTT_CTRL_TEXT_BRIGHT_RED      "[1;31m"
#define RTT_CTRL_TEXT_BRIGHT_GREEN    "[1;32m"
#define RTT_CTRL_TEXT_BRIGHT_YELLOW   "[1;33m"
#define RTT_CTRL_TEXT_BRIGHT_BLUE     "[1;34m"
#define RTT_CTRL_TEXT_BRIGHT_MAGENTA  "[1;35m"
#define RTT_CTRL_TEXT_BRIGHT_CYAN     "[1;36m"
#define RTT_CTRL_TEXT_BRIGHT_WHITE    "[1;37m"

#define RTT_CTRL_BG_BLACK             "[24;40m"
#define RTT_CTRL_BG_RED               "[24;41m"
#define RTT_CTRL_BG_GREEN             "[24;42m"
#define RTT_CTRL_BG_YELLOW            "[24;43m"
#define RTT_CTRL_BG_BLUE              "[24;44m"
#define RTT_CTRL_BG_MAGENTA           "[24;45m"
#define RTT_CTRL_BG_CYAN              "[24;46m"
#define RTT_CTRL_BG_WHITE             "[24;47m"

#define RTT_CTRL_BG_BRIGHT_BLACK      "[4;40m"
#define RTT_CTRL_BG_BRIGHT_RED        "[4;41m"
#define RTT_CTRL_BG_BRIGHT_GREEN      "[4;42m"
#define RTT_CTRL_BG_BRIGHT_YELLOW     "[4;43m"
#define RTT_CTRL_BG_BRIGHT_BLUE       "[4;44m"
#define RTT_CTRL_BG_BRIGHT_MAGENTA    "[4;45m"
#define RTT_CTRL_BG_BRIGHT_CYAN       "[4;46m"
#define RTT_CTRL_BG_BRIGHT_WHITE      "[4;47m"


/*********************************************************************
*
*       RTT API functions
*
**********************************************************************
*/

int     SEGGER_RTT_Read             (unsigned BufferIndex,       char* pBuffer, unsigned BufferSize);
int     SEGGER_RTT_Write            (unsigned BufferIndex, const char* pBuffer, unsigned NumBytes);
int     SEGGER_RTT_WriteString      (unsigned BufferIndex, const char* s);

int     SEGGER_RTT_GetKey           (void);
int     SEGGER_RTT_WaitKey          (void);
int     SEGGER_RTT_HasKey           (void);

int     SEGGER_RTT_ConfigUpBuffer   (unsigned BufferIndex, const char* sName, char* pBuffer, int BufferSize, int Flags);
int     SEGGER_RTT_ConfigDownBuffer (unsigned BufferIndex, const char* sName, char* pBuffer, int BufferSize, int Flags);

void    SEGGER_RTT_Init             (void);

/*********************************************************************
*
*       RTT "Terminal" API functions
*
**********************************************************************
*/
void    SEGGER_RTT_SetTerminal        (char TerminalId);
int     SEGGER_RTT_TerminalOut        (char TerminalId, const char* s);

/*********************************************************************
*
*       RTT printf functions (require SEGGER_RTT_printf.c)
*
**********************************************************************
*/
int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...);

/*************************** End of file ****************************/






/*********************************************************************
*              SEGGER MICROCONTROLLER GmbH & Co. KG                  *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2014-2014 SEGGER Microcontroller GmbH & Co. KG          *
*                                                                    *
*       Internet: www.segger.com Support: support@segger.com         *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT.c
Date    : 17 Dec 2014
Purpose : Implementation of SEGGER real-time terminal (RTT) which allows
          real-time terminal communication on targets which support
          debugger memory accesses while the CPU is running.

          Type "int" is assumed to be 32-bits in size
          H->T    Host to target communication
          T->H    Target to host communication

          RTT channel 0 is always present and reserved for Terminal usage.
          Name is fixed to "Terminal"

---------------------------END-OF-HEADER------------------------------
*/


/*********************************************************************
*              SEGGER MICROCONTROLLER SYSTEME GmbH                   *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996-2014 SEGGER Microcontroller Systeme GmbH           *
*                                                                    *
* Internet: www.segger.com Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT_Conf.h
Date    : 17 Dec 2014
Purpose : Implementation of SEGGER real-time terminal which allows
          real-time terminal communication on targets which support
          debugger memory accesses while the CPU is running.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define SEGGER_RTT_MAX_NUM_UP_BUFFERS             (2)     // Max. number of up-buffers (T->H) available on this target    (Default: 2)
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS           (2)     // Max. number of down-buffers (H->T) available on this target  (Default: 2)

#define BUFFER_SIZE_UP                            (1024)  // Size of the buffer for terminal output of target, up to host (Default: 1k)
#define BUFFER_SIZE_DOWN                          (16)    // Size of the buffer for terminal input to target from host (Usually keyboard input) (Default: 16)

#define SEGGER_RTT_PRINTF_BUFFER_SIZE             (64)    // Size of buffer for RTT printf to bulk-send chars via RTT     (Default: 64)

//
// Target is not allowed to perform other RTT operations while string still has not been stored completely.
// Otherwise we would probably end up with a mixed string in the buffer.
// If using  RTT from within interrupts, multiple tasks or multi processors, define the SEGGER_RTT_LOCK() and SEGGER_RTT_UNLOCK() function here.
//
#define SEGGER_RTT_LOCK()
#define SEGGER_RTT_UNLOCK()

//
// Define SEGGER_RTT_IN_RAM as 1
// when using RTT in RAM targets (init and data section both in RAM).
// This prevents the host to falsly identify the RTT Callback Structure
// in the init segment as the used Callback Structure.
//
// When defined as 1,
// the first call to an RTT function will modify the ID of the RTT Callback Structure.
// To speed up identifying on the host,
// especially when RTT functions are not called at the beginning of execution,
// SEGGER_RTT_Init() should be called at the start of the application.
//
#define SEGGER_RTT_IN_RAM                         (0)

/*************************** End of file ****************************/


#include <string.h>                 // for memcpy

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   BUFFER_SIZE_UP
  #define BUFFER_SIZE_UP                                  (1024)  // Size of the buffer for terminal output of target, up to host
#endif

#ifndef   BUFFER_SIZE_DOWN
  #define BUFFER_SIZE_DOWN                                (16)    // Size of the buffer for terminal input to target from host (Usually keyboard input)
#endif

#ifndef   SEGGER_RTT_MAX_NUM_UP_BUFFERS
  #define SEGGER_RTT_MAX_NUM_UP_BUFFERS                   (1)     // Number of up-buffers (T->H) available on this target
#endif

#ifndef   SEGGER_RTT_MAX_NUM_DOWN_BUFFERS
  #define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS                 (1)     // Number of down-buffers (H->T) available on this target
#endif

#ifndef   SEGGER_RTT_LOCK
  #define SEGGER_RTT_LOCK()
#endif

#ifndef   SEGGER_RTT_UNLOCK
  #define SEGGER_RTT_UNLOCK()
#endif

#ifndef   SEGGER_RTT_IN_RAM
  #define SEGGER_RTT_IN_RAM                               (0)
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define MIN(a, b)        (((a) < (b)) ? (a) : (b))
#define MAX(a, b)        (((a) > (b)) ? (a) : (b))

#define MEMCPY(a, b, c)  memcpy((a),(b),(c))

//
// For some environments, NULL may not be defined until certain headers are included
//
#ifndef NULL
  #define NULL 0
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

//
// Description for a circular buffer (also called "ring buffer")
// which is used as up- (T->H) or down-buffer (H->T)
//
typedef struct {
  const char* sName;                     // Optional name. Standard names so far are: "Terminal", "VCom"
  char*  pBuffer;                        // Pointer to start of buffer
  int    SizeOfBuffer;                   // Buffer size in bytes. Note that one byte is lost, as this implementation does not fill up the buffer in order to avoid the problem of being unable to distinguish between full and empty.
  volatile int WrOff;                    // Position of next item to be written by either host (down-buffer) or target (up-buffer). Must be volatile since it may be modified by host (down-buffer)
  volatile int RdOff;                    // Position of next item to be read by target (down-buffer) or host (up-buffer). Must be volatile since it may be modified by host (up-buffer)
  int    Flags;                          // Contains configuration flags
} RING_BUFFER;

//
// RTT control block which describes the number of buffers available
// as well as the configuration for each buffer
//
//
typedef struct {
  char        acID[16];                                 // Initialized to "SEGGER RTT"
  int         MaxNumUpBuffers;                          // Initialized to SEGGER_RTT_MAX_NUM_UP_BUFFERS (type. 2)
  int         MaxNumDownBuffers;                        // Initialized to SEGGER_RTT_MAX_NUM_DOWN_BUFFERS (type. 2)
  RING_BUFFER aUp[SEGGER_RTT_MAX_NUM_UP_BUFFERS];       // Up buffers, transferring information up from target via debug probe to host
  RING_BUFFER aDown[SEGGER_RTT_MAX_NUM_DOWN_BUFFERS];   // Down buffers, transferring information down from host via debug probe to target
} SEGGER_RTT_CB;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// Allocate buffers for channel 0
//
static char _acUpBuffer  [BUFFER_SIZE_UP];
static char _acDownBuffer[BUFFER_SIZE_DOWN];
//
// Initialize SEGGER Real-time-Terminal control block (CB)
//
static SEGGER_RTT_CB _SEGGER_RTT = {
#if SEGGER_RTT_IN_RAM
  "SEGGER RTTI",
#else
  "SEGGER RTT",
#endif
  SEGGER_RTT_MAX_NUM_UP_BUFFERS,
  SEGGER_RTT_MAX_NUM_DOWN_BUFFERS,
  {{ "Terminal", &_acUpBuffer[0],   sizeof(_acUpBuffer),   0, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP }},
  {{ "Terminal", &_acDownBuffer[0], sizeof(_acDownBuffer), 0, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP }},
};

static char _ActiveTerminal;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _strlen
*
*  Function description
*    ANSI compatible function to determine the length of a string
*
*  Return value
*    Length of string in bytes.
*
*  Parameters
*    s         Pointer to \0 terminated string.
*
*  Notes
*    (1) s needs to point to an \0 terminated string. Otherwise proper functionality of this function is not guaranteed.
*/
static int _strlen(const char* s) {
  int Len;

  Len = 0;
  if (s == NULL) {
    return 0;
  }
  do {
    if (*s == 0) {
      break;
    }
    Len++;
    s++;
  } while (1);
  return Len;
}

/*********************************************************************
*
*       _Init
*
*  Function description
*    In case SEGGER_RTT_IN_RAM is defined,
*    _Init() modifies the ID of the RTT CB to allow identifying the
*    RTT Control Block Structure in the data segment.
*/
static void _Init(void) {
#if SEGGER_RTT_IN_RAM
  if (_SEGGER_RTT.acID[10] == 'I') {
    _SEGGER_RTT.acID[10] = '\0';
  }
#endif
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SEGGER_RTT_Read
*
*  Function description
*    Reads characters from SEGGER real-time-terminal control block
*    which have been previously stored by the host.
*
*  Parameters
*    BufferIndex  Index of Down-buffer to be used. (e.g. 0 for "Terminal")
*    pBuffer      Pointer to buffer provided by target application, to copy characters from RTT-down-buffer to.
*    BufferSize   Size of the target application buffer
*
*  Return values
*    Number of bytes that have been read
*/
int SEGGER_RTT_Read(unsigned BufferIndex, char* pBuffer, unsigned BufferSize) {
  int NumBytesRem;
  unsigned NumBytesRead;
  int RdOff;
  int WrOff;

  SEGGER_RTT_LOCK();
  _Init();
  RdOff = _SEGGER_RTT.aDown[BufferIndex].RdOff;
  WrOff = _SEGGER_RTT.aDown[BufferIndex].WrOff;
  NumBytesRead = 0;
  //
  // Read from current read position to wrap-around of buffer, first
  //
  if (RdOff > WrOff) {
    NumBytesRem = _SEGGER_RTT.aDown[BufferIndex].SizeOfBuffer - RdOff;
    NumBytesRem = MIN(NumBytesRem, (int)BufferSize);
    MEMCPY(pBuffer, _SEGGER_RTT.aDown[BufferIndex].pBuffer + RdOff, NumBytesRem);
    NumBytesRead += NumBytesRem;
    pBuffer      += NumBytesRem;
    BufferSize   -= NumBytesRem;
    RdOff        += NumBytesRem;
    //
    // Handle wrap-around of buffer
    //
    if (RdOff == _SEGGER_RTT.aDown[BufferIndex].SizeOfBuffer) {
      RdOff = 0;
    }
  }
  //
  // Read remaining items of buffer
  //
  NumBytesRem = WrOff - RdOff;
  NumBytesRem = MIN(NumBytesRem, (int)BufferSize);
  if (NumBytesRem > 0) {
    MEMCPY(pBuffer, _SEGGER_RTT.aDown[BufferIndex].pBuffer + RdOff, NumBytesRem);
    NumBytesRead += NumBytesRem;
    pBuffer      += NumBytesRem;
    BufferSize   -= NumBytesRem;
    RdOff        += NumBytesRem;
  }
  if (NumBytesRead) {
    _SEGGER_RTT.aDown[BufferIndex].RdOff = RdOff;
  }
  SEGGER_RTT_UNLOCK();
  return NumBytesRead;
}

/*********************************************************************
*
*       SEGGER_RTT_Write
*
*  Function description
*    Stores a specified number of characters in SEGGER RTT
*    control block which is then read by the host.
*
*  Parameters
*    BufferIndex  Index of "Up"-buffer to be used. (e.g. 0 for "Terminal")
*    pBuffer      Pointer to character array. Does not need to point to a \0 terminated string.
*    NumBytes     Number of bytes to be stored in the SEGGER RTT control block.
*
*  Return values
*    Number of bytes which have been stored in the "Up"-buffer.
*
*  Notes
*    (1) If there is not enough space in the "Up"-buffer, remaining characters of pBuffer are dropped.
*/
int SEGGER_RTT_Write(unsigned BufferIndex, const char* pBuffer, unsigned NumBytes) {
  int NumBytesToWrite;
  unsigned NumBytesWritten;
  int RdOff;
  //
  // Target is not allowed to perform other RTT operations while string still has not been stored completely.
  // Otherwise we would probably end up with a mixed string in the buffer.
  //
  SEGGER_RTT_LOCK();
  _Init();
  //
  // In case we are not in blocking mode,
  // we need to calculate, how many bytes we can put into the buffer at all.
  //
  if ((_SEGGER_RTT.aUp[BufferIndex].Flags & SEGGER_RTT_MODE_MASK) != SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL) {
    RdOff = _SEGGER_RTT.aUp[BufferIndex].RdOff;
    NumBytesToWrite =  RdOff - _SEGGER_RTT.aUp[BufferIndex].WrOff - 1;
    if (NumBytesToWrite < 0) {
      NumBytesToWrite += _SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer;
    }
    //
    // If the complete data does not fit in the buffer, check if we have to skip it completely or trim the data
    //
    if ((int)NumBytes > NumBytesToWrite) {
      if ((_SEGGER_RTT.aUp[BufferIndex].Flags & SEGGER_RTT_MODE_MASK) == SEGGER_RTT_MODE_NO_BLOCK_SKIP) {
        SEGGER_RTT_UNLOCK();
        return 0;
      } else {
        NumBytes = NumBytesToWrite;
      }
    }
  }
  //
  // Early out if nothing is to do
  //
  if (NumBytes == 0) {
    SEGGER_RTT_UNLOCK();
    return 0;
  }
  //
  // Write data to buffer and handle wrap-around if necessary
  //
  NumBytesWritten = 0;
  do {
    RdOff = _SEGGER_RTT.aUp[BufferIndex].RdOff;                          // May be changed by host (debug probe) in the meantime
    NumBytesToWrite = RdOff - _SEGGER_RTT.aUp[BufferIndex].WrOff - 1;
    if (NumBytesToWrite < 0) {
      NumBytesToWrite += _SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer;
    }
    NumBytesToWrite = MIN(NumBytesToWrite, (_SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer - _SEGGER_RTT.aUp[BufferIndex].WrOff));    // Number of bytes that can be written until buffer wrap-around
    NumBytesToWrite = MIN(NumBytesToWrite, (int)NumBytes);
    MEMCPY(_SEGGER_RTT.aUp[BufferIndex].pBuffer + _SEGGER_RTT.aUp[BufferIndex].WrOff, pBuffer, NumBytesToWrite);
    NumBytesWritten     += NumBytesToWrite;
    pBuffer             += NumBytesToWrite;
    NumBytes            -= NumBytesToWrite;
    _SEGGER_RTT.aUp[BufferIndex].WrOff += NumBytesToWrite;
    if (_SEGGER_RTT.aUp[BufferIndex].WrOff == _SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer) {
      _SEGGER_RTT.aUp[BufferIndex].WrOff = 0;
    }
  } while (NumBytes);
  SEGGER_RTT_UNLOCK();
  return NumBytesWritten;
}

/*********************************************************************
*
*       SEGGER_RTT_WriteString
*
*  Function description
*    Stores string in SEGGER RTT control block.
*    This data is read by the host.
*
*  Parameters
*    BufferIndex  Index of "Up"-buffer to be used. (e.g. 0 for "Terminal")
*    s            Pointer to string.
*
*  Return values
*    Number of bytes which have been stored in the "Up"-buffer.
*
*  Notes
*    (1) If there is not enough space in the "Up"-buffer, depending on configuration,
*        remaining characters may be dropped or RTT module waits until there is more space in the buffer.
*    (2) String passed to this function has to be \0 terminated
*    (3) \0 termination character is *not* stored in RTT buffer
*/
int SEGGER_RTT_WriteString(unsigned BufferIndex, const char* s) {
  int Len;

  Len = _strlen(s);
  return SEGGER_RTT_Write(BufferIndex, s, Len);
}

/*********************************************************************
*
*       SEGGER_RTT_GetKey
*
*  Function description
*    Reads one character from the SEGGER RTT buffer.
*    Host has previously stored data there.
*
*  Return values
*    <  0    No character available (buffer empty).
*    >= 0    Character which has been read. (Possible values: 0 - 255)
*
*  Notes
*    (1) This function is only specified for accesses to RTT buffer 0.
*/
int SEGGER_RTT_GetKey(void) {
  char c;
  int r;

  r = SEGGER_RTT_Read(0, &c, 1);
  if (r == 1) {
    return (int)(unsigned char)c;
  }
  return -1;
}

/*********************************************************************
*
*       SEGGER_RTT_WaitKey
*
*  Function description
*    Waits until at least one character is avaible in the SEGGER RTT buffer.
*    Once a character is available, it is read and this function returns.
*
*  Return values
*    >=0    Character which has been read.
*
*  Notes
*    (1) This function is only specified for accesses to RTT buffer 0
*    (2) This function is blocking if no character is present in RTT buffer
*/
int SEGGER_RTT_WaitKey(void) {
  int r;

  do {
    r = SEGGER_RTT_GetKey();
  } while (r < 0);
  return r;
}

/*********************************************************************
*
*       SEGGER_RTT_HasKey
*
*  Function description
*    Checks if at least one character for reading is available in the SEGGER RTT buffer.
*
*  Return values
*    0      No characters are available to read.
*    1      At least one character is available.
*
*  Notes
*    (1) This function is only specified for accesses to RTT buffer 0
*/
int SEGGER_RTT_HasKey(void) {
  int RdOff;

  _Init();
  RdOff = _SEGGER_RTT.aDown[0].RdOff;
  if (RdOff != _SEGGER_RTT.aDown[0].WrOff) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       SEGGER_RTT_ConfigUpBuffer
*
*  Function description
*    Run-time configuration of a specific up-buffer (T->H).
*    Buffer to be configured is specified by index.
*    This includes: Buffer address, size, name, flags, ...
*
*  Return value
*    >= 0  O.K.
*     < 0  Error
*/
int SEGGER_RTT_ConfigUpBuffer(unsigned BufferIndex, const char* sName, char* pBuffer, int BufferSize, int Flags) {
  _Init();
  if (BufferIndex < (unsigned)_SEGGER_RTT.MaxNumUpBuffers) {
    SEGGER_RTT_LOCK();
    if (BufferIndex > 0) {
      _SEGGER_RTT.aUp[BufferIndex].sName        = sName;
      _SEGGER_RTT.aUp[BufferIndex].pBuffer      = pBuffer;
      _SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer = BufferSize;
      _SEGGER_RTT.aUp[BufferIndex].RdOff        = 0;
      _SEGGER_RTT.aUp[BufferIndex].WrOff        = 0;
    }
    _SEGGER_RTT.aUp[BufferIndex].Flags          = Flags;
    SEGGER_RTT_UNLOCK();
    return 0;
  }
  return -1;
}

/*********************************************************************
*
*       SEGGER_RTT_ConfigDownBuffer
*
*  Function description
*    Run-time configuration of a specific down-buffer (H->T).
*    Buffer to be configured is specified by index.
*    This includes: Buffer address, size, name, flags, ...
*
*  Return value
*    >= 0  O.K.
*     < 0  Error
*/
int SEGGER_RTT_ConfigDownBuffer(unsigned BufferIndex, const char* sName, char* pBuffer, int BufferSize, int Flags) {
  _Init();
  if (BufferIndex < (unsigned)_SEGGER_RTT.MaxNumDownBuffers) {
    SEGGER_RTT_LOCK();
    if (BufferIndex > 0) {
      _SEGGER_RTT.aDown[BufferIndex].sName        = sName;
      _SEGGER_RTT.aDown[BufferIndex].pBuffer      = pBuffer;
      _SEGGER_RTT.aDown[BufferIndex].SizeOfBuffer = BufferSize;
      _SEGGER_RTT.aDown[BufferIndex].RdOff        = 0;
      _SEGGER_RTT.aDown[BufferIndex].WrOff        = 0;
    }
    _SEGGER_RTT.aDown[BufferIndex].Flags          = Flags;
    SEGGER_RTT_UNLOCK();
    return 0;
  }
  return -1;
}

/*********************************************************************
*
*       SEGGER_RTT_Init
*
*  Function description
*    Initializes the RTT Control Block.
*    Should be used in RAM targets, at start of the application.
*
*/
void SEGGER_RTT_Init (void) {
  _Init();
}

/*********************************************************************
*
*       SEGGER_RTT_SetTerminal
*
*  Function description
*    Sets the terminal to be used for output on channel 0.
*
*/
void SEGGER_RTT_SetTerminal (char TerminalId) {
  char ac[2];

  ac[0] = 0xFF;
  if (TerminalId < 10) {
    ac[1] = '0' + TerminalId;
  } else if (TerminalId < 16) {
    ac[1] = 'A' + (TerminalId - 0x0A);
  } else {
    return; // RTT only supports up to 16 virtual terminals.
  }
  _ActiveTerminal = TerminalId;
  SEGGER_RTT_Write(0, ac, 2);
}

/*********************************************************************
*
*       SEGGER_RTT_TerminalOut
*
*  Function description
*    Writes a string to the given terminal
*     without changing the terminal for channel 0.
*
*/
int SEGGER_RTT_TerminalOut (char TerminalId, const char* s) {
  char ac[2];
  int  r;

  ac[0] = 0xFF;
  if (TerminalId < 10) {
    ac[1] = '0' + TerminalId;
  } else if (TerminalId < 16) {
    ac[1] = 'A' + (TerminalId - 0x0A);
  } else {
    return -1; // RTT only supports up to 16 virtual terminals.
  }
  SEGGER_RTT_Write(0, ac, 2);
  r = SEGGER_RTT_WriteString(0, s);
  if (TerminalId < 10) {
    ac[1] = '0' + _ActiveTerminal;
  } else if (TerminalId < 16) {
    ac[1] = 'A' + (_ActiveTerminal - 0x0A);
  }
  SEGGER_RTT_Write(0, ac, 2);
  return r;
}

/*************************** End of file ****************************/




/*********************************************************************
*              SEGGER MICROCONTROLLER GmbH & Co. KG                  *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2014-2014 SEGGER Microcontroller GmbH & Co. KG          *
*                                                                    *
*       Internet: www.segger.com Support: support@segger.com         *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT_printf.c
Date    : 17 Dec 2014
Purpose : Replacement for printf to write formatted data via RTT
---------------------------END-OF-HEADER------------------------------
*/





/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef SEGGER_RTT_PRINTF_BUFFER_SIZE
  #define SEGGER_RTT_PRINTF_BUFFER_SIZE (64)
#endif

#include <stdlib.h>
#include <stdarg.h>


#define FORMAT_FLAG_LEFT_JUSTIFY   (1 << 0)
#define FORMAT_FLAG_PAD_ZERO       (1 << 1)
#define FORMAT_FLAG_PRINT_SIGN     (1 << 2)
#define FORMAT_FLAG_ALTERNATE      (1 << 3)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  char* pBuffer;
  int   BufferSize;
  int   Cnt;

  int   ReturnValue;

  unsigned RTTBufferIndex;
} SEGGER_RTT_PRINTF_DESC;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _StoreChar
*/
static void _StoreChar(SEGGER_RTT_PRINTF_DESC * p, char c) {
  int Cnt;

  Cnt = p->Cnt;
  if ((Cnt + 1) <= p->BufferSize) {
    *(p->pBuffer + Cnt) = c;
    p->Cnt = Cnt + 1;
    p->ReturnValue++;
  }
  //
  // Write part of string, when the buffer is full
  //
  if (p->Cnt == p->BufferSize) {
    if (SEGGER_RTT_Write(p->RTTBufferIndex, p->pBuffer, p->Cnt) != p->Cnt) {
      p->ReturnValue = -1;
    } else {
      p->Cnt = 0;
    }
  }
}

/*********************************************************************
*
*       _PrintUnsigned
*/
static void _PrintUnsigned(SEGGER_RTT_PRINTF_DESC * pBufferDesc, unsigned v, unsigned Base, int NumDigits, unsigned FieldWidth, unsigned FormatFlags) {
  static const char _aV2C[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  unsigned Div;
  unsigned Digit = 1;
  unsigned Number;
  unsigned Width;
  char c;

  Number = v;

  //
  // Get actual field width
  //
  Width = 1;
  while (Number >= Base) {
    Number = (Number / Base);
    Width++;
  }
  if ((unsigned)NumDigits > Width) {
    Width = NumDigits;
  }
  //
  // Print leading chars if necessary
  //
  if ((FormatFlags & FORMAT_FLAG_LEFT_JUSTIFY) == 0) {
    if (FieldWidth != 0) {
      if (((FormatFlags & FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO) && (NumDigits == 0)) {
        c = '0';
      } else {
        c = ' ';
      }
      while ((FieldWidth != 0) && (Width < FieldWidth--)) {
        _StoreChar(pBufferDesc, c);
        if (pBufferDesc->ReturnValue < 0) {
          return;
        }
      }
    }
  }
  //
  // Count how many digits are required by precision
  //
  while (((v / Digit) >= Base) | (NumDigits-- > 1)) {
    Digit *= Base;
  }
  //
  // Output digits
  //
  do {
    Div = v / Digit;
    v -= Div * Digit;
    _StoreChar(pBufferDesc, _aV2C[Div]);
    if (pBufferDesc->ReturnValue < 0) {
      break;
    }
    Digit /= Base;
  } while (Digit);
  //
  // Print trailing spaces if necessary
  //
  if ((FormatFlags & FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY) {
    if (FieldWidth != 0) {
      while ((FieldWidth != 0) && (Width < FieldWidth--)) {
        _StoreChar(pBufferDesc, ' ');
        if (pBufferDesc->ReturnValue < 0) {
          return;
        }
      }
    }
  }
}

/*********************************************************************
*
*       _PrintInt
*/
static void _PrintInt(SEGGER_RTT_PRINTF_DESC * pBufferDesc, int v, unsigned Base, unsigned NumDigits, unsigned FieldWidth, unsigned FormatFlags) {
  unsigned Width;
  unsigned Number;

  Number = (v < 0) ? -v : v;

  //
  // Get actual field width
  //
  Width = 1;
  while (Number >= Base) {
    Number = (Number / Base);
    Width++;
  }
  if (NumDigits > Width) {
    Width = NumDigits;
  }
  if ((FieldWidth > 0) && ((v < 0) || ((FormatFlags & FORMAT_FLAG_PRINT_SIGN) == FORMAT_FLAG_PRINT_SIGN))) {
    FieldWidth--;
  }

  //
  // Print leading spaces if necessary
  //
  if ((((FormatFlags & FORMAT_FLAG_PAD_ZERO) == 0) || (NumDigits != 0)) && ((FormatFlags & FORMAT_FLAG_LEFT_JUSTIFY) == 0)) {
    if (FieldWidth != 0) {
      while ((FieldWidth != 0) && (Width < FieldWidth--)) {
        _StoreChar(pBufferDesc, ' ');
        if (pBufferDesc->ReturnValue < 0) {
          return;
        }
      }
    }
  }
  //
  // Print sign if necessary
  //
  if (v < 0) {
    v = -v;
    _StoreChar(pBufferDesc, '-');
    if (pBufferDesc->ReturnValue < 0) {
      return;
    }
  } else if ((FormatFlags & FORMAT_FLAG_PRINT_SIGN) == FORMAT_FLAG_PRINT_SIGN) {
    _StoreChar(pBufferDesc, '+');
    if (pBufferDesc->ReturnValue < 0) {
      return;
    }
  }
  //
  // Print leading zeros if necessary
  //
  if (((FormatFlags & FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO) && ((FormatFlags & FORMAT_FLAG_LEFT_JUSTIFY) == 0) && (NumDigits == 0)) {
    if (FieldWidth != 0) {
      while ((FieldWidth != 0) && (Width < FieldWidth--)) {
        _StoreChar(pBufferDesc, '0');
        if (pBufferDesc->ReturnValue < 0) {
          return;
        }
      }
    }
  }

  //
  // Print number without sign
  //
  _PrintUnsigned(pBufferDesc, v, Base, NumDigits, FieldWidth, FormatFlags);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SEGGER_RTT_vprintf
*
*  Function description
*    Stores a formatted string in SEGGER RTT control block.
*    This data is read by the host.
*
*  Parameters
*    BufferIndex  Index of "Up"-buffer to be used. (e.g. 0 for "Terminal")
*    sFormat      Pointer to format string
*    pParamList   Pointer to the list of arguments for the format string
*
*  Return values
*    >= 0:  Number of bytes which have been stored in the "Up"-buffer.
*     < 0:  Error
*/
int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList) {
  char c;
  SEGGER_RTT_PRINTF_DESC BufferDesc;
  int v;
  unsigned NumDigits;
  unsigned FormatFlags;
  unsigned FieldWidth;
  char acBuffer[SEGGER_RTT_PRINTF_BUFFER_SIZE];

  BufferDesc.pBuffer        = acBuffer;
  BufferDesc.BufferSize     = SEGGER_RTT_PRINTF_BUFFER_SIZE;
  BufferDesc.Cnt            = 0;
  BufferDesc.RTTBufferIndex = BufferIndex;
  BufferDesc.ReturnValue    = 0;

  do {
    c = *sFormat++;
    if (c == 0) {
      break;
    }
    if (c == '%') {
      //
      // Filter out flags
      //
      FormatFlags = 0;
      do {
        c = *sFormat;
        switch (c) {
        case '-': FormatFlags |= FORMAT_FLAG_LEFT_JUSTIFY; sFormat++; break;
        case '0': FormatFlags |= FORMAT_FLAG_PAD_ZERO;     sFormat++; break;
        case '+': FormatFlags |= FORMAT_FLAG_PRINT_SIGN;   sFormat++; break;
        case '#': FormatFlags |= FORMAT_FLAG_ALTERNATE;    sFormat++; break;
        default:  goto FilterFieldWidth;                   break;
        }
      } while (1);
      //
      // filter out field with
      //
FilterFieldWidth:
      FieldWidth = 0;
      do {
        c = *sFormat;
        if (c < '0' || c > '9') {
          break;
        }
        sFormat++;
        FieldWidth = FieldWidth * 10 + (c - '0');
      } while (1);

      //
      // Filter out precision (number of digits to display)
      //
      NumDigits = 0;
      c = *sFormat;
      if (c == '.') {
        sFormat++;
        do {
          c = *sFormat;
          if (c < '0' || c > '9') {
            break;
          }
          sFormat++;
          NumDigits = NumDigits * 10 + (c - '0');
        } while (1);
      }
      //
      // Filter out length modifier
      //
      c = *sFormat;
      do {
        if (c == 'l' || c == 'h') {
          c = *sFormat++;
          continue;
        }
        break;
      } while (1);
      //
      // Handle specifiers
      //
      switch (c) {
      case 'c': {
        char c0;
        v = va_arg(*pParamList, int);
        c0 = (char)v;
        _StoreChar(&BufferDesc, c0);
        break;
      }
      case 'd':
        v = va_arg(*pParamList, int);
        _PrintInt(&BufferDesc, v, 10, NumDigits, FieldWidth, FormatFlags);
        break;
      case 'u':
        v = va_arg(*pParamList, int);
        _PrintUnsigned(&BufferDesc, v, 10, NumDigits, FieldWidth, FormatFlags);
        break;
      case 'x':
      case 'X':
        v = va_arg(*pParamList, int);
        _PrintUnsigned(&BufferDesc, v, 16, NumDigits, FieldWidth, FormatFlags);
        break;
      case 's':
        {
          const char * s = va_arg(*pParamList, const char *);
          do {
            c = *s++;
            if (c == 0) {
              break;
            }
           _StoreChar(&BufferDesc, c);
          } while (BufferDesc.ReturnValue >= 0);
        }
        break;
      case 'p':
        v = va_arg(*pParamList, int);
        _PrintUnsigned(&BufferDesc, v, 16, 8, 8, 0);
        break;
      case '%':
        _StoreChar(&BufferDesc, '%');
        break;
      }
      sFormat++;
    } else {
      _StoreChar(&BufferDesc, c);
    }
  } while (BufferDesc.ReturnValue >= 0);

  if (BufferDesc.ReturnValue > 0) {
    //
    // Write remaining data, if any
    //
    if (BufferDesc.Cnt != 0) {
      SEGGER_RTT_Write(BufferIndex, acBuffer, BufferDesc.Cnt);
    }
    BufferDesc.ReturnValue += BufferDesc.Cnt;
  }
  return BufferDesc.ReturnValue;
}

/*********************************************************************
*
*       SEGGER_RTT_printf
*
*  Function description
*    Stores a formatted string in SEGGER RTT control block.
*    This data is read by the host.
*
*  Parameters
*    BufferIndex  Index of "Up"-buffer to be used. (e.g. 0 for "Terminal")
*    sFormat      Pointer to format string, followed by the arguments for conversion
*
*  Return values
*    >= 0:  Number of bytes which have been stored in the "Up"-buffer.
*     < 0:  Error
*
*  Notes
*    (1) Conversion specifications have following syntax:
*          %[flags][FieldWidth][.Precision]ConversionSpecifier
*    (2) Supported flags:
*          -: Left justify within the field width
*          +: Always print sign extension for signed conversions
*          0: Pad with 0 instead of spaces. Ignored when using '-'-flag or precision
*        Supported conversion specifiers:
*          c: Print the argument as one char
*          d: Print the argument as a signed integer
*          u: Print the argument as an unsigned integer
*          x: Print the argument as an hexadecimal integer
*          s: Print the string pointed to by the argument
*          p: Print the argument as an 8-digit hexadecimal integer. (Argument shall be a pointer to void.)
*/
int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...) {
  va_list ParamList;

  va_start(ParamList, sFormat);
  return SEGGER_RTT_vprintf(BufferIndex, sFormat, &ParamList);
}
/*************************** End of file ****************************/


#endif

}
