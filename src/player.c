/****************************************************************************
 * ResortMUD 4.0 Beta by Ntanel, Garinan, Badastaz, Josh, Digifuzz, Senir,  *
 * Kratas, Scion, Shogar and Tagith.  Special thanks to Thoric, Nivek,      *
 * Altrag, Arlorn, Justice, Samson, Dace, HyperEye and Yakkov.              *
 ****************************************************************************
 * Copyright (C) 1996 - 2001 Haslage Net Electronics: MudWorld              *
 * of Lorain, Ohio - ALL RIGHTS RESERVED                                    *
 * The text and pictures of this publication, or any part thereof, may not  *
 * be reproduced or transmitted in any form or by any means, electronic or  *
 * mechanical, includes photocopying, recording, storage in a information   *
 * retrieval system, or otherwise, without the prior written or e-mail      *
 * consent from the publisher.                                              *
 ****************************************************************************
 * GREETING must mention ResortMUD programmers and the help file named      *
 * CREDITS must remain completely intact as listed in the SMAUG license.    *
 ****************************************************************************/

/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 *  Locals
 */
char *tiny_affect_loc_name( int location );

/* For TinyMUD Bots! -- Scion */
void do_outputprefix( CHAR_DATA * ch, char *argument )
{
   if( strlen( argument ) > 40 )
   {
      send_to_char( "Sorry, that string is too long!\r\n", ch );
      return;
   }

   if( argument[0] != '\0' )
   {
      if( ch->pcdata->outputprefix )
         DISPOSE( ch->pcdata->outputprefix );
      ch->pcdata->outputprefix = str_dup( argument );
   }
}

void do_outputsuffix( CHAR_DATA * ch, char *argument )
{
   if( strlen( argument ) > 40 )
   {
      send_to_char( "Sorry, that string is too long!\r\n", ch );
      return;
   }

   if( argument[0] != '\0' )
   {
      if( ch->pcdata->outputsuffix )
         DISPOSE( ch->pcdata->outputsuffix );
      ch->pcdata->outputsuffix = str_dup( argument );
   }
}

/* Scion 4/7/1999, just after RM 3.0 release :P */
void do_reroll( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Holy smoke! NPCs can\'t do fun things like this!\r\n", ch );
      return;
   }

   if( ch->level > 1 /*|| !NOT_AUTHED(ch) */  )
   {
      send_to_char( "Sorry bud, you should have done this before being authorized or gaining level 2.\r\n", ch );
      return;
   }

   ch->perm_str = 13;
   ch->perm_dex = 13;
   ch->perm_int = 13;
   ch->perm_wis = 13;
   ch->perm_cha = 13;
   ch->perm_con = 13;
   ch->perm_lck = 13;
   switch ( class_table[ch->class]->attr_prime )
   {
      case APPLY_STR:
         ch->perm_str = 16;
         break;
      case APPLY_INT:
         ch->perm_int = 16;
         break;
      case APPLY_WIS:
         ch->perm_wis = 16;
         break;
      case APPLY_DEX:
         ch->perm_dex = 16;
         break;
      case APPLY_CON:
         ch->perm_con = 16;
         break;
      case APPLY_CHA:
         ch->perm_cha = 16;
         break;
      case APPLY_LCK:
         ch->perm_lck = 16;
         break;
   }
   ch->perm_str += race_table[ch->race]->str_plus;
   ch->perm_int += race_table[ch->race]->int_plus;
   ch->perm_wis += race_table[ch->race]->wis_plus;
   ch->perm_dex += race_table[ch->race]->dex_plus;
   ch->perm_con += race_table[ch->race]->con_plus;
   ch->perm_cha += race_table[ch->race]->cha_plus;
   ch->perm_lck += race_table[ch->race]->lck_plus;

   name_stamp_stats( ch );

   send_to_char( "Your stats have been rerolled:\r\n", ch );
   sprintf( buf, "%-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
            ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );
   send_to_char( buf, ch );
}

void do_gold( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch, "You have %d gold pieces.\r\n", ch->gold );
   return;
}


void do_worth( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   set_pager_color( AT_SCORE, ch );
   pager_printf( ch, "\r\nWorth for %s%s.\r\n", ch->name, ch->pcdata->title );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   if( !ch->pcdata->deity )
      sprintf( buf, "N/A" );
   else if( ch->pcdata->favor > 2250 )
      sprintf( buf, "loved" );
   else if( ch->pcdata->favor > 2000 )
      sprintf( buf, "cherished" );
   else if( ch->pcdata->favor > 1750 )
      sprintf( buf, "honored" );
   else if( ch->pcdata->favor > 1500 )
      sprintf( buf, "praised" );
   else if( ch->pcdata->favor > 1250 )
      sprintf( buf, "favored" );
   else if( ch->pcdata->favor > 1000 )
      sprintf( buf, "respected" );
   else if( ch->pcdata->favor > 750 )
      sprintf( buf, "liked" );
   else if( ch->pcdata->favor > 250 )
      sprintf( buf, "tolerated" );
   else if( ch->pcdata->favor > -250 )
      sprintf( buf, "ignored" );
   else if( ch->pcdata->favor > -750 )
      sprintf( buf, "shunned" );
   else if( ch->pcdata->favor > -1000 )
      sprintf( buf, "disliked" );
   else if( ch->pcdata->favor > -1250 )
      sprintf( buf, "dishonored" );
   else if( ch->pcdata->favor > -1500 )
      sprintf( buf, "disowned" );
   else if( ch->pcdata->favor > -1750 )
      sprintf( buf, "abandoned" );
   else if( ch->pcdata->favor > -2000 )
      sprintf( buf, "despised" );
   else if( ch->pcdata->favor > -2250 )
      sprintf( buf, "hated" );
   else
      sprintf( buf, "damned" );

   if( ch->level < 10 )
   {
      if( ch->alignment > 900 )
         sprintf( buf2, "devout" );
      else if( ch->alignment > 700 )
         sprintf( buf2, "noble" );
      else if( ch->alignment > 350 )
         sprintf( buf2, "honorable" );
      else if( ch->alignment > 100 )
         sprintf( buf2, "worthy" );
      else if( ch->alignment > -100 )
         sprintf( buf2, "neutral" );
      else if( ch->alignment > -350 )
         sprintf( buf2, "base" );
      else if( ch->alignment > -700 )
         sprintf( buf2, "evil" );
      else if( ch->alignment > -900 )
         sprintf( buf2, "ignoble" );
      else
         sprintf( buf2, "fiendish" );
   }
   else
      sprintf( buf2, "%d", ch->alignment );
   pager_printf( ch, "|Level: %-4d |Favor: %-10s |Alignment: %-9s |Experience: %-9d|\r\n", ch->level, buf, buf2, ch->exp );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   switch ( ch->style )
   {
      case STYLE_EVASIVE:
         sprintf( buf, "evasive" );
         break;
      case STYLE_DEFENSIVE:
         sprintf( buf, "defensive" );
         break;
      case STYLE_AGGRESSIVE:
         sprintf( buf, "aggressive" );
         break;
      case STYLE_BERSERK:
         sprintf( buf, "berserk" );
         break;
      default:
         sprintf( buf, "standard" );
         break;
   }
   pager_printf( ch, "|Glory: %-4d |Weight: %-9d |Style: &R%-13s&C |Gold: %-14d |\r\n",
                 ch->pcdata->quest_curr, ch->carry_weight, buf, ch->gold );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   if( ch->level < 15 && !IS_PKILL( ch ) )
      pager_printf( ch, "|            |Hitroll: -------- |Damroll: ----------- |                     |\r\n" );
   else
      pager_printf( ch, "|            |Hitroll: %-8d |Damroll: %-11d |                     |\r\n", GET_HITROLL( ch ),
                    GET_DAMROLL( ch ) );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   return;
}

/*
 * New score command by Haus
 */
