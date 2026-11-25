/*
  mc5s3dp_xhat.c - board specific code for the MC5S3DP-XHAT

  Part of grblHAL

  grblHAL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  grblHAL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with grblHAL. If not, see <http://www.gnu.org/licenses/>.
*/

/*
  The vacuum pump and the valves are plain aux outputs (M64/M65), and grblHAL leaves aux outputs as they are on a
  soft reset, E-stop, alarm and sleep. A spindle or coolant output would be switched off in all of these, so do the
  same here for the load outputs DQ_0..DQ_5 (OD0..OD5). The status LED (the last aux output) is left alone.
*/

#include "driver.h"

#if defined(BOARD_MC5S3DP_XHAT)

#include "grbl/ioports.h"

// Aux outputs DQ_0..DQ_5 that are not claimed by the spindle or coolant. They are the first ports, P0..P(n-1).
#define XHAT_SPINDLE_CLAIMS (XHAT_SPINDLE == 2 ? 3 : (XHAT_SPINDLE == 1 ? 1 : 0))
#define XHAT_COOLANT_CLAIMS (((COOLANT_ENABLE & COOLANT_FLOOD) ? 1 : 0) + ((COOLANT_ENABLE & COOLANT_MIST) ? 1 : 0))
#define XHAT_LOAD_OUTPUTS   (6 - XHAT_SPINDLE_CLAIMS - XHAT_COOLANT_CLAIMS)

static on_reset_ptr on_reset;
static on_state_change_ptr on_state_change;

static void outputs_off (void)
{
    uint_fast8_t port;

    for(port = 0; port < XHAT_LOAD_OUTPUTS; port++)
        ioport_digital_out(port, 0);
}

static void onReset (void)
{
    outputs_off();

    if(on_reset)
        on_reset();
}

static void onStateChanged (sys_state_t state)
{
    if(state & (STATE_ALARM|STATE_ESTOP|STATE_SLEEP))
        outputs_off();

    if(on_state_change)
        on_state_change(state);
}

void board_init (void)
{
    on_reset = grbl.on_reset;
    grbl.on_reset = onReset;

    on_state_change = grbl.on_state_change;
    grbl.on_state_change = onStateChanged;
}

#endif // BOARD_MC5S3DP_XHAT
