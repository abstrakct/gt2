#ifndef _GT_SAVELOAD_H
#define _GT_SAVELOAD_H

#define SAVE_DIRECTORY ".saves"

struct savefile_header {
        int magic;
        struct {
                int major;
                int minor;
                int revision;
        } version;
}; 


bool save_game();
bool load_game();

#endif