void do_score( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   int iLang;

   if( IS_NPC( ch ) )
   {
      do_oldscore( ch, argument );
      return;
   }
   set_pager_color( AT_SCORE, ch );

   pager_printf( ch, "\r\n&CScore for: &W%s %s%s%s&C.\r\n", ch->pcdata->extraname, ch->name, ch->pcdata->lastname,
                 ch->pcdata->title );
   if( get_trust( ch ) != ch->level )
      pager_printf( ch, "&CYou are trusted at level &W%d&C.\r\n", get_trust( ch ) );

   send_to_pager( "----------------------------------------------------------------------------\r\n", ch );

   pager_printf( ch, "&CLEVEL: &W%-3d         &CRace : &W%-10.10s        &CPlayed: &W%d &Chours\r\n",
                 ch->level, capitalize( get_race( ch ) ), ( get_age( ch ) - 17 ) * 2 );

   pager_printf( ch, "&CYEARS: &W%-6d      &CClass: &W%-11.11s       &CLog In: &W%s\r",
                 get_age( ch ), capitalize( get_class( ch ) ), ctime( &( ch->logon ) ) );

   if( ch->level >= 15 || IS_PKILL( ch ) )
   {
      pager_printf( ch, "&CSTR  : &W%2.2d&C(&w%2.2d&C)    HitRoll: &R%-4d              &CSaved:  &W%s\r",
                    get_curr_str( ch ), ch->perm_str, GET_HITROLL( ch ),
                    ch->save_time ? ctime( &( ch->save_time ) ) : "no save this session\n" );

      pager_printf( ch, "&CINT  : &W%2.2d&C(&w%2.2d&C)    DamRoll: &R%-4d              &CTime:   &W%s\r",
                    get_curr_int( ch ), ch->perm_int, GET_DAMROLL( ch ), ctime( &current_time ) );
   }
   else
   {
      pager_printf( ch, "&CSTR  : &W%2.2d&C(&w%2.2d&C)                               Saved:  &W%s\r",
                    get_curr_str( ch ), ch->perm_str, ch->save_time ? ctime( &( ch->save_time ) ) : "no\n" );

      pager_printf( ch, "&CINT  : &W%2.2d&C(&w%2.2d&c)                               Time:   &W%s\r",
                    get_curr_int( ch ), ch->perm_int, ctime( &current_time ) );
   }

   if( GET_AC( ch ) >= 101 )
      sprintf( buf, "the rags of a beggar" );
   else if( GET_AC( ch ) >= 80 )
      sprintf( buf, "improper for adventure" );
   else if( GET_AC( ch ) >= 55 )
      sprintf( buf, "shabby and threadbare" );
   else if( GET_AC( ch ) >= 40 )
      sprintf( buf, "of poor quality" );
   else if( GET_AC( ch ) >= 20 )
      sprintf( buf, "scant protection" );
   else if( GET_AC( ch ) >= 10 )
      sprintf( buf, "that of a knave" );
   else if( GET_AC( ch ) >= 0 )
      sprintf( buf, "moderately crafted" );
   else if( GET_AC( ch ) >= -10 )
      sprintf( buf, "well crafted" );
   else if( GET_AC( ch ) >= -20 )
      sprintf( buf, "the envy of squires" );
   else if( GET_AC( ch ) >= -40 )
      sprintf( buf, "excellently crafted" );
   else if( GET_AC( ch ) >= -60 )
      sprintf( buf, "the envy of knights" );
   else if( GET_AC( ch ) >= -80 )
      sprintf( buf, "the envy of barons" );
   else if( GET_AC( ch ) >= -100 )
      sprintf( buf, "the envy of dukes" );
   else if( GET_AC( ch ) >= -200 )
      sprintf( buf, "the envy of emperors" );
   else
      sprintf( buf, "that of an avatar" );
   if( ch->level > 24 )
      pager_printf( ch, "&CWIS  : &W%2.2d&C(&w%2.2d&C)      Armor: &W%s &C(&w%4.4d&C)\r\n",
                    get_curr_wis( ch ), ch->perm_wis, buf, GET_AC( ch ) );
   else
      pager_printf( ch, "&CWIS  : &W%2.2d&C(&w%2.2d&C)      Armor: &W%s \r\n", get_curr_wis( ch ), ch->perm_wis, buf );

   if( ch->alignment > 900 )
      sprintf( buf, "devout" );
   else if( ch->alignment > 700 )
      sprintf( buf, "noble" );
   else if( ch->alignment > 350 )
      sprintf( buf, "honorable" );
   else if( ch->alignment > 100 )
      sprintf( buf, "worthy" );
   else if( ch->alignment > -100 )
      sprintf( buf, "neutral" );
   else if( ch->alignment > -350 )
      sprintf( buf, "base" );
   else if( ch->alignment > -700 )
      sprintf( buf, "evil" );
   else if( ch->alignment > -900 )
      sprintf( buf, "ignoble" );
   else
      sprintf( buf, "fiendish" );
   if( ch->level < 10 )
      pager_printf( ch, "&CDEX  : &W%2.2d&C(&w%2.2d&C)      Align: &W%-20.20s    &CItems: &W%5.5d   &C(&wmax %5.5d&C)\r\n",
                    get_curr_dex( ch ), ch->perm_dex, buf, ch->carry_number, can_carry_n( ch ) );
   else
      pager_printf( ch,
                    "&CDEX  : &W%2.2d&C(&w%2.2d&C)      Align: &W%-14.14s &C(&w%+4.4d&C)  Items: &W%5.5d   &C(&wmax %5.5d&C)\r\n",
                    get_curr_dex( ch ), ch->perm_dex, buf, ch->alignment, ch->carry_number, can_carry_n( ch ) );

   switch ( ch->position )
   {
      case POS_DEAD:
         sprintf( buf, "slowly decomposing" );
         break;
      case POS_MORTAL:
         sprintf( buf, "mortally wounded" );
         break;
      case POS_INCAP:
         sprintf( buf, "incapacitated" );
         break;
      case POS_STUNNED:
         sprintf( buf, "stunned" );
         break;
      case POS_SLEEPING:
         sprintf( buf, "sleeping" );
         break;
      case POS_RESTING:
         sprintf( buf, "resting" );
         break;
      case POS_STANDING:
         sprintf( buf, "standing" );
         break;
      case POS_FIGHTING:
         sprintf( buf, "fighting" );
         break;
/*  Fighting style support -haus */
      case POS_EVASIVE:
         sprintf( buf, "fighting (evasive)" );
         break;
      case POS_DEFENSIVE:
         sprintf( buf, "fighting (defensive)" );
         break;
      case POS_AGGRESSIVE:
         sprintf( buf, "fighting (aggressive)" );
         break;
      case POS_BERSERK:
         sprintf( buf, "fighting (berserk)" );
         break;
      case POS_MOUNTED:
         sprintf( buf, "mounted" );
         break;
      case POS_SITTING:
         sprintf( buf, "sitting" );
         break;
   }
   pager_printf( ch, "&CCON  : &W%2.2d&C(&w%2.2d&C)      Pos'n: &W%-21.21s  &CWeight: &W%5.5d &C(&wmax %7.7d&C)\r\n",
                 get_curr_con( ch ), ch->perm_con, buf, ch->carry_weight, can_carry_w( ch ) );


   /*
    * Fighting style support -haus
    */
   pager_printf( ch, "&CCHA  : &W%2.2d&C(&w%2.2d&C)      Wimpy: &R%-5d&C \r\n",
                 get_curr_cha( ch ), ch->perm_cha, ch->wimpy );

   switch ( ch->style )
   {
      case STYLE_EVASIVE:
         sprintf( buf, "evasive" );
         break;
      case STYLE_DEFENSIVE:
         sprintf( buf, "defensive" );
         break;
      case STYLE_AGGRESSIVE:
         sprintf( buf, "aggressive" );
         break;
      case STYLE_BERSERK:
         sprintf( buf, "berserk" );
         break;
      default:
         sprintf( buf, "standard" );
         break;
   }
   pager_printf( ch, "Style: &R%-10.10s&C\r\n", buf );

   pager_printf( ch, "&CLCK  : &W%2.2d&C(&w%2.2d&C) \r\n", get_curr_lck( ch ), ch->perm_lck );

   pager_printf( ch, "&CGlory: &W%4.4d&C(&w%4.4d&C)                           &CPager:  [&W%c&C][&W%3d &wlines&C]    \r\n",
                 ch->pcdata->quest_curr, ch->pcdata->quest_accum,
                 ( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) ? '*' : ' ' ), ch->pcdata->pagerlen );

   pager_printf( ch, "&CPRACT: &W%3.3d           &CHit: &R%-5d &Cof &r%-5d    &CAutoExit[&W%c&C]\r\n",
                 ch->practice, ch->hit, ch->max_hit, xIS_SET( ch->act, PLR_AUTOEXIT ) ? '*' : ' ' );

   if( IS_VAMPIRE( ch ) )
      pager_printf( ch, "&CXP   : &G%-10d  &CBlood: &R%-5d &Cof &r%-5d    &CAutoLoot[&W%c&C]\r\n",
                    ch->exp, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level,
                    xIS_SET( ch->act, PLR_AUTOLOOT ) ? '*' : ' ' );
   else
      pager_printf( ch, "&CXP   : &G%-10d   &CMana: &B%-5d &Cof &b%-5d    &CAutoLoot[&W%c&C]\r\n",
                    ch->exp, ch->mana, ch->max_mana, xIS_SET( ch->act, PLR_AUTOLOOT ) ? '*' : ' ' );

   pager_printf( ch, "&CGOLD : &Y%-10d   &CMove: &W%-5d &Cof &w%-5d    &CAutoSac [&W%c&C]\r\n",
                 ch->gold, ch->move, ch->max_move, xIS_SET( ch->act, PLR_AUTOSAC ) ? '*' : ' ' );

   send_to_pager( "&C", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
      send_to_pager( "You are drunk.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
      send_to_pager( "You are in danger of dehydrating.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
      send_to_pager( "You are starving to death.\r\n", ch );
   if( ch->position != POS_SLEEPING )
      switch ( ch->mental_state / 10 )
      {
         default:
            send_to_pager( "You're completely messed up!\r\n", ch );
            break;
         case -10:
            send_to_pager( "You're barely conscious.\r\n", ch );
            break;
         case -9:
            send_to_pager( "You can barely keep your eyes open.\r\n", ch );
            break;
         case -8:
            send_to_pager( "You're extremely drowsy.\r\n", ch );
            break;
         case -7:
            send_to_pager( "You feel very unmotivated.\r\n", ch );
            break;
         case -6:
            send_to_pager( "You feel sedated.\r\n", ch );
            break;
         case -5:
            send_to_pager( "You feel sleepy.\r\n", ch );
            break;
         case -4:
            send_to_pager( "You feel tired.\r\n", ch );
            break;
         case -3:
            send_to_pager( "You could use a rest.\r\n", ch );
            break;
         case -2:
            send_to_pager( "You feel a little under the weather.\r\n", ch );
            break;
         case -1:
            send_to_pager( "You feel fine.\r\n", ch );
            break;
         case 0:
            send_to_pager( "You feel great.\r\n", ch );
            break;
         case 1:
            send_to_pager( "You feel energetic.\r\n", ch );
            break;
         case 2:
            send_to_pager( "Your mind is racing.\r\n", ch );
            break;
         case 3:
            send_to_pager( "You can't think straight.\r\n", ch );
            break;
         case 4:
            send_to_pager( "Your mind is going 100 miles an hour.\r\n", ch );
            break;
         case 5:
            send_to_pager( "You're high as a kite.\r\n", ch );
            break;
         case 6:
            send_to_pager( "Your mind and body are slipping apart.\r\n", ch );
            break;
         case 7:
            send_to_pager( "Reality is slipping away.\r\n", ch );
            break;
         case 8:
            send_to_pager( "You have no idea what is real, and what is not.\r\n", ch );
            break;
         case 9:
            send_to_pager( "You feel immortal.\r\n", ch );
            break;
         case 10:
            send_to_pager( "You are a Supreme Entity.\r\n", ch );
            break;
      }
   else if( ch->mental_state > 45 )
      send_to_pager( "Your sleep is filled with strange and vivid dreams.\r\n", ch );
   else if( ch->mental_state > 25 )
      send_to_pager( "Your sleep is uneasy.\r\n", ch );
   else if( ch->mental_state < -35 )
      send_to_pager( "You are deep in a much needed sleep.\r\n", ch );
   else if( ch->mental_state < -25 )
      send_to_pager( "You are in deep slumber.\r\n", ch );
   send_to_pager( "Languages: ", ch );
   for( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
      if( knows_language( ch, lang_array[iLang], ch ) || ( IS_NPC( ch ) && ch->speaks == 0 ) )
      {
         if( lang_array[iLang] & ch->speaking || ( IS_NPC( ch ) && !ch->speaking ) )
            set_pager_color( AT_RED, ch );
         send_to_pager( lang_names[iLang], ch );
         send_to_pager( " ", ch );
         set_pager_color( AT_SCORE, ch );
      }
   send_to_pager( "\r\n", ch );

   if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
      pager_printf( ch, "&CYou are bestowed with the command(s): &Y%s&C.\r\n", ch->pcdata->bestowments );

   if( ch->morph && ch->morph->morph )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      if( IS_IMMORTAL( ch ) )
         pager_printf( ch, "&CMorphed as (&w%d&C) %s with a timer of &R%d&C.\r\n",
                       ch->morph->morph->vnum, ch->morph->morph->short_desc, ch->morph->timer );
      else
         pager_printf( ch, "&CYou are morphed into a &W%s&C.\r\n", ch->morph->morph->short_desc );
   }
   send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
   pager_printf( ch, "NonPC DATA:  Mkills (&W%5.5d&C)     Mdeaths (&W%5.5d&C)\r\n", ch->pcdata->mkills,
                 ch->pcdata->mdeaths );
   if( CAN_PKILL( ch ) )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "PKILL DATA:  Pkills (&W%3.3d&C)     Illegal Pkills (&W%3.3d&C)     Pdeaths (&W%3.3d&C)\r\n",
                    ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths );
   }
   if( ch->arena_wins || ch->arena_deaths || ch->arena_kills )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "ARENA DATA:  Wins: (&W%3.3d&C)     Kills: (&W%3.3d&C)     Deaths: (&W%3.3d&C)\r\n",
                    ch->arena_wins, ch->arena_kills, ch->arena_deaths );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "CLAN STATS:  &W%-14.14s  &CClan AvPkills : &W%-5d  &CClan NonAvpkills : &W%-5d\r\n",
                    ch->pcdata->clan->name, ch->pcdata->clan->pkills[5],
                    ( ch->pcdata->clan->pkills[0] + ch->pcdata->clan->pkills[1] +
                      ch->pcdata->clan->pkills[2] + ch->pcdata->clan->pkills[3] + ch->pcdata->clan->pkills[4] ) );
      pager_printf( ch, "                             &CClan AvPdeaths: &W%-5d  &CClan NonAvpdeaths: &W%-5d&C\r\n",
                    ch->pcdata->clan->pdeaths[5],
                    ( ch->pcdata->clan->pdeaths[0] + ch->pcdata->clan->pdeaths[1] +
                      ch->pcdata->clan->pdeaths[2] + ch->pcdata->clan->pdeaths[3] + ch->pcdata->clan->pdeaths[4] ) );
   }
   if( ch->pcdata->deity )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      if( ch->pcdata->favor > 2250 )
         sprintf( buf, "loved" );
      else if( ch->pcdata->favor > 2000 )
         sprintf( buf, "cherished" );
      else if( ch->pcdata->favor > 1750 )
         sprintf( buf, "honored" );
      else if( ch->pcdata->favor > 1500 )
         sprintf( buf, "praised" );
      else if( ch->pcdata->favor > 1250 )
         sprintf( buf, "favored" );
      else if( ch->pcdata->favor > 1000 )
         sprintf( buf, "respected" );
      else if( ch->pcdata->favor > 750 )
         sprintf( buf, "liked" );
      else if( ch->pcdata->favor > 250 )
         sprintf( buf, "tolerated" );
      else if( ch->pcdata->favor > -250 )
         sprintf( buf, "ignored" );
      else if( ch->pcdata->favor > -750 )
         sprintf( buf, "shunned" );
      else if( ch->pcdata->favor > -1000 )
         sprintf( buf, "disliked" );
      else if( ch->pcdata->favor > -1250 )
         sprintf( buf, "dishonored" );
      else if( ch->pcdata->favor > -1500 )
         sprintf( buf, "disowned" );
      else if( ch->pcdata->favor > -1750 )
         sprintf( buf, "abandoned" );
      else if( ch->pcdata->favor > -2000 )
         sprintf( buf, "despised" );
      else if( ch->pcdata->favor > -2250 )
         sprintf( buf, "hated" );
      else
         sprintf( buf, "damned" );
      pager_printf( ch, "&CDeity:  &Y%-20s  &CFavor: &G%s&C\r\n", ch->pcdata->deity->name, buf );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "&COrder:  &W%-20s  &COrder Mkills:  &W%-6d   &COrder MDeaths:  &W%-6d\r\n",
                    ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "&CGuild:  &W%-20s  &CGuild Mkills:  &W%-6d   &CGuild MDeaths:  &W%-6d\r\n",
                    ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths );
   }
   if( IS_IMMORTAL( ch ) )
   {
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );

