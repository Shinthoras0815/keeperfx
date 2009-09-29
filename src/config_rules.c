/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_rules.c
 *     Various game configuration options support.
 * @par Purpose:
 *     Support of configuration files for game rules.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 31 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_rules.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_terrain.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_creature.h"

#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_rules_file[]="rules.cfg";

const struct NamedCommand rules_game_commands[] = {
  {"GOLDPERGOLDBLOCK",           1},
  {"POTOFGOLDHOLDS",             2},
  {"GOLDPILEVALUE",              3},
  {"GOLDPILEMAXIMUM",            4},
  {"FOODLIFEOUTOFHATCHERY",      5},
  {"DEATHMATCHSTATUEREAPPERTIME",6},
  {"DEATHMATCHOBJECTREAPPERTIME",7},
  {"DOUBLECLICKSPEED",           8},
  {"DOOROPENFOR",                9},
  {"BOULDERREDUCEHEALTHSLAP",   10},
  {"BOULDERREDUCEHEALTHWALL",   11},
  {"BOULDERREDUCEHEALTHROOM",   12},
  {"TILESTRENGTH",              13},
  {"GOLDTILESTRENGTH",          14},
  {"MINIMUMGOLD",               15},
  {"MAXGOLDLOOKUP",             16},
  {"MINGOLDTORECORD",           17},
  {"PAYDAYGAP",                 18},
  {"CHESTGOLDHOLD",             19},
  {"SLABCOLLAPSETIME",          20},
  {"DUNGEONHEARTHEALTH",        21},
  {"DUNGEONHEARTHEALTIME",      22},
  {"DUNGEONHEARTHEALHEALTH",    23},
  {"HERODOORWAITTIME",          24},
  {NULL,                         0},
  };

const struct NamedCommand rules_computer_commands[] = {
  {"AUTODIGLIMIT",               1},
  {"WAITFORROOMTIME",            2},
  {"CHECKEXPANDTIME",            3},
  {"MAXDISTANCETODIG",           4},
  {"WAITAFTERROOMAREA",          5},
  {NULL,                         0},
  };

const struct NamedCommand rules_creatures_commands[] = {
  {"RECOVERYFREQUENCY",          1},
  {"FIGHTMAXHATE",               2},
  {"FIGHTBORDERLINE",            3},
  {"FIGHTMAXLOVE",               4},
  {"BODYREMAINSFOR",             5},
  {"FIGHTHATEKILLVALUE",         6},
  {"GAMETURNSINFLEE",            7},
  {NULL,                         0},
  };

const struct NamedCommand rules_magic_commands[] = {
  {"HOLDAUDIENCETIME",              1},
  {"ARMAGEDONTELEPORTYOURTIMEGAP",  2},
  {"ARMAGEDONTELEPORTENEMYTIMEGAP", 3},
  {"ARMEGEDDONCOUNTDOWN",           4},
  {"ARMEGEDDONDURATION",            5},
  {"DISEASETRANSFERPERCENTAGE",     6},
  {"DISEASELOSEPERCENTAGEHEALTH",   7},
  {"DISEASELOSEHEALTHTIME",         8},
  {"MINDISTANCEFORTELEPORT",        9},
  {"COLLAPSEDUNGEONDAMAGE",        10},
  {"TURNSPERCOLLAPSEDUNGEONDAMAGE",11},
  {"DEFAULTSACRIFICESCOREFORHORNY",12},
  {"POWERHANDGOLDGRABAMOUNT",      13},
  {NULL,                            0},
  };

