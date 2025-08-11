/*
Copyright (C) 2022 Victor Chavez
This file is part of lwIP Arduino
lwIP Arduino is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
IOLink Device Generator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with IOLink Device Generator.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _ARDUINO_SYS_ARCH_H
#include <stdint.h>
/*

TODO: If using this Arduino lwIP port library 
with an OS, define the following datatypes

#define sys_mutex_t
#define sys_sem_t
#define sys_mbox_t
#define sys_thread_t

In addition sys_arch.c must be provided 

check 
https://www.nongnu.org/lwip/2_0_x/group__sys__os.html
https://www.nongnu.org/lwip/2_0_x/group__sys__time.html
https://www.nongnu.org/lwip/2_0_x/group__sys__prot.html

and this for summary
https://www.nongnu.org/lwip/2_0_x/group__sys__layer.html


*/
#define LWIP_PROVIDE_ERRNO

void sys_printf(const char *format, ...);

#endif //_ARDUINO_SYS_ARCH_H