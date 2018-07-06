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
 *			      Regular update module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Scion arenacode extern function */
extern void arena_update( void );

extern CHAR_DATA *find_keeper args( ( CHAR_DATA * ch ) );
extern void save_world( CHAR_DATA * ch );

/*
 * Local functions.
 */
int hit_gain args( ( CHAR_DATA * ch ) );
int mana_gain args( ( CHAR_DATA * ch ) );
int move_gain args( ( CHAR_DATA * ch ) );
void mobile_update args( ( void ) );
void time_update args( ( void ) );  /* FB */
void char_update args( ( void ) );
void char_regen args( ( void ) );   /* Scion */
void obj_update args( ( void ) );
void aggr_update args( ( void ) );
void room_act_update args( ( void ) );
void obj_act_update args( ( void ) );
void char_check args( ( void ) );
void drunk_randoms args( ( CHAR_DATA * ch ) );
void hallucinations args( ( CHAR_DATA * ch ) );
void subtract_times args( ( struct timeval * etime, struct timeval * stime ) );

/* weather functions - FB */
void adjust_vectors args( ( WEATHER_DATA * weather ) );
void get_weather_echo args( ( WEATHER_DATA * weather ) );
void get_time_echo args( ( WEATHER_DATA * weather ) );

/*
 * Global Variables
 */

CHAR_DATA *gch_prev;
OBJ_DATA *gobj_prev;

CHAR_DATA *timechar;

char *corpse_descs[] = {
   "The corpse of %s is in the last stages of decay.",
   "The corpse of %s is crawling with vermin.",
   "The corpse of %s fills the air with a foul stench.",
   "The corpse of %s is buzzing with flies.",
   "The corpse of %s lies here."
};

extern int top_exit;

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   int add_hp;
   int add_mana;
   int add_move;
   int add_prac;

/*	save_char_obj( ch );*/
   sprintf( buf, "the %s", title_table[ch->class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0] );

   if( !IS_SET( ch->pcdata->flags, PCFLAG_ANONYMOUS ) )
      set_title( ch, buf );

   add_hp = con_app[get_curr_con( ch )].hitp + number_range( class_table[ch->class]->hp_min,
                                                             class_table[ch->class]->hp_max );
   add_mana = class_table[ch->class]->fMana ? number_range( 2, ( 2 * ( get_curr_int( ch ) + get_curr_wis( ch ) ) ) / 8 ) : 0;
   add_move = number_range( 5, ( get_curr_con( ch ) + get_curr_dex( ch ) ) / 4 );
   add_prac = wis_app[get_curr_wis( ch )].practice;

   add_hp += race_table[ch->race]->hit;
   add_mana += race_table[ch->race]->mana;

   add_hp = UMAX( 1, add_hp );
   add_mana = UMAX( 0, add_mana );
   add_move = UMAX( 10, add_move );

   /*
    * bonus for deadlies 
    */
   if( IS_PKILL( ch ) )
   {
      add_mana = add_mana + add_mana * .3;
      add_move = add_move + add_move * .3;
      add_hp += 1;   /* bitch at blod if you don't like this :) */
   }

   ch->max_hit += add_hp;
   ch->max_mana += add_mana;
   ch->max_move += add_move;
   ch->practice += add_prac;

   if( !IS_NPC( ch ) )
      xREMOVE_BIT( ch->act, PLR_BOUGHT_PET );

   if( ch->level == LEVEL_PCAVATAR )
   {
      sprintf( buf, "%s has just become an Avatar!", ch->name );
      do_info( ch, buf );
   }

   if( ch->level == LEVEL_AVATAR )
   {
      /*
       * DESCRIPTOR_DATA *d;
       */

      sprintf( buf, "%s has just become a Demi-God!", ch->name );
      do_info( ch, buf );
/*	for ( d = first_descriptor; d; d = d->next )
	   if ( d->connected == CON_PLAYING && d->character != ch )
	   {
		set_char_color( AT_IMMORT, d->character );
		send_to_char( buf,	d->character );
		send_to_char( "\r\n",	d->character );
	   } -- Replace this with a do_info: Scion */
      set_char_color( AT_WHITE, ch );
      do_help( ch, "M_ADVHERO_" );
   }
   if( ch->level < LEVEL_IMMORTAL )
   {
      if( IS_VAMPIRE( ch ) )
         sprintf( buf,
                  "Your gain is: %d/%d hp, %d/%d bp, %d/%d mv %d/%d prac.\r\n",
                  add_hp, ch->max_hit, 1, ch->level + 10, add_move, ch->max_move, add_prac, ch->practice );
      else
         sprintf( buf,
                  "Your gain is: %d/%d hp, %d/%d mana, %d/%d mv %d/%d prac.\r\n",
                  add_hp, ch->max_hit, add_mana, ch->max_mana, add_move, ch->max_move, add_prac, ch->practice );
      set_char_color( AT_WHITE, ch );
      send_to_char( buf, ch );
   }
   return;
}

void gain_exp( CHAR_DATA * ch, int gain )
{
   int modgain;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || ch->level >= LEVEL_AVATAR )
      return;


   /*
    * Bonus for deadly lowbies 
    */
   modgain = gain;
   if( modgain > 0 && IS_PKILL( ch ) && ch->level < 33 )
   {
      if( ch->level <= 9 )
      {
         modgain *= 2;
      }
      if( ch->level <= 18 && ch->level >= 10 )
      {
         modgain *= 1.75;
      }
      if( ch->level <= 27 && ch->level >= 19 )
      {
         modgain *= 1.5;
      }
      if( ch->level <= 28 && ch->level >= 36 )
      {
         modgain *= 1.25;
      }
   }

   /*
    * per-race experience multipliers 
    */
   modgain *= ( race_table[ch->race]->exp_multiplier / 100.0 );

   /*
    * Deadly exp loss floor is exp floor of level 
    */
   /*
    * if(IS_PKILL(ch)&& modgain<0){
    * if( ch->exp + modgain < exp_level(ch, ch->level)){
    * modgain = exp_level(ch, ch->level) - ch->exp;
    * }
    * }
    */


   /*
    * xp cap to prevent any one event from giving enuf xp to 
    */
   /*
    * gain more than one level - FB 
    */
   modgain = UMIN( modgain, exp_level( ch, ch->level + 2 ) - exp_level( ch, ch->level + 1 ) );

   ch->exp = UMAX( 0, ch->exp + modgain );

   if( NOT_AUTHED( ch ) && ch->exp >= exp_level( ch, ch->level + 1 ) )
   {
      send_to_char( "You can not ascend to a higher level until you are authorized.\r\n", ch );
      ch->exp = ( exp_level( ch, ( ch->level + 1 ) ) - 1 );
      return;
   }

   while( ch->level < LEVEL_AVATAR && ch->exp >= exp_level( ch, ch->level + 1 ) )
   {
      set_char_color( AT_WHITE + AT_BLINK, ch );
      ch_printf( ch, "You have now obtained experience level %d!\r\n", ++ch->level );
      if( !IS_SET( ch->pcdata->flags, PCFLAG_ANONYMOUS ) )
      {
         sprintf( buf, "%s has achieved level %d!", ch->name, ch->level );
         do_info( ch, buf );
      }
      advance_level( ch );
   }
   return;
}

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA * ch )
{
   int gain;

   if( IS_NPC( ch ) )
   {
      gain = ch->level * 3 / 2;
   }
   else
   {
      gain = UMIN( 5, ( ( ch->level + get_curr_con( ch ) ) / 8 ) );

      switch ( ch->position )
      {
         case POS_DEAD:
            return 0;
         case POS_MORTAL:
            return -1;
         case POS_INCAP:
            return -1;
         case POS_STUNNED:
            return 1;
         case POS_SLEEPING:
            gain += get_curr_con( ch ) * 2.0;
            if( ch->on )
               gain += 5;
            break;
         case POS_RESTING:
            gain += get_curr_con( ch ) * 1.25;
            if( ch->on )
               gain += 2;
            break;
      }

      if( IS_VAMPIRE( ch ) )
      {
         if( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )
            gain /= 2;
         else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( 8 + ch->level ) )
            gain *= 2;
         if( IS_OUTSIDE( ch ) )
         {
            switch ( time_info.sunlight )
            {
               case SUN_RISE:
               case SUN_SET:
                  gain /= 2;
                  break;
               case SUN_LIGHT:
                  gain /= 4;
                  break;
            }
         }
      }

      if( ch->pcdata->condition[COND_FULL] == 0 )
         gain /= 2;

      if( ch->pcdata->condition[COND_THIRST] == 0 )
         gain /= 2;

   }

   if( IS_AFFECTED( ch, AFF_POISON ) )
      gain /= 4;

   return UMIN( gain, ch->max_hit - ch->hit );
}



int mana_gain( CHAR_DATA * ch )
{
   int gain;

   if( IS_NPC( ch ) )
   {
      gain = ch->level;
   }
   else
   {
      gain = UMIN( 5, ( ( ch->level + get_curr_int( ch ) ) / 8 ) );

      if( ch->position < POS_SLEEPING )
         return 0;
      switch ( ch->position )
      {
         case POS_SLEEPING:
            gain += get_curr_int( ch ) * 3.25;
            if( ch->on )
               gain += 5;
            break;
         case POS_RESTING:
            gain += get_curr_int( ch ) * 1.75;
            if( ch->on )
               gain += 2;
            break;
      }

      if( ch->pcdata->condition[COND_FULL] == 0 )
         gain /= 2;

      if( ch->pcdata->condition[COND_THIRST] == 0 )
         gain /= 2;

   }

   if( IS_AFFECTED( ch, AFF_POISON ) )
      gain /= 4;

   return UMIN( gain, ch->max_mana - ch->mana );
}



int move_gain( CHAR_DATA * ch )
{
   int gain;

   if( IS_NPC( ch ) )
   {
      gain = ch->level;
   }
   else
   {
      gain = UMAX( 15, ( ( 2 * ch->level + get_curr_dex( ch ) ) / 8 ) );

      switch ( ch->position )
      {
         case POS_DEAD:
            return 0;
         case POS_MORTAL:
            return -1;
         case POS_INCAP:
            return -1;
         case POS_STUNNED:
            return 1;
         case POS_SLEEPING:
            gain += get_curr_dex( ch ) * 4.5;
            if( ch->on )
               gain += 5;
            break;
         case POS_RESTING:
            gain += get_curr_dex( ch ) * 2.5;
            if( ch->on )
               gain += 2;
            break;
      }

      if( IS_VAMPIRE( ch ) )
      {
         if( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )
            gain /= 2;
         else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( 8 + ch->level ) )
            gain *= 2;
         if( IS_OUTSIDE( ch ) )
         {
            switch ( time_info.sunlight )
            {
               case SUN_RISE:
               case SUN_SET:
                  gain /= 2;
                  break;
               case SUN_LIGHT:
                  gain /= 4;
                  break;
            }
         }
      }

      if( ch->pcdata->condition[COND_FULL] == 0 )
         gain /= 2;

      if( ch->pcdata->condition[COND_THIRST] == 0 )
         gain /= 2;
   }

   if( IS_AFFECTED( ch, AFF_POISON ) )
      gain /= 4;

   return UMIN( gain, ch->max_move - ch->move );
}