const struct NamedCommand rules_rooms_commands[] = {
  {"SCAVENGECOSTFREQUENCY",      1},
  {"TEMPLESCAVENGEPROTECTIONTIME",2},
  {"TRAINCOSTFREQUENCY",         3},
  {"TORTURECONVERTCHANCE",       4},
  {"TIMESPENTINPRISON",          5},
  {"GHOSTCONVERTCHANCE",         6},
  {"ARMORYTIME",                 7},
  {"WORKSHOPTIME",               8},
  {"OBSERVATORYTIME",            9},
  {"OBSERVATORYGENERATE",       10},
  {"DEFAULTGENERATESPEED",      11},
  {"DEFAULTMAXCREATURESGENERATEENTRANCE", 12},
  {"DEFAULTNEUTRALENTRANCELEVEL", 13},
  {"BARRACKTIME",               14},
  {"FOODGENERATIONSPEED",       15},
  {"PRISONSKELETONCHANCE",      16},
  {"BODIESFORVAMPIRE",          17},
  {"GRAVEYARDCONVERTTIME",      18},
  {NULL,                         0},
  };

const struct NamedCommand rules_workers_commands[] = {
  {"HITSPERSLAB",                1},
  {"IMPJOBNOTSOCLOSE",           2},
  {"IMPJOBFARAWAY",              3},
  {"IMPGENERATETIME",            4},
  {"IMPROVEAREA",                5},
  {"DEFAULTIMPDIGDAMAGE",        6},
  {"DEFAULTIMPDIGOWNDAMAGE",     7},
  {"PERIMPGOLDDIGSIZE",          8},
  {NULL,                         0},
  };

const struct NamedCommand rules_health_commands[] = {
  {"HUNGERHEALTHLOSS",              1},
  {"GAMETURNSPERHUNGERHEALTHLOSS",  2},
  {"FOODHEALTHGAIN",                3},
  {"PRISONHEALTHGAIN",              4},
  {"GAMETURNSPERPRISONHEALTHGAIN",  5},
  {"TORTUREHEALTHLOSS",             6},
  {"GAMETURNSPERTORTUREHEALTHLOSS", 7},
  {NULL,                            0},
  };

const struct NamedCommand rules_research_commands[] = {
  {"RESEARCH",        1},
  {NULL,              0},
  };

const struct NamedCommand research_desc[] = {
  {"MAGIC",                1},
  {"ROOM",                 2},
  {"CREATURE",             3},
  {NULL,                   0},
};


