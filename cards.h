#ifndef CARDS_H
#define CARDS_H

/* 
 * Stuff for dealing with The Cards
 */


typedef struct {
        short color;
        short type;
        short aspect;
        char name[40];
} card_t;

extern card_t deck[7][3][10];

void generate_deck();

#define C_C_RED         0
#define C_C_BLACK       1
#define C_C_PURPLE      2

#define C_T_NAOXOMOXOAN 0
#define C_T_WISHING     1
#define C_T_OTHERWORLD  2
#define C_T_HEART       3
#define C_T_DEMON       4
#define C_T_DEATH       5
#define C_T_VORTEX      6

#define C_A_SOUL        0
#define C_A_BLOOD       1
#define C_A_HEAVEN      2
#define C_A_FLESH       3
#define C_A_CRESCENT    4
#define C_A_MASTER      5
#define C_A_REAPER      6
#define C_A_STAR        7
#define C_A_FOOL        8
#define C_A_TIGER       9

#endif
