//
// Created by 13994 on 3/5/2022.
//

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <android/native_activity.h>
#include <android/window.h>
#include <android/log.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
int SW,SH;
char *data_path,*internal_data_path;
int cw;
struct global
{
    ANativeWindow *win;
    AInputQueue *input;
    pthread_mutex_t lock;
    pthread_mutex_t ilock;
    short int fbw;
    short int fbh;
    short int fbl;
    unsigned short int *fb16;
    unsigned int *fb32;
    ANativeWindow_Buffer winbuf;
    void *unused;
} global;
void win_init(ANativeActivity *activity,ANativeWindow *window)
{
    pthread_mutex_lock(&global.lock);
    global.win=window;
    pthread_mutex_unlock(&global.lock);
}
void win_destroy(ANativeActivity *activity,ANativeWindow *window)
{
    pthread_mutex_lock(&global.lock);
    global.win=NULL;
    pthread_mutex_unlock(&global.lock);
}
void input_init(ANativeActivity *activity,AInputQueue *input)
{
    pthread_mutex_lock(&global.ilock);
    global.input=input;
    pthread_mutex_unlock(&global.ilock);
}
void input_destroy(ANativeActivity *activity,AInputQueue *input)
{
    pthread_mutex_lock(&global.ilock);
    global.input=NULL;
    pthread_mutex_unlock(&global.ilock);
}
void CR(void)
{
    if(global.fb16)
    {
        memset(global.fb16,0xff,global.fbl*global.fbh*2);
    }
    else if(global.fb32)
    {
        memset(global.fb32,0xff,global.fbl*global.fbh*4);
    }
}
void P1(void)
{
    pthread_mutex_lock(&global.lock);
    if(global.win==NULL)
    {
        pthread_mutex_unlock(&global.lock);
        return;
    }

    ANativeWindow_acquire(global.win);
    ANativeWindow_lock(global.win,&global.winbuf,NULL);
    global.fbl=global.winbuf.stride;
    if(global.fb16)
    {
            memcpy(global.winbuf.bits,global.fb16,global.fbl*global.fbh*2);

    }
    else if(global.fb32)
    {
        memcpy(global.winbuf.bits,global.fb32,global.fbl*global.fbh*4);

    }
    ANativeWindow_unlockAndPost(global.win);
    ANativeWindow_release(global.win);
    pthread_mutex_unlock(&global.lock);


}
void do_set_pixel(int x,int y,unsigned int color)
{
    unsigned int c;
    if(!(x>=0&&x<global.fbw&&y>=0&&y<global.fbh))
    {
        return;
    }
    if(global.fb16)
    {
        c=(color&0xff)>>3<<11|(color>>8&0xff)>>2<<5|(color>>16&0xff)>>3;
        global.fb16[y*global.fbl+x]=c;
    }
    else if(global.fb32)
    {
        c=(color&0xff)<<16|(color>>8&0xff)<<8|(color>>16&0xff);
        global.fb32[y*global.fbl+x]=c;
    }
}
void do_p_rect(int x,int y,int w,int h,unsigned int c)
{
    int x1,y1;
    void *ptr;
    if(x<0)
    {
        w+=x;
        x=0;
    }
    if(y<0)
    {
        h+=y;
        y=0;
    }
    if(x+w>global.fbw)
    {
        w=global.fbw-x;
    }
    if(y+h>global.fbh)
    {
        h=global.fbh-y;
    }
    if(w<=0||h<=0)
    {
        return;
    }
    if(global.fb16)
    {
        c=(c&0xff)>>3<<11|(c>>8&0xff)>>2<<5|(c>>16&0xff)>>3;
        for(x1=0;x1<w;x1++)
        {
            global.fb16[y*global.fbl+x+x1]=c;
        }
        ptr=global.fb16+y*global.fbl+x;
        for(y1=1;y1<h;y1++)
        {
            memcpy(global.fb16+(y+y1)*global.fbl+x,ptr,w*2);
        }
    }
    else if(global.fb32)
    {
        c=(c&0xff)<<16|(c>>8&0xff)<<8|(c>>16&0xff);
        for(x1=0;x1<w;x1++)
        {
            global.fb32[y*global.fbl+x+x1]=c;
        }
        ptr=global.fb32+y*global.fbl+x;
        for(y1=1;y1<h;y1++)
        {
            memcpy(global.fb32+(y+y1)*global.fbl+x,ptr,w*4);
        }
    }
}
void on_exit(void);
void *malloc_exit(size_t size)
{
    void *ptr;
    ptr=malloc(size);
    if(ptr==NULL)
    {
        on_exit();
        exit(1);
    }
    return ptr;
}

