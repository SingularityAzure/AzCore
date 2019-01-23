/*
    File: keycodes.cpp
    Author: Philip Haynes
*/

#include "keycodes.hpp"

const char *keyCodeNames[256] = {
    "None",             "Error_Roll_Over",  "NULL",             "NULL",             // 0x00
    "A",                "B",                "C",                "D",                // 0x04
    "E",                "F",                "G",                "H",                // 0x08
    "I",                "J",                "K",                "L",                // 0x0c
    "M",                "N",                "O",                "P",                // 0x10
    "Q",                "R",                "S",                "T",                // 0x14
    "U",                "V",                "W",                "X",                // 0x18
    "Y",                "Z",                "1",                "2",                // 0x1c
    "3",                "4",                "5",                "6",                // 0x20
    "7",                "8",                "9",                "0",                // 0x24
    "Enter",            "Escape",           "Backspace",        "Tab",              // 0x28
    "Space",            "Minus",            "Equals",           "Left_Bracket",     // 0x2c
    "Right_Bracket",    "Backslash",        "NULL",             "Semicolon",        // 0x30
    "Quote",            "Grave",            "Comma",            "Period",           // 0x34
    "Slash",            "CapsLock",         "F1",               "F2",               // 0x38
    "F3",               "F4",               "F5",               "F6",               // 0x3c
    "F7",               "F8",               "F9",               "F10",              // 0x40
    "F11",              "F12",              "PrintScreen",      "ScrollLock",       // 0x44
    "Pause",            "Insert",           "Home",             "PageUp",           // 0x48
    "Delete",           "End",              "Page_Down",        "Right_Arrow",      // 0x4c
    "Left_Arrow",       "Down_Arrow",       "Up_Arrow",         "KP_NumLock",       // 0x50
    "KP_Divide",        "KP_Multiply",      "KP_Subtract",      "KP_Add",           // 0x54
    "KP_Enter",         "KP_1",             "KP_2",             "KP_3",             // 0x58
    "KP_4",             "KP_5",             "KP_6",             "KP_7",             // 0x5c
    "KP_8",             "KP_9",             "KP_0",             "KP_Point",         // 0x60
    "Non-US_Backslash", "Menu",             "Power",            "KP_Equals",        // 0x64
    "F13",              "F14",              "F15",              "F16",              // 0x68
    "F17",              "F18",              "F19",              "F20",              // 0x6c
    "F21",              "F22",              "F23",              "F24",              // 0x70
    "Execute",          "Help",             "Properties",       "Select",           // 0x74
    "Stop",             "Again",            "Undo",             "Cut",              // 0x78
    "Copy",             "Paste",            "Find",             "Mute",             // 0x7c
    "Volume_Up",        "Volume_Down",      "NULL",             "NULL",             // 0x80
    "NULL",             "KP_Comma",         "NULL",             "International1",   // 0x84
    "International2",   "International3",   "International4",   "International5",   // 0x88
    "International6",   "International7",   "International8",   "International9",   // 0x8c
    "LANG1",            "LANG2",            "LANG3",            "LANG4",            // 0x90
    "LANG5",            "LANG6",            "LANG7",            "LANG8",            // 0x94
    "LANG9",            "NULL",             "NULL",             "NULL",             // 0x98
    "NULL",             "NULL",             "NULL",             "NULL",             // 0x9c
    "NULL",             "NULL",             "NULL",             "NULL",             // 0xa0
    "NULL",             "NULL",             "MB_Left",          "MB_Right",         // 0xa4
    "MB_Middle",        "MB_X1",            "MB_X2",            "Scroll_Up",        // 0xa8
    "Scroll_Down",      "Scroll_Left",      "Scroll_Right",     "NULL",             // 0xac
    "NULL",             "NULL",             "NULL",             "NULL",             // 0xb0
    "NULL",             "NULL",             "KP_LeftParen",     "KP_RightParen",    // 0xb4
    "A_Button",         "B_Button",         "C_Button",         "X_Button",         // 0xb8
    "Y_Button",         "Z_Button",         "LB",               "RB",               // 0xbc
    "LB_2",             "RB_2",             "Select",           "Start",            // 0xc0
    "Mode",             "LS_Press",         "RS_Press",         "LS_Right",         // 0xc4
    "LS_Left",          "LS_Down",          "LS_Up",            "LT_In",            // 0xc8
    "NULL",             "RS_Right",         "RS_Left",          "RS_Down",          // 0xcc
    "RS_Up",            "RT_In",            "NULL",             "H0_Right",         // 0xd0
    "H0_Left",          "H0_Down",          "H0_Up",            "Axis_LS_X",        // 0xd4
    "Axis_LS_Y",        "Axis_LT",          "Axis_RS_X",        "Axis_RS_Y",        // 0xd8
    "Axis_RT",          "Axis_H0_X",        "Axis_H0_Y",        "NULL",             // 0xdc
    "Left_Control",     "Left_Shift",       "Left_Alt",         "Left_GUI",         // 0xe0
    "Right_Control",    "Right_Shift",      "Right_Alt",        "Right_GUI",        // 0xe4
    "Media_Play/Pause", "Media_Stop_CD",    "Media_Previous",   "Media_Next",       // 0xe8
    "Media_Eject",      "Media_Volume_Up",  "Media_Volume_Down","Media_Mute",       // 0xec
    "Media_WWW",        "Media_Back",       "Media_Forward",    "Media_Stop",       // 0xf0
    "Media_Find",       "Media_Scroll_Up",  "Media_Scroll_Down","Media_Edit",       // 0xf4
    "Media_Sleep",      "Media_Coffee",     "Media_Refresh",    "Media_Calculator", // 0xf8
    "Media_Mail",       "Media_File",       "NULL",             "NULL"              // 0xfc
};

