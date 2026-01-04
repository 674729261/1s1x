ENTRY(_start)

MEMORY {
  mrom (rx)  : ORIGIN = 0x20000000, LENGTH = 4K
  sram (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS
{
  . = ORIGIN(mrom);

  .text : {
    *(entry)
    *(.text*)
  } > mrom AT> mrom

  .rodata : {
    *(.rodata*)
  } > mrom AT> mrom


  . = ORIGIN(sram);

  .data : {
  __data_start = .;
  *(.data*)
  *(.sdata*)
  *(.sdata.*)
  __data_end = .;
  } > sram AT> mrom

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