void SH_exit(int sig)
{
    exit(1);
}
#include "main/main.c"
void *T_main(void *param)
{
    AInputEvent *event=NULL;
    unsigned int mevent;
    int x,y,index,slot;
    int w,h,format,i;
    struct touch_event *touch_event;
    while(1)
    {
        pthread_mutex_lock(&global.lock);
        if(global.win!=NULL)
        {
            ANativeWindow_acquire(global.win);
            break;
        }
        pthread_mutex_unlock(&global.lock);
        usleep(10000);
    }
    w= ANativeWindow_getWidth(global.win);
    h= ANativeWindow_getHeight(global.win);
    format= ANativeWindow_getFormat(global.win);
    global.fbw=w;
    global.fbh=h;
    ANativeWindow_setBuffersGeometry(global.win,w,h,format);
    if(format==WINDOW_FORMAT_RGB_565)
    {
        global.fb16=calloc((global.fbw+64)*global.fbh,2);
    }
    else
    {
        global.fb32=calloc((global.fbw+64)*global.fbh,4);
    }
    cw=global.fbh/600;
    if(cw==0)
    {
        cw=1;
    }
    SW=global.fbw/cw;
    SH=global.fbh/cw;
    pthread_mutex_unlock(&global.lock);
    P1();
    main();
    while(1)
    {
        alarm(3);
        pthread_mutex_lock(&global.ilock);
        if(global.input)
        {
            if(AInputQueue_getEvent(global.input,&event)<0)
            {
                event = NULL;
            }

        }
        else
        {
            event=NULL;
        }

        if(event==NULL)
        {
            pthread_mutex_unlock(&global.ilock);
            usleep(50);
            continue;
        }

        switch(AInputEvent_getSource(event))
        {
            case AINPUT_SOURCE_KEYBOARD:
                switch(AKeyEvent_getKeyCode(event)) {
                    case AKEYCODE_BACK:
                    exit(0);
                }
                break;
            case AINPUT_SOURCE_TOUCHSCREEN:
                mevent=AMotionEvent_getAction(event);

                switch(mevent&AMOTION_EVENT_ACTION_MASK)
                {
                    case AMOTION_EVENT_ACTION_DOWN:
                    case AMOTION_EVENT_ACTION_POINTER_DOWN:
                        index = mevent >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                        x = AMotionEvent_getRawX(event, index);
                        y = AMotionEvent_getRawY(event, index);
                        touch_handler(index, 1, x / cw, y / cw, 0);
                        break;
                }
        }
        AInputQueue_finishEvent(global.input,event,0);
        pthread_mutex_unlock(&global.ilock);

    }
}
void cinit(ANativeActivity *activity)
{
    char *stack;
    static startup;
    if(startup)
    {
        return;
    }
    else
    {
        startup=1;
    }
    ANativeActivity_setWindowFlags(activity,AWINDOW_FLAG_KEEP_SCREEN_ON,0);
    signal(SIGSEGV,SH_exit);
    signal(SIGALRM,SH_exit);
    signal(SIGINT,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    mkdir(internal_data_path,0755);
    data_path=activity->externalDataPath;
    pthread_t T;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    stack=malloc(0x800000);
    pthread_attr_setstack(&attr,stack,0x800000);

    pthread_mutex_init(&global.lock,NULL);
    pthread_mutex_init(&global.ilock,NULL);
    pthread_mutex_init(&paint_lock,NULL);


    pthread_create(&T,&attr,T_main,NULL);
}
void cexit(ANativeActivity *activity)
{
    on_exit();
    exit(1);
}
void cnop(ANativeActivity *activity)
{

}
void ANativeActivity_onCreate(ANativeActivity *activity,void *savedState,size_t savedStateSize)
{

    activity->callbacks->onNativeWindowCreated=win_init;
    activity->callbacks->onNativeWindowDestroyed=win_destroy;
    activity->callbacks->onInputQueueCreated=input_init;
    activity->callbacks->onInputQueueDestroyed=input_destroy;
    activity->callbacks->onStart=cinit;
    activity->callbacks->onResume=cnop;
    activity->callbacks->onStop=cexit;
    activity->callbacks->onLowMemory=cexit;
    activity->callbacks->onDestroy=cexit;
}
