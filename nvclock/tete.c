/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * Copyright(C) 2001-2008 Roderick Colenbrander
 * Copyright(C) 2012 Roman Dobosz
 *
 * site: http://NVClock.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include "nvclock.h"


void unload_nvclock() {
    /* Free the config file structure */
    if(nvclock.cfg) destroy(&nvclock.cfg);
}

int get_GPU_temp() {
    nvclock.dpy = NULL;

    if(!init_nvclock()) return 0;

    atexit(unload_nvclock);

    if(!set_card(0)) return 0;

    if(!(nv_card->caps & (GPU_TEMP_MONITORING))) return 0;

    return nv_card->get_gpu_temp(nv_card->sensor);
}

int main(){
    printf("%dC\n", get_temp());
}