/*
	pager_printf(ch, "&CIMMORTAL DATA:  &wWizinvis &C[&W%s&C]  &wInvlevel &C[&W%d&C]\r\n", xIS_SET(ch->act, PLR_WIZINVIS) ? "*" : " ", ch->pcdata->wizinvis );
	pager_printf(ch, "&CBamfin:  &Y%s%s\r\n", ((ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0') ? "" : ch->name), (ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0') ? ch->pcdata->bamfin : " appears in a swirling mist.");
	pager_printf(ch, "&CBamfout: &Y%s%s\r\n", ((ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0') ? "" : ch->name), (ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0') ? ch->pcdata->bamfout : " leaves in a swirling mist.");
*/

/* Zebeid Fix */
      if( ch->pcdata->wizinvis )
         pager_printf( ch, "&CIMMORTAL DATA:  &wWizinvis &C[&W%s&C]  &wInvlevel &C[&W%d&C]\r\n",
                       xIS_SET( ch->act, PLR_WIZINVIS ) ? "*" : " ", ch->pcdata->wizinvis );

      if( ch->pcdata->bamfout )
         pager_printf( ch, "&CBamfin:  &Y%s%s\r\n", ( ( ch->pcdata->bamfin[0] != '\0' ) ? "" : ch->name ),
                       ( ch->pcdata->bamfin[0] != '\0' ) ? ch->pcdata->bamfout : " appears in a swirling mist." );
      if( ch->pcdata->bamfout )
         pager_printf( ch, "&CBamfout: &Y%s%s\r\n", ( ( ch->pcdata->bamfout[0] != '\0' ) ? "" : ch->name ),
                       ( ch->pcdata->bamfout[0] != '\0' ) ? ch->pcdata->bamfout : " leaves in a swirling mist." );

      /*
       * Area Loaded info - Scryn 8/11 
       */
      if( ch->pcdata->area )
      {
         pager_printf( ch,
                       "&CVnums:   Room (&W%-5.5d &C- &W%-5.5d&C)   Object (&W%-5.5d &C- &W%-5.5d&C)   Mob (&W%-5.5d &C- &W%-5.5d&C)\r\n",
                       ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum, ch->pcdata->area->low_o_vnum,
                       ch->pcdata->area->hi_o_vnum, ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum );
         pager_printf( ch, "&CArea Loaded [&W%s&C]\r\n",
                       ( IS_SET( ch->pcdata->area->status, AREA_LOADED ) ) ? "yes" : "no" );
      }
   }
   if( ch->first_affect )
   {
      int i;
      SKILLTYPE *sktmp;

      i = 0;
      send_to_pager( "&C----------------------------------------------------------------------------\r\n", ch );
      send_to_pager( "AFFECT DATA:                            ", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
      {
         if( ( sktmp = get_skilltype( paf->type ) ) == NULL )
            continue;
         if( ch->level < 20 )
         {
            pager_printf( ch, "&C[&W%-34.34s&C]    ", sktmp->name );
            if( i == 0 )
               i = 2;
            if( ( ++i % 3 ) == 0 )
               send_to_pager( "\r\n", ch );
         }
         if( ch->level >= 20 )
         {
            if( paf->modifier == 0 )
               pager_printf( ch, "&C[&W%-24.24s&C(&w%5d rds&C)]    ", sktmp->name, paf->duration );
            else if( paf->modifier > 999 )
               pager_printf( ch, "&C[&W%-15.15s &G%7.7s &W(&w%5d rds&W)&C]    ",
                             sktmp->name, tiny_affect_loc_name( paf->location ), paf->duration );
            else
               pager_printf( ch, "&C[&W%-11.11s &w%+-3.3d &G%7.7s &W(&w%5d rds&W)&C]    ",
                             sktmp->name, paf->modifier, tiny_affect_loc_name( paf->location ), paf->duration );
            if( i == 0 )
               i = 1;
            if( ( ++i % 2 ) == 0 )
               send_to_pager( "\r\n", ch );
         }
      }
   }
   send_to_pager( "\r\n", ch );
   return;
}

