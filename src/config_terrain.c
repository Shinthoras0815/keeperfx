/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_terrain.c
 *     Slabs, rooms, traps and doors configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for terrain elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_terrain.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"

#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_terrain_file[]="terrain.cfg";

const struct NamedCommand terrain_common_commands[] = {
  {"SLABSCOUNT",      1},
  {"ROOMSCOUNT",      2},
  {"TRAPSCOUNT",      3},
  {"DOORSCOUNT",      4},
  {NULL,              0},
  };

const struct NamedCommand terrain_slab_commands[] = {
  {"NAME",            1},
  {NULL,              0},
  };

const struct NamedCommand terrain_room_commands[] = {
  {"NAME",            1},
  {"COST",            2},
  {"HEALTH",          3},
  {NULL,              0},
  };

const struct NamedCommand terrain_door_commands[] = {
  {"NAME",            1},
  {"MANUFACTURELEVEL",2},
  {"MANUFACTUREREQUIRED",3},
  {"SELLINGVALUE",    4},
  {"HEALTH",          5},
  {NULL,              0},
  };

const struct NamedCommand terrain_trap_commands[] = {
  {"NAME",            1},
  {"MANUFACTURELEVEL",2},
  {"MANUFACTUREREQUIRED",3},
  {"SHOTS",           4},
  {"TIMEBETWEENSHOTS",5},
  {"SELLINGVALUE",    6},
  {NULL,              0},
  };

const struct NamedCommand terrain_health_commands[] = {
  {"DIRT",            1},
  {"GOLD",            2},
  {"PRETTY",          3},
  {"FLOOR",           4},
  {"ROOM",            5},
  {"DOOR_WOODEN",     6},
  {"DOOR_BRACE",      7},
  {"DOOR_STEEL",      8},
  {"DOOR_MAGIC",      9},
  {NULL,              0},
  };

/******************************************************************************/
struct SlabsConfig slab_conf;
struct NamedCommand slab_desc[TERRAIN_ITEMS_MAX];
struct NamedCommand room_desc[TERRAIN_ITEMS_MAX];
struct NamedCommand trap_desc[TERRAIN_ITEMS_MAX];
struct NamedCommand door_desc[TERRAIN_ITEMS_MAX];
/******************************************************************************/
TbBool parse_terrain_common_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  slab_conf.slab_types_count = 1;
  slab_conf.room_types_count = 1;
  slab_conf.trap_types_count = 1;
  slab_conf.door_types_count = 1;
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in terrain config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // SLABSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= TERRAIN_ITEMS_MAX))
            {
              slab_conf.slab_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_common_commands,cmd_num),block_buf);
          }
          break;
      case 2: // ROOMSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= TERRAIN_ITEMS_MAX))
            {
              slab_conf.room_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_common_commands,cmd_num),block_buf);
          }
          break;
      case 3: // TRAPSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= TERRAIN_ITEMS_MAX))
            {
              slab_conf.trap_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_common_commands,cmd_num),block_buf);
          }
          break;
      case 4: // DOORSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= TERRAIN_ITEMS_MAX))
            {
              slab_conf.door_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_common_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Terrain file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_terrain_slab_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(slab_conf.slab_names)/sizeof(slab_conf.slab_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(slab_conf.slab_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < slab_conf.slab_types_count)
    {
      slab_desc[i].name = slab_conf.slab_names[i].text;
      slab_desc[i].num = i;
    } else
    {
      slab_desc[i].name = NULL;
      slab_desc[i].num = 0;
    }
  }
  arr_size = slab_conf.slab_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"slab%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Terrain config file.",block_buf);
      continue;
    }
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_slab_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,slab_conf.slab_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Terrain config file.",
            get_conf_parameter_text(terrain_slab_commands,cmd_num),block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Terrain file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  // Block health - will be later integrated with slab blocks
    sprintf(block_buf,"block_health");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Terrain config file.",block_buf);
    } else
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_health_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.block_health[cmd_num-1] = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_health_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Terrain file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  return true;
}

