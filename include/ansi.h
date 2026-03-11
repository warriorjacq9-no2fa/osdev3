#ifndef ANSI_H
#define ANSI_H

// CS0

#define ANSI_BEL    "\a"
#define ANSI_BS     "\b"
#define ANSI_HT     "\t"
#define ANSI_LF     "\n"
#define ANSI_FF     "\f"
#define ANSI_CR     "\r"
#define ANSI_ESC    "\e"

// CS1

#define ANSI_SS2    ANSI_ESC "N"
#define ANSI_SS3    ANSI_ESC "O"
#define ANSI_DCS    ANSI_ESC "P"
#define ANSI_CSI    ANSI_ESC "["
#define ANSI_ST     ANSI_ESC "\\"
#define ANSI_OSC    ANSI_ESC "]"
#define ANSI_SOS    ANSI_ESC "X"
#define ANSI_PM     ANSI_ESC "^"
#define ANSI_APC    ANSI_ESC "_"

// CSI

#define ANSI_CUU \
    ANSI_CSI "%d" "A"
#define ANSI_CUD \
    ANSI_CSI "%d" "B"
#define ANSI_CUF \
    ANSI_CSI "%d" "C"
#define ANSI_CUB \
    ANSI_CSI "%d" "D"
#define ANSI_CNL \
    ANSI_CSI "%d" "E"
#define ANSI_CPL \
    ANSI_CSI "%d" "F"
#define ANSI_CHA \
    ANSI_CHA "%d" "G"
#define ANSI_CUP \
    ANSI_CSI "%d;%d" "H"
#define ANSI_ED \
    ANSI_CSI "%d" "J"
#define ANSI_EL \
    ANSI_CSI "%d" "K"
#define ANSI_SU \
    ANSI_CSI "%d" "S"
#define ANSI_SD \
    ANSI_CSI "%d" "T"
#define ANSI_HVP \
    ANSI_CSI "%d;%d" "f"
#define ANSI_SGR \
    ANSI_CSI "%d;%d" "m"
#define ANSI_DSR \
    ANSI_CSI "6n"

// SGR

#define ANSI_NORM       0

#define ANSI_FX_BOLD    1
#define ANSI_FX_DIM     2
#define ANSI_FX_ITALIC  3
#define ANSI_FX_UL      4
#define ANSI_FX_BSLOW   5
#define ANSI_FX_BFAST   6
#define ANSI_FX_INVERT  7
#define ANSI_FX_HIDE    8
#define ANSI_FX_STRIKE  9
#define ANSI_FX_DUL     21
#define ANSI_FX_NORM    22

#define ANSI_FG_BLACK   30
#define ANSI_FG_RED     31
#define ANSI_FG_GREEN   32
#define ANSI_FG_YELLOW  33
#define ANSI_FG_BLUE    34
#define ANSI_FG_MAGENTA 35
#define ANSI_FG_CYAN    36
#define ANSI_FG_WHITE   37
#define ANSI_FG_DEFAULT 39

#define ANSI_BG_BLACK   40
#define ANSI_BG_RED     41
#define ANSI_BG_GREEN   42
#define ANSI_BG_YELLOW  43
#define ANSI_BG_BLUE    44
#define ANSI_BG_MAGENTA 45
#define ANSI_BG_CYAN    46
#define ANSI_BG_WHITE   47
#define ANSI_BG_DEFAULT 49

#define ANSI_FX_FRAME   51
#define ANSI_FX_CIRCLE  52
#define ANSI_FX_OL      53

#endif