void do_newscore( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;

   if( IS_NPC( ch ) )
   {
      do_oldscore( ch, argument );
      return;
   }
   set_pager_color( AT_SCORE, ch );

   pager_printf_color( ch, "\r\n&C%s%s.\r\n", ch->name, ch->pcdata->title );
   if( get_trust( ch ) != ch->level )
      pager_printf( ch, "You are trusted at level %d.\r\n", get_trust( ch ) );

   send_to_pager_color( "&W----------------------------------------------------------------------------\r\n", ch );

   pager_printf_color( ch, "Level: &W%-3d         &CRace : &W%-10.10s        &CPlayed: &W%d &Chours\r\n",
                       ch->level, capitalize( get_race( ch ) ), ( get_age( ch ) - 17 ) * 2 );
   pager_printf_color( ch, "&CYears: &W%-6d      &CClass: &W%-11.11s       &CLog In: %s\r",
                       get_age( ch ), capitalize( get_class( ch ) ), ctime( &( ch->logon ) ) );

   if( ch->level >= 15 || IS_PKILL( ch ) )
   {
      pager_printf_color( ch, "&CSTR  : &W%2.2d&C(&w%2.2d&C)    HitRoll: &R%-4d               &CSaved: %s\r",
                          get_curr_str( ch ), ch->perm_str, GET_HITROLL( ch ),
                          ch->save_time ? ctime( &( ch->save_time ) ) : "no save this session\n" );

      pager_printf_color( ch, "&CINT  : &W%2.2d&C(&w%2.2d&C)    DamRoll: &R%-4d                &CTime: %s\r",
                          get_curr_int( ch ), ch->perm_int, GET_DAMROLL( ch ), ctime( &current_time ) );
   }
   else
   {
      pager_printf_color( ch, "&CSTR  : &W%2.2d&C(&w%2.2d&C)                               Saved:  %s\r",
                          get_curr_str( ch ), ch->perm_str, ch->save_time ? ctime( &( ch->save_time ) ) : "no\n" );

      pager_printf_color( ch, "&CINT  : &W%2.2d&C(&w%2.2d&C)                               Time:   %s\r",
                          get_curr_int( ch ), ch->perm_int, ctime( &current_time ) );
   }

   if( GET_AC( ch ) >= 101 )
      sprintf( buf, "the rags of a beggar" );
   else if( GET_AC( ch ) >= 80 )
      sprintf( buf, "improper for adventure" );
   else if( GET_AC( ch ) >= 55 )
      sprintf( buf, "shabby and threadbare" );
   else if( GET_AC( ch ) >= 40 )
      sprintf( buf, "of poor quality" );
   else if( GET_AC( ch ) >= 20 )
      sprintf( buf, "scant protection" );
   else if( GET_AC( ch ) >= 10 )
      sprintf( buf, "that of a knave" );
   else if( GET_AC( ch ) >= 0 )
      sprintf( buf, "moderately crafted" );
   else if( GET_AC( ch ) >= -10 )
      sprintf( buf, "well crafted" );
   else if( GET_AC( ch ) >= -20 )
      sprintf( buf, "the envy of squires" );
   else if( GET_AC( ch ) >= -40 )
      sprintf( buf, "excellently crafted" );
   else if( GET_AC( ch ) >= -60 )
      sprintf( buf, "the envy of knights" );
   else if( GET_AC( ch ) >= -80 )
      sprintf( buf, "the envy of barons" );
   else if( GET_AC( ch ) >= -100 )
      sprintf( buf, "the envy of dukes" );
   else if( GET_AC( ch ) >= -200 )
      sprintf( buf, "the envy of emperors" );
   else
      sprintf( buf, "that of an avatar" );
   if( ch->level > 24 )
      pager_printf_color( ch, "&CWIS  : &W%2.2d&C(&w%2.2d&C)      Armor: &W%-d; %s\r\n",
                          get_curr_wis( ch ), ch->perm_wis, GET_AC( ch ), buf );
   else
      pager_printf_color( ch, "&CWIS  : &W%2.2d&C(&w%2.2d&C)      Armor: &W%s \r\n", get_curr_wis( ch ), ch->perm_wis, buf );

   if( ch->alignment > 900 )
      sprintf( buf, "devout" );
   else if( ch->alignment > 700 )
      sprintf( buf, "noble" );
   else if( ch->alignment > 350 )
      sprintf( buf, "honorable" );
   else if( ch->alignment > 100 )
      sprintf( buf, "worthy" );
   else if( ch->alignment > -100 )
      sprintf( buf, "neutral" );
   else if( ch->alignment > -350 )
      sprintf( buf, "base" );
   else if( ch->alignment > -700 )
      sprintf( buf, "evil" );
   else if( ch->alignment > -900 )
      sprintf( buf, "ignoble" );
   else
      sprintf( buf, "fiendish" );
   if( ch->level < 10 )
      pager_printf_color( ch, "&CDEX  : &W%2.2d&C(&w%2.2d&C)      Align: &W%-20.20s    &CItems:  &W%d (max %d)\r\n",
                          get_curr_dex( ch ), ch->perm_dex, buf, ch->carry_number, can_carry_n( ch ) );
   else
      pager_printf_color( ch, "&CDEX  : &W%2.2d&C(&w%2.2d&C)      Align: &W%4d; %-14.14s   &CItems:  &W%d &w(max %d)\r\n",
                          get_curr_dex( ch ), ch->perm_dex, ch->alignment, buf, ch->carry_number, can_carry_n( ch ) );

   switch ( ch->position )
   {
      case POS_DEAD:
         sprintf( buf, "slowly decomposing" );
         break;
      case POS_MORTAL:
         sprintf( buf, "mortally wounded" );
         break;
      case POS_INCAP:
         sprintf( buf, "incapacitated" );
         break;
      case POS_STUNNED:
         sprintf( buf, "stunned" );
         break;
      case POS_SLEEPING:
         sprintf( buf, "sleeping" );
         break;
      case POS_RESTING:
         sprintf( buf, "resting" );
         break;
      case POS_STANDING:
         sprintf( buf, "standing" );
         break;
      case POS_FIGHTING:
         sprintf( buf, "fighting" );
         break;
/* Fighting style support -haus */
      case POS_EVASIVE:
         sprintf( buf, "fighting (evasive)" );
         break;
      case POS_DEFENSIVE:
         sprintf( buf, "fighting (defensive)" );
         break;
      case POS_AGGRESSIVE:
         sprintf( buf, "fighting (aggressive)" );
         break;
      case POS_BERSERK:
         sprintf( buf, "fighting (berserk)" );
         break;
      case POS_MOUNTED:
         sprintf( buf, "mounted" );
         break;
      case POS_SITTING:
         sprintf( buf, "sitting" );
         break;
   }
   pager_printf_color( ch, "&CCON  : &W%2.2d&C(&w%2.2d&C)      Pos'n: &W%-21.21s  &CWeight: &W%d &w(max %d)\r\n",
                       get_curr_con( ch ), ch->perm_con, buf, ch->carry_weight, can_carry_w( ch ) );


   /*
    * Fighting style support -haus
    */
   pager_printf_color( ch, "&CCHA  : &W%2.2d&C(&w%2.2d&C)      Wimpy: &Y%-5d      ",
                       get_curr_cha( ch ), ch->perm_cha, ch->wimpy );

   switch ( ch->style )
   {
      case STYLE_EVASIVE:
         sprintf( buf, "evasive" );
         break;
      case STYLE_DEFENSIVE:
         sprintf( buf, "defensive" );
         break;
      case STYLE_AGGRESSIVE:
         sprintf( buf, "aggressive" );
         break;
      case STYLE_BERSERK:
         sprintf( buf, "berserk" );
         break;
      default:
         sprintf( buf, "standard" );
         break;
   }
   pager_printf_color( ch, "\r\n&CLCK  : &W%2.2d&C(&w%2.2d&C)      Style: &R%-10.10s&C\r\n",
                       get_curr_lck( ch ), ch->perm_lck, buf );

   pager_printf_color( ch, "&CGlory: &W%d&C/&w%d\r\n", ch->pcdata->quest_curr, ch->pcdata->quest_accum );

   pager_printf_color( ch,
                       "&CPRACT: &W%3d         &CHitpoints: &G%-5d &Cof &g%5d   &CPager: (&W%c&C) &W%3d    &CAutoExit(&W%c&C)\r\n",
                       ch->practice, ch->hit, ch->max_hit, IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) ? 'X' : ' ',
                       ch->pcdata->pagerlen, xIS_SET( ch->act, PLR_AUTOEXIT ) ? 'X' : ' ' );

   if( IS_VAMPIRE( ch ) )
      pager_printf_color( ch,
                          "&CEXP  : &W%-9d       &CBlood: &R%-5d &Cof &r%5d   &CMKills:  &W%5d    &CAutoLoot(&W%c&C)\r\n",
                          ch->exp, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->pcdata->mkills,
                          xIS_SET( ch->act, PLR_AUTOLOOT ) ? 'X' : ' ' );
   else if( ch->class == CLASS_WARRIOR )
      pager_printf_color( ch, "&CEXP  : &W%-9d                               &CMKills:  &W%5d    &CAutoLoot(&W%c&C)\r\n",
                          ch->exp, ch->pcdata->mkills, xIS_SET( ch->act, PLR_AUTOLOOT ) ? 'X' : ' ' );
   else
      pager_printf_color( ch,
                          "&CEXP  : &W%-9d        &CMana: &B%-5d &Cof &b%5d   &CMKills:  &W%5d    &CAutoLoot(&W%c&C)\r\n",
                          ch->exp, ch->mana, ch->max_mana, ch->pcdata->mkills, xIS_SET( ch->act,
                                                                                        PLR_AUTOLOOT ) ? 'X' : ' ' );

   pager_printf_color( ch, "&CGOLD : &Y%-10d       &CMove: &W%-5d &Cof &w%5d   &CMdeaths: &W%5d    &CAutoSac (&W%c&C)\r\n",
                       ch->gold, ch->move, ch->max_move, ch->pcdata->mdeaths, xIS_SET( ch->act, PLR_AUTOSAC ) ? 'X' : ' ' );

   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
      send_to_pager( "You are drunk.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
      send_to_pager( "You are in danger of dehydrating.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
      send_to_pager( "You are starving to death.\r\n", ch );
   if( ch->position != POS_SLEEPING )
      switch ( ch->mental_state / 10 )
      {
         default:
            send_to_pager( "You're completely messed up!\r\n", ch );
            break;
         case -10:
            send_to_pager( "You're barely conscious.\r\n", ch );
            break;
         case -9:
            send_to_pager( "You can barely keep your eyes open.\r\n", ch );
            break;
         case -8:
            send_to_pager( "You're extremely drowsy.\r\n", ch );
            break;
         case -7:
            send_to_pager( "You feel very unmotivated.\r\n", ch );
            break;
         case -6:
            send_to_pager( "You feel sedated.\r\n", ch );
            break;
         case -5:
            send_to_pager( "You feel sleepy.\r\n", ch );
            break;
         case -4:
            send_to_pager( "You feel tired.\r\n", ch );
            break;
         case -3:
            send_to_pager( "You could use a rest.\r\n", ch );
            break;
         case -2:
            send_to_pager( "You feel a little under the weather.\r\n", ch );
            break;
         case -1:
            send_to_pager( "You feel fine.\r\n", ch );
            break;
         case 0:
            send_to_pager( "You feel great.\r\n", ch );
            break;
         case 1:
            send_to_pager( "You feel energetic.\r\n", ch );
            break;
         case 2:
            send_to_pager( "Your mind is racing.\r\n", ch );
            break;
         case 3:
            send_to_pager( "You can't think straight.\r\n", ch );
            break;
         case 4:
            send_to_pager( "Your mind is going 100 miles an hour.\r\n", ch );
            break;
         case 5:
            send_to_pager( "You're high as a kite.\r\n", ch );
            break;
         case 6:
            send_to_pager( "Your mind and body are slipping apart.\r\n", ch );
            break;
         case 7:
            send_to_pager( "Reality is slipping away.\r\n", ch );
            break;
         case 8:
            send_to_pager( "You have no idea what is real, and what is not.\r\n", ch );
            break;
         case 9:
            send_to_pager( "You feel immortal.\r\n", ch );
            break;
         case 10:
            send_to_pager( "You are a Supreme Entity.\r\n", ch );
            break;
      }
   else if( ch->mental_state > 45 )
      send_to_pager( "Your sleep is filled with strange and vivid dreams.\r\n", ch );
   else if( ch->mental_state > 25 )
      send_to_pager( "Your sleep is uneasy.\r\n", ch );
   else if( ch->mental_state < -35 )
      send_to_pager( "You are deep in a much needed sleep.\r\n", ch );
   else if( ch->mental_state < -25 )
      send_to_pager( "You are in deep slumber.\r\n", ch );

/* Languages - Ntanel
  send_to_pager("Languages: ", ch );
    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	if ( knows_language( ch, lang_array[iLang], ch )
	||  (IS_NPC(ch) && ch->speaks == 0) )
	{
	    if ( lang_array[iLang] & ch->speaking
	    ||  (IS_NPC(ch) && !ch->speaking) )
		set_pager_color( AT_RED, ch );
	    send_to_pager( lang_names[iLang], ch );
	    send_to_pager( " ", ch );
	    set_pager_color( AT_SCORE, ch );
	}
    send_to_pager( "\r\n", ch );
*/
   if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
      pager_printf_color( ch, "&CYou are bestowed with the command(s): &Y%s\r\n", ch->pcdata->bestowments );

   if( ch->morph && ch->morph->morph )
   {
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
      if( IS_IMMORTAL( ch ) )
         pager_printf( ch, "Morphed as (%d) %s with a timer of %d.\r\n",
                       ch->morph->morph->vnum, ch->morph->morph->short_desc, ch->morph->timer );
      else
         pager_printf( ch, "You are morphed into a %s.\r\n", ch->morph->morph->short_desc );
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
   }
   if( CAN_PKILL( ch ) )
   {
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
      pager_printf_color( ch, "&CPKILL DATA:  Pkills (&W%d&C)     Illegal Pkills (&W%d&C)     Pdeaths (&W%d&C)\r\n",
                          ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
   {

/* Clan Break - Ntanel */
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );

      pager_printf_color( ch, "&CCLAN STATS:  &W%-14.14s  &CClan AvPkills : &W%-5d  &CClan NonAvpkills : &W%-5d\r\n",
                          ch->pcdata->clan->name, ch->pcdata->clan->pkills[5],
                          ( ch->pcdata->clan->pkills[0] + ch->pcdata->clan->pkills[1] +
                            ch->pcdata->clan->pkills[2] + ch->pcdata->clan->pkills[3] + ch->pcdata->clan->pkills[4] ) );
      pager_printf_color( ch, "&C                             Clan AvPdeaths: &W%-5d  &CClan NonAvpdeaths: &W%-5d\r\n",
                          ch->pcdata->clan->pdeaths[5],
                          ( ch->pcdata->clan->pdeaths[0] + ch->pcdata->clan->pdeaths[1] +
                            ch->pcdata->clan->pdeaths[2] + ch->pcdata->clan->pdeaths[3] + ch->pcdata->clan->pdeaths[4] ) );
   }
   if( ch->pcdata->deity )
   {
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
      if( ch->pcdata->favor > 2250 )
         sprintf( buf, "loved" );
      else if( ch->pcdata->favor > 2000 )
         sprintf( buf, "cherished" );
      else if( ch->pcdata->favor > 1750 )
         sprintf( buf, "honored" );
      else if( ch->pcdata->favor > 1500 )
         sprintf( buf, "praised" );
      else if( ch->pcdata->favor > 1250 )
         sprintf( buf, "favored" );
      else if( ch->pcdata->favor > 1000 )
         sprintf( buf, "respected" );
      else if( ch->pcdata->favor > 750 )
         sprintf( buf, "liked" );
      else if( ch->pcdata->favor > 250 )
         sprintf( buf, "tolerated" );
      else if( ch->pcdata->favor > -250 )
         sprintf( buf, "ignored" );
      else if( ch->pcdata->favor > -750 )
         sprintf( buf, "shunned" );
      else if( ch->pcdata->favor > -1000 )
         sprintf( buf, "disliked" );
      else if( ch->pcdata->favor > -1250 )
         sprintf( buf, "dishonored" );
      else if( ch->pcdata->favor > -1500 )
         sprintf( buf, "disowned" );
      else if( ch->pcdata->favor > -1750 )
         sprintf( buf, "abandoned" );
      else if( ch->pcdata->favor > -2000 )
         sprintf( buf, "despised" );
      else if( ch->pcdata->favor > -2250 )
         sprintf( buf, "hated" );
      else
         sprintf( buf, "damned" );
      pager_printf_color( ch, "&CDeity:  &W%-20s &CFavor:  &W%s&C\r\n", ch->pcdata->deity->name, buf );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
   {
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
      pager_printf_color( ch, "&COrder:  &W%-20s  &COrder Mkills:  &W%-6d   &COrder MDeaths:  &W%-6d\r\n",
                          ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD )
   {
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
      pager_printf_color( ch, "&CGuild:  &W%-20s  &CGuild Mkills:  &W%-6d   &CGuild MDeaths:  &W%-6d\r\n",
                          ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths );
   }
   argument = one_argument( argument, arg );
   if( ch->first_affect && !str_cmp( arg, "affects" ) )
   {
      int i;
      SKILLTYPE *sktmp;

      i = 0;
      send_to_pager_color( "&W----------------------------------------------------------------------------&C\r\n", ch );
      send_to_pager_color( "AFFECT DATA:                            ", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
      {
         if( ( sktmp = get_skilltype( paf->type ) ) == NULL )
            continue;
         if( ch->level < 20 )
         {
            pager_printf_color( ch, "&C[&W%-34.34s&C]    ", sktmp->name );
            if( i == 0 )
               i = 2;
            if( ( ++i % 3 ) == 0 )
               send_to_pager( "\r\n", ch );
         }
         if( ch->level >= 20 )
         {
            if( paf->modifier == 0 )
               pager_printf_color( ch, "&C[&W%-24.24s;%5d &Crds]    ", sktmp->name, paf->duration );
            else if( paf->modifier > 999 )
               pager_printf_color( ch, "&C[&W%-15.15s; %7.7s;%5d &Crds]    ",
                                   sktmp->name, tiny_affect_loc_name( paf->location ), paf->duration );
            else
               pager_printf_color( ch, "&C[&W%-11.11s;%+-3.3d %7.7s;%5d &Crds]    ",
                                   sktmp->name, paf->modifier, tiny_affect_loc_name( paf->location ), paf->duration );
            if( i == 0 )
               i = 1;
            if( ( ++i % 2 ) == 0 )
               send_to_pager( "\r\n", ch );
         }
      }
   }
   send_to_pager( "\r\n", ch );
   return;
}

/*
 * Return ascii name of an affect location.
 */
char *tiny_affect_loc_name( int location )
{
   switch ( location )
   {
      case APPLY_NONE:
         return "NIL";
      case APPLY_STR:
         return " STR  ";
      case APPLY_DEX:
         return " DEX  ";
      case APPLY_INT:
         return " INT  ";
      case APPLY_WIS:
         return " WIS  ";
      case APPLY_CON:
         return " CON  ";
      case APPLY_CHA:
         return " CHA  ";
      case APPLY_LCK:
         return " LCK  ";
      case APPLY_SEX:
         return " SEX  ";
      case APPLY_CLASS:
         return " CLASS";
      case APPLY_LEVEL:
         return " LVL  ";
      case APPLY_AGE:
         return " AGE  ";
      case APPLY_MANA:
         return " MANA ";
      case APPLY_HIT:
         return " HV   ";
      case APPLY_MOVE:
         return " MOVE ";
      case APPLY_GOLD:
         return " GOLD ";
      case APPLY_EXP:
         return " EXP  ";
      case APPLY_AC:
         return " AC   ";
      case APPLY_HITROLL:
         return " HITRL";
      case APPLY_DAMROLL:
         return " DAMRL";
      case APPLY_SAVING_POISON:
         return "SV POI";
      case APPLY_SAVING_ROD:
         return "SV ROD";
      case APPLY_SAVING_PARA:
         return "SV PARA";
      case APPLY_SAVING_BREATH:
         return "SV BRTH";
      case APPLY_SAVING_SPELL:
         return "SV SPLL";
      case APPLY_HEIGHT:
         return "HEIGHT";
      case APPLY_WEIGHT:
         return "WEIGHT";
      case APPLY_AFFECT:
         return "AFF BY";
      case APPLY_RESISTANT:
         return "RESIST";
      case APPLY_IMMUNE:
         return "IMMUNE";
      case APPLY_SUSCEPTIBLE:
         return "SUSCEPT";
      case APPLY_WEAPONSPELL:
         return " WEAPON";
      case APPLY_BACKSTAB:
         return "BACKSTB";
      case APPLY_PICK:
         return " PICK  ";
      case APPLY_TRACK:
         return " TRACK ";
      case APPLY_STEAL:
         return " STEAL ";
      case APPLY_SNEAK:
         return " SNEAK ";
      case APPLY_HIDE:
         return " HIDE  ";
      case APPLY_PALM:
         return " PALM  ";
      case APPLY_DETRAP:
         return " DETRAP";
      case APPLY_DODGE:
         return " DODGE ";
      case APPLY_PEEK:
         return " PEEK  ";
      case APPLY_SCAN:
         return " SCAN  ";
      case APPLY_GOUGE:
         return " GOUGE ";
      case APPLY_SEARCH:
         return " SEARCH";
      case APPLY_MOUNT:
         return " MOUNT ";
      case APPLY_DISARM:
         return " DISARM";
      case APPLY_KICK:
         return " KICK  ";
      case APPLY_PARRY:
         return " PARRY ";
      case APPLY_BASH:
         return " BASH  ";
      case APPLY_STUN:
         return " STUN  ";
      case APPLY_PUNCH:
         return " PUNCH ";
      case APPLY_CLIMB:
         return " CLIMB ";
      case APPLY_GRIP:
         return " GRIP  ";
      case APPLY_SCRIBE:
         return " SCRIBE";
      case APPLY_BREW:
         return " BREW  ";
      case APPLY_WEARSPELL:
         return " WEAR  ";
      case APPLY_REMOVESPELL:
         return " REMOVE";
      case APPLY_EMOTION:
         return "EMOTION";
      case APPLY_MENTALSTATE:
         return " MENTAL";
      case APPLY_STRIPSN:
         return " DISPEL";
      case APPLY_REMOVE:
         return " REMOVE";
      case APPLY_DIG:
         return " DIG   ";
      case APPLY_FULL:
         return " HUNGER";
      case APPLY_THIRST:
         return " THIRST";
      case APPLY_DRUNK:
         return " DRUNK ";
      case APPLY_BLOOD:
         return " BLOOD ";
      case APPLY_COOK:
         return " COOK  ";
      case APPLY_RECURRINGSPELL:
         return " RECURR";
      case APPLY_CONTAGIOUS:
         return "CONTGUS";
      case APPLY_ODOR:
         return " ODOR  ";
      case APPLY_ROOMFLAG:
         return " RMFLG ";
      case APPLY_SECTORTYPE:
         return " SECTOR";
      case APPLY_ROOMLIGHT:
         return " LIGHT ";
      case APPLY_TELEVNUM:
         return " TELEVN";
      case APPLY_TELEDELAY:
         return " TELEDY";
   };

   bug( "Affect_location_name: unknown location %d.", location );
   return "(?!?!?)";
}

char *get_class( CHAR_DATA * ch )
{
   if( ch->class < MAX_NPC_CLASS && ch->class >= 0 )
      return ( npc_class[ch->class] );
   return ( "Unknown" );
}


char *get_race( CHAR_DATA * ch )
{
   if( ch->race < MAX_RACE && ch->race >= 0 )
      return ( race_table[ch->race]->race_name );
   if( ch->race < MAX_NPC_RACE && ch->race >= 0 )
      return ( npc_race[ch->race] );
   return ( "Unknown" );
}

void do_oldscore( CHAR_DATA * ch, char *argument )
{
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   set_pager_color( AT_SCORE, ch );
   pager_printf( ch,
                 "You are %s%s, level %d, %d years old (%d hours).\r\n",
                 ch->name, IS_NPC( ch ) ? "" : ch->pcdata->title, ch->level, get_age( ch ), ( get_age( ch ) - 17 ) * 2 );

   if( get_trust( ch ) != ch->level )
      pager_printf( ch, "You are trusted at level %d.\r\n", get_trust( ch ) );

   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_MOBINVIS ) )
      pager_printf( ch, "You are mobinvis at level %d.\r\n", ch->mobinvis );

   if( !IS_NPC( ch ) && IS_VAMPIRE( ch ) )
      pager_printf( ch,
                    "You have %d/%d hit, %d/%d blood level, %d/%d movement, %d practices.\r\n",
                    ch->hit, ch->max_hit,
                    ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->practice );
   else
      pager_printf( ch,
                    "You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\r\n",
                    ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->practice );

   pager_printf( ch,
                 "You are carrying %d/%d items with weight %d/%d kg.\r\n",
                 ch->carry_number, can_carry_n( ch ), ch->carry_weight, can_carry_w( ch ) );

   pager_printf( ch,
                 "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d.\r\n",
                 get_curr_str( ch ),
                 get_curr_int( ch ),
                 get_curr_wis( ch ), get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );

   pager_printf( ch, "You have scored %d exp, and have %d gold coins.\r\n", ch->exp, ch->gold );

   if( !IS_NPC( ch ) )
      pager_printf( ch,
                    "You have achieved %d glory during your life, and currently have %d.\r\n",
                    ch->pcdata->quest_accum, ch->pcdata->quest_curr );

   pager_printf( ch,
                 "Autoexit: %s   Autoloot: %s   Autosac: %s   Autogold: %s\r\n",
                 ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOEXIT ) ) ? "yes" : "no",
                 ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOLOOT ) ) ? "yes" : "no",
                 ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOSAC ) ) ? "yes" : "no",
                 ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOGOLD ) ) ? "yes" : "no" );

   pager_printf( ch, "Wimpy set to %d hit points.\r\n", ch->wimpy );

   if( !IS_NPC( ch ) )
   {
      if( ch->pcdata->condition[COND_DRUNK] > 10 )
         send_to_pager( "You are drunk.\r\n", ch );
      if( ch->pcdata->condition[COND_THIRST] == 0 )
         send_to_pager( "You are thirsty.\r\n", ch );
      if( ch->pcdata->condition[COND_FULL] == 0 )
         send_to_pager( "You are hungry.\r\n", ch );
   }

   switch ( ch->mental_state / 10 )
   {
      default:
         send_to_pager( "You're completely messed up!\r\n", ch );
         break;
      case -10:
         send_to_pager( "You're barely conscious.\r\n", ch );
         break;
      case -9:
         send_to_pager( "You can barely keep your eyes open.\r\n", ch );
         break;
      case -8:
         send_to_pager( "You're extremely drowsy.\r\n", ch );
         break;
      case -7:
         send_to_pager( "You feel very unmotivated.\r\n", ch );
         break;
      case -6:
         send_to_pager( "You feel sedated.\r\n", ch );
         break;
      case -5:
         send_to_pager( "You feel sleepy.\r\n", ch );
         break;
      case -4:
         send_to_pager( "You feel tired.\r\n", ch );
         break;
      case -3:
         send_to_pager( "You could use a rest.\r\n", ch );
         break;
      case -2:
         send_to_pager( "You feel a little under the weather.\r\n", ch );
         break;
      case -1:
         send_to_pager( "You feel fine.\r\n", ch );
         break;
      case 0:
         send_to_pager( "You feel great.\r\n", ch );
         break;
      case 1:
         send_to_pager( "You feel energetic.\r\n", ch );
         break;
      case 2:
         send_to_pager( "Your mind is racing.\r\n", ch );
         break;
      case 3:
         send_to_pager( "You can't think straight.\r\n", ch );
         break;
      case 4:
         send_to_pager( "Your mind is going 100 miles an hour.\r\n", ch );
         break;
      case 5:
         send_to_pager( "You're high as a kite.\r\n", ch );
         break;
      case 6:
         send_to_pager( "Your mind and body are slipping appart.\r\n", ch );
         break;
      case 7:
         send_to_pager( "Reality is slipping away.\r\n", ch );
         break;
      case 8:
         send_to_pager( "You have no idea what is real, and what is not.\r\n", ch );
         break;
      case 9:
         send_to_pager( "You feel immortal.\r\n", ch );
         break;
      case 10:
         send_to_pager( "You are a Supreme Entity.\r\n", ch );
         break;
   }

   switch ( ch->position )
   {
      case POS_DEAD:
         send_to_pager( "You are DEAD!!\r\n", ch );
         break;
      case POS_MORTAL:
         send_to_pager( "You are mortally wounded.\r\n", ch );
         break;
      case POS_INCAP:
         send_to_pager( "You are incapacitated.\r\n", ch );
         break;
      case POS_STUNNED:
         send_to_pager( "You are stunned.\r\n", ch );
         break;
      case POS_SLEEPING:
         send_to_pager( "You are sleeping.\r\n", ch );
         break;
      case POS_RESTING:
         send_to_pager( "You are resting.\r\n", ch );
         break;
      case POS_STANDING:
         send_to_pager( "You are standing.\r\n", ch );
         break;
      case POS_FIGHTING:
         send_to_pager( "You are fighting.\r\n", ch );
         break;
      case POS_MOUNTED:
         send_to_pager( "Mounted.\r\n", ch );
         break;
      case POS_SHOVE:
         send_to_pager( "Being shoved.\r\n", ch );
         break;
      case POS_DRAG:
         send_to_pager( "Being dragged.\r\n", ch );
         break;
   }

   if( ch->level >= 25 )
      pager_printf( ch, "AC: %d.  ", GET_AC( ch ) );

   send_to_pager( "You are ", ch );
   if( GET_AC( ch ) >= 101 )
      send_to_pager( "WORSE than naked!\r\n", ch );
   else if( GET_AC( ch ) >= 80 )
      send_to_pager( "naked.\r\n", ch );
   else if( GET_AC( ch ) >= 60 )
      send_to_pager( "wearing clothes.\r\n", ch );
   else if( GET_AC( ch ) >= 40 )
      send_to_pager( "slightly armored.\r\n", ch );
   else if( GET_AC( ch ) >= 20 )
      send_to_pager( "somewhat armored.\r\n", ch );
   else if( GET_AC( ch ) >= 0 )
      send_to_pager( "armored.\r\n", ch );
   else if( GET_AC( ch ) >= -20 )
      send_to_pager( "well armored.\r\n", ch );
   else if( GET_AC( ch ) >= -40 )
      send_to_pager( "strongly armored.\r\n", ch );
   else if( GET_AC( ch ) >= -60 )
      send_to_pager( "heavily armored.\r\n", ch );
   else if( GET_AC( ch ) >= -80 )
      send_to_pager( "superbly armored.\r\n", ch );
   else if( GET_AC( ch ) >= -100 )
      send_to_pager( "divinely armored.\r\n", ch );
   else
      send_to_pager( "invincible!\r\n", ch );

   if( ch->level >= 15 || IS_PKILL( ch ) )
      pager_printf( ch, "Hitroll: %d  Damroll: %d.\r\n", GET_HITROLL( ch ), GET_DAMROLL( ch ) );

   if( ch->level >= 10 )
      pager_printf( ch, "Alignment: %d.  ", ch->alignment );

   send_to_pager( "You are ", ch );
   if( ch->alignment > 900 )
      send_to_pager( "angelic.\r\n", ch );
   else if( ch->alignment > 700 )
      send_to_pager( "saintly.\r\n", ch );
   else if( ch->alignment > 350 )
      send_to_pager( "good.\r\n", ch );
   else if( ch->alignment > 100 )
      send_to_pager( "kind.\r\n", ch );
   else if( ch->alignment > -100 )
      send_to_pager( "neutral.\r\n", ch );
   else if( ch->alignment > -350 )
      send_to_pager( "mean.\r\n", ch );
   else if( ch->alignment > -700 )
      send_to_pager( "evil.\r\n", ch );
   else if( ch->alignment > -900 )
      send_to_pager( "demonic.\r\n", ch );
   else
      send_to_pager( "satanic.\r\n", ch );

   if( ch->first_affect )
   {
      send_to_pager( "You are affected by:\r\n", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
         if( ( skill = get_skilltype( paf->type ) ) != NULL )
         {
            pager_printf( ch, "Spell: '%s'", skill->name );

            if( ch->level >= 20 )
               pager_printf( ch,
                             " modifies %s by %d for %d rounds",
                             affect_loc_name( paf->location ), paf->modifier, paf->duration );

            send_to_pager( ".\r\n", ch );
         }
   }

   if( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
   {
      pager_printf( ch, "\r\nWizInvis level: %d   WizInvis is %s\r\n",
                    ch->pcdata->wizinvis, xIS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
      if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
         pager_printf( ch, "Room Range: %d - %d\r\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
      if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
         pager_printf( ch, "Obj Range : %d - %d\r\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
      if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
         pager_printf( ch, "Mob Range : %d - %d\r\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
   }

   return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   int x, lowlvl, hilvl;

   if( ch->level == 1 )
      lowlvl = 1;
   else
      lowlvl = UMAX( 2, ch->level - 5 );
   hilvl = URANGE( ch->level, ch->level + 5, MAX_LEVEL );
   set_char_color( AT_SCORE, ch );
   ch_printf( ch, "\r\nExperience required, levels %d to %d:\r\n______________________________________________\r\n\r\n",
              lowlvl, hilvl );
   sprintf( buf, " exp  (You have: %11d)", ch->exp );
   sprintf( buf2, " exp  (To level: %11d)", exp_level( ch, ch->level + 1 ) - ch->exp );
   for( x = lowlvl; x <= hilvl; x++ )
      ch_printf( ch, " (%2d) %11d%s\r\n", x, exp_level( ch, x ),
                 ( x == ch->level ) ? buf : ( x == ch->level + 1 ) ? buf2 : " exp" );
   send_to_char( "______________________________________________\r\n", ch );
}

/* 1997, Blodkai */
void do_remains( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   bool found = FALSE;

   if( IS_NPC( ch ) )
      return;
   set_char_color( AT_MAGIC, ch );
   if( !ch->pcdata->deity )
   {
      send_to_pager( "You have no deity from which to seek such assistance...\r\n", ch );
      return;
   }
   if( ch->pcdata->favor < ch->level * 2 )
   {
      send_to_pager( "Your favor is insufficient for such assistance...\r\n", ch );
      return;
   }
   pager_printf( ch, "%s appears in a vision, revealing that your remains... ", ch->pcdata->deity->name );
   sprintf( buf, "the corpse of %s", ch->name );
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && !str_cmp( buf, obj->short_descr ) && ( obj->pIndexData->vnum == 11 ) )
      {
         found = TRUE;
         pager_printf( ch, "\r\n  - at %s will endure for %d ticks", obj->in_room->name, obj->timer );
      }
   }
   if( !found )
      send_to_pager( " no longer exist.\r\n", ch );
   else
   {
      send_to_pager( "\r\n", ch );
      ch->pcdata->favor -= ch->level * 2;
   }
   return;
}

/* Affects-at-a-glance, Blodkai */
void do_affected( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );

   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "by" ) )
   {
      send_to_char_color( "\r\n&BImbued with:\r\n", ch );
      ch_printf_color( ch, "&C%s\r\n", !xIS_EMPTY( ch->affected_by ) ? affect_bit_name( &ch->affected_by ) : "nothing" );
      if( ch->level >= 20 )
      {
         send_to_char( "\r\n", ch );
         if( ch->resistant > 0 )
         {
            send_to_char_color( "&BResistances:  ", ch );
            ch_printf_color( ch, "&C%s\r\n", flag_string( ch->resistant, ris_flags ) );
         }
         if( ch->immune > 0 )
         {
            send_to_char_color( "&BImmunities:   ", ch );
            ch_printf_color( ch, "&C%s\r\n", flag_string( ch->immune, ris_flags ) );
         }
         if( ch->susceptible > 0 )
         {
            send_to_char_color( "&BSuscepts:     ", ch );
            ch_printf_color( ch, "&C%s\r\n", flag_string( ch->susceptible, ris_flags ) );
         }
      }
      return;
   }

   if( !ch->first_affect )
   {
      send_to_char_color( "\r\n&CNo cantrip or skill affects you.\r\n", ch );
   }
   else
   {
      send_to_char( "\r\n", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
         if( ( skill = get_skilltype( paf->type ) ) != NULL )
         {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Affected:  ", ch );
            set_char_color( AT_SCORE, ch );
            if( ch->level >= 20 || IS_PKILL( ch ) )
            {
               if( paf->duration < 25 )
                  set_char_color( AT_WHITE, ch );
               if( paf->duration < 6 )
                  set_char_color( AT_WHITE + AT_BLINK, ch );
               ch_printf( ch, "(%5d)   ", paf->duration );
            }
            ch_printf( ch, "%-18s\r\n", skill->name );
         }
   }
   return;
}

void do_inventory( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "You are carrying:\r\n", ch );
   show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
   return;
}


