ENTRY(_start)

MEMORY {
  sram (rw) : ORIGIN = 0x0f000000, LENGTH = 8K
  prog (rx)  : ORIGIN = 0x30000000, LENGTH = _pmem_size
  psram (rwx) : ORIGIN = 0x80000000, LENGTH = 2M
  psram_heap (rw) : ORIGIN = 0x80200000, LENGTH = 2M

}

SECTIONS
{
  . = ORIGIN(prog);

  .text : {
    *(entry)
    *(.bootloader)
  } > prog AT> prog

  . = ORIGIN(psram);  
  
  .rodata : {
    __prog_start = .;
    *(.text*)
    *(.rodata*)
    __prog_end = .;
  } > psram AT> prog

  __prog_load_start = LOADADDR(.rodata);
  __prog_load_end   = __prog_load_start + SIZEOF(.rodata);


  .data : {
  __data_start = .;
  *(.data*)
  *(.sdata*)
  __data_end = .;
  } > psram AT> prog

  __data_load_start = LOADADDR(.data);
  __data_load_end   = __data_load_start + SIZEOF(.data);

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
    __bss_end = .;
  } > psram

  _heap_start = ORIGIN(psram_heap);
  _heap_end = ORIGIN(psram_heap) + LENGTH(psram_heap);
  _stack_pointer = ORIGIN(sram) + LENGTH(sram);
}
