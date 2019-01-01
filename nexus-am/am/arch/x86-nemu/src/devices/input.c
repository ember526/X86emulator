#include <am.h>
#include <x86.h>
#include <amdev.h>
#include "klib.h"
size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
	case _DEVREG_INPUT_KBD: {
		_KbdReg *kbd = (_KbdReg *)buf;
		int press = inl(0x60);
		
		kbd->keycode = press;
		if(press != _KEY_NONE)
			kbd->keydown = !(kbd->keydown);
		return sizeof(_KbdReg);
	}
  }
  return 0;
}