void do_equipment( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   int iWear;
   bool found;

   set_char_color( AT_RED, ch );
   send_to_char( "You are using:\r\n", ch );
   found = FALSE;
   set_char_color( AT_OBJECT, ch );
   for( iWear = 0; iWear < MAX_WEAR; iWear++ )
   {
      for( obj = ch->first_carrying; obj; obj = obj->next_content )
         if( obj->wear_loc == iWear )
         {
            if( ( !IS_NPC( ch ) ) && ( ch->race > 0 ) && ( ch->race < MAX_RACE ) )
               send_to_char( race_table[ch->race]->where_name[iWear], ch );
            else
               send_to_char( where_name[iWear], ch );

            if( can_see_obj( ch, obj ) )
            {
               send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
               send_to_char( "\r\n", ch );
            }
            else
               send_to_char( "something.\r\n", ch );
            found = TRUE;
         }
   }

   if( !found )
      send_to_char( "Nothing.\r\n", ch );

   return;
}



void set_title( CHAR_DATA * ch, char *title )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      bug( "Set_title: NPC.", 0 );
      return;
   }

   if( isalpha( title[0] ) || isdigit( title[0] ) )
   {
      buf[0] = ' ';
      strcpy( buf + 1, title );
   }
   else
      strcpy( buf, title );