/******************************************************************************/
TbBool parse_rules_game_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"game");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_game_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // GOLDPERGOLDBLOCK
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.gold_per_gold_block = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 2: // POTOFGOLDHOLDS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.pot_of_gold_holds = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 3: // GOLDPILEVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.gold_pile_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 4: // GOLDPILEMAXIMUM
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.gold_pile_maximum = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 5: // FOODLIFEOUTOFHATCHERY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.food_life_out_of_hatchery = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 6: // DEATHMATCHSTATUEREAPPERTIME
          //Unused
          break;
      case 7: // DEATHMATCHOBJECTREAPPERTIME
          //Unused
          break;
      case 8: // DOUBLECLICKSPEED
          //Unused
          break;
      case 9: // DOOROPENFOR
          //Unused
          break;
      case 10: // BOULDERREDUCEHEALTHSLAP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.boulder_reduce_health_slap = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 11: // BOULDERREDUCEHEALTHWALL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.boulder_reduce_health_wall = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 12: // BOULDERREDUCEHEALTHROOM
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.boulder_reduce_health_room = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 13: // TILESTRENGTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.tile_strength = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 14: // GOLDTILESTRENGTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.gold_tile_strength = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 15: // MINIMUMGOLD
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.minimum_gold = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 16: // MAXGOLDLOOKUP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.max_gold_lookup = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 17: // MINGOLDTORECORD
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.min_gold_to_record = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 18: // PAYDAYGAP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.pay_day_gap = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 19: // CHESTGOLDHOLD
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.chest_gold_hold = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 20: // SLABCOLLAPSETIME
          //Unused
          break;
      case 21: // DUNGEONHEARTHEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.dungeon_heart_health = k;
            game.objects_config[5].health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 22: // DUNGEONHEARTHEALTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.dungeon_heart_heal_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 23: // DUNGEONHEARTHEALHEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.dungeon_heart_heal_health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 24: // HERODOORWAITTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hero_door_wait_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_game_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_rules_computer_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"computer");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_computer_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // AUTODIGLIMIT
          //Unused
          break;
      case 2: // WAITFORROOMTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.wait_for_room_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_computer_commands,cmd_num),block_buf);
          }
          break;
      case 3: // CHECKEXPANDTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.check_expand_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_computer_commands,cmd_num),block_buf);
          }
          break;
      case 4: // MAXDISTANCETODIG
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.max_distance_to_dig = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_computer_commands,cmd_num),block_buf);
          }
          break;
      case 5: // WAITAFTERROOMAREA
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.wait_after_room_area = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_computer_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_rules_creatures_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"creatures");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_creatures_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // RECOVERYFREQUENCY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.recovery_frequency = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 2: // FIGHTMAXHATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_max_hate = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 3: // FIGHTBORDERLINE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_borderline = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 4: // FIGHTMAXLOVE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_max_love = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 5: // BODYREMAINSFOR
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.body_remains_for = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 6: // FIGHTHATEKILLVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_hate_kill_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 7: // GAMETURNSINFLEE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.game_turns_in_flee = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_creatures_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_rules_magic_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"magic");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_magic_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // HOLDAUDIENCETIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hold_audience_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 2: // ARMAGEDONTELEPORTYOURTIMEGAP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armagedon_teleport_your_time_gap = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 3: // ARMAGEDONTELEPORTENEMYTIMEGAP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armagedon_teleport_enemy_time_gap = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 4: // ARMEGEDDONCOUNTDOWN
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armageddon.count_down = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 5: // ARMEGEDDONDURATION
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armageddon.duration = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 6: // DISEASETRANSFERPERCENTAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.disease_transfer_percentage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 7: // DISEASELOSEPERCENTAGEHEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.disease_lose_percentage_health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 8: // DISEASELOSEHEALTHTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.disease_lose_health_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 9: // MINDISTANCEFORTELEPORT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.min_distance_for_teleport = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 10: // COLLAPSEDUNGEONDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.collapse_dungeon_damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 11: // TURNSPERCOLLAPSEDUNGEONDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.turns_per_collapse_dngn_dmg = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 12: // DEFAULTSACRIFICESCOREFORHORNY
          //Unused
          break;
      case 13: // POWERHANDGOLDGRABAMOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.power_hand_gold_grab_amount = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_magic_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_rules_rooms_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"rooms");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_rooms_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // SCAVENGECOSTFREQUENCY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.scavenge_cost_frequency = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 2: // TEMPLESCAVENGEPROTECTIONTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.temple_scavenge_protection_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 3: // TRAINCOSTFREQUENCY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.train_cost_frequency = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 4: // TORTURECONVERTCHANCE
          //Unused
          break;
      case 5: // TIMESPENTINPRISON
          //Unused
          break;
      case 6: // GHOSTCONVERTCHANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.ghost_convert_chance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 7: // ARMORYTIME
          //Unused
          break;
      case 8: // WORKSHOPTIME
          //Unused
          break;
      case 9: // OBSERVATORYTIME
          //Unused
          break;
      case 10: // OBSERVATORYGENERATE
          //Unused
          break;
      case 11: // DEFAULTGENERATESPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_generate_speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 12: // DEFAULTMAXCREATURESGENERATEENTRANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_max_crtrs_gen_entrance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 13: // DEFAULTNEUTRALENTRANCELEVEL
          //Unused
          break;
      case 14: // BARRACKTIME
          //Unused
          break;
      case 15: // FOODGENERATIONSPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.food_generation_speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 16: // PRISONSKELETONCHANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.prison_skeleton_chance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 17: // BODIESFORVAMPIRE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.bodies_for_vampire = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 18: // GRAVEYARDCONVERTTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.graveyard_convert_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_rooms_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  //SYNCMSG("Prison skeleton chance = %d",game.prison_skeleton_chance);
  return true;
}

