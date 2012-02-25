#include <stdio.h>
#include <string.h>

#include "cards.h"

char *ccolors[] = { "Red", "Black", "Purple" };
char *ctypes[]  = { "Naoxomoxoan", "Wishing", "Other-world", "Heart", "Demon", "Death", "Vortex" };
char *aspects[] = { "Soul", "Blood", "Heavens", "Flesh", "Crescent", "Master", "Reaper", "Star", "Fool", "Tiger" };
card_t deck[7][3][10];

void generate_deck()
{
        int t, c, a;

        for(t=0;t<7;t++) {
                for(c=0;c<3;c++) {
                        for(a=0;a<10;a++) {
                                deck[t][c][a].color = c;
                                deck[t][c][a].type = t;
                                deck[t][c][a].aspect = a;
                                sprintf(deck[t][c][a].name, "%s %s card of the %s", ccolors[deck[t][c][a].color], ctypes[deck[t][c][a].type], aspects[deck[t][c][a].aspect]);
//fprintf(stderr, "DEBUG: %s:%d - Generated card: The %s\n", __FILE__, __LINE__, deck[t][c][a].name);
                        }
                }
        }
}