const u8 mapKeyCodeToEvdev[256] = {
0xff,   0xff,   0xff,   0xff,   0x26,   0x38,   0x36,   0x28,   // 0x00
0x1a,   0x29,   0x2a,   0x2b,   0x1f,   0x2c,   0x2d,   0x2e,   // 0x08
0x3a,   0x39,   0x20,   0x21,   0x18,   0x1b,   0x27,   0x1c,   // 0x10
0x1e,   0x37,   0x19,   0x35,   0x1d,   0x34,   0x0a,   0x0b,   // 0x18
0x0c,   0x0d,   0x0e,   0x0f,   0x10,   0x11,   0x12,   0x13,   // 0x20
0x24,   0x09,   0x16,   0x17,   0x41,   0x14,   0x15,   0x22,   // 0x28
0x23,   0x33,   0xff,   0x2f,   0x30,   0x31,   0x3b,   0x3c,   // 0x30
0x3d,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,   0x48,   // 0x38
0x49,   0x4a,   0x4b,   0x4c,   0x5f,   0x60,   0x6b,   0x4e,   // 0x40
0x7f,   0x76,   0x6e,   0x70,   0x77,   0x73,   0x75,   0x72,   // 0x48
0x71,   0x74,   0x6f,   0x4d,   0x6a,   0x3f,   0x52,   0x56,   // 0x50
0x68,   0x57,   0x58,   0x59,   0x53,   0x54,   0x55,   0x4f,   // 0x58
0x50,   0x51,   0x5a,   0x5b,   0x5e,   0x87,   0x7c,   0x7d,   // 0x60
0xbf,   0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   // 0x68
0xc7,   0xc8,   0xc9,   0xca,   0x8e,   0x92,   0x8a,   0x8c,   // 0x70
0x88,   0x89,   0x8b,   0x91,   0x8d,   0x8f,   0x90,   0x79,   // 0x78
0x7b,   0x7a,   0xff,   0xff,   0xff,   0x81,   0xff,   0x61,   // 0x80
0x65,   0x84,   0x64,   0x66,   0x67,   0xff,   0xff,   0xff,   // 0x88
0x82,   0x83,   0x62,   0x63,   0x5d,   0xff,   0xff,   0xff,   // 0x90
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0x98
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xa0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xa8
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xbb,   0xbc,   // 0xb0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xb8
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xc0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xc8
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xd0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xd8
0x25,   0x32,   0x40,   0x85,   0x69,   0x3e,   0x6c,   0x86,   // 0xe0
0xac,   0xae,   0xad,   0xab,   0xa9,   0x7b,   0x7a,   0x79,   // 0xe8
0xb4,   0xa6,   0xa7,   0x88,   0x90,   0xb9,   0xba,   0xb8,   // 0xf0
0x96,   0xa0,   0xb5,   0x94,   0xa3,   0x98,   0xff,   0xff    // 0xf8
};

