#include "SoundPlayer.h"
#include <sys/mman.h>
#include <pthread.h>
#include <pulse/simple.h>
#include <pulse/error.h>

void *soundThread(void *arg) {
    printf("Launching sound thread\n");
    ((SoundPlayer *)arg)->run();
    pthread_exit(NULL);
    return NULL;
}

/* SoundPlayer constructor */
SoundPlayer::SoundPlayer() {
    // launch the sound playing thread
    stop = false;
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param param;

    sem_init(&semaphore, 0, 0);

    pthread_attr_init(&attr);
    param.sched_priority = sched_get_priority_min(SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_create(&thread, &attr, soundThread, this);
}

SoundPlayer::~SoundPlayer() {
    stop = true;
    sem_post(&semaphore);
}

void SoundPlayer::run() {
    /* The Sample format to use */
    static pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 22050;
    ss.channels = 1;

    pa_simple *s = NULL;
    int error;
    
    /* Create a new playback stream */
    s = pa_simple_new(NULL, "fcamera", PA_STREAM_PLAYBACK, 
                      NULL, "playback", &ss, NULL, NULL, &error);
    if (!s) printf("pa_simple_new: %s\n", pa_strerror(error));

    unsigned short buf[2048];
    for (int i = 0; i < 1024; i++) {
        buf[i] = (1024-i)*20*sin(i*0.07 + 0.1*sin(i*0.07));
    }
    for (int i = 1024; i < 2048; i++) {
        buf[i] = 0;
    }

    // make sure the buf doesn't get swapped out, or the sound will
    // play too late.
    mlock(buf, sizeof(buf));

    while (!stop) {
        sem_wait(&semaphore);
        if (stop) break;
        printf("Beep!\n");
        if (pa_simple_write(s, buf, sizeof(buf), &error) < 0) {
            printf("pa_simple_write: %s\n", pa_strerror(error));
        }
        if (pa_simple_drain(s, &error) < 0) {
            printf("pa_simple_drain: %s\n", pa_strerror(error));
        }
    }
    
    pa_simple_free(s);
    munlock(buf, sizeof(buf));
}

/* Play a buffer */
void SoundPlayer::beep() {
    sem_post(&semaphore);
}

int SoundPlayer::getLatency() {
    return 5000;
}

/***************************************************************/
/* SoundPlayer::SoundAction implementation                     */
/***************************************************************/

/* SoundAction constructors */
SoundPlayer::SoundAction::SoundAction(SoundPlayer * a) {
    player = a;
    time = 0;
    latency = a ? a->getLatency() : 0;
}

SoundPlayer::SoundAction::SoundAction(SoundPlayer * a, int t) {
    player = a;
    time = t;
    latency = a ? a->getLatency() : 0;
}



