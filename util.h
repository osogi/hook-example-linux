#include <dlfcn.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>


//техника удобная для формирования отдельных полей структуры, обязательно должно быть обернуто в union
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define CREATE_MEMBER(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;} 


typedef u_char byte_t;

#define PUSH_RAX_RDI 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57
#define POP_RDI_RAX  0x5f, 0x5e, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58

#define PUSH_R8_15 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57 
#define POP_R15_8  0x41, 0x5f, 0x41, 0x5e, 0x41, 0x5d, 0x41, 0x5c, 0x41, 0x5b, 0x41, 0x5a, 0x41, 0x59, 0x41, 0x58	

size_t getBaddr(pid_t pid, char* name);
byte_t* InjectAlloc(size_t inject_address, u_int32_t len_of_nop, byte_t* source, u_int32_t len_of_source);
int mprotect_aligned(void* addr, size_t len, int prot);

void add_jmp32(byte_t* dst, size_t jmp_addr);
void add_jmp64(byte_t* dst, size_t jmp_addr);