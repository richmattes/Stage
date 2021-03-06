/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id$
 */
// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Power interface
- PLAYER_POWER_DATA_STATE
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
using namespace Stg;

InterfacePower::InterfacePower( player_devaddr_t addr, 
				  StgDriver* driver,
				  ConfigFile* cf,
				  int section )
  : InterfaceModel( addr, driver, cf, section, "power" )
{ 
  // nothing to see here. move along.
}

void InterfacePower::Publish( void )
{
  Model* mod = this->mod;
  PowerPack* pp = mod->FindPowerPack();

  // don't publish anything if there is no power pack
  if(! pp )
    return;

  // translate stage data to player data packet
  player_power_data_t pen;
  pen.valid = 0;

  pen.valid |= PLAYER_POWER_MASK_JOULES;
  pen.joules = pp->GetStored();

  pen.valid |= PLAYER_POWER_MASK_CHARGING;
  pen.charging = pp->GetCharging();

  pen.valid |= PLAYER_POWER_MASK_PERCENT;
  pen.percent = pp->ProportionRemaining();
  
  //printf( "player power data: valid %d joules %.2f watts %.2f percent %.2f charging %d\n",
  //  pen.valid, pen.joules, pen.watts, pen.percent, pen.charging );
 
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_POWER_DATA_STATE,
			(void*)&pen, sizeof(pen), NULL);
}

int InterfacePower::ProcessMessage(QueuePointer &resp_queue,
                                      player_msghdr_t* hdr,
                                      void* data)
{
  // Don't know how to handle this message.
  PRINT_WARN2( "stg_power doesn't support msg with type %d subtype %d",
	       hdr->type, hdr->subtype);
  return(-1);
}


