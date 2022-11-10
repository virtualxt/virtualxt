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

#include "vxtp.h"

#include <string.h>

#ifndef VXTP_PPI_TONE_VOLUME
    #define VXTP_PPI_TONE_VOLUME 8192
#endif

#define MAX_EVENTS 16
#define INT64 long long

#define DATA_READY 1
#define COMMAND_READY 2

#define DISABLE_KEYBOARD_CMD 0xAD
#define ENABLE_KEYBOARD_CMD 0xAE
#define READ_INPUT_CMD 0xD0
#define WRITE_OUTPUT_CMD 0xD1

VXT_PIREPHERAL(ppi, {
	vxt_byte data_port;
    vxt_byte command_port;
    vxt_byte port_61;
    vxt_byte xt_switches;

    vxt_byte command;
    vxt_byte a20_data;
    vxt_byte refresh_request;
    bool keyboard_enable;

    int queue_size;
    enum vxtp_scancode queue[MAX_EVENTS];

    INT64 spk_sample_index;
	bool spk_enabled;

    void (*speaker_callback)(struct vxt_pirepheral*,double,void*);
    void *speaker_callback_data;

    struct vxt_pirepheral *pit;
})

static vxt_byte in(struct vxt_pirepheral *p, vxt_word port) {
    VXT_DEC_DEVICE(c, ppi, p);
	switch (port) {
        case 0x60:
            c->command_port = 0;
            return c->data_port;
        case 0x61:
            #ifdef VXT_CPU_286
                c->refresh_request ^= 0x10;
            #endif
            return (c->port_61 & 0xEF) | c->refresh_request;
        case 0x62:
            return c->xt_switches;
        case 0x64:
            return c->command_port;
	}
	return 0;
}

static void out(struct vxt_pirepheral *p, vxt_word port, vxt_byte data) {
    VXT_DEC_DEVICE(c, ppi, p);
    switch (port) {
        #ifdef VXT_CPU_286
            case 0x60:
                if (!c->keyboard_enable && (c->command == WRITE_OUTPUT_CMD)) {
                    c->command = 0;
                    c->a20_data = data;
                    vxt_system_set_a20(VXT_GET_SYSTEM(ppi, p), (data & 2) != 0);
                }
                break;
        #endif
        case 0x61:
        {
            c->port_61 = data;
            bool enabled = (data & 3) == 3;
            if (enabled != c->spk_enabled) {
                c->spk_enabled = enabled;
                c->spk_sample_index = 0;

                if (c->speaker_callback)
                    c->speaker_callback(p, enabled ? vxtp_pit_get_frequency(c->pit, 2) : 0.0, c->speaker_callback_data);
            }
            break;
        }
        #ifdef VXT_CPU_286
            case 0x64:
                c->command = data;
                switch (data) {
                    case ENABLE_KEYBOARD_CMD:
                        c->keyboard_enable = true;
                        break;
                    case DISABLE_KEYBOARD_CMD:
                        c->keyboard_enable = false;
                        break;
                }
                break;
        #endif
    }
}

static vxt_error install(vxt_system *s, struct vxt_pirepheral *p) {
    vxt_system_install_io(s, p, 0x60, 0x62);
    vxt_system_install_io_at(s, p, 0x64);
    return VXT_NO_ERROR;
}

static vxt_error step(struct vxt_pirepheral *p, int cycles) {
    (void)cycles;
    VXT_DEC_DEVICE(c, ppi, p);
    c->command_port |= COMMAND_READY;
    if (c->keyboard_enable) {
        if (c->queue_size && !(c->command_port & DATA_READY)) {
            c->command_port |= DATA_READY;
            c->data_port = (vxt_byte)c->queue[0];
            memmove(c->queue, &c->queue[1], --c->queue_size);
            vxt_system_interrupt(VXT_GET_SYSTEM(ppi, p), 1);
        }
    } else if (c->command == READ_INPUT_CMD) {
        c->command_port |= DATA_READY;
        c->data_port = c->a20_data;
    }
    return VXT_NO_ERROR;
}

static vxt_error reset(struct vxt_pirepheral *p) {
    VXT_DEC_DEVICE(c, ppi, p);
    c->command_port = c->data_port = 0;
    c->port_61 = 14;

	c->spk_sample_index = 0;
	c->spk_enabled = false;

    if (c->speaker_callback)
        c->speaker_callback(p, 0.0, c->speaker_callback_data);

    c->keyboard_enable = true;
    c->queue_size = 0;

    c->a20_data = 0;
    vxt_system_set_a20(VXT_GET_SYSTEM(ppi, p), false);
    return VXT_NO_ERROR;
}

static vxt_error destroy(struct vxt_pirepheral *p) {
    vxt_system_allocator(VXT_GET_SYSTEM(ppi, p))(p, 0);
    return VXT_NO_ERROR;
}

static const char *name(struct vxt_pirepheral *p) {
    (void)p; return "PPI (Intel 8255)";
}

struct vxt_pirepheral *vxtp_ppi_create(vxt_allocator *alloc, struct vxt_pirepheral *pit) {
    struct vxt_pirepheral *p = (struct vxt_pirepheral*)alloc(NULL, VXT_PIREPHERAL_SIZE(ppi));
    vxt_memclear(p, VXT_PIREPHERAL_SIZE(ppi));
    VXT_DEC_DEVICE(c, ppi, p);

    // Reference: https://bochs.sourceforge.io/techspec/PORTS.LST
    //            https://github.com/skiselev/8088_bios/blob/master/bios.asm
    c->xt_switches = 0x2; // CGA video bits.

    c->pit = pit;

    p->install = &install;
    p->destroy = &destroy;
    p->reset = &reset;
    p->step = &step;
    p->name = &name;
    p->io.in = &in;
    p->io.out = &out;
    return p;
}

bool vxtp_ppi_key_event(struct vxt_pirepheral *p, enum vxtp_scancode key, bool force) {
    VXT_DEC_DEVICE(c, ppi, p);
    if (c->queue_size < MAX_EVENTS) {
        c->queue[c->queue_size++] = key;
        return true;
    } else if (force) {
        c->queue[MAX_EVENTS - 1] = key;
    }
    return false;
}

void vxtp_ppi_set_speaker_callback(struct vxt_pirepheral *p, void (*f)(struct vxt_pirepheral*,double,void*), void *userdata) {
    VXT_DEC_DEVICE(c, ppi, p);
    c->speaker_callback = f;
    c->speaker_callback_data = userdata;
}

vxt_int16 vxtp_ppi_generate_sample(struct vxt_pirepheral *p, int freq) {
    VXT_DEC_DEVICE(c, ppi, p);

    double tone_hz = vxtp_pit_get_frequency(c->pit, 2);
    if (!c->spk_enabled || (tone_hz <= 0.0))
        return 0;

    INT64 square_wave_period = (INT64)((double)freq / tone_hz);
    INT64 half_square_wave_period = square_wave_period / 2;

    if (!half_square_wave_period)
        return 0;
    return ((++c->spk_sample_index / half_square_wave_period) % 2) ? VXTP_PPI_TONE_VOLUME : -VXTP_PPI_TONE_VOLUME;
}

void vxtp_ppi_set_xt_switches(struct vxt_pirepheral *p, vxt_byte data) {
    (VXT_GET_DEVICE(ppi, p))->xt_switches = data;
}
