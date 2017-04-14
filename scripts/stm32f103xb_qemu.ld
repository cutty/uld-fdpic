ENTRY(vector_reset)

MEMORY
{
  FLASH          (rw)   : ORIGIN = 0x08000000, LENGTH = 127K
  ULD_PDATA      (rw)   : ORIGIN = 0x0801FC00, LENGTH = 1K - 4
  ULD_PSTORE_PTR (rw)   : ORIGIN = 0x0801FFFC, LENGTH = 4
  RAM            (rwx)  : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
  .vectors        : { KEEP (*(.vectors)) } >FLASH
  .sw_vectors     : { KEEP (*(.sw_vectors)) } >FLASH
  .interp         : { *(.interp) } >FLASH
  .note.ABI-tag   : { *(.note.ABI-tag) } >FLASH
  .hash           : { *(.hash) } >FLASH
  .dynsym         : { *(.dynsym) } >FLASH
  .dynstr         : { *(.dynstr) } >FLASH
  .version        : { *(.version) } >FLASH
  .version_d      : { *(.version_d) } >FLASH
  .version_r      : { *(.version_r) } >FLASH
  .rel.dyn        : { *(.rel.dyn) } >FLASH
  .rela.dyn       : { *(.rela.dyn) } >FLASH
  .rel.plt        : { *(.rel.plt) } >FLASH
  .rela.plt       : { *(.rela.plt) } >FLASH
  .init           : { KEEP (*(.init)) } >FLASH
  .plt            : { *(.plt) } >FLASH
  .text           : { *(.text .text.*) } >FLASH
  .fini           : { KEEP (*(.fini)) } >FLASH
  PROVIDE(__etext = .);
  PROVIDE(_etext = .);
  PROVIDE(etext = .);

  .rodata         : { *(.rodata .rodata.*) }

  .preinit_array  :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >FLASH
  .init_array     :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >FLASH
  .fini_array     :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >FLASH

  .dynamic        : { *(.dynamic) } >FLASH

  .got            : { *(.got) } >FLASH
  .got.plt        : { *(.got.plt) } >FLASH

  .uld_rt         :
  {
    . = ALIGN(4);
    _s_uld_rt = .;
    KEEP(*(.uld_rt.rt_tbl_ptr))
    KEEP(*(.uld_rt.mem_vectors))
    _e_mem_vector_table = .;
    *(.uld_rt.bss)
    *(.uld_rt.bss*)
    *(.uld_rt.resv)
    *(.uld_rt.resv*)
    . = ALIGN(4);
    _e_uld_rt = .;
  } > RAM

  _sidata = LOADADDR(.data);
  .data           :
  {
    __data_start = .;
    _sdata = .;
    *(.data .data.*)
    . = ALIGN(4);
  } >RAM AT> FLASH
  _edata = .;
  PROVIDE(edata = .);

  . = LOADADDR(.data) + SIZEOF(.data);
  .fs_table :
  {
    . = ALIGN(4);
    _s_fs_table = .;
    KEEP(*(.fs_table))
    KEEP(*(.fs_table*))
    . = ALIGN(4);
    _e_fs_table = .;
  } >FLASH
  _fs_table_size = SIZEOF(.fs_table);

  .files ALIGN(8) :
  {
    . = ALIGN(8);
    _s_files = .;
    KEEP(*(.files))
    KEEP(*(.files*))
    . = ALIGN(8);
  } >FLASH

  .uld_pdata :
  {
    . = ALIGN(4);
    _s_uld_pdata = .;
    KEEP(*(.uld_pdata))
    KEEP(*(.uld_pdata*))
    . = ALIGN(4);
    _e_uld_pdata = .;
  } >ULD_PDATA

  .uld_pstore_ptr :
  {
    KEEP(*(.uld_pstore_ptr))
    KEEP(*(.uld_pstore_ptr*))
  } >ULD_PSTORE_PTR

  . = ADDR(.data) + SIZEOF(.data);
  . = ALIGN(4);
  __bss_start = .;
  __bss_start__ = .;
  .bss            :
  {
    _sbss = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(. != 0 ? 32 / 8 : 1);
    _ebss = .;
  } >RAM
  __bss_end__ = .;
  _bss_end__ = .;

  /* Set the location counter to the end of the last section in flash. */
  . = ADDR(.files) + SIZEOF(.files);
  . = ALIGN(4);
  __end = .;
  _end = .;
  PROVIDE(end = .);

  /DISCARD/       :
  {
  }

  _eflash = ORIGIN(FLASH) + LENGTH(FLASH);
  _files_size = _eflash - _s_files;
  _estack = ORIGIN(RAM) + LENGTH(RAM);
}
