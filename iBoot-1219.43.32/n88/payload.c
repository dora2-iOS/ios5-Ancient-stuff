/* payload.c - payload for ios5 iboot */

#include <stdio.h>

/* asm.h - from iloader made by xerub */
#define INSN2_MOV_R0_0__STR_R0_R3    0x60182000
#define INSN2_MOV_R0_0__MOV_R0_0     0x20002000

#define INSNT_MOV_R_I(d, i)        (0x2000 | (((d) & 7) << 8) | ((i) & 0xFF))

/*
 * Device Informations
 * DEVICE:  iPhone2,1
 * VERSION: 5.0.1
 * BUILD:   9A405
 */

#define BASE_ADDR           0x4ff00000
#define LOADADDR            0x40000000
#define JUMPADDR            0x44000000
#define IMAGE_SIZE          0x2b000
#define GPIO_BASE           0x83000000

#define HOME_BUTTON         0x2d8

/* iBoot functions */
#define PRINTF              (0x216a0 + 1)
#define BCOPY               (0x219e8)
#define CLEAR_INSN_CACHE    (0x15008)
/* not used */
#define IMAGE_LOAD_TYPE     (0x17f24 + 1)

/* offsets */
#define VERIFY_SHSH         (0xf79c)
#define NVRAM_BOOTPARTITION (0x81a)
#define NVRAM_AUTOBOOT      (0xdd4)
#define GO_CMD_HANDLER      (0x27E48)

/* payload */
#define PAYLOAD_START       (0x2d57c)
#define NETTOYEUR           (PAYLOAD_START + 0x100)
#define PREPARE_AND_JUMP    (PAYLOAD_START + 0x300 + 1)
#define SHELLCODE_START     (PAYLOAD_START + 0x400)
#define SHELLCODE_BASE      (0x28b60)
#define SHELLCODE_SIZE      (0xbc)
#define HOOK_ADDR           (0xc90)

typedef void (*printf_t)(const char *fmt, ...);
typedef void (*bcopy_t)(const void *src, void *dest, size_t n);
typedef void (*clear_insn_cache_t)(void);

typedef void (*nettoyeur_t)(void);
typedef void (*prepare_and_jump_t)(void);

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

/*  make_bl - from iloader made by xerub
 *  but not used :/
uint32_t make_bl(int blx, int pos, int tgt)
{
    int delta;
    unsigned short pfx;
    unsigned short sfx;

    unsigned int omask = 0xF800;
    unsigned int amask = 0x7FF;

    if (blx) { // XXX untested
        omask = 0xE800;
        amask = 0x7FE;
        pos &= ~3;
    }

    delta = tgt - pos - 4; // range: 0x400000
    pfx = 0xF000 | ((delta >> 12) & 0x7FF);
    sfx =  omask | ((delta >>  1) & amask);

    return (unsigned int)pfx | ((unsigned int)sfx << 16);
}
*/

int main() {

    /*---- Ancient-iBoot-Fun Payload by JonathanSeals & Ralph0045 ----*/
    clear_insn_cache_t clear_insn_cache = (clear_insn_cache_t)(BASE_ADDR+CLEAR_INSN_CACHE);
    printf_t printf = (printf_t)(BASE_ADDR + PRINTF);
    bcopy_t bcopy = (bcopy_t)(BASE_ADDR + BCOPY);
    nettoyeur_t nettoyeur = (nettoyeur_t)(BASE_ADDR + NETTOYEUR);
    prepare_and_jump_t prepare_and_jump = (prepare_and_jump_t)(BASE_ADDR + PREPARE_AND_JUMP);
    int val;
    
    clear_insn_cache();
    printf("Hacked\n");
    
    bcopy((void*)BASE_ADDR, (void*)JUMPADDR, (uint32_t)IMAGE_SIZE);
    
    nettoyeur();
    
    /* iBoot Patches */
    *(uint32_t*)(JUMPADDR + VERIFY_SHSH) = INSN2_MOV_R0_0__STR_R0_R3;   /* allow unsigned images */
    
    /* ignore boot-partition value
    *(uint32_t*)(JUMPADDR + NVRAM_BOOTPARTITION) = INSN2_MOV_R0_0__MOV_R0_0;
     */
    /*---- end ----*/
    
    /*--- determine if the home button is pressed or not. ---*/
    val = (*(uint32_t*)(GPIO_BASE + HOME_BUTTON) & 1);
    printf("patch mode: %d\n", val);
    
    if(val == 1){
        /* --- Patch to put the device in recovery mode ---
         *
         *  If the Home button was pressed, enable the patch that puts the device in recovery mode and restart iBoot.
         *  - force auto-boot to false: Forces the device into recovery mode.
         *  - set the go cmd handler to loadaddr: Allows to run payload with go command via irecovery.
         */
        
        /* force auto-boot to false */
        *(uint16_t*)(JUMPADDR + NVRAM_AUTOBOOT) = INSNT_MOV_R_I(5, 0); // r5 = sub_4ff0ce04("auto-boot", 0x0) -> r5 = 0x0;
        /* set the go cmd handler to loadaddr */
        *(uint32_t*)(JUMPADDR + GO_CMD_HANDLER) = LOADADDR;
    } else {
        /* --- jump mode  ---
         *
         *  After restarting the iBoot, run shellcode.S to jump the (new)iBoot with the 'ibob' type.
         *  - copy the payload to SHELLCODE_BASE: Use _bcopy() to write the payload
         *  - payload hook: Hook a function to run shellcode
         */
        
        /* copy the payload to SHELLCODE_BASE */
        bcopy((void*)(BASE_ADDR + SHELLCODE_START), (void*)(JUMPADDR + SHELLCODE_BASE), (uint32_t)SHELLCODE_SIZE);
        /* payload hook */
        *(uint32_t*)(JUMPADDR + HOOK_ADDR) = 0xff66f027;    /* make_bl(0, HOOK_ADDR, SHELLCODE_BASE) */
    }
    
    printf("jumping into image at %p\n", JUMPADDR);
    
    prepare_and_jump();
    
    return 0;
}
