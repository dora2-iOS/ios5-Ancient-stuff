    .text
    .syntax unified

    .org    0xc94
    .thumb
    .thumb_func
_orig:
    bx  lr

    .org    0x1116
    .thumb
    .thumb_func
_go_cmd:
    bx  lr

    .org    0x123d0
    .thumb
    .thumb_func
sub_4ff123d0:
    bx  lr

    .org    0x17f24
    .thumb
    .thumb_func
_image_load_type:
    bx  lr

    .org    0x216a0
    .thumb
    .thumb_func
_printf:
    bx  lr

    .org	0x28b60             @ shellcode base
    .global	_image_load_func
    .thumb
    .thumb_func
_image_load_func:
    ldr r3, =0x4ff28d00
    mov r0, 0x44000000
    str r0, [r3]

    ldr r2, =0x4ff28d04
    mov r0, #0x00100000
    str r0, [r2]

    mov r0, r3
    mov r1, r2
    ldr r2, =0x69626f62         @ 'ibob'
    bl  _image_load_type        @ ret = _image_type_load(addr, sz, type);

    cbz r0, _boot_start

_back_to_orig:
    adr r0, FAILED_STRING
    bl _printf

    ldr r0, =0x4ff0f79c
    ldr r1, =0x60182000
    str r1, [r0]                @ allow unsigned images

    @ ldr r0, =0x4ff0081a
    @ ldr r1, =0x20002000
    @ str r1, [r0]              @ ignore boot-partition value

    ldr r0, =0x4ff27E48
    mov r1, 0x40000000
    str r1, [r0]                @ go cmd_handler

    ldr r0, =0x4ff00dd4
    mov r1, #0x2500
    strh r1, [r0]               @ enter recovery

    bl  sub_4ff123d0
    b   _orig                   @ return to bootstrap

_boot_start:
    adr r0, JUMP_STRING
    mov r1, 0x44000000
    bl _printf
    
    movs r0, #0
    mov r1, 0x44000000          @ jumpaddr
    mov r2, r0
    b   _go_cmd                 @ jump to the image of jumpaddr using go cmd.


JUMP_STRING:
.asciz "jumping into image at %p\n"

FAILED_STRING:
.asciz "failed to load ibob. start recovery mode.\n"
