#include "util.h"


byte_t JMP_PATTERN64[] = {
    0x68, 0xcb, 0x12, 0x47, 0x23,     // 0  push 0x234712cb */
    0xc7, 0x44, 0x24, 0x04,           // 5
    0xf6, 0x7f, 0x00, 0x00,           //    mov dword [rsp + 4], 0x7ff6 
    0xc3                              //13  ret
};


byte_t JMP_PATTERN32[] = {
0xe9, 0xcb, 0x12, 0x47, 0x23,        /* 0 jmp <...> */ 
};




bool is32bit() {
    return sizeof(void*) == 4;
}

size_t len_jmp_pattern() {
    if(is32bit())
        return sizeof(JMP_PATTERN32);
    else
        return sizeof(JMP_PATTERN64);
}

void add_jmp64(byte_t* dst, size_t jmp_addr) {
    memcpy(dst, JMP_PATTERN64, sizeof(JMP_PATTERN64));
    u_int32_t first_half = jmp_addr & 0xffffffff;
    u_int32_t second_half = (jmp_addr>>32) & 0xffffffff;
    *((u_int32_t*)(dst + 1)) = first_half;
    *((u_int32_t*)(dst + 5+4)) = second_half;
}

void add_jmp32(byte_t* dst, size_t jmp_addr) {
    memcpy(dst, JMP_PATTERN32, sizeof(JMP_PATTERN32));
    *((u_int32_t*)(dst + 1)) = (u_int32_t)jmp_addr-((size_t)dst+5);
}

void add_jmp(byte_t* dst, size_t jmp_addr) {
    if (is32bit())
        add_jmp32(dst, jmp_addr);
    else
        add_jmp64(dst, jmp_addr);
}


int mprotect_aligned(void* addr, size_t len, int prot){
    size_t page_size = getpagesize();
    size_t naddr = (size_t)addr -((size_t)addr % page_size);
    size_t nlen=len+((size_t)addr -naddr);
    return mprotect((void*)naddr, nlen, PROT_READ | PROT_WRITE | PROT_EXEC);
}

byte_t* InjectAlloc(size_t inject_address, u_int32_t len_of_nop, byte_t* source, u_int32_t len_of_source) {
    byte_t* res = (byte_t*)malloc(len_of_source + len_jmp_pattern());
    if (len_of_nop >= len_jmp_pattern()) {
        if (res) {
            mprotect_aligned((void*)inject_address, len_of_nop, PROT_READ | PROT_WRITE | PROT_EXEC);
            memset((byte_t*)inject_address, 0x90, len_of_nop);
            //заполняем его нопами

            add_jmp((byte_t*)inject_address, (size_t)res);
            //добавляем прыжок на выделенную памяти
            
            //тут по хорошему надо вернуть защиту обратно


            mprotect_aligned(res, len_of_source + len_jmp_pattern(), PROT_READ | PROT_WRITE | PROT_EXEC);
            memcpy(res, source, len_of_source);
            //копируем в выделенную память нужные байты
            add_jmp(res + len_of_source, inject_address+ len_jmp_pattern());
            //добавляем прыжок назад (на иструкцию после прыжка) 
        }
    }
    return res;
}




char* getMapName(char* str){
	int ind=0;
	char* res=&str[ind];
	while(str[ind]!='\n'){
		if(str[ind]=='/'){ 
			res=&str[ind+1];
			//printf("%x\n", res);
		}
	ind++;
	}
	str[ind]=0;
	return res;
}

size_t getBaddr(pid_t pid, char* name){
	FILE *fp;
	char buf[1024];
	unsigned long res;
	sprintf(buf, "/proc/%d/maps", pid);
   	if((fp = fopen(buf, "r"))==NULL){
		return 0xffffffff;
	};
	do{
	if(fgets(buf, 1024, fp)==0){
		return 0xffffffff;
	}
	}
	while (strcmp(getMapName(buf), name));
	sscanf(buf, "%lx", &res);
    fclose(fp);
	return (size_t)res;
}
