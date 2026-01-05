ENTRY(_start)

MEMORY {
  flash (rx)  : ORIGIN = 0x30000000, LENGTH = 16K
  sram (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS
{
  . = ORIGIN(flash);

  .text : {
    *(entry)
    *(.text*)
  } > flash AT> flash

  .rodata : {
    *(.rodata*)
  } > flash AT> flash


  . = ORIGIN(sram);

  .data : {
  __data_start = .;
  *(.data*)
  *(.sdata*)
  __data_end = .;
  } > sram AT> flash

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
