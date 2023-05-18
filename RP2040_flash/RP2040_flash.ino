/*
 * Copyright 2022 Jarrod A. Smith (MakerMatrix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.* Testing/examples for reading/writing flash on the RP2040.
 * 
 * Resources I drew from:
 * https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__flash.html
 * https://kevinboone.me/picoflash.html
 * 
 * You must disable interrupts while programming flash.  This requires you to
 * include hardware/sync.h and use save_and_disable_interrupts(), then restore_interrupts()
 * 
 * You must include hardware/flash.h for the erase/program functions.
 * Interesting macros defined in flash.h:
 * FLASH_PAGE_SIZE, FLASH_SECTOR_SIZE, FLASH_BLOCK_SIZE
 * functions:
 * flash_range_erase() 
 * flash_range_program()
 * 
 * pico.h gets included from there.  Interesting macros with respect to flash there:
 * PICO_FLASH_SIZE_BYTES
 * 
 * and from addresses.h:
 * XIP_BASE - the ARM address of the end of the onboard 256KB RAM.
 * On RP2040 this is 0x10000000 and the first byte of flash is the next address.  
 * 
 * Note that the flash_range_erase() and flash_range_program() functions from flash.h
 * do not use ARM address space, only offsets into flash (i.e. don't count the
 * first 0x10000000 bytes of address space representing the RAM.
 * 
 * But to read, point a pointer directly to a memory-mapped address somewhere in the
 * flash (so, use XIP_BASE to get past RAM) and read directly from it.
*/

extern "C" {
  #include <hardware/sync.h>
  #include <hardware/flash.h>
};

// Set the target offest to the last sector of flash
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

int buf[FLASH_PAGE_SIZE/sizeof(int)];  // One page buffer of ints
int *p, addr;
unsigned int page; // prevent comparison of unsigned and signed int
int first_empty_page = -1;

void setup() {
  Serial.begin(9600);
  while(!Serial) ;
  Serial.println("FLASH_PAGE_SIZE = " + String(FLASH_PAGE_SIZE, DEC));
  Serial.println("FLASH_SECTOR_SIZE = " + String(FLASH_SECTOR_SIZE,DEC));
  Serial.println("FLASH_BLOCK_SIZE = " + String(FLASH_BLOCK_SIZE, DEC));
  Serial.println("PICO_FLASH_SIZE_BYTES = " + String(PICO_FLASH_SIZE_BYTES, DEC));
  Serial.println("XIP_BASE = 0x" + String(XIP_BASE, HEX));
  
  // Read the flash using memory-mapped addresses
  // For that we must skip over the XIP_BASE worth of RAM
  // int addr = FLASH_TARGET_OFFSET + XIP_BASE;
  for(page = 0; page < FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE; page++){
    addr = XIP_BASE + FLASH_TARGET_OFFSET + (page * FLASH_PAGE_SIZE);
    p = (int *)addr;
    Serial.print("First four bytes of page " + String(page, DEC) );
    Serial.print("( at 0x" + (String(int(p), HEX)) + ") = ");
    Serial.println(*p);
    if( *p == -1 && first_empty_page < 0){
      first_empty_page = page;
      Serial.println("First empty page is #" + String(first_empty_page, DEC));
    }
  }
 
 *buf = 123456;

  if (first_empty_page < 0){
    Serial.println("Full sector, erasing...");
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    first_empty_page = 0;
    restore_interrupts (ints);
  }
  Serial.println("Writing to page #" + String(first_empty_page, DEC));
  uint32_t ints = save_and_disable_interrupts();
  flash_range_program(FLASH_TARGET_OFFSET + (first_empty_page*FLASH_PAGE_SIZE), (uint8_t *)buf, FLASH_PAGE_SIZE);
  restore_interrupts (ints);
}


void loop() {
}
