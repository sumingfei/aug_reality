#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H


#include <string>

#include <FCam/FCam.h>
#include <FCam/Action.h>
#include <FCam/Device.h>

#include <pthread.h>

/* An FCam Device to play a beeping noise synchronized to
 * exposure. Unfortunately it's not synchronized very
 * well. player->beep() seems to occasionally take a long time. */
class SoundPlayer : public FCam::Device {
    
public:
    
    SoundPlayer();
    ~SoundPlayer();
    
    /*
     * An action representing the playback of a .WAV file.
     */
    class SoundAction : public FCam::CopyableAction<SoundAction> {
    public:

        /* Constructors and destructor */
        SoundAction(SoundPlayer * b);
        SoundAction(SoundPlayer * b, int time);

        /* Implementation of doAction() as required */
        void doAction() {player->beep();}
        
    protected:
        SoundPlayer * player;
        int size;
    };
    
    /* Normally, this is where a device would add metadata tags to a
     * just-created frame , based on the timestamps in the
     * Frame. However, we don't have anything useful to add here, so
     * tagFrame does nothing. */
    void tagFrame(FCam::Frame) {}
    
    /* Play a noise */
    void beep();
    
    /* Returns latency in microseconds */
    int getLatency();

    /* Starts the sound player thread. No need to call this. */
    void run();
    
protected:
    sem_t semaphore;
    bool stop;
};

#endif