void gain_condition( CHAR_DATA * ch, int iCond, int value )
{
   int condition;
   ch_ret retcode = rNONE;
   char buf[MAX_STRING_LENGTH];

   if( value == 0 || IS_NPC( ch ) || ch->level >= LEVEL_IMMORTAL || NOT_AUTHED( ch ) )
      return;

   /*
    * Make away flag go on automagically after 5 mins -- Scion 
    */
   if( ( ch->desc )
       && ( ( ch->desc->connected == CON_PLAYING ) && ( ch->desc->idle > 1200 && !xIS_SET( ch->act, PLR_AFK ) ) ) )
      do_afk( ch, "" );

   condition = ch->pcdata->condition[iCond];

   if( condition == -1 )
      return;

   if( iCond == COND_BLOODTHIRST )
      ch->pcdata->condition[iCond] = URANGE( 0, condition + value, ( 10 + ch->level ) );
   else
      ch->pcdata->condition[iCond] = URANGE( 0, condition + value, 48 );

   if( ch->pcdata->condition[iCond] == 0 )
   {
      switch ( iCond )
      {
         case COND_FULL:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_HUNGRY, ch );
               send_to_char( "You are STARVING!\r\n", ch );
               act( AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM );
               if( !IS_PKILL( ch ) || number_bits( 1 ) == 0 )
                  worsen_mental_state( ch, 1 );
               retcode = damage( ch, ch, 1, TYPE_UNDEFINED );
            }
            break;

         case COND_THIRST:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_THIRSTY, ch );
               send_to_char( "You are DYING of THIRST!\r\n", ch );
               act( AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM );
               worsen_mental_state( ch, IS_PKILL( ch ) ? 1 : 2 );
               retcode = damage( ch, ch, 2, TYPE_UNDEFINED );
            }
            break;

         case COND_BLOODTHIRST:
            if( ch->level < LEVEL_AVATAR )
            {
               if( !IS_VAMPIRE( ch ) )
               {
                  sprintf( buf, "gain_condition bloodthirst update for non blood ch: %s", ch->name );
                  bug( buf );
                  retcode = rNONE;
               }
               else
               {
                  set_char_color( AT_BLOOD, ch );
                  send_to_char( "You are starved to feast on blood!\r\n", ch );
                  act( AT_BLOOD, "$n is suffering from lack of blood!", ch, NULL, NULL, TO_ROOM );
                  worsen_mental_state( ch, 2 );
                  retcode = damage( ch, ch, ch->max_hit / 20, TYPE_UNDEFINED );
               }
            }
            break;
         case COND_DRUNK:
            if( condition != 0 )
            {
               set_char_color( AT_SOBER, ch );
               send_to_char( "You are sober.\r\n", ch );
            }
            retcode = rNONE;
            break;
         default:
            bug( "Gain_condition: invalid condition type %d", iCond );
            retcode = rNONE;
            break;
      }
   }

   if( retcode != rNONE )
      return;

   if( ch->pcdata->condition[iCond] == 1 )
   {
      switch ( iCond )
      {
         case COND_FULL:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_HUNGRY, ch );
               send_to_char( "You are really hungry.\r\n", ch );
               act( AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM );
               if( number_bits( 1 ) == 0 )
                  worsen_mental_state( ch, 1 );
            }
            break;

         case COND_THIRST:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_THIRSTY, ch );
               send_to_char( "You are really thirsty.\r\n", ch );
               worsen_mental_state( ch, 1 );
               act( AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM );
            }
            break;

         case COND_BLOODTHIRST:
            if( ch->level < LEVEL_AVATAR )
            {
               set_char_color( AT_BLOOD, ch );
               send_to_char( "You have a growing need to feast on blood!\r\n", ch );
               act( AT_BLOOD, "$n gets a strange look in $s eyes...", ch, NULL, NULL, TO_ROOM );
               worsen_mental_state( ch, 1 );
            }
            break;
         case COND_DRUNK:
            if( condition != 0 )
            {
               set_char_color( AT_SOBER, ch );
               send_to_char( "You are feeling a little less light headed.\r\n", ch );
            }
            break;
      }
   }


   if( ch->pcdata->condition[iCond] == 2 )
   {
      switch ( iCond )
      {
         case COND_FULL:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_HUNGRY, ch );
               send_to_char( "You are hungry.\r\n", ch );
            }
            break;

         case COND_THIRST:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_THIRSTY, ch );
               send_to_char( "You are thirsty.\r\n", ch );
            }
            break;

         case COND_BLOODTHIRST:
            if( ch->level < LEVEL_AVATAR )
            {
               set_char_color( AT_BLOOD, ch );
               send_to_char( "You feel an urgent need for blood.\r\n", ch );
            }
            break;
      }
   }

   if( ch->pcdata->condition[iCond] == 3 )
   {
      switch ( iCond )
      {
         case COND_FULL:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_HUNGRY, ch );
               send_to_char( "You are a mite peckish.\r\n", ch );
            }
            break;

         case COND_THIRST:
            if( ch->level < LEVEL_AVATAR && ch->race != RACE_VAMPIRE )
            {
               set_char_color( AT_THIRSTY, ch );
               send_to_char( "You could use a sip of something refreshing.\r\n", ch );
            }
            break;

         case COND_BLOODTHIRST:
            if( ch->level < LEVEL_AVATAR )
            {
               set_char_color( AT_BLOOD, ch );
               send_to_char( "You feel an aching in your fangs.\r\n", ch );
            }
            break;
      }
   }

   return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update( void )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch;
   EXIT_DATA *pexit;
   int door;
   ch_ret retcode;

   retcode = rNONE;

   /*
    * Examine all mobs. 
    */
   for( ch = last_char; ch; ch = gch_prev )
   {

      if( ( !ch->pcdata ) && !( xIS_SET( ch->act, ACT_IS_NPC ) ) )
      {
         bug( "Mob %d in room %d was not set with the NPC flag!  Fixing...", ch->pIndexData->vnum, ch->in_room->vnum );
         xSET_BIT( ch->act, ACT_IS_NPC );
      }

      set_cur_char( ch );
      if( ch == first_char && ch->prev )
      {
         bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
         ch->prev = NULL;
      }

      gch_prev = ch->prev;

      if( gch_prev && gch_prev->next != ch )
      {
         sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.", ch->name );
         bug( buf, 0 );
         bug( "Short-cutting here", 0 );
         gch_prev = NULL;
         ch->prev = NULL;
         if( ch )
            do_shout( ch, "Scion says, 'Ack! Run for your lives!'" );
      }

      if( !IS_NPC( ch ) )
      {
         drunk_randoms( ch );
         hallucinations( ch );
         continue;
      }

      if( !ch->in_room || IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_PARALYSIS ) )
         continue;

/* Clean up 'animated corpses' that are not charmed' - Scryn */

      if( ch->pIndexData->vnum == 5 && !IS_AFFECTED( ch, AFF_CHARM ) )
      {
         if( ch->in_room->first_person )
            act( AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM );

         if( IS_NPC( ch ) )   /* Guard against purging switched? */
            extract_char( ch, TRUE );
         continue;
      }

      if( !xIS_SET( ch->act, ACT_RUNNING ) && !xIS_SET( ch->act, ACT_SENTINEL ) && !ch->fighting && ch->hunting )
      {
         WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
         /*
          * Commented out temporarily to avoid spam - Scryn 
          * sprintf( buf, "%s hunting %s from %s.", ch->name,
          * ch->hunting->name,
          * ch->in_room->name );
          * log_string( buf ); 
          */
         hunt_victim( ch );
         continue;
      }

      /*
       * Examine call for special procedure 
       */
      if( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_fun )
      {
         if( ( *ch->spec_fun ) ( ch ) )
            continue;
         if( char_died( ch ) )
            continue;
      }

      /*
       * Check for mudprogram script on mob 
       */
      if( HAS_PROG( ch->pIndexData, SCRIPT_PROG ) )
      {
         mprog_script_trigger( ch );
         continue;
      }

      if( ch != cur_char )
      {
         bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
         continue;
      }

      /*
       * That's all for sleeping / busy monster 
       */
      if( ch->position != POS_STANDING )
         continue;

      if( xIS_SET( ch->act, ACT_MOUNTED ) )
      {
         if( xIS_SET( ch->act, ACT_AGGRESSIVE ) || xIS_SET( ch->act, ACT_META_AGGR ) )
            do_emote( ch, "snarls and growls." );
         continue;
      }

      if( IS_SET( ch->in_room->room_flags, ROOM_SAFE )
          && ( xIS_SET( ch->act, ACT_AGGRESSIVE ) || xIS_SET( ch->act, ACT_META_AGGR ) ) )
         do_emote( ch, "glares around and snarls." );


      /*
       * something else is wrong if I have to do this ... but until I can
       * track it down, this will have to do - shogar 
       */
      if( !ch->in_room->area )
      {
         bug( "Room %d for mob %s is not associated with an area?", ch->in_room->vnum, ch->name );
         if( ch->was_in_room )
            bug( "Was in room %d", ch->was_in_room->vnum );
         extract_char( ch, TRUE );
         continue;
      }
      /*
       * MOBprogram random trigger 
       */
      if( ch->in_room->area->nplayer > 0 )
      {
         mprog_random_trigger( ch );
         if( char_died( ch ) )
            continue;
         if( ch->position < POS_STANDING )
            continue;
      }

      /*
       * MOBprogram hour trigger: do something for an hour 
       */
      mprog_hour_trigger( ch );

      if( char_died( ch ) )
         continue;

      rprog_hour_trigger( ch );
      if( char_died( ch ) )
         continue;

      if( ch->position < POS_STANDING )
         continue;