TbBool parse_terrain_room_blocks(char *buf,long len)
{
  struct RoomStats *rstat;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the rooms array
  int arr_size = sizeof(slab_conf.room_names)/sizeof(slab_conf.room_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(slab_conf.room_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < slab_conf.room_types_count)
    {
      room_desc[i].name = slab_conf.room_names[i].text;
      room_desc[i].num = i;
    } else
    {
      room_desc[i].name = NULL;
      room_desc[i].num = 0;
    }
  }
  arr_size = slab_conf.room_types_count;
  for (i=0; i < arr_size; i++)
  {
    rstat = &game.room_stats[i];
    rstat->cost = 0;
    rstat->health = 0;
  }
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"room%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Terrain config file.",block_buf);
      continue;
    }
    rstat = &game.room_stats[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_room_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,slab_conf.room_names[i].text,COMMAND_WORD_LEN) > 0)
          {
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Terrain config file.",
            get_conf_parameter_text(terrain_room_commands,cmd_num),block_buf);
          }
          break;
      case 2: // COST
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            rstat->cost = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_room_commands,cmd_num),block_buf);
          }
          break;
      case 3: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            rstat->health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_room_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Terrain file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool parse_terrain_trap_blocks(char *buf,long len)
{
  struct ManfctrConfig *mconf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the traps array
  int arr_size = sizeof(slab_conf.trap_names)/sizeof(slab_conf.trap_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(slab_conf.trap_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < slab_conf.trap_types_count)
    {
      trap_desc[i].name = slab_conf.trap_names[i].text;
      trap_desc[i].num = i;
    } else
    {
      trap_desc[i].name = NULL;
      trap_desc[i].num = 0;
    }
  }
  arr_size = slab_conf.trap_types_count;
  for (i=0; i < arr_size; i++)
  {
    mconf = &game.traps_config[i];
    mconf->manufct_level = 0;
    mconf->manufct_required = 0;
    mconf->shots = 0;
    mconf->shots_delay = 0;
    mconf->selling_value = 0;
  }
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"trap%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Terrain config file.",block_buf);
      continue;
    }
    mconf = &game.traps_config[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_trap_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,slab_conf.trap_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Terrain config file.",
            get_conf_parameter_text(terrain_trap_commands,cmd_num),block_buf);
            break;
          }
          break;
      case 2: // MANUFACTURELEVEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_level = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_trap_commands,cmd_num),block_buf);
          }
          break;
      case 3: // MANUFACTUREREQUIRED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_required = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_trap_commands,cmd_num),block_buf);
          }
          break;
      case 4: // SHOTS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->shots = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_trap_commands,cmd_num),block_buf);
          }
          break;
      case 5: // TIMEBETWEENSHOTS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->shots_delay = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_trap_commands,cmd_num),block_buf);
          }
          break;
      case 6: // SELLINGVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->selling_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_trap_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Terrain file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool parse_terrain_door_blocks(char *buf,long len)
{
  struct ManfctrConfig *mconf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the doors array
  int arr_size = sizeof(slab_conf.door_names)/sizeof(slab_conf.door_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(slab_conf.door_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < slab_conf.door_types_count)
    {
      door_desc[i].name = slab_conf.door_names[i].text;
      door_desc[i].num = i;
    } else
    {
      door_desc[i].name = NULL;
      door_desc[i].num = 0;
    }
  }
  arr_size = slab_conf.door_types_count;
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"door%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Terrain config file.",block_buf);
      continue;
    }
    mconf = &game.doors_config[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_door_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,slab_conf.door_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Terrain config file.",
            get_conf_parameter_text(terrain_door_commands,cmd_num),block_buf);
            break;
          }
          break;
      case 2: // MANUFACTURELEVEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_level = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_door_commands,cmd_num),block_buf);
          }
          break;

      case 3: // MANUFACTUREREQUIRED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_required = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_door_commands,cmd_num),block_buf);
          }
          break;
      case 4: // SELLINGVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->selling_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_door_commands,cmd_num),block_buf);
          }
          break;
      case 5: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            door_stats[i][0].health = k;
            door_stats[i][1].health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Terrain file.",
                get_conf_parameter_text(terrain_door_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Terrain file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool load_terrain_config(const char *conf_fname,unsigned short flags)
{
  static const char *func_name="load_terrain_config";
  char *fname;
  char *buf;
  long len,pos;
  int cmd_num;
  TbBool result;
  SYNCDBG(0,"Reading terrain config file \"%s\".",conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("Terrain config file \"%s\" doesn't exist or is too small.",conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("Terrain config file \"%s\" is too large.",conf_fname);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  result = (len > 0);
  if (result)
  {
    result = parse_terrain_common_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing terrain file \"%s\" common blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_terrain_slab_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing terrain file \"%s\" slab blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_terrain_room_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing terrain file \"%s\" room blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_terrain_trap_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing terrain file \"%s\" trap blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_terrain_door_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing terrain file \"%s\" door blocks failed.",conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}

/*
 * Zeroes all the costs for all rooms.
 */
TbBool make_all_rooms_free(void)
{
  struct RoomStats *rstat;
  long i;
  for (i=0; i < slab_conf.room_types_count; i++)
  {
    rstat = &game.room_stats[i];
    rstat->cost = 0;
  }
  return true;
}

/*
 * Makes all rooms to be available to research for the player.
 */
TbBool make_all_rooms_researchable(long plyr_idx)
{
  static const char *func_name="make_all_rooms_researchable";
  struct Dungeon *dungeon;
  long i;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  for (i=0; i < slab_conf.room_types_count; i++)
  {
    dungeon->room_resrchable[i] = 1;
  }
  return true;
}

/*
 * Sets room availability state.
 */
TbBool set_room_available(long plyr_idx, long room_idx, long resrch, long avail)
{
  struct Dungeon *dungeon;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  if ((room_idx < 0) || (room_idx >= ROOM_TYPES_COUNT))
  {
    ERRORLOG("Can't add incorrect room %ld to player %ld",room_idx, plyr_idx);
    return false;
  }
  dungeon->room_resrchable[room_idx] = resrch;
  if (resrch != 0)
    dungeon->room_buildable[room_idx] = avail;
  else
    dungeon->room_buildable[room_idx] = 0;
  return true;
}

/*
 * Makes all the rooms, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_rooms(long plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  SYNCDBG(0,"Starting");
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  for (i=0; i < ROOM_TYPES_COUNT; i++)
  {
    if (dungeon->room_resrchable[i])
    {
      dungeon->room_buildable[i] = 1;
    }
  }
  return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
