/* n88ap - iBoot-1219.43.32 */
#define BASE_ADDR 0x4ff00000
#define TEXT_SIZE 0xfeedface /* not used */
#define PAYLOAD_ADDR 0x4ff2d57d
#define PRINTF (0x216a0+0x1)
#define BDEV_READ (0xc3f0+0x1)
#define PAYLOAD_BASEADDR (IBOOT_BASEADDR+0x28E58+0x1) /* Reliance... */
#define CLEAR_INSN_CACHE (0x15008)