/*    if ( ch->pcdata->title) */
   STRFREE( ch->pcdata->title );
   ch->pcdata->title = STRALLOC( buf );
   return;
}

void set_extraname( CHAR_DATA * ch, char *title )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      bug( "Set_extraname: NPC.", 0 );
      return;
   }

   if( isalpha( title[0] ) || isdigit( title[0] ) )
   {
      buf[0] = ' ';
      strcpy( buf + 1, title );
   }
   else
      strcpy( buf, title );

/*    if (ch->pcdata->extraname) */
   STRFREE( ch->pcdata->extraname );
   ch->pcdata->extraname = STRALLOC( buf );
   return;
}

void set_lastname( CHAR_DATA * ch, char *title )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      bug( "Set_lastname: NPC.", 0 );
      return;
   }

   if( isalpha( title[0] ) || isdigit( title[0] ) )
   {
      buf[0] = ' ';
      strcpy( buf + 1, title );
   }
   else
      strcpy( buf, title );

   if( ch->pcdata->lastname )
      STRFREE( ch->pcdata->lastname );
   ch->pcdata->lastname = STRALLOC( buf );
   return;
}

void do_title( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );
   if( ch->level < 5 )
   {
      send_to_char( "Sorry... you must be at least level 5 to set your title...\r\n", ch );
      return;
   }
   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "The Gods prohibit you from changing your title.\r\n", ch );
      return;
   }