/*	if (number_range(1,5) < 4) {
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		OBJ_DATA *big_obj;
		bool fBought = FALSE;

		if (((keeper = find_keeper(ch)) != NULL) && (keeper != ch)) {
			big_obj = keeper->last_carrying;
			for ( obj = keeper->last_carrying; obj; obj = obj->prev_content ) {
				if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) ) {
					if ((obj->cost > big_obj->cost) && (ch->gold >= big_obj->cost)) 
						big_obj = obj;
		        	}
			}
			if (big_obj!=NULL) { 
				if(!get_obj_wear(ch, big_obj->name)
					&& !get_obj_carry(ch, big_obj->name)) {
					do_buy(ch, big_obj->name);
					if (big_obj == ch->last_carrying)
						fBought=TRUE;
					do_wear(ch, big_obj->name); 
				}
			}
			if (fBought == FALSE) {
				for (obj=ch->last_carrying; obj; obj=obj->next_content)
					if (obj->wear_loc==WEAR_NONE)
						do_sell(ch, obj->name);
			}
		}
	} */

      /*
       * Scavenge 
       */
      if( ch->in_room->first_content && number_bits( 2 ) == 0 )
      {
         OBJ_DATA *obj;
         OBJ_DATA *obj_best;
         int max;

         if( !xIS_SET( ch->act, ACT_SCAVENGER ) && number_percent(  ) < 70 )
         {
            max = 1;
            obj_best = NULL;
            for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
            {
               if( ( CAN_WEAR( obj, ITEM_TAKE ) && ( obj->cost > max )
                     && ( !IS_OBJ_STAT( obj, ITEM_BURIED ) || IS_AFFECTED( ch, AFF_TRUESIGHT ) ) )
                   || ( ch->in_room->area->weather->precip > 10 ) )
               {
                  obj_best = obj;
                  max = obj->cost;
               }
            }

            if( obj_best && CAN_WEAR( obj_best, ITEM_TAKE ) )
            {
               if( ch->in_room->area->weather->precip > 10 )
                  xREMOVE_BIT( obj_best->extra_flags, ITEM_BURIED );

               else if( IS_OBJ_STAT( obj_best, ITEM_BURIED ) )
               {
                  act( AT_ACTION, "$n digs up $p.", ch, obj_best, NULL, TO_ROOM );
                  xREMOVE_BIT( obj_best->extra_flags, ITEM_BURIED );
               }
               else
                  do_get( ch, obj_best->name );
               do_wear( ch, "all" );
            }
         }
      }

      /*
       * Handle objects in inventory -- Scion 
       */
      if( ch->last_carrying && find_keeper( ch ) != ch )
         do_wear( ch, "all" );

      /*
       * Wander 
       */
      if( !xIS_SET( ch->act, ACT_RUNNING ) && !xIS_SET( ch->act, ACT_SENTINEL ) && !xIS_SET( ch->act, ACT_PROTOTYPE ) && ( door = number_bits( 5 ) ) <= 11  /* Was 9, 11 is secret+closed */
          && ( pexit = get_exit( ch->in_room, door ) ) != NULL && pexit->to_room
          /*
           * &&   !IS_SET(pexit->exit_info, EX_CLOSED) <-- Allowing MOBs to open doors! -- Scion 
           */
          && !IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
          && !IS_SET( pexit->to_room->room_flags, ROOM_DEATH )
          && ( !xIS_SET( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
      {
         if( ( ( door == 3 ) || ( door == 11 ) ) && ( IS_SET( pexit->exit_info, EX_CLOSED ) ) )
            do_open( ch, pexit->keyword );
         retcode = move_char( ch, pexit, 0 );
         if( ( door == 1 ) && ( number_range( 0, 10 ) < 4 ) )
            do_close( ch, pexit->keyword );
         /*
          * If ch changes position due
          * to it's or someother mob's
          * movement via MOBProgs,
          * continue - Kahn 
          */
         if( char_died( ch ) )
            continue;
         if( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL ) || ch->position < POS_STANDING )
            continue;
      }

      /*
       * Flee 
       */
      if( ch->hit < ch->max_hit / 2
          && ( door = number_bits( 4 ) ) <= 9
          && ( pexit = get_exit( ch->in_room, door ) ) != NULL
          && pexit->to_room && !IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
      {
         CHAR_DATA *rch;
         bool found;

         found = FALSE;
         for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
         {
            if( is_fearing( ch, rch ) )
            {
               switch ( number_bits( 2 ) )
               {
                  case 0:
                     sprintf( buf, "Get away from me, %s!", rch->name );
                     break;
                  case 1:
                     sprintf( buf, "Leave me be, %s!", rch->name );
                     break;
                  case 2:
                     sprintf( buf, "%s is trying to kill me!  Help!", rch->name );
                     break;
                  case 3:
                     sprintf( buf, "Someone save me from %s!", rch->name );
                     break;
               }
               do_yell( ch, buf );
               found = TRUE;
               break;
            }
         }
         if( found )
            retcode = move_char( ch, pexit, 0 );
      }
   }

   return;
}



void char_regen( void )
{
   CHAR_DATA *ch;

   for( ch = last_char; ch; ch = gch_prev )
   {
      if( ch == first_char && ch->prev )
      {
         bug( "char_gains: first_char->prev != NULL... fixed", 0 );
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      set_cur_char( ch );
      if( gch_prev && gch_prev->next != ch )
      {
         bug( "char_gains: ch->prev->next != ch", 0 );
         return;
      }

      if( char_died( ch ) )
         continue;

      if( ch->position >= POS_STUNNED )
      {
         if( ch->hit < ch->max_hit )
            ch->hit += hit_gain( ch );

         if( ch->mana < ch->max_mana )
            ch->mana += mana_gain( ch );

         if( ch->move < ch->max_move )
            ch->move += move_gain( ch );
      }
   }
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{
   CHAR_DATA *ch;
   CHAR_DATA *ch_save;
   short save_count = 0;

   ch_save = NULL;
   for( ch = last_char; ch; ch = gch_prev )
   {
      if( ch == first_char && ch->prev )
      {
         bug( "char_update: first_char->prev != NULL... fixed", 0 );
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      set_cur_char( ch );
      if( gch_prev && gch_prev->next != ch )
      {
         bug( "char_update: ch->prev->next != ch", 0 );
         return;
      }

      /*
       *  Do a room_prog rand check right off the bat
       *   if ch disappears (rprog might wax npc's), continue
       */
      if( !IS_NPC( ch ) )
         rprog_random_trigger( ch );

      if( char_died( ch ) )
         continue;

      if( IS_NPC( ch ) )
         mprog_time_trigger( ch );

      if( char_died( ch ) )
         continue;

      rprog_time_trigger( ch );

      if( char_died( ch ) )
         continue;

      /*
       * See if player should be auto-saved.
       */
      if( !IS_NPC( ch )
          && ( !ch->desc || ch->desc->connected == CON_PLAYING )
          && ch->level >= 2 && current_time - ch->save_time > ( sysdata.save_frequency * 60 ) )
         ch_save = ch;
      else
         ch_save = NULL;

/* move to another function -- Scion 
	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	}
 end move */

      if( ch->position == POS_STUNNED )
         update_pos( ch );

      /*
       * Morph timer expires 
       */

      if( ch->morph )
      {
         if( ch->morph->timer > 0 )
         {
            ch->morph->timer--;
            if( ch->morph->timer == 0 )
               do_unmorph_char( ch );
         }
      }

      /*
       * To make people with a nuisance's flags life difficult 
       * * --Shaddai
       */

      if( !IS_NPC( ch ) && ch->pcdata->nuisance )
      {
         long int temp;

         if( ch->pcdata->nuisance->flags < MAX_NUISANCE_STAGE )
         {
            temp = ch->pcdata->nuisance->max_time - ch->pcdata->nuisance->time;
            temp *= ch->pcdata->nuisance->flags;
            temp /= MAX_NUISANCE_STAGE;
            temp += ch->pcdata->nuisance->time;
            if( temp < current_time )
               ch->pcdata->nuisance->flags++;
         }
      }

      if( !IS_NPC( ch ) && ch->level < LEVEL_IMMORTAL )
      {
         OBJ_DATA *obj;

         if( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] > 0 )
         {
            if( --obj->value[2] == 0 && ch->in_room )
            {
               ch->in_room->light -= obj->count;
               if( ch->in_room->light < 0 )
                  ch->in_room->light = 0;
               act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
               act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
               if( obj->serial == cur_obj )
                  global_objcode = rOBJ_EXPIRED;
               extract_obj( obj );
            }
         }


         if( ++ch->timer >= 12 )
         {
            if( ch->timer == 12 && ch->in_room )
            {
               /*
                * ch->was_in_room = ch->in_room;
                */
               if( ch->fighting )
                  stop_fighting( ch, TRUE );
               act( AT_ACTION, "$n disappears into the void.", ch, NULL, NULL, TO_ROOM );
               send_to_char( "You disappear into the void.\r\n", ch );
               if( IS_SET( sysdata.save_flags, SV_IDLE ) )
                  save_char_obj( ch );
               char_from_room( ch );
               char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
            }
         }

         if( ch->pcdata->condition[COND_DRUNK] > 8 )
            worsen_mental_state( ch, ch->pcdata->condition[COND_DRUNK] / 8 );
         if( ch->pcdata->condition[COND_FULL] > 1 )
         {
            switch ( ch->position )
            {
               case POS_SLEEPING:
                  better_mental_state( ch, 4 );
                  break;
               case POS_RESTING:
                  better_mental_state( ch, 3 );
                  break;
               case POS_SITTING:
               case POS_MOUNTED:
                  better_mental_state( ch, 2 );
                  break;
               case POS_STANDING:
                  better_mental_state( ch, 1 );
                  break;
               case POS_FIGHTING:
               case POS_EVASIVE:
               case POS_DEFENSIVE:
               case POS_AGGRESSIVE:
               case POS_BERSERK:
                  if( number_bits( 2 ) == 0 )
                     better_mental_state( ch, 1 );
                  break;
            }
         }
         if( ch->pcdata->condition[COND_THIRST] > 1 )
         {
            switch ( ch->position )
            {
               case POS_SLEEPING:
                  better_mental_state( ch, 5 );
                  break;
               case POS_RESTING:
                  better_mental_state( ch, 3 );
                  break;
               case POS_SITTING:
               case POS_MOUNTED:
                  better_mental_state( ch, 2 );
                  break;
               case POS_STANDING:
                  better_mental_state( ch, 1 );
                  break;
               case POS_FIGHTING:
               case POS_EVASIVE:
               case POS_DEFENSIVE:
               case POS_AGGRESSIVE:
               case POS_BERSERK:
                  if( number_bits( 2 ) == 0 )
                     better_mental_state( ch, 1 );
                  break;
            }
         }
/*
	    gain_condition( ch, COND_FULL,   -1 );
*/
         gain_condition( ch, COND_DRUNK, -1 );
         gain_condition( ch, COND_FULL, -1 + race_table[ch->race]->hunger_mod );

         if( IS_VAMPIRE( ch ) && ch->level >= 10 )
         {
            if( time_info.hour < 21 && time_info.hour > 5 )
               gain_condition( ch, COND_BLOODTHIRST, -1 );
         }

         if( CAN_PKILL( ch ) && ch->pcdata->condition[COND_THIRST] - 9 > 10 )
            gain_condition( ch, COND_THIRST, -9 );

         if( !IS_NPC( ch ) && ch->pcdata->nuisance )
         {
            int value;

            value = ( ( 0 - ch->pcdata->nuisance->flags ) * ch->pcdata->nuisance->power );
            gain_condition( ch, COND_THIRST, value );
            gain_condition( ch, COND_FULL, --value );
         }

         if( ch->in_room )
            switch ( ch->in_room->sector_type )
            {
               default:
                  gain_condition( ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod );
                  break;
               case SECT_DESERT:
                  gain_condition( ch, COND_THIRST, -3 + race_table[ch->race]->thirst_mod );
                  break;
               case SECT_UNDERWATER:
               case SECT_OCEANFLOOR:
                  if( number_bits( 1 ) == 0 )
                     gain_condition( ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod );
                  break;
            }

      }
      if( !IS_NPC( ch ) && !IS_IMMORTAL( ch ) && ch->pcdata->release_date > 0 && ch->pcdata->release_date <= current_time )
      {
         ROOM_INDEX_DATA *location;
         if( ch->pcdata->clan )
            location = get_room_index( ch->pcdata->clan->recall );
         else
            location = get_room_index( ROOM_VNUM_TEMPLE );
         if( !location )
            location = ch->in_room;
         MOBtrigger = FALSE;
         char_from_room( ch );
         char_to_room( ch, location );
         send_to_char( "The gods have released you from hell as your sentence is up!\r\n", ch );
         do_look( ch, "auto" );
         STRFREE( ch->pcdata->helled_by );
         ch->pcdata->release_date = 0;
         save_char_obj( ch );
      }

      if( !char_died( ch ) )
      {
         /*
          * Careful with the damages here,
          *   MUST NOT refer to ch after damage taken, without checking
          *   return code and/or char_died as it may be lethal damage.
          */
         if( IS_AFFECTED( ch, AFF_POISON ) )
         {
            act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
            act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = URANGE( 20, ch->mental_state + ( IS_NPC( ch ) ? 2 : IS_PKILL( ch ) ? 3 : 4 ), 100 );
            damage( ch, ch, 6, gsn_poison );
         }
         else if( ch->position == POS_INCAP )
            damage( ch, ch, 1, TYPE_UNDEFINED );
         else if( ch->position == POS_MORTAL )
            damage( ch, ch, 4, TYPE_UNDEFINED );
         if( char_died( ch ) )
            continue;

         /*
          * Recurring spell affect
          */
         if( IS_AFFECTED( ch, AFF_RECURRINGSPELL ) )
         {
            AFFECT_DATA *paf, *paf_next;
            SKILLTYPE *skill;
            bool found = FALSE, died = FALSE;

            for( paf = ch->first_affect; paf; paf = paf_next )
            {
               paf_next = paf->next;
               if( paf->location == APPLY_RECURRINGSPELL )
               {
                  found = TRUE;
                  if( IS_VALID_SN( paf->modifier )
                      && ( skill = skill_table[paf->modifier] ) != NULL && skill->type == SKILL_SPELL )
                  {
                     if( ( *skill->spell_fun ) ( paf->modifier, ch->level, ch, ch ) == rCHAR_DIED || char_died( ch ) )
                     {
                        died = TRUE;
                        break;
                     }
                  }
               }
            }
            if( died )
               continue;
            if( !found )
               xREMOVE_BIT( ch->affected_by, AFF_RECURRINGSPELL );
         }


         /*
          *  Race alignment restrictions, h
          */
         if( !IS_NPC( ch ) )
         {
            if( ch->alignment < race_table[ch->race]->minalign )
            {
               set_char_color( AT_BLOOD, ch );
               send_to_char( "Your evil actions trouble you, clouding your judgement.\r\n", ch );
               worsen_mental_state( ch, 10 );
            }

            if( ch->alignment > race_table[ch->race]->maxalign )
            {
               set_char_color( AT_BLOOD, ch );
               send_to_char( "Being so good is beginning to take its toll...  Your strength is leaving you!\r\n", ch );
               worsen_mental_state( ch, 10 );
            }
         }
         if( ch->mental_state >= 30 )
            switch ( ( ch->mental_state + 5 ) / 10 )
            {
               case 3:
                  send_to_char( "You feel feverish.\r\n", ch );
                  act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 4:
                  send_to_char( "You do not feel well at all.\r\n", ch );
                  act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 5:
                  send_to_char( "You need help!\r\n", ch );
                  act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 6:
                  send_to_char( "Seekest thou a cleric.\r\n", ch );
                  act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 7:
                  send_to_char( "You feel reality slipping away...\r\n", ch );
                  act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 8:
                  send_to_char( "You begin to understand... everything.\r\n", ch );
                  act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
                  break;
               case 9:
                  send_to_char( "You are ONE with the universe.\r\n", ch );
                  act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL,
                       TO_ROOM );
                  break;
               case 10:
                  send_to_char( "You feel the end is near.\r\n", ch );
                  act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
                  break;
            }
         if( ch->mental_state <= -30 )
            switch ( ( abs( ch->mental_state ) + 5 ) / 10 )
            {
               case 10:
                  if( ch->position > POS_SLEEPING )
                  {
                     if( ( ch->position == POS_STANDING
                           || ch->position < POS_FIGHTING ) && number_percent(  ) + 10 < abs( ch->mental_state ) )
                        do_sleep( ch, "" );
                     else
                        send_to_char( "You're barely conscious.\r\n", ch );
                  }
                  break;
               case 9:
                  if( ch->position > POS_SLEEPING )
                  {
                     if( ( ch->position == POS_STANDING
                           || ch->position < POS_FIGHTING ) && ( number_percent(  ) + 20 ) < abs( ch->mental_state ) )
                        do_sleep( ch, "" );
                     else
                        send_to_char( "You can barely keep your eyes open.\r\n", ch );
                  }
                  break;
               case 8:
                  if( ch->position > POS_SLEEPING )
                  {
                     if( ch->position < POS_SITTING && ( number_percent(  ) + 30 ) < abs( ch->mental_state ) )
                        do_sleep( ch, "" );
                     else
                        send_to_char( "You're extremely drowsy.\r\n", ch );
                  }
                  break;
               case 7:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel very unmotivated.\r\n", ch );
                  break;
               case 6:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel sedated.\r\n", ch );
                  break;
               case 5:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel sleepy.\r\n", ch );
                  break;
               case 4:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel tired.\r\n", ch );
                  break;
               case 3:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You could use a rest.\r\n", ch );
                  break;
            }
         if( ch->timer > 24 )
            do_quit( ch, "" );
         else if( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO ) && ++save_count < 10 ) /* save max of 10 per tick */
            save_char_obj( ch );
      }
   }

   return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
   OBJ_DATA *obj;
   short AT_TEMP;

   for( obj = last_object; obj; obj = gobj_prev )
   {
      CHAR_DATA *rch;
      char *message;

      if( obj == first_object && obj->prev )
      {
         bug( "obj_update: first_object->prev != NULL... fixed", 0 );
         obj->prev = NULL;
      }
      gobj_prev = obj->prev;
      if( gobj_prev && gobj_prev->next != obj )
      {
         bug( "obj_update: obj->prev->next != obj", 0 );
         return;
      }
      set_cur_obj( obj );
      if( obj->carried_by )
         oprog_random_trigger( obj );
      else if( obj->in_room && obj->in_room->area->nplayer > 0 )
         oprog_random_trigger( obj );

      if( obj_extracted( obj ) )
         continue;

      if( obj->item_type == ITEM_PIPE )
      {
         if( IS_SET( obj->value[3], PIPE_LIT ) )
         {
            if( --obj->value[1] <= 0 )
            {
               obj->value[1] = 0;
               REMOVE_BIT( obj->value[3], PIPE_LIT );
            }
            else if( IS_SET( obj->value[3], PIPE_HOT ) )
               REMOVE_BIT( obj->value[3], PIPE_HOT );
            else
            {
               if( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
               {
                  REMOVE_BIT( obj->value[3], PIPE_LIT );
                  REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
               }
               else
                  SET_BIT( obj->value[3], PIPE_GOINGOUT );
            }
            if( !IS_SET( obj->value[3], PIPE_LIT ) )
               SET_BIT( obj->value[3], PIPE_FULLOFASH );
         }
         else
            REMOVE_BIT( obj->value[3], PIPE_HOT );
      }


/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

      if( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
      {
         short timerfrac = UMAX( 1, obj->timer - 1 );
         if( obj->item_type == ITEM_CORPSE_PC )
            timerfrac = ( int )( obj->timer / 8 + 1 );

         if( obj->timer > 0 && obj->value[2] > timerfrac )
         {
            char buf[MAX_STRING_LENGTH];
            char name[MAX_STRING_LENGTH];
            char *bufptr;
            bufptr = one_argument( obj->short_descr, name );
            bufptr = one_argument( bufptr, name );
            bufptr = one_argument( bufptr, name );

            separate_obj( obj );
            obj->value[2] = timerfrac;
            sprintf( buf, corpse_descs[UMIN( timerfrac - 1, 4 )], bufptr );

            STRFREE( obj->description );
            obj->description = STRALLOC( buf );
         }
      }

      /*
       * don't let inventory decay 
       */
      if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
         continue;

      /*
       * groundrot items only decay on the ground 
       */
      if( IS_OBJ_STAT( obj, ITEM_GROUNDROT ) && !obj->in_room )
         continue;

      if( ( obj->timer <= 0 || --obj->timer > 0 ) )
         continue;

      /*
       * if we get this far, object's timer has expired. 
       */

      if( xIS_SET( obj->extra_flags, ITEM_POISONED ) )
      {
         separate_obj( obj );
         xREMOVE_BIT( obj->extra_flags, ITEM_POISONED );
         return;
      }

      AT_TEMP = AT_PLAIN;
      switch ( obj->item_type )
      {
         default:
            message = "$p mysteriously vanishes.";
            AT_TEMP = AT_PLAIN;
            break;
         case ITEM_CONTAINER:
            message = "$p falls apart, tattered from age.";
            AT_TEMP = AT_OBJECT;
            break;
         case ITEM_PORTAL:
            message = "$p unravels and winks from existence.";
            remove_portal( obj );
            obj->item_type = ITEM_TRASH;  /* so extract_obj  */
            AT_TEMP = AT_MAGIC;  /* doesn't remove_portal */
            break;
         case ITEM_FOUNTAIN:
            message = "$p dries up.";
            AT_TEMP = AT_BLUE;
            break;
         case ITEM_CORPSE_NPC:
         case ITEM_CORPSE_PC:
            message = "$p decays into dust and blows away.";
            /*
             * dump all the eq on the ground instead of making is disappear -- Scion 
             */
            {
               OBJ_DATA *obj1;

               for( obj1 = obj->first_content; obj1; obj1 = obj->first_content )
               {
                  obj_from_obj( obj1 );
                  if( obj->in_room )
                     obj_to_room( obj1, obj->in_room );
                  else
                  {
                     bug( "Obj %d could not be removed from corpse, put in limbo.", obj1->pIndexData->vnum );
                     obj_to_room( obj1, get_room_index( ROOM_VNUM_LIMBO ) );
                  }
               }
            }
            AT_TEMP = AT_MAGIC;
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            message = "$p is devoured by a swarm of maggots.";
            AT_TEMP = AT_HUNGRY;
            break;
         case ITEM_BLOOD:
            message = "$p slowly seeps into the ground.";
            AT_TEMP = AT_BLOOD;
            break;
         case ITEM_BLOODSTAIN:
            message = "$p dries up into flakes and blows away.";
            AT_TEMP = AT_BLOOD;
            break;
         case ITEM_SCRAPS:
            message = "$p crumble and decay into nothing.";
            AT_TEMP = AT_OBJECT;
            break;
         case ITEM_FIRE:
            if( obj->in_room )
            {
               --obj->in_room->light;
               if( obj->in_room->light < 0 )
                  obj->in_room->light = 0;
            }
            message = "$p burns out.";
            AT_TEMP = AT_FIRE;
      }

      if( obj->carried_by )
      {
         act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
      }
      else if( obj->in_room && ( rch = obj->in_room->first_person ) != NULL && !IS_OBJ_STAT( obj, ITEM_BURIED ) )
      {
         act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
         act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
      }

      if( obj->serial == cur_obj )
         global_objcode = rOBJ_EXPIRED;
      extract_obj( obj );
   }
   return;
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
   CHAR_DATA *ch, *ch_next;
   OBJ_DATA *obj;
   EXIT_DATA *pexit;
   static int cnt = 0;
   int door, retcode;

   /*
    * This little counter can be used to handle periodic events 
    */
   cnt = ( cnt + 1 ) % SECONDS_PER_TICK;

   for( ch = first_char; ch; ch = ch_next )
   {
      set_cur_char( ch );
      ch_next = ch->next;
      will_fall( ch, 0 );

      if( char_died( ch ) )
         continue;

      if( IS_NPC( ch ) )
      {
         if( ( cnt & 1 ) )
            continue;

         /*
          * running mobs  -Thoric 
          */
         if( xIS_SET( ch->act, ACT_RUNNING ) )
         {
            if( !xIS_SET( ch->act, ACT_SENTINEL ) && !ch->fighting && ch->hunting )
            {
               WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
               hunt_victim( ch );
               continue;
            }

            if( ch->spec_fun )
            {
               if( ( *ch->spec_fun ) ( ch ) )
                  continue;
               if( char_died( ch ) )
                  continue;
            }

            if( !xIS_SET( ch->act, ACT_SENTINEL )
                && !xIS_SET( ch->act, ACT_PROTOTYPE )
                && ( door = number_bits( 4 ) ) <= 9
                && ( pexit = get_exit( ch->in_room, door ) ) != NULL
                && pexit->to_room
                && !IS_SET( pexit->exit_info, EX_CLOSED )
                && !IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
                && !IS_SET( pexit->to_room->room_flags, ROOM_DEATH )
                && ( !xIS_SET( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
            {
               retcode = move_char( ch, pexit, 0 );
               if( char_died( ch ) )
                  continue;
               if( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL ) || ch->position < POS_STANDING )
                  continue;
            }
         }
         continue;
      }
      else
      {
         if( ch->mount && ch->in_room != ch->mount->in_room )
         {
            xREMOVE_BIT( ch->mount->act, ACT_MOUNTED );
            ch->mount = NULL;
            ch->position = POS_STANDING;
            send_to_char( "No longer upon your mount, you fall to the ground...\r\nOUCH!\r\n", ch );
         }

         if( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
             || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
         {
            if( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
            {
               if( ch->level < LEVEL_IMMORTAL )
               {
                  int dam;

                  /*
                   * Changed level of damage at Brittany's request. -- Narn 
                   */
                  dam = number_range( ch->max_hit / 100, ch->max_hit / 50 );
                  dam = UMAX( 1, dam );
                  if( number_bits( 3 ) == 0 )
                     send_to_char( "You cough and choke as you try to breathe water!\r\n", ch );
                  damage( ch, ch, dam, TYPE_UNDEFINED );
               }
            }
         }

         if( char_died( ch ) )
            continue;

         if( ch->in_room
             && ( ( ch->in_room->sector_type == SECT_WATER_NOSWIM ) || ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
         {
            if( !IS_AFFECTED( ch, AFF_FLYING )
                && !IS_AFFECTED( ch, AFF_FLOATING ) && !IS_AFFECTED( ch, AFF_AQUA_BREATH ) && !ch->mount )
            {
               for( obj = ch->first_carrying; obj; obj = obj->next_content )
                  if( obj->item_type == ITEM_BOAT )
                     break;

               if( !obj )
               {
                  if( ch->level < LEVEL_IMMORTAL )
                  {
                     int mov;
                     int dam;

                     if( ch->move > 0 )
                     {
                        mov = number_range( ch->max_move / 20, ch->max_move / 5 );
                        mov = UMAX( 1, mov );

                        if( ch->move - mov < 0 )
                           ch->move = 0;
                        else
                           ch->move -= mov;
                     }
                     else
                     {
                        dam = number_range( ch->max_hit / 20, ch->max_hit / 5 );
                        dam = UMAX( 1, dam );

                        if( number_bits( 3 ) == 0 )
                           send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\r\n", ch );
                        damage( ch, ch, dam, TYPE_UNDEFINED );
                     }
                  }
               }
            }
         }

         /*
          * beat up on link dead players 
          */
         if( !ch->desc )
         {
            CHAR_DATA *wch, *wch_next;

            for( wch = ch->in_room->first_person; wch; wch = wch_next )
            {
               wch_next = wch->next_in_room;

               if( !IS_NPC( wch )
                   || wch->fighting
                   || IS_AFFECTED( wch, AFF_CHARM )
                   || !IS_AWAKE( wch ) || ( xIS_SET( wch->act, ACT_WIMPY ) && IS_AWAKE( ch ) ) || !can_see( wch, ch ) )
                  continue;

               if( is_hating( wch, ch ) )
               {
                  found_prey( wch, ch );
                  continue;
               }

               if( ( !xIS_SET( wch->act, ACT_AGGRESSIVE )
                     && !xIS_SET( wch->act, ACT_META_AGGR ) )
                   || xIS_SET( wch->act, ACT_MOUNTED ) || IS_SET( wch->in_room->room_flags, ROOM_SAFE ) )
                  continue;
               global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );
            }
         }
      }
   }
}


/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
void aggr_update( void )
{
   DESCRIPTOR_DATA *d, *dnext;
   CHAR_DATA *wch;
   CHAR_DATA *ch;
   CHAR_DATA *ch_next;
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   CHAR_DATA *victim;
   struct act_prog_data *apdtmp;

#ifdef UNDEFD
   /*
    *  GRUNT!  To do
    *
    */
   if( IS_NPC( wch ) && wch->mpactnum > 0 && wch->in_room->area->nplayer > 0 )
   {
      MPROG_ACT_LIST *tmp_act, *tmp2_act;
      for( tmp_act = wch->mpact; tmp_act; tmp_act = tmp_act->next )
      {
         oprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
         DISPOSE( tmp_act->buf );
      }
      for( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
      {
         tmp2_act = tmp_act->next;
         DISPOSE( tmp_act );
      }
      wch->mpactnum = 0;
      wch->mpact = NULL;
   }
#endif

   /*
    * check mobprog act queue 
    */
   while( ( apdtmp = mob_act_list ) != NULL )
   {
      wch = mob_act_list->vo;
      if( !char_died( wch ) && wch->mpactnum > 0 )
      {
         MPROG_ACT_LIST *tmp_act;

         while( ( tmp_act = wch->mpact ) != NULL )
         {
            if( tmp_act->obj && obj_extracted( tmp_act->obj ) )
               tmp_act->obj = NULL;
            if( tmp_act->ch && !char_died( tmp_act->ch ) )
               mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
            wch->mpact = tmp_act->next;
            DISPOSE( tmp_act->buf );
            DISPOSE( tmp_act );
         }
         wch->mpactnum = 0;
         wch->mpact = NULL;
      }
      mob_act_list = apdtmp->next;
      DISPOSE( apdtmp );
   }


   /*
    * Just check descriptors here for victims to aggressive mobs
    * We can check for linkdead victims in char_check   -Thoric
    */
   for( d = first_descriptor; d; d = dnext )
   {
      dnext = d->next;
      if( d->connected != CON_PLAYING || ( wch = d->character ) == NULL )
         continue;

      if( char_died( wch ) || IS_NPC( wch ) || wch->level >= LEVEL_IMMORTAL || !wch->in_room )
         continue;

      for( ch = wch->in_room->first_person; ch; ch = ch_next )
      {
         int count;

         ch_next = ch->next_in_room;

         if( !IS_NPC( ch )
             || ch->fighting
             || IS_AFFECTED( ch, AFF_CHARM )
             || !IS_AWAKE( ch ) || ( xIS_SET( ch->act, ACT_WIMPY ) && IS_AWAKE( wch ) ) || !can_see( ch, wch ) )
            continue;

         if( is_hating( ch, wch ) )
         {
            found_prey( ch, wch );
            continue;
         }

         if( ( !xIS_SET( ch->act, ACT_AGGRESSIVE )
               && !xIS_SET( ch->act, ACT_META_AGGR ) )
             || xIS_SET( ch->act, ACT_MOUNTED ) || IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
            continue;

         /*
          * Ok we have a 'wch' player character and a 'ch' npc aggressor.
          * Now make the aggressor fight a RANDOM pc victim in the room,
          *   giving each 'vch' an equal chance of selection.
          *
          * Depending on flags set, the mob may attack another mob
          */
         count = 0;
         victim = NULL;
         for( vch = wch->in_room->first_person; vch; vch = vch_next )
         {
            vch_next = vch->next_in_room;

            if( ( !IS_NPC( vch ) || xIS_SET( ch->act, ACT_META_AGGR )
                  || xIS_SET( vch->act, ACT_ANNOYING ) )
                && vch->level < LEVEL_IMMORTAL
                && ( !xIS_SET( ch->act, ACT_WIMPY ) || !IS_AWAKE( vch ) ) && can_see( ch, vch ) )
            {
               if( number_range( 0, count ) == 0 )
                  victim = vch;
               count++;
            }
         }

         if( !victim )
         {
            bug( "Aggr_update: null victim.", count );
            continue;
         }

         /*
          * backstabbing mobs (Thoric) 
          */
         if( IS_NPC( ch ) && xIS_SET( ch->attacks, ATCK_BACKSTAB ) )
         {
            OBJ_DATA *obj;

            if( !ch->mount
                && ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL
                && ( obj->value[3] == 11 || obj->value[3] == 2 ) && !victim->fighting && victim->hit >= victim->max_hit )
            {
               check_attacker( ch, victim );
               WAIT_STATE( ch, skill_table[gsn_backstab]->beats );
               if( !IS_AWAKE( victim ) || number_percent(  ) + 5 < ch->level )
               {
                  global_retcode = multi_hit( ch, victim, gsn_backstab );
                  continue;
               }
               else
               {
                  global_retcode = damage( ch, victim, 0, gsn_backstab );
                  continue;
               }
            }
         }
         global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
      }
   }

   return;
}

/* From interp.c */
bool check_social args( ( CHAR_DATA * ch, char *command, char *argument ) );

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms( CHAR_DATA * ch )
{
   CHAR_DATA *rvch = NULL;
   CHAR_DATA *vch;
   short drunk;
   short position;

   if( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )
      return;

   if( number_percent(  ) < 30 )
      return;

   drunk = ch->pcdata->condition[COND_DRUNK];
   position = ch->position;
   ch->position = POS_STANDING;

   if( number_percent(  ) < ( 2 * drunk / 20 ) )
      check_social( ch, "burp", "" );
   else if( number_percent(  ) < ( 2 * drunk / 20 ) )
      check_social( ch, "hiccup", "" );
   else if( number_percent(  ) < ( 2 * drunk / 20 ) )
      check_social( ch, "drool", "" );
   else if( number_percent(  ) < ( 2 * drunk / 20 ) )
      check_social( ch, "fart", "" );
   else if( drunk > ( 10 + ( get_curr_con( ch ) / 5 ) ) && number_percent(  ) < ( 2 * drunk / 18 ) )
   {
      for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
         if( number_percent(  ) < 10 )
            rvch = vch;
      check_social( ch, "puke", ( rvch ? rvch->name : "" ) );
   }

   ch->position = position;
   return;
}

/*
 * Random hallucinations for those suffering from an overly high mentalstate
 * (Hats off to Albert Hoffman's "problem child")	-Thoric
 */
void hallucinations( CHAR_DATA * ch )
{
   if( ch->mental_state >= 30 && number_bits( 5 - ( ch->mental_state >= 50 ) - ( ch->mental_state >= 75 ) ) == 0 )
   {
      char *t;

      switch ( number_range( 1, UMIN( 21, ( ch->mental_state + 5 ) / 5 ) ) )
      {
         default:
         case 1:
            t = "You feel very restless... you can't sit still.\r\n";
            break;
         case 2:
            t = "You're tingling all over.\r\n";
            break;
         case 3:
            t = "Your skin is crawling.\r\n";
            break;
         case 4:
            t = "You suddenly feel that something is terribly wrong.\r\n";
            break;
         case 5:
            t = "Those damn little fairies keep laughing at you!\r\n";
            break;
         case 6:
            t = "You can hear your mother crying...\r\n";
            break;
         case 7:
            t = "Have you been here before, or not?  You're not sure...\r\n";
            break;
         case 8:
            t = "Painful childhood memories flash through your mind.\r\n";
            break;
         case 9:
            t = "You hear someone call your name in the distance...\r\n";
            break;
         case 10:
            t = "Your head is pulsating... you can't think straight.\r\n";
            break;
         case 11:
            t = "The ground... seems to be squirming...\r\n";
            break;
         case 12:
            t = "You're not quite sure what is real anymore.\r\n";
            break;
         case 13:
            t = "It's all a dream... or is it?\r\n";
            break;
         case 14:
            t = "You hear your grandchildren praying for you to watch over them.\r\n";
            break;
         case 15:
            t = "They're coming to get you... coming to take you away...\r\n";
            break;
         case 16:
            t = "You begin to feel all powerful!\r\n";
            break;
         case 17:
            t = "You're light as air... the heavens are yours for the taking.\r\n";
            break;
         case 18:
            t = "Your whole life flashes by... and your future...\r\n";
            break;
         case 19:
            t = "You are everywhere and everything... you know all and are all!\r\n";
            break;
         case 20:
            t = "You feel immortal!\r\n";
            break;
         case 21:
            t = "Ahh... the power of a Supreme Entity... what to do...\r\n";
            break;
      }
      send_to_char( t, ch );
   }
   return;
}

void tele_update( void )
{
   TELEPORT_DATA *tele, *tele_next;

   if( !first_teleport )
      return;

   for( tele = first_teleport; tele; tele = tele_next )
   {
      tele_next = tele->next;
      if( --tele->timer <= 0 )
      {
         if( tele->room->first_person )
         {
            if( IS_SET( tele->room->room_flags, ROOM_TELESHOWDESC ) )
               teleport( tele->room->first_person, tele->room->tele_vnum, TELE_SHOWDESC | TELE_TRANSALL );
            else
               teleport( tele->room->first_person, tele->room->tele_vnum, TELE_TRANSALL );
         }
         UNLINK( tele, first_teleport, last_teleport, next, prev );
         DISPOSE( tele );
      }
   }
}

#if FALSE
/* 
 * Write all outstanding authorization requests to Log channel - Gorog
 */
void auth_update( void )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char log_buf[MAX_INPUT_LENGTH];
   bool first_time = TRUE; /* so titles are only done once */

   for( d = first_descriptor; d; d = d->next )
   {
      victim = d->character;
      if( victim && IS_WAITING_FOR_AUTH( victim ) )
      {
         if( first_time )
         {
            first_time = FALSE;
            strcpy( log_buf, "Pending authorizations:" );
            /*
             * log_string( log_buf ); 
             */
            to_channel( log_buf, CHANNEL_AUTH, "Auth", 1 );
         }
         sprintf( log_buf, " %s@%s new %s %s", victim->name,
                  victim->desc->host, race_table[victim->race]->race_name, class_table[victim->class]->who_name );
/*         log_string( log_buf ); */
         to_channel( log_buf, CHANNEL_AUTH, "Auth", 1 );
      }
   }
}
#endif

void auth_update( void )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char buf[MAX_INPUT_LENGTH], log_buf[MAX_INPUT_LENGTH];
   bool found_hit = FALSE; /* was at least one found? */

   strcpy( log_buf, "Pending authorizations:\r\n" );
   for( d = first_descriptor; d; d = d->next )
   {
      if( ( victim = d->character ) && IS_WAITING_FOR_AUTH( victim ) )
      {
         found_hit = TRUE;
         sprintf( buf, " %s@%s new %s %s\r\n", victim->name,
                  victim->desc->host, race_table[victim->race]->race_name, class_table[victim->class]->who_name );
         strcat( log_buf, buf );
      }
   }
   if( found_hit )
   {
/*	log_string( log_buf ); */
      to_channel( log_buf, CHANNEL_AUTH, "Auth", 1 );
   }
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
   static int pulse_area;
   static int pulse_mobile;
   static int pulse_violence;
   static int pulse_point;
   static int pulse_second;
   static int pulse_arena; /* Scion arenacode */
   static int pulse_regen;
   struct timeval stime;
   struct timeval etime;

   if( timechar )
   {
      set_char_color( AT_PLAIN, timechar );
      send_to_char( "Starting update timer.\r\n", timechar );
      gettimeofday( &stime, NULL );
   }

   if( --pulse_area <= 0 )
   {
      pulse_area = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
      area_update(  );
   }

   if( --pulse_mobile <= 0 )
   {
      pulse_mobile = PULSE_MOBILE;
      mobile_update(  );
   }

   if( --pulse_violence <= 0 )
   {
      pulse_violence = PULSE_VIOLENCE;
      violence_update(  );
   }

   if( --pulse_point <= 0 )
   {
      pulse_point = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );

      auth_update(  );  /* Gorog */
      time_update(  );
      weather_update(  );
      obj_update(  );
      char_update(  );
      clear_vrooms(  ); /* remove virtual rooms */
   }

   if( --pulse_regen <= 0 )
   {
      pulse_regen = number_range( ( PULSE_TICK * 0.75 / 4 ), ( PULSE_TICK * 1.25 / 4 ) );

      char_regen(  );   /* Sped up regen to 4 times normal, left everything else in char_update -- Scion */
   }

   if( --pulse_second <= 0 )
   {
      pulse_second = PULSE_PER_SECOND;
      char_check(  );
      reboot_check( 0 );
   }

   if( --auction->pulse <= 0 )
   {
      auction->pulse = PULSE_AUCTION;
      auction_update(  );
   }

   if( --pulse_arena <= 0 )   /* Scion arenacode */
   {
      pulse_arena = PULSE_AREA;  /* 1 minute */
      arena_update(  );
   }

   tele_update(  );
   aggr_update(  );
   obj_act_update(  );
   room_act_update(  );
   clean_obj_queue(  ); /* dispose of extracted objects */
   clean_char_queue(  );   /* dispose of dead mobs/quitting chars */
   if( timechar )
   {
      gettimeofday( &etime, NULL );
      set_char_color( AT_PLAIN, timechar );
      send_to_char( "Update timing complete.\r\n", timechar );
      subtract_times( &etime, &stime );
      ch_printf( timechar, "Timing took %d.%06d seconds.\r\n", etime.tv_sec, etime.tv_usec );
      timechar = NULL;
   }
   tail_chain(  );
   return;
}


void remove_portal( OBJ_DATA * portal )
{
   ROOM_INDEX_DATA *fromRoom, *toRoom;
   EXIT_DATA *pexit;
   bool found;

   if( !portal )
   {
      bug( "remove_portal: portal is NULL", 0 );
      return;
   }

   fromRoom = portal->in_room;
   found = FALSE;
   if( !fromRoom )
   {
      bug( "remove_portal: portal->in_room is NULL", 0 );
      return;
   }

   for( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
      if( IS_SET( pexit->exit_info, EX_PORTAL ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
   {
      bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
      return;
   }

   if( pexit->vdir != DIR_PORTAL )
      bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

   if( ( toRoom = pexit->to_room ) == NULL )
      bug( "remove_portal: toRoom is NULL", 0 );

   extract_exit( fromRoom, pexit );

   return;
}

void reboot_check( time_t reset )
{
   static char *tmsg[] = { "You feel the ground shake as the end comes near!",
      "Lightning crackles in the sky above!",
      "Crashes of thunder sound across the land!",
      "The sky has suddenly turned midnight black.",
      "You notice the life forms around you slowly dwindling away.",
      "The seas across the realm have turned frigid.",
      "The aura of magic that surrounds the realms seems slightly unstable.",
      "You sense a change in the magical forces surrounding you."
   };
   static const int times[] = { 60, 120, 180, 240, 300, 600, 900, 1800 };
   static const int timesize = UMIN( sizeof( times ) / sizeof( *times ), sizeof( tmsg ) / sizeof( *tmsg ) );
   char buf[MAX_STRING_LENGTH];
   static int trun;
   static bool init = FALSE;

   if( !init || reset >= current_time )
   {
      for( trun = timesize - 1; trun >= 0; trun-- )
         if( reset >= current_time + times[trun] )
            break;
      init = TRUE;
      return;
   }

   if( ( current_time % 1800 ) == 0 )
   {
      sprintf( buf, "%.24s: %d players", ctime( &current_time ), num_descriptors );
      append_to_file( USAGE_FILE, buf );
      sprintf( buf, "%.24s:  %dptn  %dpll  %dsc %dbr  %d global loot",
               ctime( &current_time ),
               sysdata.upotion_val, sysdata.upill_val, sysdata.scribed_used, sysdata.brewed_used, sysdata.global_looted );
      append_to_file( ECONOMY_FILE, buf );
   }

   if( new_boot_time_t - boot_time < 60 * 60 * 18 && !set_boot_time->manual )
      return;

   if( new_boot_time_t <= current_time )
   {
      CHAR_DATA *vch;

      if( auction->item )
      {
         sprintf( buf, "Sale of %s has been stopped by mud.", auction->item->short_descr );
         talk_auction( buf );
         obj_to_char( auction->item, auction->seller );
         auction->item = NULL;
         if( auction->buyer && auction->buyer != auction->seller )
         {
            auction->buyer->gold += auction->bet;
            send_to_char( "Your money has been returned.\r\n", auction->buyer );
         }
      }
      echo_to_all( AT_YELLOW, "You are forced from these realms by a strong "
                   "magical presence\r\nas life here is reconstructed.", ECHOTAR_ALL );
      log_string( "Automatic Reboot" );
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );
      mud_down = TRUE;
      return;
   }

   if( trun != -1 && new_boot_time_t - current_time <= times[trun] )
   {
      echo_to_all( AT_YELLOW, tmsg[trun], ECHOTAR_ALL );
      if( trun <= 5 )
         sysdata.DENY_NEW_PLAYERS = TRUE;
      --trun;
      return;
   }
   return;
}

/* the auction update*/

void auction_update( void )
{
   int tax, pay;
   char buf[MAX_STRING_LENGTH];

   if( !auction->item )
   {
      if( AUCTION_MEM > 0 && auction->history[0] && ++auction->hist_timer == 6 * AUCTION_MEM )
      {
         int i;

         for( i = AUCTION_MEM - 1; i >= 0; i-- )
         {
            if( auction->history[i] )
            {
               auction->history[i] = NULL;
               auction->hist_timer = 0;
               break;
            }
         }
      }
      return;
   }

   switch ( ++auction->going )   /* increase the going state */
   {
      case 1: /* going once */
      case 2: /* going twice */
         if( auction->bet > auction->starting )
            sprintf( buf, "%s: going %s for %d.", auction->item->short_descr,
                     ( ( auction->going == 1 ) ? "once" : "twice" ), auction->bet );
         else
            sprintf( buf, "%s: going %s (bid not received yet).", auction->item->short_descr,
                     ( ( auction->going == 1 ) ? "once" : "twice" ) );

         talk_auction( buf );
         break;

      case 3: /* SOLD! */
         if( !auction->buyer && auction->bet )
         {
            bug( "Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet );
            auction->bet = 0;
         }
         if( auction->bet > 0 && auction->buyer != auction->seller )
         {
            sprintf( buf, "%s sold to %s for %d.",
                     auction->item->short_descr,
                     IS_NPC( auction->buyer ) ? auction->buyer->short_descr : auction->buyer->name, auction->bet );
            talk_auction( buf );

            act( AT_ACTION, "The auctioneer materializes before you, and hands you $p.",
                 auction->buyer, auction->item, NULL, TO_CHAR );
            act( AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.",
                 auction->buyer, auction->item, NULL, TO_ROOM );

            if( ( auction->buyer->carry_weight + get_obj_weight( auction->item ) ) > can_carry_w( auction->buyer ) )
            {
               act( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item,
                    NULL, TO_CHAR );
               act( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item,
                    NULL, TO_ROOM );
               obj_to_room( auction->item, auction->buyer->in_room );
            }
            else
               obj_to_char( auction->item, auction->buyer );
            pay = ( int )auction->bet * 0.9;
            tax = ( int )auction->bet * 0.1;
            boost_economy( auction->seller->in_room->area, tax );
            auction->seller->gold += pay; /* give him the money, tax 10 % */
            sprintf( buf, "The auctioneer pays you %d gold, charging an auction fee of %d.\r\n", pay, tax );
            send_to_char( buf, auction->seller );
            auction->item = NULL;   /* reset item */
            if( IS_SET( sysdata.save_flags, SV_AUCTION ) )
            {
               save_char_obj( auction->buyer );
               save_char_obj( auction->seller );
            }
         }
         else  /* not sold */
         {
            sprintf( buf, "No bids received for %s - removed from auction.\r\n", auction->item->short_descr );
            talk_auction( buf );
            act( AT_ACTION, "The auctioneer appears before you to return $p to you.",
                 auction->seller, auction->item, NULL, TO_CHAR );
            act( AT_ACTION, "The auctioneer appears before $n to return $p to $m.",
                 auction->seller, auction->item, NULL, TO_ROOM );
            if( ( auction->seller->carry_weight + get_obj_weight( auction->item ) ) > can_carry_w( auction->seller ) )
            {
               act( AT_PLAIN, "You drop $p as it is just too much to carry"
                    " with everything else you're carrying.", auction->seller, auction->item, NULL, TO_CHAR );
               act( AT_PLAIN, "$n drops $p as it is too much extra weight"
                    " for $m with everything else.", auction->seller, auction->item, NULL, TO_ROOM );
               obj_to_room( auction->item, auction->seller->in_room );
            }
            else
               obj_to_char( auction->item, auction->seller );
            tax = ( int )auction->item->cost * 0.05;
            boost_economy( auction->seller->in_room->area, tax );
            sprintf( buf, "The auctioneer charges you an auction fee of %d.\r\n", tax );
            send_to_char( buf, auction->seller );
            if( ( auction->seller->gold - tax ) < 0 )
               auction->seller->gold = 0;
            else
               auction->seller->gold -= tax;
            if( IS_SET( sysdata.save_flags, SV_AUCTION ) )
               save_char_obj( auction->seller );
         }  /* else */
         auction->item = NULL;   /* clear auction */
   }  /* switch */
}  /* func */

void subtract_times( struct timeval *etime, struct timeval *stime )
{
   etime->tv_sec -= stime->tv_sec;
   etime->tv_usec -= stime->tv_usec;
   while( etime->tv_usec < 0 )
   {
      etime->tv_usec += 1000000;
      etime->tv_sec--;
   }
   return;
}

/*
 * Function to update weather vectors according to climate
 * settings, random effects, and neighboring areas.
 * Last modified: July 18, 1997
 * - Fireblade
 */
void adjust_vectors( WEATHER_DATA * weather )
{
   NEIGHBOR_DATA *neigh;
   double dT, dP, dW;

   if( !weather )
   {
      bug( "adjust_vectors: NULL weather data.", 0 );
      return;
   }

   dT = 0;
   dP = 0;
   dW = 0;

   /*
    * Add in random effects 
    */
   dT += number_range( -rand_factor, rand_factor );
   dP += number_range( -rand_factor, rand_factor );
   dW += number_range( -rand_factor, rand_factor );

   /*
    * Add in climate effects
    */
   dT += climate_factor * ( ( ( weather->climate_temp - 2 ) * weath_unit ) - ( weather->temp ) ) / weath_unit;
   dP += climate_factor * ( ( ( weather->climate_precip - 2 ) * weath_unit ) - ( weather->precip ) ) / weath_unit;
   dW += climate_factor * ( ( ( weather->climate_wind - 2 ) * weath_unit ) - ( weather->wind ) ) / weath_unit;


   /*
    * Add in effects from neighboring areas 
    */
   for( neigh = weather->first_neighbor; neigh; neigh = neigh->next )
   {
      /*
       * see if we have the area cache'd already 
       */
      if( !neigh->address )
      {
         /*
          * try and find address for area 
          */
         neigh->address = get_area( neigh->name );

         /*
          * if couldn't find area ditch the neigh 
          */
         if( !neigh->address )
         {
            NEIGHBOR_DATA *temp;
            bug( "adjust_weather: " "invalid area name.", 0 );
            temp = neigh->prev;
            UNLINK( neigh, weather->first_neighbor, weather->last_neighbor, next, prev );
            STRFREE( neigh->name );
            DISPOSE( neigh );
            neigh = temp;
            continue;
         }
      }

      dT += ( neigh->address->weather->temp - weather->temp ) / neigh_factor;
      dP += ( neigh->address->weather->precip - weather->precip ) / neigh_factor;
      dW += ( neigh->address->weather->wind - weather->wind ) / neigh_factor;
   }

   /*
    * now apply the effects to the vectors 
    */
   weather->temp_vector += ( int )dT;
   weather->precip_vector += ( int )dP;
   weather->wind_vector += ( int )dW;

   /*
    * Make sure they are within the right range 
    */
   weather->temp_vector = URANGE( -max_vector, weather->temp_vector, max_vector );
   weather->precip_vector = URANGE( -max_vector, weather->precip_vector, max_vector );
   weather->wind_vector = URANGE( -max_vector, weather->wind_vector, max_vector );

   return;
}

/*
 * function updates weather for each area
 * Last Modified: July 31, 1997
 * Fireblade
 */
void weather_update(  )
{
   AREA_DATA *pArea;
   DESCRIPTOR_DATA *d;
   int limit;

   limit = 3 * weath_unit;

   for( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
   {
      /*
       * Apply vectors to fields 
       */
      pArea->weather->temp += pArea->weather->temp_vector;
      pArea->weather->precip += pArea->weather->precip_vector;
      pArea->weather->wind += pArea->weather->wind_vector;

      /*
       * Make sure they are within the proper range 
       */
      pArea->weather->temp = URANGE( -limit, pArea->weather->temp, limit );
      pArea->weather->precip = URANGE( -limit, pArea->weather->precip, limit );
      pArea->weather->wind = URANGE( -limit, pArea->weather->wind, limit );

      /*
       * get an appropriate echo for the area 
       */
      get_weather_echo( pArea->weather );
   }

   for( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
   {
      adjust_vectors( pArea->weather );
   }

   /*
    * display the echo strings to the appropriate players 
    */
   for( d = first_descriptor; d; d = d->next )
   {
      WEATHER_DATA *weath;

      if( d->connected == CON_PLAYING &&
          IS_OUTSIDE( d->character ) && !NO_WEATHER_SECT( d->character->in_room->sector_type ) && IS_AWAKE( d->character ) )
      {
         weath = d->character->in_room->area->weather;
         if( !weath->echo )
            continue;
         set_char_color( weath->echo_color, d->character );
         ch_printf( d->character, weath->echo );
      }
   }

   return;
}

/*
 * get weather echo messages according to area weather...
 * stores echo message in weath_data.... must be called before
 * the vectors are adjusted
 * Last Modified: August 10, 1997
 * Fireblade
 */
void get_weather_echo( WEATHER_DATA * weath )
{
   int n;
   int temp, precip, wind;
   int dT, dP, dW;
   int tindex, pindex, windex;

   /*
    * set echo to be nothing 
    */
   weath->echo = NULL;
   weath->echo_color = AT_GREY;

   /*
    * get the random number 
    */
   n = number_bits( 2 );

   /*
    * variables for convenience 
    */
   temp = weath->temp;
   precip = weath->precip;
   wind = weath->wind;

   dT = weath->temp_vector;
   dP = weath->precip_vector;
   dW = weath->wind_vector;

   tindex = ( temp + 3 * weath_unit - 1 ) / weath_unit;
   pindex = ( precip + 3 * weath_unit - 1 ) / weath_unit;
   windex = ( wind + 3 * weath_unit - 1 ) / weath_unit;

   /*
    * get the echo string... mainly based on precip 
    */
   switch ( pindex )
   {
      case 0:
         if( precip - dP > -2 * weath_unit )
         {
            char *echo_strings[4] = {
               "The clouds disappear.\r\n",
               "The clouds disappear.\r\n",
               "The sky begins to break through " "the clouds.\r\n",
               "The clouds are slowly " "evaporating.\r\n"
            };

            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         break;

      case 1:
         if( precip - dP <= -2 * weath_unit )
         {
            char *echo_strings[4] = {
               "The sky is getting cloudy.\r\n",
               "The sky is getting cloudy.\r\n",
               "Light clouds cast a haze over " "the sky.\r\n",
               "Billows of clouds spread through " "the sky.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_GREY;
         }
         break;

      case 2:
         if( precip - dP > 0 )
         {
            if( tindex > 1 )
            {
               char *echo_strings[4] = {
                  "The rain stops.\r\n",
                  "The rain stops.\r\n",
                  "The rainstorm tapers " "off.\r\n",
                  "The rain's intensity " "breaks.\r\n"
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_CYAN;
            }
            else
            {
               char *echo_strings[4] = {
                  "The snow stops.\r\n",
                  "The snow stops.\r\n",
                  "The snow showers taper " "off.\r\n",
                  "The snow flakes disappear " "from the sky.\r\n"
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_WHITE;
            }
         }
         break;

      case 3:
         if( precip - dP <= 0 )
         {
            if( tindex > 1 )
            {
               char *echo_strings[4] = {
                  "It starts to rain.\r\n",
                  "It starts to rain.\r\n",
                  "A droplet of rain falls " "upon you.\r\n",
                  "The rain begins to " "patter.\r\n"
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_CYAN;
            }
            else
            {
               char *echo_strings[4] = {
                  "It starts to snow.\r\n",
                  "It starts to snow.\r\n",
                  "Crystal flakes begin to " "fall from the " "sky.\r\n",
                  "Snow flakes drift down " "from the clouds.\r\n"
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_WHITE;
            }
         }
         else if( tindex < 2 && temp - dT > -weath_unit )
         {
            char *echo_strings[4] = {
               "The temperature drops and the rain " "becomes a light snow.\r\n",
               "The temperature drops and the rain " "becomes a light snow.\r\n",
               "Flurries form as the rain freezes.\r\n",
               "Large snow flakes begin to fall " "with the rain.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         else if( tindex > 1 && temp - dT <= -weath_unit )
         {
            char *echo_strings[4] = {
               "The snow flurries are gradually " "replaced by pockets of rain.\r\n",
               "The snow flurries are gradually " "replaced by pockets of rain.\r\n",
               "The falling snow turns to a cold drizzle.\r\n",
               "The snow turns to rain as the air warms.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
         }
         break;

      case 4:
         if( precip - dP > 2 * weath_unit )
         {
            if( tindex > 1 )
            {
               char *echo_strings[4] = {
                  "The lightning has stopped.\r\n",
                  "The lightning has stopped.\r\n",
                  "The sky settles, and the " "thunder surrenders.\r\n",
                  "The lightning bursts fade as " "the storm weakens.\r\n"
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_GREY;
            }
         }
         else if( tindex < 2 && temp - dT > -weath_unit )
         {
            char *echo_strings[4] = {
               "The cold rain turns to snow.\r\n",
               "The cold rain turns to snow.\r\n",
               "Snow flakes begin to fall " "amidst the rain.\r\n",
               "The driving rain begins to freeze.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         else if( tindex > 1 && temp - dT <= -weath_unit )
         {
            char *echo_strings[4] = {
               "The snow becomes a freezing rain.\r\n",
               "The snow becomes a freezing rain.\r\n",
               "A cold rain beats down on you " "as the snow begins to melt.\r\n",
               "The snow is slowly replaced by a heavy " "rain.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
         }
         break;

      case 5:
         if( precip - dP <= 2 * weath_unit )
         {
            if( tindex > 1 )
            {
               char *echo_strings[4] = {
                  "Lightning flashes in the " "sky.\r\n",
                  "Lightning flashes in the " "sky.\r\n",
                  "A flash of lightning splits " "the sky.\r\n",
                  "The sky flashes, and the " "ground trembles with " "thunder.\r\n"
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_YELLOW;
            }
         }
         else if( tindex > 1 && temp - dT <= -weath_unit )
         {
            char *echo_strings[4] = {
               "The sky rumbles with thunder as " "the snow changes to rain.\r\n",
               "The sky rumbles with thunder as " "the snow changes to rain.\r\n",
               "The falling turns to freezing rain " "amidst flashes of " "lightning.\r\n",
               "The falling snow begins to melt as " "thunder crashes overhead.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         else if( tindex < 2 && temp - dT > -weath_unit )
         {
            char *echo_strings[4] = {
               "The lightning stops as the rainstorm " "becomes a blinding " "blizzard.\r\n",
               "The lightning stops as the rainstorm " "becomes a blinding " "blizzard.\r\n",
               "The thunder dies off as the " "pounding rain turns to " "heavy snow.\r\n",
               "The cold rain turns to snow and " "the lightning stops.\r\n"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
         }
         break;

      default:
         bug( "echo_weather: invalid precip index" );
         weath->precip = 0;
         break;
   }

   return;
}

/*
 * get echo messages according to time changes...
 * some echoes depend upon the weather so an echo must be
 * found for each area
 * Last Modified: August 10, 1997
 * Fireblade
 */
void get_time_echo( WEATHER_DATA * weath )
{
   int n;
   int pindex;

   n = number_bits( 2 );
   pindex = ( weath->precip + 3 * weath_unit - 1 ) / weath_unit;
   weath->echo = NULL;
   weath->echo_color = AT_GREY;

   switch ( time_info.hour )
   {
      case 5:
      {
         char *echo_strings[4] = {
            "The day has begun.\r\n",
            "The day has begun.\r\n",
            "The sky slowly begins to glow.\r\n",
            "The sun slowly embarks upon a new day.\r\n"
         };
         time_info.sunlight = SUN_RISE;
         weath->echo = echo_strings[n];
         weath->echo_color = AT_YELLOW;
         break;
      }
      case 6:
      {
         char *echo_strings[4] = {
            "The sun rises in the east.\r\n",
            "The sun rises in the east.\r\n",
            "The hazy sun rises over the horizon.\r\n",
            "Day breaks as the sun lifts into the sky.\r\n"
         };
         time_info.sunlight = SUN_LIGHT;
         weath->echo = echo_strings[n];
         weath->echo_color = AT_ORANGE;
         break;
      }
      case 12:
      {
         if( pindex > 0 )
         {
            weath->echo = "It's noon.\r\n";
         }
         else
         {
            char *echo_strings[2] = {
               "The intensity of the sun " "heralds the noon hour.\r\n",
               "The sun's bright rays beat down " "upon your shoulders.\r\n"
            };
            weath->echo = echo_strings[n % 2];
         }
         time_info.sunlight = SUN_LIGHT;
         weath->echo_color = AT_WHITE;
         break;
      }
      case 19:
      {
         char *echo_strings[4] = {
            "The sun slowly disappears in the west.\r\n",
            "The reddish sun sets past the horizon.\r\n",
            "The sky turns a reddish orange as the sun " "ends its journey.\r\n",
            "The sun's radiance dims as it sinks in the " "sky.\r\n"
         };
         time_info.sunlight = SUN_SET;
         weath->echo = echo_strings[n];
         weath->echo_color = AT_RED;
         break;
      }
      case 20:
      {
         if( pindex > 0 )
         {
            char *echo_strings[2] = {
               "The night begins.\r\n",
               "Twilight descends around you.\r\n"
            };
            weath->echo = echo_strings[n % 2];
         }
         else
         {
            char *echo_strings[2] = {
               "The moon's gentle glow diffuses " "through the night sky.\r\n",
               "The night sky gleams with " "glittering starlight.\r\n"
            };
            weath->echo = echo_strings[n % 2];
         }
         time_info.sunlight = SUN_DARK;
         weath->echo_color = AT_DBLUE;
         break;
      }
   }

   return;
}

/*
 * update the time
 */
void time_update(  )
{
   AREA_DATA *pArea;
   DESCRIPTOR_DATA *d;
   WEATHER_DATA *weath;

   switch ( ++time_info.hour )
   {
      case 5:
      case 6:
      case 12:
         save_world( NULL );
      case 19:
      case 20:
         for( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
         {
            get_time_echo( pArea->weather );
         }

         for( d = first_descriptor; d; d = d->next )
         {
            if( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && IS_AWAKE( d->character ) )
            {
               weath = d->character->in_room->area->weather;
               if( !weath->echo )
                  continue;
               set_char_color( weath->echo_color, d->character );
               ch_printf( d->character, weath->echo );
            }
         }
         break;
      case 24:
         time_info.hour = 0;
         time_info.day++;
         save_world( NULL );
         break;
   }

   if( time_info.day >= 29 )
   {
      time_info.day = 0;
      time_info.month++;
   }

   if( time_info.month >= 17 )
   {
      time_info.month = 0;
      time_info.year++;
   }

   return;
}