const u8 mapKeyCodeFromEvdev[256] = {
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0x00
0xff,   0x29,   0x1e,   0x1f,   0x20,   0x21,   0x22,   0x23,   // 0x08
0x24,   0x25,   0x26,   0x27,   0x2d,   0x2e,   0x2a,   0x2b,   // 0x10
0x14,   0x1a,   0x08,   0x15,   0x17,   0x1c,   0x18,   0x0c,   // 0x18
0x12,   0x13,   0x2f,   0x30,   0x28,   0xe0,   0x04,   0x16,   // 0x20
0x07,   0x09,   0x0a,   0x0b,   0x0d,   0x0e,   0x0f,   0x33,   // 0x28
0x34,   0x35,   0xe1,   0x31,   0x1d,   0x1b,   0x06,   0x19,   // 0x30
0x05,   0x11,   0x10,   0x36,   0x37,   0x38,   0xe5,   0x55,   // 0x38
0xe2,   0x2c,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   // 0x40
0x3f,   0x40,   0x41,   0x42,   0x43,   0x53,   0x47,   0x5f,   // 0x48
0x60,   0x61,   0x56,   0x5c,   0x5d,   0x5e,   0x57,   0x59,   // 0x50
0x5a,   0x5b,   0x62,   0x63,   0xff,   0x94,   0x64,   0x44,   // 0x58
0x45,   0x87,   0x92,   0x93,   0x8a,   0x88,   0x8b,   0x8c,   // 0x60
0x58,   0xe4,   0x54,   0x46,   0xe6,   0xff,   0x4a,   0x52,   // 0x68
0x4b,   0x50,   0x4f,   0x4d,   0x51,   0x4e,   0x49,   0x4c,   // 0x70
0xff,   0x7f,   0x81,   0x80,   0x66,   0x67,   0xff,   0x48,   // 0x78
0xff,   0x85,   0x90,   0x91,   0x89,   0xe3,   0xe7,   0x65,   // 0x80
0x78,   0x79,   0x76,   0x7a,   0x77,   0x7c,   0x74,   0x7d,   // 0x88
0x7e,   0x7b,   0x75,   0xff,   0xfb,   0xff,   0xf8,   0xff,   // 0x90
0xfd,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0x98
0xf9,   0xff,   0xff,   0xfc,   0xff,   0xff,   0xf1,   0xf2,   // 0xa0
0xff,   0xec,   0xff,   0xeb,   0xe8,   0xea,   0xe9,   0xff,   // 0xa8
0xff,   0xff,   0xff,   0xff,   0xf0,   0xfa,   0xff,   0xff,   // 0xb0
0xf7,   0xf5,   0xf6,   0xb6,   0xb7,   0xff,   0xff,   0x68,   // 0xb8
0x69,   0x6a,   0x6b,   0x6c,   0x6d,   0x6e,   0x6f,   0x70,   // 0xc0
0x71,   0x72,   0x73,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xc8
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xd0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xd8
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xe0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xe8
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   // 0xf0
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0x00    // 0xf8
};

// TODO: The Windows mappings are incomplete

const u8 mapKeyCodeToWinVK[256] = {
0xff,   0xff,   0xff,   0xff,   0x41,   0x42,   0x43,   0x44,
0x45,   0x46,   0x47,   0x48,   0x49,   0x4a,   0x4b,   0x4c,
0x4d,   0x4e,   0x4f,   0x50,   0x51,   0x52,   0x53,   0x54,
0x55,   0x56,   0x57,   0x58,   0x59,   0x5a,   0x31,   0x32,
0x33,   0x34,   0x35,   0x36,   0x37,   0x38,   0x39,   0x30,
0x0d,   0x1b,   0x08,   0x09,   0x20,   0xbd,   0xbb,   0xdb,
0xdd,   0xdc,   0xff,   0xba,   0xde,   0xc0,   0xbc,   0xbe,
0xbf,   0x14,   0x70,   0x71,   0x72,   0x73,   0x74,   0x75,
0x76,   0x77,   0x78,   0x79,   0x7a,   0x7b,   0x2c,   0x91,
0x13,   0x2d,   0x24,   0x21,   0x2e,   0x23,   0x22,   0x27,
0x25,   0x28,   0x26,   0x90,   0x6f,   0x6a,   0x6d,   0x6b,
0xff,   0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,
0x68,   0x69,   0x60,   0x6e,   0xff,   0xff,   0xff,   0xff,
0x7c,   0x7d,   0x7e,   0x7f,   0x80,   0x81,   0x82,   0x83,
0x84,   0x85,   0x86,   0x87,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0x11,   0x10,   0x12,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
};