/* Title - Ntanel */
   if( argument[0] == '\0' )
   {
      send_to_char( "Change your title to what?\r\n", ch );
      return;
   }
   if( !str_cmp( argument, "none" ) )
   {
      argument = "";
      send_to_char( "Your title is now blank.\r\n", ch );
   }


   if( strlen( argument ) > 50 )
      argument[50] = '\0';

   smash_tilde( argument );
   set_title( ch, argument );
   send_to_char( "Ok.\r\n", ch );
}

void do_lastname( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );
   if( ch->level < 5 )
   {
      send_to_char( "Sorry... you must be at least level 5 to set your last name...\r\n", ch );
      return;
   }
   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "The Gods prohibit you from changing your last name.\r\n", ch );
      return;
   }

/* Lastname - Ntanel */

   if( argument[0] == '\0' )
   {
      send_to_char( "Change your last name to what?\r\n", ch );
      return;
   }
   if( !str_cmp( argument, "none" ) )
   {
      argument = "";
      send_to_char( "Your lastname is now blank.\r\n", ch );
   }

   if( strlen( argument ) > 15 )
      argument[15] = '\0';

   smash_tilde( argument );
   set_lastname( ch, argument );
   send_to_char( "Ok.\r\n", ch );
}

void do_extraname( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );
   if( ch->level < 5 )
   {
      send_to_char( "Sorry... you must be at least level 5 to set your extra name...\r\n", ch );
      return;
   }
   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "The Gods prohibit you from changing your extraname.\r\n", ch );
      return;
   }

