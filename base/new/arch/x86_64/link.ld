/* 
 * File: link.ld
 * ld script to make the x86_64 kernel image.
 * Base:
 *     0x30000000
 * Entry point: 
 *     0x30001000
 *     Kernel entry point in: 
 *     entrance/warden/unit0/x86_64/head_64.asm. 
 * History:
 *     2021 - Created by Fred Nora.
 */
/* See: https://wiki.osdev.org/Linker_Scripts */
/* See: https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
/* 64-bit ELF format for the x86-64 architecture. */

OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_kernel_begin)

SECTIONS 
{
    . = 0x30001000;
    .text 0x30001000:
    {
        _kernel_begin = .;   
        _start_of_kernel_image = .;
        
        _code_begin = .;
        *(.head_x86_64)  *(.text) 
        _code_end = .; 
    }

    . = ALIGN(4096);
    .rodata :
    {
        _rodata_begin = .;
        *(.rdata)  *(.rodata) 
        _rodata_end = .;
    }

    . = ALIGN(4096);
    .data :  
    { 
        _data_begin = .;
        *(.data) 
        _data_end = .; 
    }
    
    . = ALIGN(4096);
    .bss : 
    { 
        _bss_begin = .; 
        *(.bss)  *(COMMON)
        _bss_end = .; 
    }

    _kernel_end = .;
    _end_of_kernel_image = .; 
}