const u8 mapKeyCodeFromWinVK[256] = {
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0x2a,   0x2b,   0xff,   0xff,   0xff,   0x28,   0xff,   0xff,
0xe1,   0xe0,   0xe2,   0x48,   0x39,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0x29,   0xff,   0xff,   0xff,   0xff,
0x2c,   0x4b,   0x4e,   0x4d,   0x4a,   0x50,   0x52,   0x4f,
0x51,   0xff,   0xff,   0xff,   0x46,   0x49,   0x4c,   0xff,
0x27,   0x1e,   0x1f,   0x20,   0x21,   0x22,   0x23,   0x24,
0x25,   0x26,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0x04,   0x05,   0x06,   0x07,   0x08,   0x09,   0x0a,
0x0b,   0x0c,   0x0d,   0x0e,   0x0f,   0x10,   0x11,   0x12,
0x13,   0x14,   0x15,   0x16,   0x17,   0x18,   0x19,   0x1a,
0x1b,   0x1c,   0x1d,   0xff,   0xff,   0xff,   0xff,   0xff,
0x62,   0x59,   0x5a,   0x5b,   0x5c,   0x5d,   0x5e,   0x5f,
0x60,   0x61,   0x55,   0x57,   0xff,   0x56,   0x63,   0x54,
0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,   0x40,   0x41,
0x42,   0x43,   0x44,   0x45,   0x68,   0x69,   0x6a,   0x6b,
0x6c,   0x6d,   0x6e,   0x6f,   0x70,   0x71,   0x72,   0x73,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0x53,   0x47,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0x33,   0x2e,   0x36,   0x2d,   0x37,   0x38,
0x35,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0x2f,   0x31,   0x30,   0x34,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0x00
};

const u8 mapKeyCodeToWinScan[256] = {
0xff,   0xff,   0xff,   0xff,   0x1e,   0x30,   0x2e,   0x20,
0x12,   0x21,   0x22,   0x23,   0x17,   0x24,   0x25,   0x26,
0x32,   0x31,   0x18,   0x19,   0x10,   0x13,   0x1f,   0x14,
0x16,   0x2f,   0x11,   0x2d,   0x15,   0x2c,   0x02,   0x03,
0x04,   0x05,   0x06,   0x07,   0x08,   0x09,   0x0a,   0x0b,
0x1c,   0x01,   0x0e,   0x0f,   0x39,   0x0c,   0x0d,   0x1a,
0x1b,   0x2b,   0xff,   0x27,   0x28,   0x29,   0x33,   0x34,
0x35,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,   0x40,
0x41,   0x42,   0x43,   0x44,   0x57,   0x58,   0x37,   0x46,
0x45,   0x52,   0x47,   0x49,   0x53,   0x4f,   0x51,   0x4d,
0x4b,   0x50,   0x48,   0x45,   0x35,   0x37,   0x4a,   0x4e,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0x53,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0x1d,   0x2a,   0x38,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
};

const u8 mapKeyCodeFromWinScan[256] = {
0xff,   0x29,   0x1e,   0x1f,   0x20,   0x21,   0x22,   0x23,
0x24,   0x25,   0x26,   0x27,   0x2d,   0x2e,   0x2a,   0x2b,
0x14,   0x1a,   0x08,   0x15,   0x17,   0x1c,   0x18,   0x0c,
0x12,   0x13,   0x2f,   0x30,   0x28,   0xe0,   0x04,   0x16,
0x07,   0x09,   0x0a,   0x0b,   0x0d,   0x0e,   0x0f,   0x33,
0x34,   0x35,   0xe1,   0x31,   0x1d,   0x1b,   0x06,   0x19,
0x05,   0x11,   0x10,   0x36,   0x37,   0x38,   0xff,   0x46,
0xe2,   0x2c,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,
0x3f,   0x40,   0x41,   0x42,   0x43,   0x48,   0x47,   0x4a,
0x52,   0x4b,   0x56,   0x50,   0xff,   0x4f,   0x57,   0x4d,
0x51,   0x4e,   0x49,   0x4c,   0xff,   0xff,   0xff,   0x44,
0x45,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0x00
};

const char* KeyCodeName(u8 keyCode) {
    return keyCodeNames[keyCode];
}

u8 KeyCodeToEvdev(u8 keyCode) {
    return mapKeyCodeToEvdev[keyCode];
}

u8 KeyCodeFromEvdev(u8 keyCode) {
    return mapKeyCodeFromEvdev[keyCode];
}

u8 KeyCodeToWinVK(u8 keyCode) {
    return mapKeyCodeToWinVK[keyCode];
}

u8 KeyCodeFromWinVK(u8 keyCode) {
    return mapKeyCodeFromWinVK[keyCode];
}

u8 KeyCodeToWinScan(u8 keyCode) {
    return mapKeyCodeToWinScan[keyCode];
}

u8 KeyCodeFromWinScan(u8 keyCode) {
    return mapKeyCodeFromWinScan[keyCode];
}