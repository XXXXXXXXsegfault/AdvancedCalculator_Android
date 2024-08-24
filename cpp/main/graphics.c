void do_set_pixel(int x,int y,unsigned int color);
void do_p_rect(int x,int y,int w,int h,unsigned int c);
void set_pixel(int x,int y,unsigned int color)
{
	do_p_rect(x*cw,y*cw,cw,cw,color);
}
void p_rect(int x,int y,int w,int h,unsigned int c)
{
	do_p_rect(x*cw,y*cw,w*cw,h*cw,c);
}
#include "font.c"