/* Extraname - Ntanel */
   if( argument[0] == '\0' )
   {
      send_to_char( "Change your extraname to what?\r\n", ch );
      return;
   }
   if( !str_cmp( argument, "none" ) )
   {
      argument = "";
      send_to_char( "Your extraname is now blank.\r\n", ch );
   }


   if( strlen( argument ) > 10 )
      argument[10] = '\0';

   smash_tilde( argument );
   set_extraname( ch, argument );
   send_to_char( "Ok.\r\n", ch );
}

void do_homepage( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( ch->level < 5 )
   {
      send_to_char( "Sorry... you must be at least level 5 to do that.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->homepage )
         ch->pcdata->homepage = str_dup( "" );
      ch_printf( ch, "Your homepage is: %s\r\n", show_tilde( ch->pcdata->homepage ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->homepage )
         DISPOSE( ch->pcdata->homepage );
      ch->pcdata->homepage = str_dup( "" );
      send_to_char( "Homepage cleared.\r\n", ch );
      return;
   }

   if( strstr( argument, "://" ) )
      strcpy( buf, argument );
   else
      sprintf( buf, "http://%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->homepage )
      DISPOSE( ch->pcdata->homepage );
   ch->pcdata->homepage = str_dup( buf );
   send_to_char( "Homepage set.\r\n", ch );
}


void do_email( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( ch->level < 5 )
   {
      send_to_char( "Sorry... you must be at least level 5 to do that.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->email )
         ch->pcdata->email = str_dup( "" );
      ch_printf( ch, "Your email is: %s\r\n", show_tilde( ch->pcdata->email ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->email )
         DISPOSE( ch->pcdata->email );
      ch->pcdata->email = str_dup( "" );
      send_to_char( "Email cleared.\r\n", ch );
      return;
   }

   if( strstr( argument, "mailto:" ) )
      strcpy( buf, argument );
   else
      sprintf( buf, "%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->email )
      DISPOSE( ch->pcdata->email );
   ch->pcdata->email = str_dup( buf );
   send_to_char( "Email set.\r\n", ch );
}


/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Monsters are too dumb to do that!\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "do_description: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_description: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_DESC;
         ch->dest_buf = ch;
         start_editing( ch, ch->description );
         return;

      case SUB_PERSONAL_DESC:
         STRFREE( ch->description );
         ch->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs cannot set a bio.\r\n", ch );
      return;
   }
   if( ch->level < 5 )
   {
      set_char_color( AT_SCORE, ch );
      send_to_char( "You must be at least level five to write your bio...\r\n", ch );
      return;
   }
   if( !ch->desc )
   {
      bug( "do_bio: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_bio: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_BIO;
         ch->dest_buf = ch;
         start_editing( ch, ch->pcdata->bio );
         return;

      case SUB_PERSONAL_BIO:
         STRFREE( ch->pcdata->bio );
         ch->pcdata->bio = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}



/*
 * New stat and statreport command coded by Morphina
 * Bug fixes by Shaddai
 */

void do_statreport( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( IS_VAMPIRE( ch ) )
   {
      ch_printf( ch, "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
                 10 + ch->level, ch->move, ch->max_move, ch->exp );
      sprintf( buf, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.",
               ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
               10 + ch->level, ch->move, ch->max_move, ch->exp );
      act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      ch_printf( ch, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );
      sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
               ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );
      act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   }

   ch_printf( ch, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );
   sprintf( buf, "$n's base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
            ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   ch_printf( ch, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
              get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );
   sprintf( buf, "$n's current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
            get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
            get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   return;
}

void do_stat( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( IS_VAMPIRE( ch ) )
      ch_printf( ch, "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
                 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      ch_printf( ch, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   ch_printf( ch, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );

   ch_printf( ch, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
              get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );
   return;
}


void do_report( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) && ch->fighting )
      return;

   if( IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      send_to_char( "You can't do that in your current state of mind!\r\n", ch );
      return;
   }


   if( IS_VAMPIRE( ch ) )
      ch_printf( ch,
                 "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit,
                 ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      ch_printf( ch,
                 "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   if( IS_VAMPIRE( ch ) )
      sprintf( buf, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
               ch->hit, ch->max_hit,
               ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
               ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   return;
}

void do_fprompt( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }
   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current fighting prompt string:\r\n", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
      return;
   }
   send_to_char( "Replacing old prompt of:\r\n", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
   if( ch->pcdata->fprompt )
      STRFREE( ch->pcdata->fprompt );
   if( strlen( argument ) > 128 )
      argument[128] = '\0';

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->fprompt = STRALLOC( "" );
   else
      ch->pcdata->fprompt = STRALLOC( argument );
   return;
}

void do_prompt( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }
   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current prompt string:\r\n", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
      return;
   }
   send_to_char( "Replacing old prompt of:\r\n", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
   if( ch->pcdata->prompt )
      STRFREE( ch->pcdata->prompt );
   if( strlen( argument ) > 128 )
      argument[128] = '\0';

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->prompt = STRALLOC( "" );
   else
      ch->pcdata->prompt = STRALLOC( argument );
   return;
}

/*
 * Figured this belonged here seeing it involves players... 
 * really simple little function to tax players with a large
 * amount of gold to help reduce the overall gold pool...
 *  --TRI
 */
void tax_player( CHAR_DATA * ch )
{
   int gold = ch->gold;
   int mgold = ( ch->level * 2000000 );
   int tax = ( ch->gold * .05 );

   if( gold > mgold )
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "You are level %d and carry more than %d coins.\r\n", ch->level, mgold );
      ch_printf( ch, "You are being taxed 5 percent (%d coins) of your %d coins,\r\n", tax, ch->gold );
      ch_printf( ch, "and that leaves you with %d coins.\r\n", ( ch->gold - tax ) );
      ch->gold -= tax;

   }
   return;
}