TbBool parse_rules_workers_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"workers");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_workers_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // HITSPERSLAB
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hits_per_slab = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_workers_commands,cmd_num),block_buf);
          }
          break;
      case 2: // IMPJOBNOTSOCLOSE
          //Unused
          break;
      case 3: // IMPJOBFARAWAY
          //Unused
          break;
      case 4: // IMPGENERATETIME
          //Unused
          break;
      case 5: // IMPROVEAREA
          //Unused
          break;
      case 6: // DEFAULTIMPDIGDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_imp_dig_damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_workers_commands,cmd_num),block_buf);
          }
          break;
      case 7: // DEFAULTIMPDIGOWNDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_imp_dig_own_damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_workers_commands,cmd_num),block_buf);
          }
          break;
      case 8: // PERIMPGOLDDIGSIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.per_imp_gold_dig_size = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_workers_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_rules_health_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"health");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_health_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // HUNGERHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hunger_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_health_commands,cmd_num),block_buf);
          }
          break;
      case 2: // GAMETURNSPERHUNGERHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.turns_per_hunger_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_health_commands,cmd_num),block_buf);
          }
          break;
      case 3: // FOODHEALTHGAIN
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.food_health_gain = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_health_commands,cmd_num),block_buf);
          }
          break;
      case 4: // PRISONHEALTHGAIN
          //Unused
          break;
      case 5: // GAMETURNSPERPRISONHEALTHGAIN
          //Unused
          break;
      case 6: // TORTUREHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.torture_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_health_commands,cmd_num),block_buf);
          }
          break;
      case 7: // GAMETURNSPERTORTUREHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.turns_per_torture_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_health_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

long get_research_id(long item_type, char *trg_name, const char *func_name)
{
  long item_id;
  char *text;
  switch (item_type)
  {
  case 1:
       item_id = get_id(power_desc, trg_name);
      if (item_id == -1)
      {
        ERRORMSG("%s(line %lu): " "Unknown magic, '%s'", func_name , text_line_number, trg_name);
        return -1;
      }
      break;
  case 2:
      item_id = get_id(room_desc, trg_name);
      if (item_id == -1)
      {
        ERRORMSG("%s(line %lu): " "Unknown room, '%s'", func_name , text_line_number, trg_name);
        return -1;
      }
      break;
  case 3:
      item_id = get_id(creature_desc, trg_name);
      if (item_id == -1)
      {
        ERRORMSG("%s(line %lu): " "Unknown creature, '%s'", func_name , text_line_number, trg_name);
        return -1;
      }
      break;
  case -1:
  default:
      ERRORMSG("%s(line %lu): " "Unhandled research type, '%s'", func_name , text_line_number, item_type);
      return -1;
  }
  return item_id;
}

TbBool parse_rules_research_blocks(char *buf,long len)
{
  long pos;
  int i,k,l,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"research");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in rules config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,rules_research_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // RESEARCH
          i = 0;
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = get_id(research_desc, word_buf);
            if (i >= 0)
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            l = get_research_id(i, word_buf, __func__);
            if (l >= 0)
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            n++;
          }
          if (n < 3)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Rules file.",
                get_conf_parameter_text(rules_research_commands,cmd_num),block_buf);
            break;
          }
          add_research_to_all_players(i, l, k);
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Rules file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}


TbBool load_rules_config(const char *conf_fname,unsigned short flags)
{
  char *fname;
  char *buf;
  long len,pos;
  int cmd_num;
  TbBool result;
  SYNCDBG(0,"Reading rules config file \"%s\".",conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("Game Rules config file \"%s\" doesn't exist or is too small.",conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("Game Rules config file \"%s\" is too large.",conf_fname);
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
    result = parse_rules_game_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" game blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_computer_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" computer blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_creatures_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" creatures blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_magic_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" magic blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_rooms_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" rooms blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_workers_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" workers blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_health_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" health blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_rules_research_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing rules file \"%s\" research blocks failed.",conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
