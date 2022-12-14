// Copyright (c) 2019-2022 Andreas T Jonsson <mail@andreasjonsson.se>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software in
//    a product, an acknowledgment (see the following) in the product
//    documentation is required.
//
//    Portions Copyright (c) 2019-2022 Andreas T Jonsson <mail@andreasjonsson.se>
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.

#include <vxt/vxt.h>
#include <vxt/vxtu.h>
#include <printf.h>

#include "uart.h"
#include "video.h"

int ENTRY(int argc, char *argv[]) {
	(void)argc; (void)argv;

	uart_init();
	video_init(SCREEN_WIDTH, SCREEN_HEIGHT);

	printf("hello world!\n");

	//vxt_dword col = 0;
	for (;;) {
		for (int y = 0; y < video_height(); y++) {
			for (int x = 0; x < video_width(); x++) {
				video_put_pixel(x, y, 0x555555);
			}
		}
	}
	return 0;
}
