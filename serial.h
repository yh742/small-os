#ifndef SERIAL_H_
#define SERIAL_H_

#define COM1 0x3F8
#define COM2 0x2F8

void serial_init(unsigned short com, unsigned short divisor);
void serial_write(unsigned short com, char a);

#endif
