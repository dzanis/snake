#ifndef SOUND_H
#define SOUND_H

#include "ufmod.h"
#include "music.xm.h"
#include <string.h> // for strcmp

typedef struct
{
   char * name;
   char * arr;
   int size;

}XM;

XM xm [2] = {
    {"BReWErS-XBlade+7trn",music,musicSize},
    {"Canterwood",music2,music2Size}
};

int sound_muted = 0;

void sound_volume(int volume)
{
    uFMOD_SetVolume( volume );
}

void sound_play(char * name)
{
    //if(uFMOD_PlaySong( "music.xm", 0, XM_FILE ) == 0)printf("music error  \n");
    for(int i=0;i<sizeof(xm);i++)
    {
        int result  = strcmp(name ,xm[i].name);
        if(result == 0 )
        {
            uFMOD_PlaySong( (void*)xm[i].arr, (void*)xm[i].size, XM_MEMORY );
            printf("music title %s \n",uFMOD_GetTitle());
            if(sound_muted) uFMOD_Pause();
             return;
        }
    }

    printf("sound \"%s\" not found  \n",name);

}

void sound_stop()
{
    uFMOD_Rewind();// чтобы после стопа играла сначала
    uFMOD_Pause();
}

void sound_pause()
{
    uFMOD_Pause();
}

void sound_resume()
{
    if(sound_muted) return;
    uFMOD_Resume();
}

void sound_mute()
{
    sound_muted = !sound_muted;
    if(sound_muted)
    sound_pause();
    else
    sound_resume();
}

#endif // SOUND_H
