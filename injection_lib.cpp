#include "util.h"
#include <stdio.h>
#include <cstdlib>
#include <ctime>


class Well;

typedef void(__cdecl* move_fnc_t)(Well*);
move_fnc_t wellMoveRight; 
move_fnc_t wellMoveLeft;


typedef void(__cdecl* rotate_fnc_t)(Well*, u_int8_t);
rotate_fnc_t wellRotate; 


void countExternalFunctions(size_t baddr){
	wellMoveRight=(move_fnc_t)(baddr+0x0BA800);
	wellMoveLeft=(move_fnc_t)(baddr+0x0BA6F0);
	wellRotate=(rotate_fnc_t)(baddr+0x0BA920);
}


struct WellEvent {
	enum class Type: u_int8_t {
        PIECE_LOCKED,
        PIECE_ROTATED,
        PIECE_MOVED_HORIZONTALLY,
        NEXT_REQUESTED,
        HOLD_REQUESTED,
        LINE_CLEAR_ANIMATION_START,
        LINE_CLEAR,
        TSPIN_DETECTED,
        MINI_TSPIN_DETECTED,
        SOFTDROPPED,
        HARDDROPPED,
        GAME_OVER,
    };
	union
	{
		CREATE_MEMBER(Type, eventType, 0x0);
	};
	
};

struct Well{
	void moveRight(){
		//printf("Rigt\n");
		wellMoveRight(this);
	}

	void moveLeft(){
		//printf("Left\n");
		wellMoveLeft(this);
	}

	void rotate(u_int8_t direction){
		direction &=1;
		wellRotate(this, direction);
	}
};





void eventHandler(WellEvent* event){
	printf("Called event_%hhu\n", event->eventType);
}



size_t InjectNotifyFun(size_t baddr){
	size_t note_addr = 0xBA51E;
	byte_t code_inj[]={
        PUSH_R8_15,									//0
		PUSH_RAX_RDI, 								//16
		0x48, 0xb8, 0x77, 0x66, 0x55,               //24
		0x44, 0x33, 0x22, 0x11, 0x00,               //29  movabs rax, 0x11223344556677 */
		0x48, 0x89, 0xf7,                           //34  mov rdi, rsi */
 		0xff, 0xd0,                           		//37  call rax */
		POP_RDI_RAX,								//39
		POP_R15_8,									//47
		//lost bytes
		0x48, 0x8b, 0xbf, 0x78, 0x19, 0x00, 0x00,   //63  mov rdi, qword [rdi + 0x1978] */
        0x64, 0x48, 0x8b, 0x04, 0x25, 				//70
		0x28, 0x00, 0x00, 0x00,  					//75  mov rax, qword fs:[0x28] */
		//
	};
	size_t paddr = baddr+note_addr;
	size_t res = (size_t)InjectAlloc(paddr, 16, code_inj, sizeof(code_inj));
	size_t buf = (size_t)&eventHandler;
	*((size_t*)(res+24+2))=buf;
	return res;
}

void randomPos(Well* well){
    int pos = (std::rand()%10)-5;
	
	for(int i=0; i<pos; i++)
		well->moveRight();
	for(int i=0; i>pos; i--)
		well->moveLeft();

	int rotates = std::rand()%4;
	for(int i=0; i<rotates; i++)
		well->rotate(0);
}

size_t InjectAddPiece(size_t baddr){
	size_t addpiece_addr = 0x0BB004;
	byte_t code_inj[]={
		//lost bytes
		0xbe, 0x03, 0x00, 0x00, 0x00,         		//0 mov esi, 3 */
 		0xba, 0x14, 0x00, 0x00, 0x00,         		//5 mov edx, 0x14 */
 		0x66, 0x41, 0x89, 0x87, 0x10, 				//10
		0x19, 0x00, 0x00,                           //15 mov word [r15 + 0x1910], ax */
		//
		PUSH_R8_15,									//18
		PUSH_RAX_RDI, 								//34
		0x48, 0xb8, 0x77, 0x66, 0x55,               //42
		0x44, 0x33, 0x22, 0x11, 0x00,               //47  movabs rax, 0x11223344556677 */
		0x4c, 0x89, 0xff,                           //52 mov rdi, r15 */
 		0xff, 0xd0,                           		//55  call rax */
		POP_RDI_RAX,								//57
		POP_R15_8,									//65
	};
	size_t paddr = baddr+addpiece_addr;
	size_t res = (size_t)InjectAlloc(paddr, 18, code_inj, sizeof(code_inj));
	size_t buf = (size_t)&randomPos;
	*((size_t*)(res+42+2))=buf;
	return res;
}

void hello()
{
	printf("I just got loaded cpp\n And my pid is %d\n", getpid());
}

__attribute__((constructor))
void loadMsg()
{
	std::srand(std::time(nullptr));
	
	hello();
	unsigned long baddr=getBaddr(getpid(), "openblok.elf");
	printf("Baddr=%lx\n", baddr);
	countExternalFunctions(baddr);

	InjectNotifyFun(baddr);
	InjectAddPiece(baddr);
}
