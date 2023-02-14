#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit limit; 
    /*
    	rlimit has to feilds; 1.rlim_cur that get the current limiation, 2.rlim_max that get the maximum limitation
    	this struct also have 2 functions; 1. getrlimit that has 2 arguments, first is the type of Limit that you wannaa know about(see man getrlimit), second is the rlimit struct address.
    					   2. setrlimit that has the same arguments as getrlimit and can be used for changing the limitation.
    	the rlim_max is ceil of limitation that you can have.
    */
    getrlimit (RLIMIT_STACK, &limit);
    printf("stack size: %ld\n", limit.rlim_cur);
    
    getrlimit (RLIMIT_NPROC, &limit);
    printf("process limit: %ld\n", limit.rlim_cur);
    
    getrlimit (RLIMIT_NOFILE, &limit);
    printf("max file descriptors: %ld\n", limit.rlim_cur);
    return 0;
}
