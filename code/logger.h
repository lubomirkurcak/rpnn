/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include <time.h>

internal char *get_short_name(char *full_name)
{
    smm length = string_length(full_name);

    for(smm index=length-1; index>=0; index -= 1)
    {
        if(full_name[index] == '/' || full_name[index] == '\\')
        {
            return full_name + index + 1;
        }
    }

    return full_name;
}

internal void print_current_time()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[32];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%X ", &tstruct);
    fprintf(stderr, buf);
}

#define lograw(format, ...) fprintf(stderr, format, __VA_ARGS__)

#define lograwwithtime(format, ...) {print_current_time(); fprintf(stderr, format, __VA_ARGS__);}

#define lognewline lograw("\n")

#define logfilestamp lograw(" in file %s at line " STRINGIZE(__LINE__) "\n", get_short_name(__FILE__))

#define logcontext(context) lograwwithtime("[%s] ", context)

#define logprint(context, format, ...){         \
    logcontext(context);          \
    lograw(format, __VA_ARGS__);       \
    lognewline;                      \
    logfilestamp;}

#define logwarning logprint
#define logerror logprint

#if 0
#define logmundane logprint
#else
#define logmundane(...)
#endif

// NOTE(lubo): Don't print file and line number
#define loginfo(context, format, ...){          \
    logcontext(context);          \
    lograw(format, __VA_ARGS__);       \
    lognewline;}

internal void logstring(string a)
{
    lograw("%.*s", (int)a.size, a.memory);
}

#if 0
// NOTE(lubomir): This is what I imagine they coded like in the 80s
internal void logbuffer(buffer a)
{
lograw("-------------------------\nbuffer size: %zd\n",a.size);
int newline_after=16,space_after=8,line_index=0;
u8 *at=a.memory,*linestart=a.memory;smm index=0,last_real_index=-1;
b32x loop=true;while(loop){if(index==a.size&&line_index==0)break;if(index<a.size){
last_real_index=index;lograw("%02x ",*at);}else{lograw("   ");}
at+=1;index+=1;line_index+=1;if(line_index==newline_after){lograw("| ");at=linestart;
for(int index2=0;index2<newline_after;++index2)
{if(at>=a.memory+a.size){loop=false;break;}char c=*at;at++;lograw("%c",(c<32)?'.':c);}
linestart=at;line_index=0;lognewline;
}else if(line_index==space_after){lograw(" ");}}
lograw("-------------------------\n");
}
#endif

//#define Assert(Expr, ...) if(!(Expr)){lograwwithtime("Assertion " #Expr " failed in file " __FILE__ " at line " STRINGIZE(__LINE__) ". " __VA_ARGS__ "\n");*(int *)0=0;}
#define Assert(Expr, ...) if(!(Expr)){lograwwithtime("Assertion %s failed in file " __FILE__ " at line " STRINGIZE(__LINE__) ". " __VA_ARGS__ "\n", #Expr);*(int *)0=0;}
//#define Assert(Expr, ...) if(!(Expr)){lograwwithtime("Assertion %s failed in file " __FILE__ " at line " STRINGIZE(__LINE__) ". " __VA_ARGS__ "\n", #Expr);}
#define AssertWarning(Expr, ...) if(!(Expr)) lograwwithtime("Assertion " #Expr " failed in file " __FILE__ " at line " STRINGIZE(__LINE__) ". " __VA_ARGS__ "\n")
#define AssertCleanup // TODO(lubo): enabled this once we want to do cleanup
#define NotImplemented Assert(!"Not Implemented")
#define InvalidDefaultCase default: Assert(!"Invalid Default Case"); break
#define InvalidCodePath Assert(!"Invalid Code Path")
