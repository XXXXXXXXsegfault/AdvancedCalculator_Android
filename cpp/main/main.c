#include "graphics.c"
pthread_mutex_t paint_lock;
void paint_all(void);
void paint_key(int x,int y,char *str,int func)
{
    int width=SH/8,y1,l;
    x=SW-width*3+width*x;
    y1=SH*(y+1)/8;
    y=SH*y/8;
    l=strlen(str);
    p_rect(x,y,width,y1-y,func?0xffff00:(l==1?0x00ffff:0x00c000));
    p_str(str,x+width/2-l*4,(y+y1)/2-8,0x000000);
    p_rect(x,y,width,1,0xc0c0c0);
    p_rect(x,y,1,y1-y,0xc0c0c0);
    p_rect(x,y1-1,width,1,0xc0c0c0);
    p_rect(x+width-1,y,1,y1-y,0xc0c0c0);
}
void paint_keyboard(void)
{
    char *key_strs[]={"7","8","9","4","5","6",
                      "1","2","3","Up","Down","0","Left",
                      "Right","Set","Add","Sub","Mul","Div",
                      "Goto","Overflow","Label","()","[]"};
    int x=0,y=0,i=0;
    while(i<24)
    {
        paint_key(x, y, key_strs[i], 0);
        ++x;
        if (x == 3) {
            x = 0;
            ++y;
        }
        ++i;
    }
    paint_key(-2,0,"Delete",0);
    paint_key(-1,3,"GotoIf =",0);
    paint_key(-1,4,"GotoIf <",0);
    paint_key(-1,5,"Output",0);
    paint_key(-1,6,"Start",1);
    paint_key(-1,7,"Stop",1);
}
#include "program.c"
void paint_all(void)
{
    pthread_mutex_lock(&paint_lock);
    CR();
    paint_prog();
    paint_keyboard();
    P1();
    pthread_mutex_unlock(&paint_lock);
}
int move=1;
void insert_digit(int digit)
{
    long long val;
    struct instr *instr;
    if(count_instr==0)
    {
        return;
    }
    instr=instr_array+prog_pointer;
    val=instr->op[current_arg].value;
    if(val*10<-VALUE_MAX||val*10>VALUE_MAX||move)
    {
        val=digit;
        move=0;
    }
    else
    {
        val=val*10+digit;
    }
    instr->op[current_arg].value=val;
}
void button_press(int X,int Y)
{
    int bx=(SW-X)/(SH/8),by=Y*8/SH;
    int bnum=bx+by*10;
    struct instr *instr;
    if(bx>=10)
    {
        return;
    }
    if(prog_end)
    {
        switch(bnum)
        {
            case 40:
                instr_insert(5);
                paint_all();
                break;
            case 50:
                instr_insert(3);
                paint_all();
                break;
            case 51:
                instr_insert(2);
                paint_all();
                break;
            case 52:
                instr_insert(1);
                paint_all();
                break;
            case 61:
                instr_insert(6);
                paint_all();
                break;
            case 62:
                instr_insert(4);
                paint_all();
                break;
            case 72:
                instr_insert(7);
                paint_all();
                break;
            case 33:
                instr_insert(9);
                paint_all();
                break;
            case 43:
                instr_insert(10);
                paint_all();
                break;
            case 0:
                insert_digit(9);
                paint_all();
                break;
            case 1:
                insert_digit(8);
                paint_all();
                break;
            case 2:
                insert_digit(7);
                paint_all();
                break;
            case 10:
                insert_digit(6);
                paint_all();
                break;
            case 11:
                insert_digit(5);
                paint_all();
                break;
            case 12:
                insert_digit(4);
                paint_all();
                break;
            case 20:
                insert_digit(3);
                paint_all();
                break;
            case 21:
                insert_digit(2);
                paint_all();
                break;
            case 22:
                insert_digit(1);
                paint_all();
                break;
            case 30:
                insert_digit(0);
                paint_all();
                break;
            case 31:
                if(count_instr!=0)
                {
                    prog_pointer+=1;
                    if(prog_pointer==count_instr)
                    {
                        prog_pointer=0;
                    }
                    current_arg=0;
                    paint_all();
                }
                move=1;
                break;
            case 32:
                if(count_instr!=0)
                {
                    if(prog_pointer==0)
                    {
                        prog_pointer=count_instr-1;
                    }
                    else
                    {
                        --prog_pointer;
                    }
                    current_arg=0;
                    paint_all();
                }
                move=1;
                break;
            case 41:
                if(count_instr!=0)
                {
                    instr=instr_array+prog_pointer;
                    if(!(current_arg>=2||current_arg>=0&&(instr->type==6||instr->type==7||instr->type==8)||current_arg>=1&&instr->type==5))
                    {
                        ++current_arg;
                        paint_all();
                    }
                }
                move=1;
                break;
            case 42:
                if(count_instr!=0)
                {
                    if(current_arg!=0)
                    {
                        --current_arg;
                        paint_all();
                    }
                }
                move=1;
                break;
            case 60:
                if(count_instr!=0)
                {
                    instr=instr_array+prog_pointer;
                    instr->op[current_arg].value=VALUE_OVERFLOW;
                    paint_all();
                }
                break;
            case 70:
                if(count_instr!=0)
                {
                    instr=instr_array+prog_pointer;
                    if(instr->type!=7)
                    {
                        instr->op[current_arg].ismem^=1;
                        paint_all();
                    }
                }
                break;
            case 71:
                if(count_instr!=0)
                {
                    instr=instr_array+prog_pointer;
                    if(instr->type!=7)
                    {
                        instr->op[current_arg].isvar^=1;
                        paint_all();
                    }
                }
                break;
            case 63:
                if(count_instr!=0)
                {
                    prog_run();
                }
                break;
            case 53:
                instr_insert(8);
                paint_all();
                break;
            case 4:
                instr_delete();
                paint_all();
        }
    }
    else
    {
        if(bnum==73)
        {
            prog_end=1;
        }
    }
}
void touch_handler(int slot,int type,int x,int y,int unused)
{
    if(type==1)
    {
        button_press(x,y);
    }
}
void main(void)
{
    prog_load();
    paint_all();
}
void on_exit(void)
{
    prog_save();
}
