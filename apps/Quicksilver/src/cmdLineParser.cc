/// \file
#ifdef HAVE_CALIPER
#include<caliper/cali.h>
#endif
/// A parser for command line arguments.
///
/// A general purpose command line parser that uses getopt_long() to parse
/// the command line.
///
/// \author Sriram Swaminarayan
/// \date July 24, 2007

#include "cmdLineParser.hh"

#include <cstdio>
#include <cinttypes>
#include <getopt.h>
#include <cstring>

#include "memUtils.hh"

#define nextOption(o) ((MyOption*) o->next)

typedef struct MyOptionSt
{
    char* help;
    char* longArg;
    unsigned char shortArg[2];
    int argFlag;
    char type;
    int sz;
    void* ptr;
    void* next;
} MyOption;

static int longest = 1;
static MyOption* myargs=NULL;

static char* dupString(const char* s)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    char* d;
    if ( ! s ) s = "";
    d = (char*)qsCalloc((strlen(s)+1),sizeof(char));
    strcpy(d, s);
    return d;
}

static MyOption* myOptionAlloc(
    const char* longOption, const char shortOption,
    int has_arg, const char type, void* dataPtr, int dataSize, const char* help)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    static int iBase=129;
    MyOption* o = (MyOption*)qsCalloc(1, sizeof(MyOption));
    o->help = dupString(help);
    o->longArg = dupString(longOption);
    if(shortOption) o->shortArg[0] = (unsigned char)shortOption;
    else
    {
        o->shortArg[0] = iBase;
        iBase++;
    }
    o->argFlag = has_arg;
    o->type = type;
    o->ptr = dataPtr;
    o->sz = dataSize;
    if(longOption) longest = (longest>strlen(longOption) ? longest : strlen(longOption));
    return o;
}

static MyOption* myOptionFree(MyOption* o)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    MyOption* r;
    if(!o) return NULL;
    r = nextOption(o);
    if(o->longArg) qsFree(o->longArg);
    if(o->help) qsFree(o->help);
    qsFree(o);
    return r;
}

static MyOption* lastOption(MyOption* o)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    if ( ! o) return o;
    while(nextOption(o)) o = nextOption(o);
    return o;
}

static MyOption* findOption(MyOption* o, unsigned char shortArg)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    while(o)
    {
        if (o->shortArg[0] == shortArg) return o;
        o = nextOption(o);
    }
    return o;
}


int addArg(const char* longOption, const char shortOption,
           int has_arg, const char type, void* dataPtr, int dataSize,
           const char* help)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    MyOption* o;
    MyOption* p;
    o = myOptionAlloc(longOption,shortOption,has_arg,type,dataPtr,dataSize, help);
    if ( ! o ) return 1;
    if ( ! myargs) myargs = o;
    else
    {
        p = lastOption(myargs);
        p->next = (void *)o;
    }
    return 0;
}


void freeArgs()
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    while(myargs)
    {
        myargs = myOptionFree(myargs);
    }
    return;
}

void printArgs()
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    MyOption* o = myargs;
    char s[4096];
    unsigned char *shortArg;
    fprintf(stdout,"\n"
            "  Arguments are: \n");
    sprintf(s,"   --%%-%ds",longest);
    while(o)
    {
        if(o->shortArg[0]<0xFF) shortArg = o->shortArg;
        else shortArg = (unsigned char *) "---";
        fprintf(stdout,s,o->longArg);
        fprintf(stdout," -%c  arg=%1d type=%c  %s\n",shortArg[0],o->argFlag,o->type,o->help);
        o = nextOption(o);

    }
    fprintf(stdout,"\n\n");
    return;
}

void processArgs(int argc, char** argv)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    MyOption* o;
    int n=0;
    struct option* opts;
    char* sArgs;
    int c;

    if ( ! myargs) return;
    o = myargs;
    while(o)
    {n++,o=nextOption(o); }

    o = myargs;
    sArgs= (char*)qsCalloc(2*(n+2),sizeof(char));
    opts = (struct option*)qsCalloc(n,sizeof(struct option));
    for (int i=0; i<n; i++)
    {
        opts[i].name = o->longArg;
        opts[i].has_arg = o->argFlag;
        opts[i].flag    = 0;
        opts[i].val     = o->shortArg[0];

        strcat(sArgs,(char*) o->shortArg);
        if(o->argFlag) strcat(sArgs,":");
        o = nextOption(o);
    }

    while(1)
    {

        int option_index = 0;

        c = getopt_long (argc, argv, sArgs, opts, &option_index);
        if ( c == -1) break;
        o = findOption(myargs,c);
        if ( ! o )
        {
            fprintf(stdout,"\n\n"
                    "    invalid switch : -%c in getopt()\n"
                    "\n\n",
                    c);
            break;
        }
        if(! o->argFlag)
        {
            int* i = (int*)o->ptr;
            *i = 1;
        }
        else
        {
            switch(o->type)
            {
                case 'i' :
                    sscanf(optarg,"%d",(int*)o->ptr);
                    break;
                case 'u' :
                    sscanf(optarg,"%" SCNu64,(uint64_t*)o->ptr);
                    break;
                case 'f' :
                    sscanf(optarg,"%f",(float*)o->ptr);
                    break;
                case 'd' :
                    sscanf(optarg,"%lf",(double*)o->ptr);
                    break;
                case 's' :
                    strncpy((char*)o->ptr,(char*)optarg,o->sz);
                    ((char*)o->ptr)[o->sz-1] = '\0';
                    break;
                case 'c' :
                    sscanf(optarg,"%c",(char*)o->ptr);
                    break;
                default :
                    fprintf(stdout,"\n\n"
                            "    invalid type : %c in getopt()\n"
                            "    valid values are 'i','u','d','f','s', and 'c'\n"
                            "\n\n",
                            c);
            }
        }
    }

    qsFree(opts);
    qsFree(sArgs);

    return;
}
