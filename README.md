# RP2040_flash_programming
A simple Arduino IDE example for programming the RP2040 onboard flash, including rudimentary wear-leveling logic.  This is explained in detail [on my blog](https://www.makermatrix.com/blog/read-and-write-data-with-the-pi-pico-onboard-flash).


Enable your RP2040-based board in the board manager (e.g. Pi Pico).  Plug your board into your computer, and select the detected commuincation port under Tools->Port.
Download this sketch and program it to the board.

The code will block until you open the serial monitor.  It will then report some values of the relevant macros,
traverse the last sector of the flash one page at a time, reporting the value of the first four bytes of each page, cast as int.
It will then write an int to the first four bytes of the first empty page. It will do this one time, each time it executes, until the sector is full.
It will then erase the sector and start over at the beginning.
