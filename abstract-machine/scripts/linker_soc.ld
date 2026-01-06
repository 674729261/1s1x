ENTRY(_start)

MEMORY {
  prog (rx)  : ORIGIN = _pmem_start, LENGTH = _pmem_size
  sram (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
  psram (rwx) : ORIGIN = 0x80000000, LENGTH = 4M
}

SECTIONS
{
  . = ORIGIN(prog);

  .text : {
    *(entry)
    *(.bootloader)
  } > prog AT> prog


  . = ORIGIN(sram);

  .rodata : {
    __prog_start = .;
    *(.text*)
    *(.rodata*)
    __prog_end = .;
  } > sram AT> prog

  __prog_load_start = LOADADDR(.rodata);
  __prog_load_end   = __prog_load_start + SIZEOF(.rodata);

  .data : {
  __data_start = .;
  *(.data*)
  *(.sdata*)
  __data_end = .;
  } > sram AT> prog

  __data_load_start = LOADADDR(.data);
  __data_load_end   = __data_load_start + SIZEOF(.data);

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
    __bss_end = .;
  } > sram

  . = ALIGN(32);
  _heap_start = .;
  _stack_pointer = ORIGIN(sram) + LENGTH(sram);
}
