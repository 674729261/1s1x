ENTRY(_start)

MEMORY {
  prog (rx)  : ORIGIN = _pmem_start, LENGTH = 4K
  sram (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS
{
  . = ORIGIN(prog);

  .text : {
    *(entry)
    *(.text*)
  } > prog AT> prog

  .rodata : {
    *(.rodata*)
  } > prog AT> prog


  . = ORIGIN(sram);

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
