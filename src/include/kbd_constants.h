/**
* \file kbd_constants.h
* Our own internal functions for keys on the keyboard, used by OS and tool to
* create memory image.
*/

#ifndef __KBD_CONSTANTS_H
#define __KBD_CONSTANTS_H


// First row
#define KBD_EVENT_ESC         0x01
#define KBD_EVENT_F1          0x02
#define KBD_EVENT_F2          0x03
#define KBD_EVENT_F3          0x04
#define KBD_EVENT_F4          0x05
#define KBD_EVENT_F5          0x06
#define KBD_EVENT_F6          0x07
#define KBD_EVENT_F7          0x08
#define KBD_EVENT_F8          0x09
#define KBD_EVENT_F9          0x0a
#define KBD_EVENT_F10         0x0b
#define KBD_EVENT_F11         0x0c
#define KBD_EVENT_F12         0x0d
#define KBD_EVENT_PRNTSCRN    0x0e
#define KBD_EVENT_SCROLL      0x0f
#define KBD_EVENT_PAUSE       0x10

// Second row
#define KBD_EVENT_BTICK     0x21
#define KBD_EVENT_1         0x22
#define KBD_EVENT_2         0x23
#define KBD_EVENT_3         0x24
#define KBD_EVENT_4         0x25
#define KBD_EVENT_5         0x26
#define KBD_EVENT_6         0x27
#define KBD_EVENT_7         0x28
#define KBD_EVENT_8         0x29
#define KBD_EVENT_9         0x2a
#define KBD_EVENT_0         0x2b
#define KBD_EVENT_DASH      0x2c
#define KBD_EVENT_EQUAL     0x2d
#define KBD_EVENT_BACKSPACE 0x2e
#define KBD_EVENT_INSERT    0x2f
#define KBD_EVENT_HOME      0x30
#define KBD_EVENT_PGUP      0x31
#define KBD_EVENT_NUM       0x32
#define KBD_EVENT_KP/       0x33
#define KBD_EVENT_KP*       0x34
#define KBD_EVENT_KP-       0x35


// Third row
#define KBD_EVENT_TAB       0x41
#define KBD_EVENT_Q         0x42
#define KBD_EVENT_W         0x43
#define KBD_EVENT_E         0x44
#define KBD_EVENT_R         0x45
#define KBD_EVENT_T         0x46
#define KBD_EVENT_Y         0x47
#define KBD_EVENT_U         0x48
#define KBD_EVENT_I         0x49
#define KBD_EVENT_O         0x4a
#define KBD_EVENT_P         0x4b
#define KBD_EVENT_P         0x4c
#define KBD_EVENT_SQOPN     0x4d
#define KBD_EVENT_SQCLS     0x4e
#define KBD_EVENT_BACKSLASH 0x4f
#define KBD_EVENT_DELETE    0x50
#define KBD_EVENT_END       0x51
#define KBD_EVENT_PGDN      0x52
#define KBD_EVENT_KP7       0x53
#define KBD_EVENT_KP8       0x54
#define KBD_EVENT_KP9       0x55
#define KBD_EVENT_KP+       0x56




// Fourth row
#define KBD_EVENT_CAPS      0x61
#define KBD_EVENT_A         0x62
#define KBD_EVENT_S         0x63
#define KBD_EVENT_D         0x64
#define KBD_EVENT_F         0x65
#define KBD_EVENT_G         0x66
#define KBD_EVENT_H         0x67
#define KBD_EVENT_J         0x68
#define KBD_EVENT_K         0x69
#define KBD_EVENT_L         0x6a
#define KBD_EVENT_SCOLON    0x6b
#define KBD_EVENT_SQUOTE    0x6c
#define KBD_EVENT_ENTER     0x6d
#define KBD_EVENT_KP4       0x6e
#define KBD_EVENT_KP5       0x6f
#define KBD_EVENT_KP6       0x70


// Fifth row

#define KBD_EVENT_LSHIFT    0x81
#define KBD_EVENT_Z         0x82
#define KBD_EVENT_X         0x83
#define KBD_EVENT_C         0x84
#define KBD_EVENT_V         0x85
#define KBD_EVENT_B         0x86
#define KBD_EVENT_N         0x87
#define KBD_EVENT_M         0x88
#define KBD_EVENT_COMMA     0x89
#define KBD_EVENT_DOT       0x8a
#define KBD_EVENT_SLASH     0x8b
#define KBD_EVENT_RSHIFT    0x8c
#define KBD_EVENT_UARROW    0x8d
#define KBD_EVENT_KP1       0x8e
#define KBD_EVENT_KP2       0x8f
#define KBD_EVENT_KP3       0x90
#define KBD_EVENT_KPEN      0x91



// Sixth row
#define KBD_EVENT_LCTRL     0xA1
#define KBD_EVENT_LGUI      0xA2
#define KBD_EVENT_LALT      0xA3
#define KBD_EVENT_SPACE     0xA4
#define KBD_EVENT_RALT      0xA5
#define KBD_EVENT_RGUI      0xA6
#define KBD_EVENT_APPS      0xA7
#define KBD_EVENT_RCTRL     0xA8
#define KBD_EVENT_LARROW    0xA9
#define KBD_EVENT_DARROW    0xAA
#define KBD_EVENT_RARROW    0xAB
#define KBD_EVENT_KP0       0xAC
#define KBD_EVENT_KP.       0xAD



// Other stuff
#define KBD_EVENT_POWER       0xC1
#define KBD_EVENT_SLEEP       0xC2
#define KBD_EVENT_WAKE        0xC3
#define KBD_EVENT_NTRACK      0xC4
#define KBD_EVENT_PREVTRACK   0xC5
#define KBD_EVENT_STOPMM      0xC6
#define KBD_EVENT_PLAYPAUSE   0xC7
#define KBD_EVENT_MUTE        0xC8
#define KBD_EVENT_VOLUP       0xC9
#define KBD_EVENT_VOLDOWN     0xCA
#define KBD_EVENT_MEDIASEL    0xCB
#define KBD_EVENT_EMAIL       0xCC
#define KBD_EVENT_CALC        0xCD
#define KBD_EVENT_MYCOMP      0xCE
#define KBD_EVENT_WWWSEARCH   0xCF
#define KBD_EVENT_WWWHOME     0xD0
#define KBD_EVENT_WWWBACK     0xD1
#define KBD_EVENT_WWWFRWD     0xD2
#define KBD_EVENT_WWWSTOP     0xD3
#define KBD_EVENT_WWWREFRESH  0xD4
#define KBD_EVENT_WWWFAVOR    0xD5


#endif
