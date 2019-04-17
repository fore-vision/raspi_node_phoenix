
#ifndef __INVENTORY_H_
#define __INVENTORY_H_

#include <napi.h>

void init_inventory(Napi::Env env,Napi::Object exports);
enum {
		 IDLE = 0,
		 OPEN,
		INVENTORY};

#endif


