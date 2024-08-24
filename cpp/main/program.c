#define VALUE_MAX 999999999999999
#define VALUE_OVERFLOW (VALUE_MAX+1)
long long output_board=0;

struct htab_node
{
    long long key;
    long long value;
    struct htab_node *next;
    struct htab_node *track_next;
} *track_list;
#define HTAB_LEN 300017
struct htab_node *var_htab[HTAB_LEN],*mem_htab[HTAB_LEN];
struct htab_node *htab_node_new(struct htab_node **htab,long long key)
{
    int index;
    struct htab_node *node;
    index=(key+VALUE_MAX)%HTAB_LEN;
    node=malloc(sizeof(*node));
    if(node==NULL)
    {
        return NULL;
    }
    node->key=key;
    node->value=0;
    node->next=htab[index];
    htab[index]=node;
    node->track_next=track_list;
    track_list=node;
    return node;
}
void htab_cleanup(void)
{
    struct htab_node *node;
    while(node=track_list)
    {
        track_list=node->next;
        free(node);
    }
    memset(mem_htab,0,sizeof(mem_htab));
    memset(var_htab,0,sizeof(var_htab));
}
long long htab_value_get(struct htab_node **htab,long long key)
{
    int index;
    struct htab_node *node;
    index=(key+VALUE_MAX)%HTAB_LEN;
    node=htab[index];
    while(node)
    {
        if(node->key==key)
        {
            return node->value;
        }
        node=node->next;
    }
    return 0;
}
int htab_value_set(struct htab_node **htab,long long key,long long value)
{
    int index;
    struct htab_node *node;
    index=(key+VALUE_MAX)%HTAB_LEN;
    node=htab[index];
    while(node)
    {
        if(node->key==key)
        {
            node->value=value;
            return 0;
        }
        node=node->next;
    }
    node=htab_node_new(htab,key);
    if(node==NULL)
    {
        return -1;
    }
    node->value=value;
    return 0;
}
struct operand
{
    long long value;
    int isvar;
    int ismem;
};
struct instr
{
    int type;
    struct operand op[3];
} *instr_array;
int count_instr,prog_pointer,current_arg;
long long get_operand_val(struct operand *op)
{
    long long val = op->value;
    if (op->isvar)
    {
        val = htab_value_get(var_htab, val);
    }
    if (op->ismem)
    {
        val = htab_value_get(mem_htab, val);
    }
    return val;
}
int set_operand_val(struct operand *op,long long newval)
{
    long long val = op->value;
    if (op->isvar)
    {
        if (op->ismem)
        {
            val = htab_value_get(var_htab, val);
            return htab_value_set(mem_htab, val,newval);
        }
        else
        {
            return htab_value_set(var_htab,val,newval);
        }
    }
    else
    {
        if (op->ismem)
        {
            return htab_value_set(mem_htab, val,newval);
        }
        return 0;
    }
}
int instr_insert(int type)
{
    struct instr *new_array;
    new_array=realloc(instr_array,(count_instr+1)*sizeof(*instr_array));

    if(new_array==NULL)
    {
        return -1;
    }
    if(count_instr!=0)
    {
        ++prog_pointer;
        current_arg=0;
    }
    memmove(new_array+prog_pointer+1,new_array+prog_pointer,sizeof(*instr_array)*(count_instr-prog_pointer));
    memset(new_array+prog_pointer,0,sizeof(*instr_array));
    new_array[prog_pointer].type=type;
    ++count_instr;
    instr_array=new_array;
    return 0;
}
void instr_delete(void)
{
    if(count_instr==0)
    {
        return;
    }
    memmove(instr_array+prog_pointer,instr_array+prog_pointer+1,(count_instr-prog_pointer-1)*sizeof(*instr_array));
    if(prog_pointer==count_instr-1)
    {
        --prog_pointer;
    }
    if(prog_pointer<0)
    {
        prog_pointer=0;
    }
    --count_instr;
}
int prog_end=1;
void *T_prog(void *arg)
{
    struct instr *instr;
    long long val1,val2,val3;
    int i;
    while(1)
    {
        if(prog_end)
        {
            paint_all();
            pthread_exit(NULL);
        }
        paint_all();
        usleep(20000);
        instr=instr_array+prog_pointer;
        ++prog_pointer;
        if(prog_pointer==count_instr)
        {
            prog_pointer=0;
        }
        switch(instr->type)
        {
            case 1:
                val2= get_operand_val(instr->op+1);
                val3= get_operand_val(instr->op+2);
                if(val2==VALUE_OVERFLOW||val3==VALUE_OVERFLOW)
                {
                    val1=VALUE_OVERFLOW;
                }
                else
                {
                    val1 = val2 + val3;
                    if(val1<-VALUE_MAX||val1>VALUE_MAX)
                    {
                        val1=VALUE_OVERFLOW;
                    }
                }
                set_operand_val(instr->op,val1);
                break;
            case 2:
                val2= get_operand_val(instr->op+1);
                val3= get_operand_val(instr->op+2);
                if(val2==VALUE_OVERFLOW||val3==VALUE_OVERFLOW)
                {
                    val1=VALUE_OVERFLOW;
                }
                else
                {
                    val1 = val2 - val3;
                    if(val1<-VALUE_MAX||val1>VALUE_MAX)
                    {
                        val1=VALUE_OVERFLOW;
                    }
                }
                set_operand_val(instr->op,val1);
                break;
            case 3:
                val2= get_operand_val(instr->op+1);
                val3= get_operand_val(instr->op+2);
                if(val2==VALUE_OVERFLOW||val3==VALUE_OVERFLOW)
                {
                    val1=VALUE_OVERFLOW;
                }
                else
                {
                    val1 = val2 * val3;
                    if(val3<0)
                    {
                        if (val2 > VALUE_MAX / -val3 || val2 < -(VALUE_MAX / -val3))
                        {
                            val1 = VALUE_OVERFLOW;
                        }
                    }
                    else if(val3>0)
                    {
                        if (val2 > VALUE_MAX / val3 || val2 < -(VALUE_MAX / val3))
                        {
                            val1 = VALUE_OVERFLOW;
                        }
                    }
                }
                set_operand_val(instr->op,val1);
                break;
            case 4:
                val2= get_operand_val(instr->op+1);
                val3= get_operand_val(instr->op+2);
                if(val2==VALUE_OVERFLOW||val3==VALUE_OVERFLOW||val3==0)
                {
                    val1=VALUE_OVERFLOW;
                }
                else
                {
                    if(val3<0)
                    {
                        val1 = -val2 / -val3;
                        if(val1*-val3>-val2)
                        {
                            --val1;
                        }
                    }
                    else
                    {
                        val1= val2 / val3;
                        if(val1*val3>val2)
                        {
                            --val1;
                        }
                    }
                }
                set_operand_val(instr->op,val1);
                break;
            case 5:
                val2=get_operand_val(instr->op+1);
                set_operand_val(instr->op,val2);
                break;
            case 6:
                for(i=0;i<count_instr;++i)
                {
                    if (instr_array[i].type == 7)
                    {
                        if (get_operand_val(instr_array[i].op) == get_operand_val(instr->op))
                        {
                            prog_pointer = i;
                            break;
                        }
                    }
                }
                break;
            case 8:
                output_board=get_operand_val(instr->op);
                if(prog_pointer==0)
                {
                    prog_pointer=count_instr-1;
                }
                else
                {
                    --prog_pointer;
                }
                prog_end=1;
                break;
            case 9:
                val2 = get_operand_val(instr->op+1);
                val3 = get_operand_val(instr->op+2);
                if(val2!=val3)
                {
                    break;
                }
                for(i=0;i<count_instr;++i)
                {
                    if (instr_array[i].type == 7)
                    {
                        if (get_operand_val(instr_array[i].op) == get_operand_val(instr->op))
                        {
                            prog_pointer = i;
                            break;
                        }
                    }
                }
                break;
            case 10:
                val2 = get_operand_val(instr->op+1);
                val3 = get_operand_val(instr->op+2);
                if(val2>=val3||val2==VALUE_OVERFLOW||val3==VALUE_OVERFLOW)
                {
                    break;
                }
                for(i=0;i<count_instr;++i)
                {
                    if (instr_array[i].type == 7)
                    {
                        if (get_operand_val(instr_array[i].op) == get_operand_val(instr->op))
                        {
                            prog_pointer = i;
                            break;
                        }
                    }
                }
                break;
        }
    }
}
void prog_run(void)
{
    pthread_t T;
    prog_end=0;
    if(pthread_create(&T,NULL, T_prog,NULL))
    {
        prog_end=1;
    }
    else
    {
        pthread_detach(T);
        current_arg=0;
    }
}
void paint_prog(void)
{
    int height=SH/16;
    int i=prog_pointer-height/2,y=0,count_arg,j;
    char buf[40];
    struct instr *instr;
    if(i<0)
    {
        i=0;
    }
    while(y<height&&i<count_instr)
    {
        instr=instr_array+i;
        count_arg=0;
        if(prog_pointer==i)
        {
            p_rect(0,y*16,SW,16,0xc0c0c0);
        }
        switch(instr->type)
        {
            case 1:
                p_rect(0,y*16,80,16,0x00ff00);
                p_str("ADD",0,y*16,0x000000);
                count_arg=3;
                break;
            case 2:
                p_rect(0,y*16,80,16,0x00ff00);
                p_str("SUB",0,y*16,0x000000);
                count_arg=3;
                break;
            case 3:
                p_rect(0,y*16,80,16,0x00ff00);
                p_str("MUL",0,y*16,0x000000);
                count_arg=3;
                break;
            case 4:
                p_rect(0,y*16,80,16,0x00ff00);
                p_str("DIV",0,y*16,0x000000);
                count_arg=3;
                break;
            case 5:
                p_rect(0,y*16,80,16,0x00ff00);
                p_str("SET",0,y*16,0x000000);
                count_arg=2;
                break;
            case 6:
                p_rect(0,y*16,80,16,0xffff00);
                p_str("GOTO",0,y*16,0x000000);
                count_arg=1;
                break;
            case 7:
                p_rect(0,y*16,80,16,0x00ffff);
                p_str("LABEL",0,y*16,0x000000);
                count_arg=1;
                break;
            case 8:
                p_rect(0,y*16,80,16,0xff8000);
                p_str("OUTPUT",0,y*16,0x000000);
                count_arg=1;
                break;
            case 9:
                p_rect(0,y*16,80,16,0xffff00);
                p_str("GOTOIF =",0,y*16,0x000000);
                count_arg=3;
                break;
            case 10:
                p_rect(0,y*16,80,16,0xffff00);
                p_str("GOTOIF <",0,y*16,0x000000);
                count_arg=3;
                break;
        }
        for(j=0;j<count_arg;++j)
        {
            if(instr->op[j].value==VALUE_OVERFLOW)
            {
                if (instr->op[j].ismem)
                {
                    if (instr->op[j].isvar)
                    {
                        sprintf(buf, "[(Overflow)]");
                    }
                    else
                    {
                        sprintf(buf, "[Overflow]");
                    }
                }
                else
                {
                    if (instr->op[j].isvar)
                    {
                        sprintf(buf, "(Overflow)");
                    }
                    else
                    {
                        sprintf(buf, "Overflow");
                    }
                }

            }
            else
            {
                if (instr->op[j].ismem)
                {
                    if (instr->op[j].isvar)
                    {
                        sprintf(buf, "[(%lld)]", instr->op[j].value);
                    }
                    else
                    {
                        sprintf(buf, "[%lld]", instr->op[j].value);
                    }
                }
                else
                {
                    if (instr->op[j].isvar)
                    {
                        sprintf(buf, "(%lld)", instr->op[j].value);
                    }
                    else
                    {
                        sprintf(buf, "%lld", instr->op[j].value);
                    }
                }

            }
            p_rect(80 + 168 * j, y * 16, 168, 16,
                   prog_pointer == i && current_arg == j ? 0xff00ff : 0x800080);
            p_str(buf, 80 + 168 * j, y * 16, 0x000000);
        }
        ++y;
        ++i;
    }
    p_rect(SW-SH/8*4-160,SH-24,160,24,0x404040);
    if(output_board==VALUE_OVERFLOW)
    {
        sprintf(buf,"Overflow");
    }
    else
    {
        sprintf(buf,"%lld",output_board);
    }

    p_str(buf,SW-SH/8*4-160+4,SH-24+4,0xffffff);
}
void prog_save(void)
{
    int dfd,fd;
    dfd=open(data_path,0);
    if(dfd>=0)
    {
        fd=openat(dfd,"prog.bin",O_CREAT|O_TRUNC|O_WRONLY,0644);
        if(fd>=0)
        {
            write(fd,&prog_pointer,4);
            write(fd,&count_instr,4);
            write(fd,instr_array,sizeof(*instr_array)*count_instr);
            close(fd);
        }
        close(dfd);
    }

}
void prog_load(void)
{
    int dfd,fd;
    dfd=open(data_path,0);
    if(dfd>=0)
    {
        fd=openat(dfd,"prog.bin",0);
        if(fd>=0)
        {
            read(fd,&prog_pointer,4);
            read(fd,&count_instr,4);
            instr_array=malloc_exit(sizeof(*instr_array)*count_instr);
            read(fd,instr_array,sizeof(*instr_array)*count_instr);
            close(fd);
        }
        close(dfd);
    }

}