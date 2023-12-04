//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_custom.cc
//
//  Custom item, enemy, etc. for ZQuest.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <map>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "zc_custom.h"
#include "zq_custom.h"
#include "tiles.h"
#include "zq_tiles.h"
#include "zq_misc.h"
#include "zquest.h"
#include "items.h"
#include "zsys.h"
#include "sfx.h"
#include "init.h"
#include "ffasm.h"
#include "defdata.h"
#include "selectors.h"
#include "zc_malloc.h"
#include "backend/AllBackends.h"

extern int ex;
extern void reset_itembuf(itemdata *item, const ItemDefinitionRef &id);
extern const char *item_class_list(int index, int *list_size);
extern const char *sfxlist(int index, int *list_size);
// zq_subscr.cpp
extern int d_qtile_proc(int msg,DIALOG *d,int c);
extern int d_ctl_proc(int msg,DIALOG *d,int c);
extern int d_csl_proc(int msg,DIALOG *d,int c);
extern int d_csl2_proc(int msg,DIALOG *d,int c);
extern int d_tileblock_proc(int msg,DIALOG *d,int c);
extern int d_stilelist_proc(int msg,DIALOG *d,int c);
extern int sstype_drop_proc(int msg,DIALOG *d,int c);
extern int jwin_fontdrop_proc(int msg,DIALOG *d,int c);
extern int jwin_tflpcheck_proc(int msg,DIALOG *d,int c);
extern int jwin_lscheck_proc(int msg,DIALOG *d,int c);

extern int biic_cnt;



#ifdef _MSC_VER
#define stricmp _stricmp
#define snprintf _snprintf
#endif


int link_animation_speed = 1; //lower is faster animation

int d_ecstile_proc(int msg,DIALOG *d,int c);

int d_cstile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    switch(msg)
    {
    case MSG_CLICK:
    {
        int f = 0;
        int t = d->d1;
        int cs = d->d2;
        
        if(select_tile(t,f,1,cs,true))
        {
            d->d1 = t;
            d->d2 = cs;
            return D_REDRAW;
        }
    }
    break;
    
    case MSG_DRAW:
        if(is_large())
        {
            d->w = 36;
            d->h = 36;
        }
        
        BITMAP *buf = create_bitmap_ex(8,20,20);
        BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
        
        if(buf && bigbmp)
        {
            clear_bitmap(buf);
            
            if(d->d1)
                overtile16(buf,d->d1,2,2,d->d2,0);
                
            stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->w-2, d->h-2);
            destroy_bitmap(buf);
            jwin_draw_frame(bigbmp,0,0,d->w,d->h,FR_DEEP);
            blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
            destroy_bitmap(bigbmp);
        }
        
        //    text_mode(d->bg);
        FONT *fonty = is_large() ? font : pfont;
        textprintf_ex(screen,fonty,d->x+d->w,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile: %d",d->d1);
        textprintf_ex(screen,fonty,d->x+d->w,d->y+text_height(fonty)+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
        break;
    }
    
    return D_O_K;
}

extern int d_msg_preview_proc(int msg, DIALOG *d, int c);
extern int d_msg_edit_proc(int msg, DIALOG *d, int c);
extern int d_itemdropedit_proc(int msg, DIALOG *d, int c);
extern int d_misccolors_tab_proc(int msg, DIALOG *d, int c);
extern int fs_edit_proc(int msg, DIALOG *d, int c);
extern int fs_elist_proc(int msg, DIALOG *d, int c);
extern int fs_flist_proc(int msg, DIALOG *d, int c);
extern int fs_dlist_proc(int msg, DIALOG *d, int c);

DIALOG *resizeDialog(DIALOG *d, float largeSize)
{
	int len = 0;
	while (d[len].proc != NULL)
		len++;

	len++;

	DIALOG *newd = new DIALOG[len];
	memcpy(newd, d, len * sizeof(DIALOG));

	for (int i = 0; i < len; i++)
	{
		if ( (newd[i].proc == jwin_tab_proc 
			|| newd[i].proc == d_msg_preview_proc
			|| newd[i].proc == d_itemdropedit_proc
			|| newd[i].proc == d_misccolors_tab_proc
			|| newd[i].proc == d_ffcombolist_proc
			|| newd[i].proc == fs_edit_proc
			|| newd[i].proc == fs_elist_proc
			|| newd[i].proc == fs_flist_proc
			|| newd[i].proc == fs_dlist_proc			
			|| newd[i].proc == d_msg_edit_proc
			|| newd[i].proc == jwin_initlist_proc
			|| newd[i].proc == d_ecstile_proc
			)			
			&& newd[i].dp3 == d)
			newd[i].dp3 = newd;
	}

	if (is_large())
	{
		int oldwidth = newd[0].w;
		int oldheight = newd[0].h;
		int oldx = newd[0].x;
		int oldy = newd[0].y;
		newd[0].x -= int(float(newd[0].w) / largeSize);
		newd[0].y -= int(float(newd[0].h) / largeSize);
		newd[0].w = int(float(newd[0].w)*largeSize);
		newd[0].h = int(float(newd[0].h)*largeSize);

		for (int i = 1; newd[i].proc != NULL; i++)
		{
			// Place elements horizontally
			double xpc = ((double)(newd[i].x - oldx) / (double)oldwidth);
			newd[i].x = int(newd[0].x + (xpc*double(newd[0].w)));

			// Horizontally resize elements
			if ((newd[i].proc == d_maptile_proc && newd[i].dp2 != (void*)1) || newd[i].proc == d_intro_edit_proc || newd[i].proc == d_title_edit_proc)
			{
				newd[i].x += (int)(float(newd[i].w) / 4.f);
			}
			else if (newd[i].proc == d_comboframe_proc)
			{
				newd[i].w *= 2;
				newd[i].w -= 4;
			}
			else if (newd[i].proc == d_wflag_proc || newd[i].proc == d_bitmap_proc || newd[i].proc == d_maptile_proc || newd[i].proc == d_qtile_proc || newd[i].proc == d_tileblock_proc)
			{
				newd[i].w *= 2;
			}
			else if (newd[i].proc == jwin_button_proc)
				newd[i].w = int(newd[i].w*1.5);
			else newd[i].w = int(float(newd[i].w)*largeSize);

			// Place elements vertically
			double ypc = ((double)(newd[i].y - oldy) / (double)oldheight);
			newd[i].y = int(newd[0].y + (ypc*double(newd[0].h)));

			// Vertically resize elements
			if ((newd[i].proc == d_maptile_proc && newd[i].dp2 != (void*)1) || newd[i].proc == d_intro_edit_proc || newd[i].proc == d_title_edit_proc)
			{
			}
			else if (newd[i].proc == jwin_edit_proc || newd[i].proc == jwin_check_proc || newd[i].proc == jwin_checkfont_proc || newd[i].proc == jwin_tflpcheck_proc || newd[i].proc == jwin_lscheck_proc)
			{
				newd[i].h = int((double)newd[i].h*1.5);
			}
			else if (newd[i].proc == jwin_droplist_proc || newd[i].proc == d_ndroplist_proc || newd[i].proc == d_dropdmaplist_proc
				|| newd[i].proc == d_dropdmaptypelist_proc || newd[i].proc == jwin_as_droplist_proc || newd[i].proc == d_ffcombolist_proc || newd[i].proc == sstype_drop_proc || newd[i].proc == d_ctl_proc
				|| newd[i].proc == jwin_fontdrop_proc || newd[i].proc == d_csl_proc || newd[i].proc == d_csl2_proc || newd[i].proc == d_stilelist_proc || newd[i].proc == d_comboalist_proc)
			{
				newd[i].y += int((double)newd[i].h*0.25);
				newd[i].h = int((double)newd[i].h*1.25);
			}
			else if (newd[i].proc == d_comboframe_proc)
			{
				newd[i].h *= 2;
				newd[i].h -= 4;
			}
			else if (newd[i].proc == d_wflag_proc || newd[i].proc == d_bitmap_proc || newd[i].proc == d_maptile_proc || newd[i].proc == d_qtile_proc || newd[i].proc == d_tileblock_proc)
			{
				newd[i].h *= 2;
			}
			else if (newd[i].proc == jwin_button_proc)
				newd[i].h = int(newd[i].h*1.5);
			else newd[i].h = int(float(newd[i].h)*largeSize);

			// Fix frames
			if (newd[i].proc == jwin_frame_proc)
			{
				newd[i].x++;
				newd[i].y++;
				newd[i].w -= 4;
				newd[i].h -= 4;
			}

			// Fix menus
			if (newd[i].proc == jwin_menu_proc)
			{
				newd[i].y = newd[0].y + 23;
				newd[i].h = 13;
			}
		}


		for (int i = 1; newd[i].proc != NULL; i++)
		{
			if (newd[i].proc == jwin_slider_proc)
				continue;

			// Bigger font
			bool bigfontproc = (newd[i].proc != jwin_initlist_proc && newd[i].proc != jwin_droplist_proc && newd[i].proc != jwin_abclist_proc && newd[i].proc != jwin_list_proc && newd[i].proc != d_dmaplist_proc
				&& newd[i].proc != d_dropdmaplist_proc && newd[i].proc != d_xmaplist_proc && newd[i].proc != d_dropdmaptypelist_proc && newd[i].proc != d_warplist_proc && newd[i].proc != d_warplist_proc && newd[i].proc != d_wclist_proc && newd[i].proc != d_ndroplist_proc
				&& newd[i].proc != jwin_as_droplist_proc && newd[i].proc != d_ffcombolist_proc && newd[i].proc != d_enelist_proc && newd[i].proc != sstype_drop_proc && newd[i].proc != d_ctl_proc
				&& newd[i].proc != jwin_fontdrop_proc && newd[i].proc != d_csl_proc && newd[i].proc != d_csl2_proc && newd[i].proc != d_stilelist_proc && newd[i].proc != d_comboalist_proc && d_modlist_proc != newd[i].proc && d_ilist_proc != newd[i].proc 
                && d_wlist_proc != newd[i].proc && d_en_modlist_proc != newd[i].proc);

			if (bigfontproc && !newd[i].dp2)
			{
				newd[i].dp2 = lfont_l;
			}
			else if (!bigfontproc)
			{
				((ListData *)newd[i].dp)->font = &lfont_l;
			}

			// Make checkboxes work
			if (newd[i].proc == jwin_check_proc)
				newd[i].proc = jwin_checkfont_proc;
			else if (newd[i].proc == jwin_radio_proc)
				newd[i].proc = jwin_radiofont_proc;
		}
	}
    
    jwin_center_dialog(newd);
	return newd;
}

/*****************************/
/******  onCustomItems  ******/
/*****************************/

struct ItemNameInfo
{
    int iclass;
    char *power;
    char *misc1;
    char *misc2;
    char *misc3;
    char *misc4;
    char *misc5;
    char *misc6;
    char *misc7;
    char *misc8;
    char *misc9;
    char *misc10;
    char *flag1;
    char *flag2;
    char *flag3;
    char *flag4;
    char *flag5;
    char *wpn1;
    char *wpn2;
    char *wpn3;
    char *wpn4;
    char *wpn5;
    char *wpn6;
    char *wpn7;
    char *wpn8;
    char *wpn9;
    char *wpn10;
    char *actionsnd;
    char *weapduration;
    char *weaprange;
    char *usedefence;
    char *useweapon;
    char *weap_pattern[ITEM_MOVEMENT_PATTERNS];
};


static ItemNameInfo inameinf[]=
{
    //itemclass                          power                                      misc1                                        misc2                                      misc3                              misc4                              misc5                              misc6                              misc7                              misc8                              misc9                              misc10                             flag1                                      flag2                              flag3                              flag4                              flag5                              wpn1                               wpn2                                       wpn3                                       wpn4                                       wpn5                                       wpn6                                       wpn7                                       wpn8                                       wpn9                                       wpn10                                      action sound
    { itype_fairy,                       NULL, (char *)"HP Regained:", (char *)"MP Regained:", (char *)"Step Speed:",             NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"HP. R. Is Percent", (char *)"MP. R. Is Percent",       NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Item Drop Sound:"                 },
    { itype_triforcepiece,               NULL, (char *)"Cutscene MIDI:", (char *)"Cutscene Type (0-1):",            NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Side Warp Out",                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_shield,                      NULL, (char *)"Block Flags:", (char *)"Reflect Flags:",                  NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"De/Reflection Sound:"             },
    { itype_agony, (char *)"Sensitivity:", (char *)"Vibration Frequency",               NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_wealthmedal,                 NULL, (char *)"Discount Amount:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"D. A. Is Percent",                NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_cbyrna, (char *)"Beam Damage:", (char *)"Beam Speed Divisor:", (char *)"Orbit Radius:", (char *)"Number Of Beams:",        NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Penetrates Enemies",              NULL,                              NULL, (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Beam Sprite:", (char *)"Sparkle Sprite:", (char *)"Damaging Sparkle Sprite:",        NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Orbit Sound:"                     },
    { itype_whispring, (char *)"Jinx Divisor:", (char *)"Jinx Type:",                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Perm. Jinx Are Temp.",            NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_ladder, (char *)"Four-Way:",                       NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_magickey, (char *)"Dungeon Level:",			      NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Lesser D. Levels Also",           NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_wallet, (char *)"Infinite Supply:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_quiver, (char *)"Infinite Supply:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_bombbag, (char *)"Infinite Supply:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Super Bombs Also",                NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_quakescroll2, (char *)"Damage Multiplier:", (char *)"Stun Duration:", (char *)"Stun Radius:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Quake Sound:"                     },
    { itype_quakescroll, (char *)"Damage Multiplier:", (char *)"Stun Duration:", (char *)"Stun Radius:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Quake Sound:"                     },
    { itype_perilscroll,                 NULL, (char *)"Maximum Hearts:",                   NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_spinscroll2, (char *)"Damage Multiplier:", (char *)"Number of Spins:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Spinning Sound:"                  },
    { itype_spinscroll, (char *)"Damage Multiplier:", (char *)"Number of Spins:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Spinning Sound:"                  },
    { itype_clock,                       NULL, (char *)"Duration (0 = Infinite):",          NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_magicring, (char *)"Infinite Magic:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_heartring,                   NULL, (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    //itemclass                          power                                      misc1                                        misc2                                      misc3                              misc4                              misc5                              misc6                              misc7                              misc8                              misc9                              misc10                             flag1                                      flag2                              flag3                              flag4                              flag5                              wpn1                               wpn2                                       wpn3                                       wpn4                                       wpn5                                       wpn6                                       wpn7                                       wpn8                                       wpn9                                       wpn10                                      action sound
    { itype_chargering,                  NULL, (char *)"Charging Duration:", (char *)"Magic C. Duration:",              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_hoverboots,                  NULL, (char *)"Hover Duration:",                   NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Halo Sprite:",            NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Hovering Sound:"                  },
    { itype_rocs, (char *)"Height Multiplier:",              NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Jumping Sound:"                   },
    { itype_sbomb, (char *)"Damage:", (char *)"Fuse Duration (0 = Remote):", (char *)"Max. On Screen:", (char *)"Damage to Link:",	     NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Bomb Sprite:", (char *)"Explosion Sprite:",               NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Explosion Sound:"                 },
    { itype_bomb, (char *)"Damage:", (char *)"Fuse Duration (0 = Remote):", (char *)"Max. On Screen:", (char *)"Damage to Link:",	     NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Use 1.92 Timing",                 NULL,                              NULL,                              NULL,                              NULL, (char *)"Bomb Sprite:", (char *)"Explosion Sprite:",               NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Explosion Sound:"                 },
    { itype_nayruslove,                  NULL, (char *)"Duration:",                         NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Rocket Flickers", (char *)"Translucent Rocket", (char *)"Translucent Shield", (char *)"Shield Flickers",         NULL, (char *)"Left Rocket Sprite:", (char *)"L. Rocket Return Sprite:", (char *)"L. Rocket Sparkle Sprite:", (char *)"L. Return Sparkle Sprite:", (char *)"Shield Sprite (2x2, Over):", (char *)"Right Rocket Sprite:", (char *)"R. Rocket Return Sprite:", (char *)"R. Rocket Sparkle Sprite:", (char *)"R. Return Sparkle Sprite:", (char *)"Shield Sprite (2x2, Under):", (char *)"Shield Sound:"                    },
    { itype_faroreswind,                 NULL, (char *)"Warp Animation (0-2):",             NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Wind Sound:"                      },
    { itype_dinsfire, (char *)"Damage:", (char *)"Number of Flames:", (char *)"Circle Width:",                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Don't Provide Light", (char *)"Falls in Sideview",       NULL,                              NULL, (char *)"Rocket Up Sprite:", (char *)"Rocket Down Sprite:", (char *)"R. Up Sparkle Sprite:", (char *)"R. Down Sparkle Sprite:", (char *)"Flame Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Ring Sound:"                      },
    { itype_hammer, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Hammer Sprite:", (char *)"Smack Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Pound Sound:"                     },
    { itype_lens,                        NULL, (char *)"Lens Width:",                       NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Activation Sound:"                },
    { itype_hookshot, (char *)"Damage:", (char *)"Chain Length:", (char *)"Chain Links:", (char *)"Block Flags:", (char *)"Reflect Flags:",          NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"No Handle Damage",                 (char *)"Allow Diagonal",                              NULL,                              NULL,                              NULL, (char *)"Tip Sprite:", (char *)"Chain Sprite (H):", (char *)"Chain Sprite (V):", (char *)"Handle Sprite:",                  NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_boots, (char *)"Damage Combo Level:",             NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              (char *)"Iron",                              NULL, (char *)"Not Solid Combos",                NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_bracelet, (char *)"Push Combo Level:",               NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Once Per Screen",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_book, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Fire Magic",                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Magic Sprite:", (char *)"Flame Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_ring, (char *)"Damage Divisor:", (char *)"Link Sprite Pal:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_wand, (char *)"Damage:",                         (char *)"W. Type:",                                        (char *)"W. Speed:",                                      (char *)"W. Range:",                              NULL,                              (char *)"Move Effect:",                              (char *)"Mvt Arg1:",                              (char *)"Mvt Arg2:",                              (char *)"No. of Clones:",                              (char *)"Clone Pattern:",                              NULL, (char *)"Allow Magic w/o Book",            (char *)"Wand Moves",                              NULL, (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Projectile Sprite:",      (char *)"Projectile Misc:",                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_bait,                        NULL, (char *)"Duration:",                         NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Bait Sprite:",            NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Baiting Sound:"                   },
    { itype_potion,                      NULL, (char *)"HP Regained:", (char *)"MP Regained:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"HP R. Is Percent", (char *)"MP R. Is Percent",        NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_whistle,                     NULL, (char *)"Whirlwind Direction:", (char *)"Warp Ring:",                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"One W.Wind Per Scr.",             NULL,                              NULL,                              NULL,                              NULL, (char *)"Whirlwind Sprite:",       NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Music Sound:"                     },
    { itype_candle, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Once Per Screen", (char *)"Don't Provide Light",     NULL, (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Flame Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_arrow, (char *)"Damage:", (char *)"Duration (0 = Infinite):",          NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Penetrate Enemies",               NULL,                              NULL,                              NULL,                              NULL, (char *)"Arrow Sprite:", (char *)"Sparkle Sprite:", (char *)"Damaging Sparkle Sprite:",        NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_brang, (char *)"Damage:", (char *)"Range (0 = Infinite):",             NULL, (char *)"Block Flags:", (char *)"Reflect Flags:",          NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Corrected Animation", (char *)"Directional Sprites", (char *)"Do Not Return",           NULL,                              NULL, (char *)"Boomerang Sprite:", (char *)"Sparkle Sprite:", (char *)"Damaging Sparkle Sprite:",        NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Spinning Sound:"                  },
    { itype_sword, (char *)"Damage:", (char *)"Beam Hearts:", (char *)"Beam Damage:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"B.H. Is Percent", (char *)"B.D. Is Percent", (char *)"B. Penetrates Enemies", (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Beam Sprite:",                    NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Stabbing/Slashing Sound:"         },
    { itype_whimsicalring, (char *)"Damage Bonus:", (char *)"Chance (1 in n):",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Whimsy Sound:"                    },
    { itype_perilring, (char *)"Damage Divisor:", (char *)"Maximum Hearts:",                   NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_stompboots, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_bow, (char *)"Arrow Speed:",                    NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { -1,                                NULL,                                      NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       }
};

static std::map<int, ItemNameInfo *> *inamemap = NULL;

std::map<int, ItemNameInfo *> *getItemNameMap()
{
    if(inamemap == NULL)
	    
    {
        inamemap = new std::map<int, ItemNameInfo *>();
        
        for(int i=0;; i++)
        {
            ItemNameInfo *inf = &inameinf[i];
            
            if(inf->iclass == -1)
                break;
                
            (*inamemap)[inf->iclass] = inf;
        }
    }
    
    return inamemap;
}

static int itemdata_flags_list[] =
{
    // dialog control number
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, -1
};

 
static int itemdata_gfx_list[] =
{
    // dialog control number
    57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, -1
};

static int itemdata_pickup_list[] =
{
    // dialog control number
    92, 93, 94, 95, 96, 97, 98, 99, 100, /*101, 102,*/ 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, -1
};

static int itemdata_script_list[] =
{
    // dialog control number
    /*131, 132,*/ 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, -1
};

static int itemdata_scriptargs_list[] =
{
    // dialog control number
    101, 102, 131, 132, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, -1
};

static int itemdata_weaponargs_list[] =
{
    // dialog control number
    199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,216,217,218,219,220, -1
};

/*
static int itemdata_movementpattern_list[] =
{
    // dialog control number
    101, 102, 131, 132, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, -1
};
*/


static TABPANEL itemdata_tabs[] =
{
    // (text)
    { (char *)"Data",         D_SELECTED,    itemdata_flags_list,         0, NULL },
    { (char *)"GFX",          0,             itemdata_gfx_list,           0, NULL },
    { (char *)"Pickup",       0,             itemdata_pickup_list,        0, NULL },
    { (char *)"Action",       0,             itemdata_script_list,        0, NULL },
    { (char *)"Scripts",      0,             itemdata_scriptargs_list,    0, NULL },
    { (char *)"Weapon",      0,             itemdata_weaponargs_list,    0, NULL },
    { NULL,                   0,             NULL,                        0, NULL }
};

static ListData item_class__list(item_class_list, &pfont);

static char itemdata_weaponlist_str_buf[14];

const char *itemdata_weaponlist(int index, int *list_size)
{
    if(index >= 0)
    {
        bound(index,0,40);
        
        switch(index)
        {

	case 0: return "None";
	case 1: return "Sword"; 
        case 2: return "Sword Beam";
        case 3: return "Boomerang";
        case 4: return "Bomb Blast";
        case 5: return "S.Bomb Blast";
        case 6: return "Bomb";
        case 7: return "Super Bomb";
	case 8: return "Arrow";
	case 9: return "Fire";
	case 10: return "Whistle";
	case 11: return "Bait";
	case 12: return "Wand";
	case 13: return "Magic";
	case 14: return "Candle";
	case 15: return "Wind";	
	case 16: return "Ref. Magic";
	case 17: return "Ref. Fireball";
	case 18: return "Ref. Rock";
	case 19: return "Hammer";
	case 20: return "Hookshot";
	case 21: return "Hookshot Handle";
	case 22: return "Hookshot Chain";
	case 23: return "Sparkle";
	case 24: return "Fire Sparkle";
	case 25: return "wSmack";
	case 26: return "wPhantom";
	case 27: return "Cane of Byrna";
	case 28: return "Ref. Beam";
	case 29: return "wStomp";
	case 30: return "n/a"; //lwMAX 2.50
	case 32: return "Script 2";
	case 33: return "Script 3";
	case 34: return "Script 4";
	case 35: return "Script 5";
	case 36: return "Script 6";
	case 37: return "Script 7";
	case 38: return "Script 8";
	case 39: return "Script 9";
	case 40: return "Script 10";
	
	//! planned additions. -Z
	//case 41: return "Ice";
	//case 42: return "Sonic"; //Digdogger split def. -Z
	//case 43: return "Rock"; //thrown rock
	//case 44: return "Pot"; //thrown pot
	//case 45: return "Electricity";
	//case 46: return "Z3 Sword";
	//case 47: return "GB Sword";
	//case 48: return "Spin Attack";
	//case 49: return "Shield Bash";
	//case 50: return "Shovel";

	
        default: return "None";
          //  sprintf(counterlist_str_buf,"Script %d",index-7);
          //  return counterlist_str_buf;
        }
    }
    
    *list_size = 41;
    return NULL;
}

static ListData itemdata_weapon_list(itemdata_weaponlist, &pfont);

static char itemdata_weapontypelist_str_buf[14];

const char *itemdata_weapontypelist(int index, int *list_size)
{
    if(index >= 0)
    {
        bound(index,0,40);
        
        switch(index)
        {
	case 0: return "None";
	case 1: return "Sword"; 
        case 2: return "Sword Beam";
        case 3: return "Boomerang";
        case 4: return "Bomb Blast";
        case 5: return "S.Bomb Blast";
        case 6: return "Bomb";
        case 7: return "Super Bomb";
	case 8: return "Arrow";
	case 9: return "Fire";
	case 10: return "Whistle";
	case 11: return "Bait";
	case 12: return "Wand";
	case 13: return "Magic";
	case 14: return "Candle";
	case 15: return "Wind";	
	case 16: return "Ref. Magic";
	case 17: return "Ref. Fireball";
	case 18: return "Ref. Rock";
	case 19: return "Hammer";
	case 20: return "Hookshot";
	case 21: return "Hookshot Handle";
	case 22: return "Hookshot Chain";
	case 23: return "Sparkle";
	case 24: return "Fire Sparkle";
	case 25: return "wSmack";
	case 26: return "wPhantom";
	case 27: return "Cane of Byrna";
	case 28: return "Ref. Beam";
	case 29: return "wStomp";
	case 30: return "n/a"; //lwMAX 2.50
	case 32: return "Script 2";
	case 33: return "Script 3";
	case 34: return "Script 4";
	case 35: return "Script 5";
	case 36: return "Script 6";
	case 37: return "Script 7";
	case 38: return "Script 8";
	case 39: return "Script 9";
	case 40: return "Script 10";
	
	//! planned additions. -Z
	//case 41: return "Ice";
	//case 42: return "Sonic"; //Digdogger split def. -Z
	//case 43: return "Rock"; //thrown rock
	//case 44: return "Pot"; //thrown pot
	//case 45: return "Electricity";
	//case 46: return "Z3 Sword";
	//case 47: return "GB Sword";
	//case 48: return "Spin Attack";
	//case 49: return "Shield Bash";
	//case 50: return "Shovel";

	
        default: return "None";
          //  sprintf(counterlist_str_buf,"Script %d",index-7);
          //  return counterlist_str_buf;
        }
    }
    
    *list_size = 41;
    return NULL;
}

static ListData itemdata_weapon_type_list(itemdata_weapontypelist, &pfont);

static char weapon_pattern_list_buf[10];

const char *weapon_patternlist(int index, int *list_size)
{
    if(index >= 0)
    {
        bound(index,0,32);
        
        switch(index)
        {
        case 0: return "None";
	case 1: return "Line";
	case 2: return "Sine Wave";
        case 3: return "Cosine";
        case 4: return "Circular";
	case 5: return "Arc";
	case 6: return "Pattern A";
	case 7: return "Pattern B";
	case 8: return "Pattern C";
	case 9: return "Pattern D";
	case 10: return "Pattern E";
	case 11: return "Pattern F";
	default: return "None";
	
        }
    }
    
    *list_size = 12;
    return NULL;
}

static ListData weapon_pattern_llist(weapon_patternlist, &pfont);

static char counterlist_str_buf[12];

const char *counterlist(int index, int *list_size)
{
    if(index >= 0)
    {
        bound(index,0,32);
        
        switch(index)
        {
        case 0:
            return "None";
            
        case 1:
            return "Life";
            
        case 2:
            return "Rupees";
            
        case 3:
            return "Bombs";
            
        case 4:
            return "Arrows";
            
        case 5:
            return "Magic";
            
        case 6:
            return "Keys";
            
        case 7:
            return "Super Bombs";
            
        default:
            sprintf(counterlist_str_buf,"Script %d",index-7);
            return counterlist_str_buf;
        }
    }
    
    *list_size = 33;
    return NULL;
}

static ListData counter_list(counterlist, &pfont);

//Moved defenselist up here so that it is also available to itemdata. -Z

const char *defenselist(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,edLAST-1);
        
        switch(index)
        {
        default:
            return "(None)";
            
        case edHALFDAMAGE:
            return "1/2 Damage";
            
        case edQUARTDAMAGE:
            return "1/4 Damage";
	
            
        case edSTUNONLY:
            return "Stun";
            
        case edSTUNORCHINK:
            return "Stun Or Block";
            
        case edSTUNORIGNORE:
            return "Stun Or Ignore";
            
        case edCHINKL1:
            return "Block If < 1";
            
        case edCHINKL2:
            return "Block If < 2";
            
        case edCHINKL4:
            return "Block If < 4";
            
        case edCHINKL6:
            return "Block If < 6";
            
        case edCHINKL8:
            return "Block If < 8";
	
	
            
        case edCHINK:
            return "Block";
            
        case edIGNOREL1:
            return "Ignore If < 1";
            
        case edIGNORE:
            return "Ignore";
	
	    
	
            
        case ed1HKO:
            return "One-Hit-Kill";
	
	case edCHINKL10: //If damage is less than 10
		return "Block if Power < 10";
	
	case ed2x: //Double damage
		return "Double Damage";
	case ed3x: //Triple Damage
		return "Triple Damage";
	case ed4x: //4x damage
		return "Quadruple Damage";
	
	case edHEAL: //recover the weapon damage in HP
		return "Enemy Gains HP = Damage";
	
	case edTRIGGERSECRETS: //Triggers screen secrets. 
		return "Trigger Screen Secrets";
	
	
        }
    }
    
    *list_size = edLAST;
    return NULL;
}

static ListData defense_list(defenselist, &font);

int jwin_nbutton_proc(int msg, DIALOG *d, int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    d=d;
    
    FONT *tfont=font;
    
    switch(msg)
    {
    case MSG_DRAW:
        tfont=font;
        font=is_large()?lfont_l:nfont;
        jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, true);
        font=tfont;
        return D_O_K;
        break;
    }
    
    return jwin_button_proc(msg, d, c);
}

const char *itemscriptdroplist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = biitems_cnt;
        return NULL;
    }
    
    return biitems[index].first.c_str();
}

//droplist like the dialog proc, naming scheme for this stuff is awful...
static ListData itemscript_list(itemscriptdroplist, &pfont);

static DIALOG itemdata_dlg[] =
{
    // (dialog proc)           (x)     (y)     (w)     (h)        (fg)                   (bg)                   (key)       (flags)     (d1)     (d2)    (dp)                                            (dp2)               (dp3)
    { jwin_win_proc,            0,      0,    320,    237,    vc(14),                 vc(1),                   0,       D_EXIT,      0,    0,  NULL,                                           NULL,   NULL                  },
    { d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_tab_proc,            4,     25,    312,    182,    0,                      0,                       0,       0,           0,    0, (void *) itemdata_tabs,                         NULL, (void *)itemdata_dlg  },
    { jwin_nbutton_proc,       58,    212,     61,     21,    vc(14),                 vc(1),                  13,       D_EXIT,      0,    0, (void *) "OK",                                  NULL,   NULL                  },
    { jwin_nbutton_proc,      129,    212,     61,     21,    vc(14),                 vc(1),                  27,       D_EXIT,      0,    0, (void *) "Cancel",                              NULL,   NULL                  },
    { jwin_nbutton_proc,      200,    212,     61,     21,    vc(14),                 vc(1),                   0,       D_EXIT,      0,    0, (void *) "Defaults",                            NULL,   NULL                  },
    
    // 6
    { jwin_text_proc,           8,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Name:",                               NULL,   NULL                  },
    { jwin_edit_proc,          55,     44,    150,     16,    vc(12),                 vc(1),                   0,       0,          63,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_text_proc,           8,     66,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Class:",                              NULL,   NULL                  },
    { jwin_droplist_proc,      55,     62,    150,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      3,    0, (void *) &item_class__list,                       NULL,   NULL                  },
    
    //10
    { jwin_text_proc,         236,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Level:",                              NULL,   NULL                  },
    { jwin_edit_proc,         283,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_text_proc,           8,     84,     60,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Power:",                              NULL,   NULL                  },
    { jwin_edit_proc,          87,     80,     28,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //14
    { jwin_check_proc,        120,     80,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Equipment Item",                      NULL,   NULL                  },
    { jwin_check_proc,        120,     90,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 1",                        NULL,   NULL                  },
    { jwin_check_proc,        120,    100,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 2",                        NULL,   NULL                  },
    { jwin_check_proc,        215,     80,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 3",                        NULL,   NULL                  },
    { jwin_check_proc,        215,     90,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 4",                        NULL,   NULL                  },
    { jwin_check_proc,        215,    100,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 5",                        NULL,   NULL                  },
    
    //20
    { jwin_text_proc,           8,    115,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 1:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    111,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    133,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 2:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    129,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    151,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 3:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    147,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    169,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 4:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    165,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    187,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 5:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    183,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //30
    { jwin_text_proc,         177,    115,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 6:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    111,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    133,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 7:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    129,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    151,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 8:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    147,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    169,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 9:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    165,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    187,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 10:",                 NULL,   NULL                  },
    { jwin_edit_proc,         276,    183,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //40
    { jwin_button_proc,        205,    65,     12,     12,   vc(14),					vc(1),					 0,      D_EXIT,       0,    0, (void *) "?",								     NULL,	 NULL				  },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //47
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //57
    { jwin_text_proc,           8,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Flash CSet:",                         NULL,   NULL                  },
    { jwin_edit_proc,         107,     44,     35,     16,    vc(12),                 vc(1),                   0,       0,           2,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     66,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Animation Frames:",                   NULL,   NULL                  },
    { jwin_edit_proc,         107,     62,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     84,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Animation Speed:",                    NULL,   NULL                  },
    { jwin_edit_proc,         107,     80,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    102,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Initial Delay:",                      NULL,   NULL                  },
    { jwin_edit_proc,         107,     98,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    120,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Link Tile Modification:",             NULL,   NULL                  },
    { jwin_edit_proc,         107,    116,     35,     16,    vc(12),                 vc(1),                   0,       0,           6,    0,  NULL,                                           NULL,   NULL                  },
    
    //67
    { d_dummy_proc,          8,    136,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) NULL,       NULL,   NULL                  },
    
    { d_cstile_proc,          145,     44,     20,     20,    vc(11),                 vc(1),                   0,       0,           0,    6,  NULL,                                           NULL,   NULL                  },
    { jwin_check_proc,        145,     69,     65,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flash ",                              NULL,   NULL                  },
    { jwin_check_proc,        145,     80,     65,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "2-Hand",                              NULL,   NULL                  },
    
    { jwin_button_proc,       149,    100,     53,     21,    vc(14),                 vc(1),                   't',     D_EXIT,      0,    0, (void *) "&Test",                               NULL,   NULL                  },
    
    //72
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //82
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //92
    { jwin_text_proc,           8,     66,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Increase Amount:",                    NULL,   NULL                  },
    { jwin_edit_proc,         107,     62,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_check_proc,        147,     66,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Gradual",                       NULL,   NULL                  },
    
    { jwin_text_proc,           8,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Counter Reference:",                  NULL,   NULL                  },
    { jwin_droplist_proc,     107,     44,     72,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &counter_list,						 NULL,   NULL 				   },
    { jwin_text_proc,         147,     84,     60,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "But Not Above:",                           NULL,   NULL                  },
    { jwin_edit_proc,         204,     80,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     84,     60,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Increase Counter Max:",                               NULL,   NULL                  },
    { jwin_edit_proc,         107,     80,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //101
    { jwin_text_proc,          112+10,    47+38,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Pickup Script:",                            NULL,   NULL                  },
    { jwin_droplist_proc,      112+10,    47+38+10,     150,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &itemscript_list,                   NULL,   NULL 				   },
    //{ jwin_edit_proc,          55,     98,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,          8,    102,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sound:",                              NULL,   NULL                  },
    { jwin_edit_proc,         107,     98,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_text_proc,           8,    120,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Hearts Required:",                    NULL,   NULL                  },
    { jwin_edit_proc,         107,    116,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_check_proc,          8,    142,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Keep Lower Level Items",              NULL,   NULL                  },
    { jwin_check_proc,          8,    152,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Gain All Lower Level Items",          NULL,   NULL                  },
    { jwin_check_proc,          8,    162,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Can Be Eaten By Enemies",             NULL,   NULL                  },
    { jwin_check_proc,          8,    172,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Upgrade Item When Collected Twice",   NULL,   NULL                  },
    
    //111
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //121
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //131
    { jwin_text_proc,           112+10,  47+38+10 + 18,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Action Script:",                      NULL,   NULL                  },
    { jwin_droplist_proc,       112+10,  47+38+10*2 + 18,     150,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &itemscript_list,                   NULL,   NULL 				   },
    //{ jwin_edit_proc,          55,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     48,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Magic Cost:",                              NULL,   NULL                  },
    { jwin_edit_proc,         125,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         161,     48,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sound:",                              NULL,   NULL                  },
    { jwin_edit_proc,         284,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_check_proc,          8,     62,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Remove Item When Used",               NULL,   NULL                  },
    { jwin_check_proc,        161,     62,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Uses Rupees Instead Of Magic",        NULL,   NULL                  },
    
    //139
    { jwin_text_proc,           8,     74,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[0]:",                           NULL,   NULL                  },
    { jwin_button_proc,         8,     83,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,           8,    100,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[1]:",                           NULL,   NULL                  },
    { jwin_button_proc,         8,    109,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,           8,    126,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[2]:",                           NULL,   NULL                  },
    { jwin_button_proc,         8,    135,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,           8,    152,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[3]:",                           NULL,   NULL                  },
    { jwin_button_proc,         8,    161,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,           8,    178,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[4]:",                           NULL,   NULL                  },
    { jwin_button_proc,         8,    187,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    
    //149
    { jwin_text_proc,         161,     74,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[5]:",                           NULL,   NULL                  },
    { jwin_button_proc,       161,     83,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,         161,    100,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[6]:",                           NULL,   NULL                  },
    { jwin_button_proc,       161,    109,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,         161,    126,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[7]:",                           NULL,   NULL                  },
    { jwin_button_proc,       161,    135,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,         161,    152,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[8]:",                           NULL,   NULL                  },
    { jwin_button_proc,       161,    161,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    { jwin_text_proc,         161,    178,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sprites[9]:",                           NULL,   NULL                  },
    { jwin_button_proc,       161,    187,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      0,    0, NULL,                                             NULL,   NULL                  },
    
    //159
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //169
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //179
    { jwin_text_proc,       6+10,   29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D0:", NULL, NULL },
    { jwin_text_proc,       6+10,   47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D1:", NULL, NULL },
    { jwin_text_proc,       6+10,   65+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D2:", NULL, NULL },
    { jwin_text_proc,       6+10,   83+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D3:", NULL, NULL },
    { jwin_text_proc,       6+10,  101+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D4:", NULL, NULL },
    { jwin_text_proc,       6+10,  119+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D5:", NULL, NULL },
    { jwin_text_proc,       6+10,  137+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D6:", NULL, NULL },
    { jwin_text_proc,       6+10,  155+20,   24,    12,   0,        0,       0,       0,          0,             0, (void *) "D7:", NULL, NULL },
    { jwin_edit_proc,      34+10,   25+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   43+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   61+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   79+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   97+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  115+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  133+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  151+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    //195
    { jwin_text_proc,       112+10,  29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "A1:", NULL, NULL },
    { jwin_text_proc,       112+10,  47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "A2:", NULL, NULL },
    //197
    { jwin_edit_proc,      140+10,  25+20,   32,    16,   vc(12),   vc(1),   0,       0,          2,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      140+10,  43+20,   32,    16,   vc(12),   vc(1),   0,       0,          2,             0,       NULL, NULL, NULL },
    //199
    { jwin_text_proc,           8,     50,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon Type",                  NULL,   NULL                  },
    { jwin_droplist_proc,     107,     48,     72,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &itemdata_weapon_list,						 NULL,   NULL 				   },
    //201
    { jwin_text_proc,           8,     70,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Default Defense",                  NULL,   NULL                  },
    { jwin_droplist_proc,     107,     68,     72,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &defense_list,						 NULL,   NULL 				   },
    //203
    { jwin_text_proc,           8,     90,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Movement Pattern",                  NULL,   NULL                  },
    { jwin_droplist_proc,     107,     88,     72,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &weapon_pattern_llist,						 NULL,   NULL 				   },
    //205
    { jwin_text_proc,           8,     110,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Movement Arg 1:",                              NULL,   NULL                  },
    { jwin_edit_proc,         107,     108,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //207
    { jwin_text_proc,           8,     130,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Movement Arg 2:",                              NULL,   NULL                  },
    { jwin_edit_proc,         107,     128,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //209
    { jwin_text_proc,           8,     150,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon Range:",                              NULL,   NULL                  },
    { jwin_edit_proc,         107,     148,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //211
    { jwin_text_proc,           8,     170,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon Duration:",                              NULL,   NULL                  },
    { jwin_edit_proc,         107,     168,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //213
     { jwin_text_proc,           177,     110,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Movement Arg 3:",                              NULL,   NULL                  },
    { jwin_edit_proc,         276,     108,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //215
    { jwin_text_proc,           177,     130,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Movement Arg 4:",                              NULL,   NULL                  },
    { jwin_edit_proc,         276,     128,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //217
    { jwin_text_proc,           177,     150,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Other 1:",                              NULL,   NULL                  },
    { jwin_edit_proc,         276,     148,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    //219
    { jwin_text_proc,           177,     170,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Other 2:",                              NULL,   NULL                  },
    { jwin_edit_proc,         276,     168,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    /*
    { jwin_text_proc,       6+10,   29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[0]:", NULL, NULL },
    { jwin_text_proc,       6+10,   47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[1]:", NULL, NULL },
    { jwin_text_proc,       6+10,   65+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[2]:", NULL, NULL },
    { jwin_text_proc,       6+10,   83+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[3]:", NULL, NULL },
    { jwin_text_proc,       6+10,  101+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[4]:", NULL, NULL },
    { jwin_text_proc,       6+10,  119+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[5]:", NULL, NULL },
    { jwin_text_proc,       6+10,  137+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "Misc[6]:", NULL, NULL },
    { jwin_text_proc,       6+10,  155+20,   24,    12,   0,        0,       0,       0,          0,             0, (void *) "Misc[7]:", NULL, NULL },
    { jwin_edit_proc,      34+10,   25+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   43+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   61+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   79+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   97+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  115+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  133+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  151+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    */
    { NULL,                     0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                  },
};

void setLabels(int iclass, DIALOG *d)
{
    std::map<int, ItemNameInfo *> *nmap = getItemNameMap();
    std::map<int, ItemNameInfo *>::iterator it = nmap->find(iclass);
    ItemNameInfo *inf = NULL;
    
    if(it != nmap->end())
        inf = it->second;
        
        
    if(inf == NULL)
        d[12].dp = (void *)"Power:";
    else if(inf->power == NULL)
    {
        d[12].dp = (void *)"<Unused>";
        d[13].flags &= ~D_DISABLED;
    }
    else
    {
        d[12].dp = inf->power;
        d[13].flags &= ~D_DISABLED;
    }
    
    // Disable the Equipment item checkbox
    if((iclass >= itype_rupee && iclass <= itype_bombammo) ||
            (iclass >= itype_key && iclass <= itype_magiccontainer) ||
            (iclass >= itype_map && iclass <= itype_bosskey) || iclass == itype_clock ||
            iclass==itype_lkey || iclass==itype_misc ||
            iclass==itype_bowandarrow || iclass==itype_letterpotion)
    {
        d[14].dp = (void *)"<Unused>";
        d[14].flags &= ~D_DISABLED;
    }
    else
    {
        d[14].dp = (void *)"Equipment Item";
        d[14].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag1 == NULL))
    {
        d[15].dp = (void *)"Flags[0]";
        d[15].flags &= ~D_DISABLED;
    }
    else
    {
        d[15].dp = inf->flag1;
        d[15].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag2 == NULL))
    {
        d[16].dp = (void *)"Flags[1]";
        d[16].flags &= ~D_DISABLED;
    }
    else
    {
        d[16].dp = inf->flag2;
        d[16].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag3 == NULL))
    {
        d[17].dp = (void *)"Flags[2]";
        d[17].flags &= ~D_DISABLED;
    }
    else
    {
        d[17].dp = inf->flag3;
        d[17].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag4 == NULL))
    {
        d[18].dp = (void *)"Flags[3]";
        d[18].flags &= ~D_DISABLED;
    }
    else
    {
        d[18].dp = inf->flag4;
        d[18].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag5 == NULL))
    {
        d[19].dp = (void *)"Flags[4]";
        d[19].flags &= ~D_DISABLED;
    }
    else
    {
        d[19].dp = inf->flag5;
        d[19].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc1 == NULL))
    {
        d[20].dp = (void *)"Attributes[0]";
        d[21].flags &= ~D_DISABLED;
    }
    else
    {
        d[20].dp = inf->misc1;
        d[21].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc2 == NULL))
    {
        d[22].dp = (void *)"Attributes[1]";
        d[23].flags &= ~D_DISABLED;
    }
    else
    {
        d[22].dp = inf->misc2;
        d[23].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc3 == NULL))
    {
        d[24].dp = (void *)"Attributes[2]";
        d[25].flags &= ~D_DISABLED;
    }
    else
    {
        d[24].dp = inf->misc3;
        d[25].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc4 == NULL))
    {
        d[26].dp = (void *)"Attributes[3]";
        d[27].flags &= ~D_DISABLED;
    }
    else
    {
        d[26].dp = inf->misc4;
        d[27].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc5 == NULL))
    {
        d[28].dp = (void *)"Attributes[4]";
        d[29].flags &= ~D_DISABLED;
    }
    else
    {
        d[28].dp = inf->misc5;
        d[29].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc6 == NULL))
    {
        d[30].dp = (void *)"Attributes[5]";
        d[31].flags &= ~D_DISABLED;
    }
    else
    {
        d[30].dp = inf->misc6;
        d[31].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc7 == NULL))
    {
        d[32].dp = (void *)"Attributes[6]";
        d[33].flags &= ~D_DISABLED;
    }
    else
    {
        d[32].dp = inf->misc7;
        d[33].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc8 == NULL))
    {
        d[34].dp = (void *)"Attributes[7]";
        d[35].flags &= ~D_DISABLED;
    }
    else
    {
        d[34].dp = inf->misc8;
        d[35].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc9 == NULL))
    {
        d[36].dp = (void *)"Attributes[8]";
        d[37].flags &= ~D_DISABLED;
    }
    else
    {
        d[36].dp = inf->misc9;
        d[37].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc10 == NULL))
    {
        d[38].dp = (void *)"Attributes[9]";
        d[39].flags &= ~D_DISABLED;
    }
    else
    {
        d[38].dp = inf->misc10;
        d[39].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->actionsnd == NULL))
        d[135].dp = (void *)"UseSound";
    else
        d[135].dp = inf->actionsnd;
        
    if((inf == NULL) || (inf->wpn1 == NULL))
    {
        d[139].dp = (void *)"Sprites[0]";
        d[140].flags &= ~D_DISABLED;
    }
    else
    {
        d[139].dp = inf->wpn1;
        d[140].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn2 == NULL))
    {
        d[141].dp = (void *)"Sprites[1]";
        d[142].flags &= ~D_DISABLED;
    }
    else
    {
        d[141].dp = inf->wpn2;
        d[142].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn3 == NULL))
    {
        d[143].dp = (void *)"Sprites[2]";
        d[144].flags &= ~D_DISABLED;
    }
    else
    {
        d[143].dp = inf->wpn3;
        d[144].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn4 == NULL))
    {
        d[145].dp = (void *)"Sprites[3]";
        d[146].flags &= ~D_DISABLED;
    }
    else
    {
        d[145].dp = inf->wpn4;
        d[146].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn5 == NULL))
    {
        d[147].dp = (void *)"Sprites[4]";
        d[148].flags &= ~D_DISABLED;
    }
    else
    {
        d[147].dp = inf->wpn5;
        d[148].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn6 == NULL))
    {
        d[149].dp = (void *)"Sprites[5]";
        d[150].flags &= ~D_DISABLED;
    }
    else
    {
        d[149].dp = inf->wpn6;
        d[150].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn7 == NULL))
    {
        d[151].dp = (void *)"Sprites[6]";
        d[152].flags &= ~D_DISABLED;
    }
    else
    {
        d[151].dp = inf->wpn7;
        d[152].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn8 == NULL))
    {
        d[153].dp = (void *)"Sprites[7]";
        d[154].flags &= ~D_DISABLED;
    }
    else
    {
        d[153].dp = inf->wpn8;
        d[154].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn9 == NULL))
    {
        d[155].dp = (void *)"Sprites[8]";
        d[156].flags &= ~D_DISABLED;
    }
    else
    {
        d[155].dp = inf->wpn9;
        d[156].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn10 == NULL))
    {
        d[157].dp = (void *)"Sprites[9]";
        d[158].flags &= ~D_DISABLED;
    }
    else
    {
        d[157].dp = inf->wpn10;
        d[158].flags &= ~D_DISABLED;
    }
    /*
    //!! New itemdata values. -Z
    char wrange[8], wdur[8], wdef[8], wweap[8], wptrn[8], warg1[8], warg2[8];
    sprintf(wrange,"%d",inf->weaprange);
    sprintf(wdur,"%d",inf->weapduration);
    sprintf(wdef,"%d",inf->usedefence);
    sprintf(wweap,"%d",inf->useweapon);
    sprintf(wptrn,"%d",inf->weap_pattern[0]);
    sprintf(warg1,"%d",inf->weap_pattern[1]);
    sprintf(warg2,"%d",inf->weap_pattern[2]);
    
    
   //!  This is crashing ZQuest. Why? What is the proper way to populate these fields from the item buffer? -Z
    itemdata_dlg[200].dp = wweap; //atoi(inf->useweapon);
    itemdata_dlg[202].dp = wdef; //atoi(inf->usedefence);
    itemdata_dlg[204].dp = wptrn; //atoi(inf->weap_pattern[0]);
    itemdata_dlg[206].dp = warg1; //atoi(inf->weap_pattern[1]);
    itemdata_dlg[208].dp = warg2; //atoi(inf->weap_pattern[2]);
    itemdata_dlg[210].dp = wrange; // atoi(inf->weaprange);
    itemdata_dlg[212].dp = wdur; //atoi(inf->weapduration);
    */
    
    /* Also crashing...
    itemdata_dlg[200].d1 = atoi(inf->useweapon);
    itemdata_dlg[202].d1 = atoi(inf->usedefence);
    itemdata_dlg[204].d1 = atoi(inf->weap_pattern[0]);
    itemdata_dlg[206].d1 = atoi(inf->weap_pattern[1]);
    itemdata_dlg[208].d1 = atoi(inf->weap_pattern[2]);
    itemdata_dlg[210].d1 = atoi(inf->weaprange);
    itemdata_dlg[212].d1 = atoi(inf->weapduration);
    */
}	



void itemdata_help(int id)
{
    if(id < 0 || id > itype_max) return;
    
    if(id >= itype_custom1 && id <= itype_custom20)
    {
        jwin_alert(itype_names[id],"This has no built-in effect, but can be given","special significance using ZScripts or ZASM.","","O&K",NULL,'k',0,lfont);
    }
    else if(id >= itype_last)
    {
        jwin_alert("Undefined Item Class","This type currently has no use. Do not","use it in your quests, as its purpose may change","in future versions of Zelda Classic.","O&K",NULL,'k',0,lfont);
    }
    else if(id<itype_custom1)
        jwin_alert(itype_names[id],itemclass_help_string[id*3],itemclass_help_string[1+(id*3)],itemclass_help_string[2+(id*3)],"O&K",NULL,'k',0,lfont);
    else // Beyond itype_custom20...
        jwin_alert(itype_names[id],itemclass_help_string[(id-20)*3],itemclass_help_string[1+((id-20)*3)],itemclass_help_string[2+((id-20)*3)],"O&K",NULL,'k',0,lfont);
}

void test_item(itemdata test, int x, int y)
{
    //TODO FIX THIS SUPER DIRTY HACK!!
    //itemdata *hold = itemsbuf;
    //itemsbuf = &test;
    Quest *oldquest = curQuest;
    Quest tmp;
    curQuest = &tmp;
    tmp.getModule("TMP").itemDefTable().addItemDefinition(test);
    BITMAP *buf = create_bitmap_ex(8,16,16);
    BITMAP *buf2 = create_bitmap_ex(8,64,64);
    
    item temp((fix)0,(fix)0,(fix)0,ItemDefinitionRef("TMP",0),0,0);
    temp.yofs = 0;
    go();
    itemdata_dlg[0].flags=0;
    jwin_win_proc(MSG_DRAW, itemdata_dlg, 0);
    itemdata_dlg[0].flags=D_EXIT;
    //  d_ctext_proc(MSG_DRAW, itemdata_dlg+1, 0);
    
    frame=0;
//  jwin_draw_frame(screen,itemdata_dlg[0].x+(itemdata_dlg[0].w/2)-32,itemdata_dlg[0].y+(itemdata_dlg[0].h/2)-32, is_large?68:20,is_large?68:20,FR_DEEP);
    jwin_draw_frame(screen, x, y, is_large()?68:20, is_large() ?68:20,FR_DEEP);
    
    for(;;)
    {
    
        ++frame;
        clear_bitmap(buf);
        temp.animate(0);
        temp.draw(buf);
        
        if(is_large())
        {
            stretch_blit(buf,buf2,0,0,16,16,0,0,64,64);
//      blit(buf2,screen,0,0,64,64,itemdata_dlg[0].x+(itemdata_dlg[0].w/2)-30,itemdata_dlg[0].y+(itemdata_dlg[0].h/2)-30);
            blit(buf2,screen,0,0,x+2,y+2,64,64);
        }
        else
        {
            blit(buf,screen,0,0,x+2,y+2,16,16);
        }
        
		Backend::graphics->waitTick();
		Backend::graphics->showBackBuffer();
        
        //if(zqwin_scale > 1)
        {
            //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        //else
        {
            //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        
        if(keypressed())
        {
            readkey();
            break;
        }
        
        if(Backend::mouse->anyButtonClicked())
            break;
    }
    
    comeback();
    
    while(Backend::mouse->anyButtonClicked())
    {
		Backend::graphics->waitTick();
		Backend::graphics->showBackBuffer();
        /* do nothing */
    }
    
    destroy_bitmap(buf);
    destroy_bitmap(buf2);
    curQuest = oldquest;
}


void edit_itemdata(ItemDefinitionRef itemref)
{
    
    char frm[32], spd[32], fcs[32], dly[32], ltm[32];
    char /*cls[8],*/ cll[32], amt[32], fmx[32], max[32], asn[32];
    char snd[32], mgc[32], hrt[32], pow[32];
    char name[64];
    char ms1[32], ms2[32], ms3[32], ms4[32], ms5[32], ms6[32], ms7[32], ms8[32], ms9[32], ms10[32];
	char wrange[32], wdur[32], wdef[32], wweap[32], wptrn[32], warg1[32], warg2[32], warg3[32], warg4[32], warg5[32], warg6[32];
    char itemnumstr[75];
    char da[10][13];
    
    sprintf(itemnumstr,"Item %d: %s", itemref.slot, curQuest->getItemDefinition(itemref).name.c_str());
    sprintf(fcs,"%d",curQuest->getItemDefinition(itemref).csets>>4);
    sprintf(frm,"%d",curQuest->getItemDefinition(itemref).frames);
    sprintf(spd,"%d",curQuest->getItemDefinition(itemref).speed);
    sprintf(dly,"%d",curQuest->getItemDefinition(itemref).delay);
    sprintf(ltm,"%ld",curQuest->getItemDefinition(itemref).ltm);
    sprintf(cll,"%d",curQuest->getItemDefinition(itemref).fam_type);
    sprintf(amt,"%d",(curQuest->getItemDefinition(itemref).amount&0x4000)?(-(curQuest->getItemDefinition(itemref).amount&0x3FFF)):(curQuest->getItemDefinition(itemref).amount&0x3FFF));
    sprintf(fmx,"%d",curQuest->getItemDefinition(itemref).max);
    sprintf(max,"%d",curQuest->getItemDefinition(itemref).setmax);
    sprintf(snd,"%d",curQuest->getItemDefinition(itemref).playsound);
    sprintf(ms1,"%ld",curQuest->getItemDefinition(itemref).misc1);
    sprintf(ms2,"%ld",curQuest->getItemDefinition(itemref).misc2);
    sprintf(ms3,"%ld",curQuest->getItemDefinition(itemref).misc3);
    sprintf(ms4,"%ld",curQuest->getItemDefinition(itemref).misc4);
    sprintf(ms5,"%ld",curQuest->getItemDefinition(itemref).misc5);
    sprintf(ms6,"%ld",curQuest->getItemDefinition(itemref).misc6);
    sprintf(ms7,"%ld",curQuest->getItemDefinition(itemref).misc7);
    sprintf(ms8,"%ld",curQuest->getItemDefinition(itemref).misc8);
    sprintf(ms9,"%ld",curQuest->getItemDefinition(itemref).misc9);
    sprintf(ms10,"%ld",curQuest->getItemDefinition(itemref).misc10);
    sprintf(mgc,"%d",curQuest->getItemDefinition(itemref).magic);
    sprintf(hrt,"%d",curQuest->getItemDefinition(itemref).pickup_hearts);
    sprintf(pow,"%d",curQuest->getItemDefinition(itemref).power);
    sprintf(asn,"%d",curQuest->getItemDefinition(itemref).usesound);
    
    //New itemdata vars
    sprintf(wrange,"%d",curQuest->getItemDefinition(itemref).weaprange);
    sprintf(wdur,"%d",curQuest->getItemDefinition(itemref).weapduration);
    sprintf(wdef,"%d",curQuest->getItemDefinition(itemref).usedefence);
    sprintf(wweap,"%d",curQuest->getItemDefinition(itemref).useweapon);
    sprintf(wptrn,"%d",curQuest->getItemDefinition(itemref).weap_pattern[0]);
    sprintf(warg1,"%d",curQuest->getItemDefinition(itemref).weap_pattern[1]);
    sprintf(warg2,"%d",curQuest->getItemDefinition(itemref).weap_pattern[2]);
    sprintf(warg3,"%d",curQuest->getItemDefinition(itemref).weap_pattern[3]);
    sprintf(warg4,"%d",curQuest->getItemDefinition(itemref).weap_pattern[4]);
    sprintf(warg5,"%d",curQuest->getItemDefinition(itemref).weap_pattern[5]);
    sprintf(warg6,"%d",curQuest->getItemDefinition(itemref).weap_pattern[6]);
  
    
    snprintf(name, 63, "%s",curQuest->getItemDefinition(itemref).name.c_str());
    
    
    
    
    
    for(int j=0; j<8; j++)
        sprintf(da[j],"%.4f",curQuest->getItemDefinition(itemref).initiald[j]/10000.0);
        
    sprintf(da[8],"%d",curQuest->getItemDefinition(itemref).initiala[0]/10000);
    sprintf(da[9],"%d",curQuest->getItemDefinition(itemref).initiala[1]/10000);
    
    itemdata_dlg[0].dp = itemnumstr;
    itemdata_dlg[0].dp2 = lfont;
    
    if(biic_cnt==-1)
    {
        build_biic_list();
    }
    
    itemdata_dlg[7].dp = name;
    
    for(int j=0; j<biic_cnt; j++)
    {
        if(biic[j].i == curQuest->getItemDefinition(itemref).family)
            itemdata_dlg[9].d1 = j;
    }
    
    itemdata_dlg[11].dp = cll;
    itemdata_dlg[13].dp = pow;
    itemdata_dlg[14].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_GAMEDATA) ? D_SELECTED : 0;
    itemdata_dlg[15].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG1) ? D_SELECTED : 0;
    itemdata_dlg[16].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG2) ? D_SELECTED : 0;
    itemdata_dlg[17].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG3) ? D_SELECTED : 0;
    itemdata_dlg[18].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG4) ? D_SELECTED : 0;
    itemdata_dlg[19].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG5) ? D_SELECTED : 0;
    itemdata_dlg[21].dp = ms1;
    itemdata_dlg[23].dp = ms2;
    itemdata_dlg[25].dp = ms3;
    itemdata_dlg[27].dp = ms4;
    itemdata_dlg[29].dp = ms5;
    itemdata_dlg[31].dp = ms6;
    itemdata_dlg[33].dp = ms7;
    itemdata_dlg[35].dp = ms8;
    itemdata_dlg[37].dp = ms9;
    itemdata_dlg[39].dp = ms10;
    
    itemdata_dlg[58].dp = fcs;
    itemdata_dlg[60].dp = frm;
    itemdata_dlg[62].dp = spd;
    itemdata_dlg[64].dp = dly;
    itemdata_dlg[66].dp = ltm;
    itemdata_dlg[68].d1 = curQuest->getItemDefinition(itemref).tile;
    itemdata_dlg[68].d2 = curQuest->getItemDefinition(itemref).csets&15;
    itemdata_dlg[69].flags = (curQuest->getItemDefinition(itemref).misc&1) ? D_SELECTED : 0;
    itemdata_dlg[70].flags = (curQuest->getItemDefinition(itemref).misc&2) ? D_SELECTED : 0;
    
    itemdata_dlg[93].dp = amt;
    itemdata_dlg[94].flags = (curQuest->getItemDefinition(itemref).amount & 0x8000)  ? D_SELECTED : 0;
    itemdata_dlg[96].d1 = curQuest->getItemDefinition(itemref).count+1;
    itemdata_dlg[98].dp = fmx;
    itemdata_dlg[100].dp = max;
    itemdata_dlg[104].dp = snd;
    itemdata_dlg[106].dp = hrt;
    
    itemdata_dlg[107].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_KEEPOLD) ? D_SELECTED : 0;
    itemdata_dlg[108].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_GAINOLD) ? D_SELECTED : 0;
    itemdata_dlg[109].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_EDIBLE) ? D_SELECTED : 0;
    itemdata_dlg[110].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_COMBINE) ? D_SELECTED : 0;
    
    itemdata_dlg[134].dp = mgc;
    itemdata_dlg[136].dp = asn;
    itemdata_dlg[137].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_DOWNGRADE) ? D_SELECTED : 0;
    itemdata_dlg[138].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_RUPEE_MAGIC) ? D_SELECTED : 0;
    
    const char *nonestring = "(None)";

    for(int i=0; i<10; ++i)
    {
        itemdata_dlg[140+(i*2)].dp3 = is_large() ? lfont_l : pfont;
        itemdata_dlg[140 + (i * 2)].dp = (void *)(curQuest->isValid(curQuest->getItemDefinition(itemref).wpns[i]) ? curQuest->getSpriteDefinition(curQuest->getItemDefinition(itemref).wpns[i]).name.c_str() : nonestring);
    }
        
    for(int j=0; j<8; j++)
        itemdata_dlg[187+j].dp = da[j];
        
    itemdata_dlg[197].dp = da[8];
    itemdata_dlg[198].dp = da[9];
    
    build_biitems_list();
    int script = 0, pickupscript = 0;
    
    for(int j = 0; j < biitems_cnt; j++)
    {
        if(biitems[j].second == curQuest->getItemDefinition(itemref).script - 1)
            script = j;
            
        if(biitems[j].second == curQuest->getItemDefinition(itemref).collect_script - 1)
            pickupscript = j;
    }
    
    itemdata_dlg[102].d1 = pickupscript;
    itemdata_dlg[132].d1 = script;
    
    //This is causing ZQuest to crash. Why? -Z
    /*
    //strings or ints here? -Z
    itemdata_dlg[200].dp = wweap; //useweapon;
    itemdata_dlg[202].dp = wdef; //usedefence;
    itemdata_dlg[204].dp = wptrn; //weap_pattern[0];
    itemdata_dlg[206].dp = warg1; //weap_pattern[1];
    itemdata_dlg[208].dp = warg2; //weap_pattern[2];
    itemdata_dlg[210].dp = wrange; //weaprange;
    itemdata_dlg[212].dp = wdur; //weapduration;
    
    */
    //the original fields were crashing because they entered a switch stmt with no case 0 or default. 
    
    //! These now store in the editor, but if you change the values, save the quest, and reload, 
	//! ZQuest crashes on reading items (bad token)
    
    
    //These cannot be .dp. That crashes ZQuest; but they are not being retained when changed. -Z
     itemdata_dlg[200].d1 = curQuest->getItemDefinition(itemref).useweapon;
    itemdata_dlg[202].d1 = curQuest->getItemDefinition(itemref).usedefence;
    itemdata_dlg[204].d1 = curQuest->getItemDefinition(itemref).weap_pattern[0];
    
    //.dp is correcxt here, and these now work, and are retained. -Z
    itemdata_dlg[206].dp = warg1; //itemsbuf[index].weap_pattern[1];
    itemdata_dlg[208].dp = warg2; //itemsbuf[index].weap_pattern[2];
    itemdata_dlg[210].dp = wrange; //itemsbuf[index].weaprange;
    itemdata_dlg[212].dp = wdur; //itemsbuf[index].weapduration;
    
    itemdata_dlg[214].dp = warg3; //itemsbuf[index].weap_pattern[1];
    itemdata_dlg[216].dp = warg4; //itemsbuf[index].weap_pattern[2];
    itemdata_dlg[218].dp = warg5; //itemsbuf[index].weaprange;
    itemdata_dlg[220].dp = warg6; //itemsbuf[index].weapduration;
    
    
    int ret;
    itemdata test = curQuest->getItemDefinition(itemref);
    test.playsound = 25;
    
    setLabels(curQuest->getItemDefinition(itemref).family, itemdata_dlg);
    FONT *tfont=font;
    font=pfont;

	DIALOG *itemdata_cpy = resizeDialog(itemdata_dlg, 1.5);

            
    do
    {
        ret = zc_popup_dialog(itemdata_cpy, 3);
        test.misc = 0;
        test.flags = 0;

        test.fam_type = vbound(atoi(cll), 1, 255);
        test.count = itemdata_cpy[96].d1 - 1;
        test.amount = atoi(amt) < 0 ? -(vbound(atoi(amt), -0x3FFF, 0)) | 0x4000 : vbound(atoi(amt), 0, 0x3FFF);
        test.setmax = atoi(max);
        test.max = atoi(fmx);
        test.script = biitems[itemdata_cpy[132].d1].second + 1;
        test.playsound = vbound(atoi(snd), 0, 127);
        test.collect_script = biitems[itemdata_cpy[102].d1].second + 1;
        test.misc1 = atoi(ms1);
        test.misc2 = atoi(ms2);
        test.misc3 = atoi(ms3);
        test.misc4 = atoi(ms4);
        test.misc5 = atoi(ms5);
        test.misc6 = atoi(ms6);
        test.misc7 = atoi(ms7);
        test.misc8 = atoi(ms8);
        test.misc9 = atoi(ms9);
        test.misc10 = atoi(ms10);
        test.magic = vbound(atoi(mgc), 0, 255);
        test.pickup_hearts = vbound(atoi(hrt), 0, 255);
        test.power = vbound(atoi(pow), 0, 255);
        test.usesound = vbound(atoi(asn), 0, 127);

        test.family = vbound(biic[itemdata_cpy[9].d1].i, 0, 255);

        //New itemdata vars -Z

        //! These now store in the editor, but if you change the values, save the quest, and reload, 
        //! ZQuest crashes on reading items (bad token)
        test.usedefence = itemdata_cpy[202].d1; //atoi(wdef);
        test.weaprange = vbound(atoi(wrange), 0, 214747);
        test.weapduration = vbound(atoi(wdur), 0, 214747);
        test.useweapon = itemdata_cpy[200].d1;; //atoi(wweap);
        test.weap_pattern[0] = itemdata_cpy[204].d1;; //atoi(wptrn);
        test.weap_pattern[1] = vbound(atoi(warg1), -214747, 214747);
        test.weap_pattern[2] = vbound(atoi(warg2), -214747, 214747);

        test.weap_pattern[3] = vbound(atoi(warg3), -214747, 214747);
        test.weap_pattern[4] = vbound(atoi(warg4), -214747, 214747);
        test.weap_pattern[5] = vbound(atoi(warg5), -214747, 214747);
        test.weap_pattern[6] = vbound(atoi(warg6), -214747, 214747);

        if (itemdata_cpy[14].flags & D_SELECTED)
            test.flags |= itemdata::IF_GAMEDATA;

        if (itemdata_cpy[15].flags & D_SELECTED)
            test.flags |= itemdata::IF_FLAG1;

        if (itemdata_cpy[16].flags & D_SELECTED)
            test.flags |= itemdata::IF_FLAG2;

        if (itemdata_cpy[17].flags & D_SELECTED)
            test.flags |= itemdata::IF_FLAG3;

        if (itemdata_cpy[18].flags & D_SELECTED)
            test.flags |= itemdata::IF_FLAG4;

        if (itemdata_cpy[19].flags & D_SELECTED)
            test.flags |= itemdata::IF_FLAG5;

        test.tile = itemdata_cpy[68].d1;
        test.csets = itemdata_cpy[68].d2;

        //New itemdata -Z



        if (itemdata_cpy[69].flags & D_SELECTED)
            test.misc |= 1;

        if (itemdata_cpy[70].flags & D_SELECTED)
            test.misc |= 2;

        test.amount |= (itemdata_cpy[94].flags & D_SELECTED) ? 0x8000 : 0;

        if (itemdata_cpy[107].flags & D_SELECTED)
            test.flags |= itemdata::IF_KEEPOLD;

        if (itemdata_cpy[108].flags & D_SELECTED)
            test.flags |= itemdata::IF_GAINOLD;

        if (itemdata_cpy[109].flags & D_SELECTED)
            test.flags |= itemdata::IF_EDIBLE;

        if (itemdata_cpy[110].flags & D_SELECTED)
            test.flags |= itemdata::IF_COMBINE;

        if (itemdata_cpy[137].flags & D_SELECTED)
            test.flags |= itemdata::IF_DOWNGRADE;

        if (itemdata_cpy[138].flags & D_SELECTED)
            test.flags |= itemdata::IF_RUPEE_MAGIC;

        test.csets |= (atoi(fcs) & 15) << 4;
        test.frames = zc_min(atoi(frm), 255);
        test.speed = zc_min(atoi(spd), 255);
        test.delay = zc_min(atoi(dly), 255);
        test.ltm = zc_max(zc_min(atol(ltm), NEWMAXTILES - 1), 0 - (NEWMAXTILES - 1));


        for (int j = 0; j < 10; j++)
        {
            if (ret == 140 + 2 * j)
            {
                int status;
                SpriteDefinitionRef ref = select_sprite("Select Weapon Sprite", test.wpns[j], false, status);
                if (status == 4)
                {
                    test.wpns[j] = ref;
                    itemdata_cpy[140 + (j * 2)].dp = (void *)(curQuest->isValid(test.wpns[j]) ? curQuest->getSpriteDefinition(test.wpns[j]).name.c_str() : nonestring);
                }
            }
        }

        for (int j = 0; j < 8; j++)
            test.initiald[j] = vbound(ffparse(da[j]), -327680000, 327680000);

        test.initiala[0] = vbound(atoi(da[8]) * 10000, 0, 320000);
        test.initiala[1] = vbound(atoi(da[9]) * 10000, 0, 320000);

        if (ret == 40)
        {
            itemdata_help(test.family);
        }

        if (ret == 71)
        {
            test_item(test, itemdata_cpy[0].x + itemdata_cpy[0].w / 2 - (is_large() ? 34 : 10), itemdata_cpy[0].y + itemdata_cpy[0].h / 2 - (is_large() ? 34 : 10));
            sprintf(fcs, "%d", test.csets >> 4);
            sprintf(frm, "%d", test.frames);
            sprintf(spd, "%d", test.speed);
            sprintf(dly, "%d", test.delay);
            sprintf(ltm, "%ld", test.ltm);

            sprintf(cll, "%d", test.fam_type);
            sprintf(amt, "%d", test.amount & 0x4000 ? -(test.amount & 0x3FFF) : test.amount & 0x3FFF);
            sprintf(fmx, "%d", test.max);
            sprintf(max, "%d", test.setmax);
        }

        if (ret == 5)
        {
            //do not reset graphics fields! -DD
            //memset(&test, 0, sizeof(itemdata));
            //test.family=0xFF;

            reset_itembuf(&test, itemref);

            sprintf(fcs, "%d", test.csets >> 4);
            sprintf(frm, "%d", test.frames);
            sprintf(spd, "%d", test.speed);
            sprintf(dly, "%d", test.delay);
            sprintf(ltm, "%ld", test.ltm);
            sprintf(cll, "%d", test.fam_type);
            sprintf(amt, "%d", test.amount & 0x4000 ? -(test.amount & 0x3FFF) : test.amount & 0x3FFF);
            sprintf(fmx, "%d", test.max);
            sprintf(max, "%d", test.setmax);
            pickupscript = test.script;
            sprintf(snd, "%d", test.playsound);
            pickupscript = test.collect_script;
            sprintf(ms1, "%ld", test.misc1);
            sprintf(ms2, "%ld", test.misc2);
            sprintf(ms3, "%ld", test.misc3);
            sprintf(ms4, "%ld", test.misc4);
            sprintf(ms5, "%ld", test.misc5);
            sprintf(ms6, "%ld", test.misc6);
            sprintf(ms7, "%ld", test.misc7);
            sprintf(ms8, "%ld", test.misc8);
            sprintf(ms9, "%ld", test.misc9);
            sprintf(ms10, "%ld", test.misc10);
            sprintf(mgc, "%d", test.magic);
            sprintf(hrt, "%d", test.pickup_hearts);
            sprintf(pow, "%d", test.power);
            sprintf(asn, "%d", test.usesound);

            //New itemdata vars -Z

            sprintf(wrange, "%d", test.weaprange);
            sprintf(wdur, "%d", test.weapduration);
            sprintf(wdef, "%d", test.usedefence);
            sprintf(wweap, "%d", test.useweapon);
            sprintf(wptrn, "%d", test.weap_pattern[0]);
            sprintf(warg1, "%d", test.weap_pattern[1]);
            sprintf(warg2, "%d", test.weap_pattern[2]);

            sprintf(name, "%s", curQuest->getItemDefinition(itemref).name.c_str());

            for (int j = 0; j < 8; j++)
                sprintf(da[j], "%.4f", test.initiald[j] / 10000.0);

            sprintf(da[8], "%d", test.initiala[0] / 10000);
            sprintf(da[9], "%d", test.initiala[1] / 10000);
            sprintf(itemnumstr, "Item %d: %s", itemref.slot, name);

            itemdata_cpy[0].dp = itemnumstr;
            itemdata_cpy[0].dp2 = lfont;

            if (biic_cnt == -1)
            {
                build_biic_list();
            }

            itemdata_cpy[7].dp = name;

            for (int j = 0; j < biic_cnt; j++)
            {
                if (biic[j].i == test.family)
                    itemdata_cpy[9].d1 = j;
            }

            itemdata_cpy[11].dp = cll;
            itemdata_cpy[13].dp = pow;
            itemdata_cpy[14].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_GAMEDATA) ? D_SELECTED : 0;
            itemdata_cpy[15].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG1) ? D_SELECTED : 0;
            itemdata_cpy[16].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG2) ? D_SELECTED : 0;
            itemdata_cpy[17].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG3) ? D_SELECTED : 0;
            itemdata_cpy[18].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG4) ? D_SELECTED : 0;
            itemdata_cpy[19].flags = (curQuest->getItemDefinition(itemref).flags & itemdata::IF_FLAG5) ? D_SELECTED : 0;
            itemdata_cpy[21].dp = ms1;
            itemdata_cpy[23].dp = ms2;
            itemdata_cpy[25].dp = ms3;
            itemdata_cpy[27].dp = ms4;
            itemdata_cpy[29].dp = ms5;
            itemdata_cpy[31].dp = ms6;
            itemdata_cpy[33].dp = ms7;
            itemdata_cpy[35].dp = ms8;
            itemdata_cpy[37].dp = ms9;
            itemdata_cpy[39].dp = ms10;

            itemdata_cpy[58].dp = fcs;
            itemdata_cpy[60].dp = frm;
            itemdata_cpy[62].dp = spd;
            itemdata_cpy[64].dp = dly;
            itemdata_cpy[66].dp = ltm;
            itemdata_cpy[68].d1 = test.tile;
            itemdata_cpy[68].d2 = test.csets & 15;
            itemdata_cpy[69].flags = (test.misc & 1) ? D_SELECTED : 0;
            itemdata_cpy[70].flags = (test.misc & 2) ? D_SELECTED : 0;

            itemdata_cpy[93].dp = amt;
            itemdata_cpy[94].flags = (test.amount & 0x8000) ? D_SELECTED : 0;
            itemdata_cpy[96].d1 = curQuest->getItemDefinition(itemref).count + 1;
            itemdata_cpy[98].dp = fmx;
            itemdata_cpy[100].dp = max;
            itemdata_cpy[102].d1 = pickupscript;
            itemdata_cpy[104].dp = snd;
            itemdata_cpy[106].dp = hrt;

            itemdata_cpy[107].flags = (test.flags & itemdata::IF_KEEPOLD) ? D_SELECTED : 0;
            itemdata_cpy[108].flags = (test.flags & itemdata::IF_GAINOLD) ? D_SELECTED : 0;
            itemdata_cpy[109].flags = (test.flags & itemdata::IF_EDIBLE) ? D_SELECTED : 0;
            itemdata_cpy[110].flags = (test.flags & itemdata::IF_COMBINE) ? D_SELECTED : 0;

            itemdata_cpy[132].d1 = script;
            itemdata_cpy[134].dp = mgc;
            itemdata_cpy[136].dp = asn;
            itemdata_cpy[137].flags = (test.flags & itemdata::IF_DOWNGRADE) ? D_SELECTED : 0;
            itemdata_cpy[138].flags = (test.flags & itemdata::IF_RUPEE_MAGIC) ? D_SELECTED : 0;

            for (int i = 0; i < 10; ++i)
            {
                itemdata_cpy[140 + (i * 2)].dp = (void *)(curQuest->isValid(test.wpns[i]) ? curQuest->getSpriteDefinition(test.wpns[i]).name.c_str() : nonestring);
            }

            //! These now store in the editor, but if you change the values, save the quest, and reload, 
    //! ZQuest crashes on reading items (bad token)
            //string or int here? -Z
            itemdata_cpy[200].d1 = test.useweapon; //atoi(wweap);
            itemdata_cpy[202].d1 = test.usedefence; //atoi(wdef);
            itemdata_cpy[204].d1 = test.weap_pattern[0]; //atoi(wptrn);
            itemdata_cpy[206].dp = warg1; //test.weap_pattern[1]; //atoi(warg1);
            itemdata_cpy[208].dp = warg2; //test.weap_pattern[2]; //atoi(warg2);
            itemdata_cpy[210].dp = wrange; //test.weaprange; //atoi(wrange);
            itemdata_cpy[212].dp = wdur;// = test.weapduration; //atoi(wdur);
            itemdata_cpy[214].dp = warg3; //test.weap_pattern[1]; //atoi(warg1);
            itemdata_cpy[216].dp = warg4; //test.weap_pattern[2]; //atoi(warg2);
            itemdata_cpy[218].dp = warg5; //test.weaprange; //atoi(wrange);
            itemdata_cpy[220].dp = warg6;// = test.weapduration; //atoi(wdur);

            for (int j = 0; j < 8; j++)
                itemdata_cpy[187 + j].dp = da[j];

            itemdata_cpy[197].dp = da[8];
            itemdata_cpy[198].dp = da[9];

            setLabels(test.family, itemdata_cpy);
        }

        if (ret == 9)
            setLabels(test.family, itemdata_cpy);
    }
    while(ret != 0 && ret != 3 && ret != 4);
    
	delete[] itemdata_cpy;
    font=tfont;
    
    if(ret==3)
    {
        test.name = name;
        curQuest->getItemDefinition(itemref) = test;
        saved = false;
    }
    
}

int onCustomItems()
{
    int ret;
    ItemDefinitionRef index = select_item("Select Item",ItemDefinitionRef(),true,ret);
    
    
    while (curQuest->isValid(index))
    {
        
        edit_itemdata(index);
        index = select_item("Select Item", index, true, ret);
    }
    
    refresh(rMAP+rCOMBOS);
    return D_O_K;
}

/****************************/
/******  onCustomWpns  ******/
/****************************/

static DIALOG wpndata_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     55,   40,   210,  163,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { d_cstile_proc,     198,  84,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
    { jwin_button_proc,  90,   176,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,  170,  176,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_check_proc,   198,  109,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Auto-flash", NULL, NULL },
    { jwin_check_proc,   198,  120,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "2P Flash", NULL, NULL },
    { jwin_check_proc,   198,  131,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "H-Flip", NULL, NULL },
    { jwin_check_proc,   198,  142,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "V-Flip", NULL, NULL },
    { jwin_text_proc,    61,   88,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Flash CSet:", NULL, NULL },
    //10
    { jwin_text_proc,    61,   106,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Animation Frames:", NULL, NULL },
    { jwin_text_proc,    61,   124,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Animation Speed:", NULL, NULL },
    { jwin_text_proc,    61,   142,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Miscellaneous Type:", NULL, NULL },
    { jwin_edit_proc,    160,   88,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,    160,  106,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,    160,  124,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,    160,  142,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_check_proc,   198,  153,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Behind", NULL, NULL },
    { jwin_edit_proc,     92,  65,   155,  16,   vc(12),  vc(1),  0,       0,          64,             0,       NULL, NULL, NULL },
    { jwin_text_proc,     61,  69,   35,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Name:", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};




void edit_weapondata(const SpriteDefinitionRef &spriteref)
{
    char frm[8], spd[8], fcs[8], typ[8];
    char name[64];
    char wpnnumstr[75];
    
    sprintf(wpnnumstr, "Sprite %d: %s", spriteref.slot, curQuest->getSpriteDefinition(spriteref).name.c_str());
    wpndata_dlg[0].dp  = wpnnumstr;
    wpndata_dlg[0].dp2 = lfont;
    wpndata_dlg[2].d1  = curQuest->getSpriteDefinition(spriteref).tile;
    wpndata_dlg[2].d2  = curQuest->getSpriteDefinition(spriteref).csets&15;
    
    for(int i=0; i<4; i++)
        wpndata_dlg[i+5].flags = (curQuest->getSpriteDefinition(spriteref).misc&(1<<i)) ? D_SELECTED : 0;
        
    wpndata_dlg[17].flags = (curQuest->getSpriteDefinition(spriteref).misc & wpndata::WF_BEHIND) ? D_SELECTED : 0;
    
    sprintf(fcs,"%d",curQuest->getSpriteDefinition(spriteref).csets>>4);
    sprintf(frm,"%d",curQuest->getSpriteDefinition(spriteref).frames);
    sprintf(spd,"%d",curQuest->getSpriteDefinition(spriteref).speed);
    sprintf(typ,"%d",curQuest->getSpriteDefinition(spriteref).type);
    wpndata_dlg[13].dp = fcs;
    wpndata_dlg[14].dp = frm;
    wpndata_dlg[15].dp = spd;
    wpndata_dlg[16].dp = typ;
    sprintf(name,"%s",curQuest->getSpriteDefinition(spriteref).name.c_str());
    wpndata_dlg[18].dp = name;

	DIALOG *wpndata_cpy = resizeDialog(wpndata_dlg, 1.5);
    
    int ret;
    wpndata test;
    
    do
    {
        ret = zc_popup_dialog(wpndata_cpy,3);
        
        test.tile  = wpndata_cpy[2].d1;
        test.csets = wpndata_cpy[2].d2;
        
        test.misc  = 0;
        
        for(int i=0; i<4; i++)
            if(wpndata_cpy[i+5].flags & D_SELECTED)
                test.misc |= 1<<i;
                
        test.misc |= (wpndata_cpy[17].flags & D_SELECTED) ? wpndata::WF_BEHIND : 0;
        
        test.csets  |= (atoi(fcs)&15)<<4;
        test.frames = atoi(frm);
        test.speed  = atoi(spd);
        test.type   = atoi(typ);
        test.script = 0; // Not used yet
    }
    while(ret==17);
    
    if(ret==3)
    {
        test.name = std::string(name);
        curQuest->getSpriteDefinition(spriteref) = test;
        saved = false;
    }

	delete[] wpndata_cpy;
}

int onCustomWpns()
{
    int dummy;
    SpriteDefinitionRef index = select_sprite("Select Sprite",SpriteDefinitionRef(), true, dummy);
    
    while(curQuest->isValid(index))
    {
        edit_weapondata(index);
        index = select_sprite("Select Sprite",index, true, dummy);
    }
    
    refresh(rMAP+rCOMBOS);
    return D_O_K;
}

/****************************/
/***** onCustomEnemies  *****/
/****************************/


static int enedata_data_list[] =
{
    2,3,4,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
    31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,138,139,140,141,143,-1
};

static int enedata_data2_list[] =
{
    54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,179,180,181,182,183,184,187,188,189,190,235,236,-1
};

static int enedata_flags_list[] =
{
    74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,98,99,100,101,102,103,104,105,106,120,121,-1
};

static int enedata_flags2_list[] =
{
    90,91,92,93,94,95,96,97,-1
};

static int enedata_flags3_list[] =
{
    107,108,109,110,111,112,113,114,115,116,117,118,119,185,186,-1
};

static int enedata_defense_list[] =
{
    144,145,146,147,148,149,150,151,152,161,162,163,164,165,166,167,168,169,178,-1
};

static int enedata_defense2_list[] =
{
    153,154,155,156,157,158,159,160,170,171,172,173,174,175,176,177,191,192,-1
};

static int enedata_defense3_list[] =
{
    193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,-1
};

static int enedata_spritesize_list[] =
{
    213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,237,238,239,240,241,242,243,244,245,246,-1
};

static TABPANEL enedata_tabs[] =
{
    { (char *)"Data 1",       D_SELECTED,      enedata_data_list,     0, NULL },
    { (char *)"Data 2",       0,               enedata_data2_list,    0, NULL },
    { (char *)"Misc. Flags",	 0,               enedata_flags_list,    0, NULL },
//{ (char *)"Flags 2",	    0,               enedata_flags2_list,   0, NULL },
    { (char *)"Defs 1",	 0,               enedata_defense_list,   0, NULL },
    { (char *)"Defs 2",	 0,               enedata_defense2_list,   0, NULL },
    { (char *)"Defs 3",	 0,               enedata_defense3_list,   0, NULL },
    { (char *)"Spawn Flags",	 0,               enedata_flags3_list,   0, NULL },
    { (char *)"Size",	 0,               enedata_spritesize_list,   0, NULL },
    { NULL,                   0,               NULL,                  0, NULL }
};

list_data_struct bief[eeMAX];
int bief_cnt=-1;

void build_bief_list()
{
    int start=bief_cnt=0;
    
    for(int i=start; i<eeMAX; i++)
    {
        if(enetype_string[i][0]!='-')
        {
            bief[bief_cnt].s = (char *)enetype_string[i];
            bief[bief_cnt].i = i;
            ++bief_cnt;
        }
    }
    
    // No alphabetic sorting for this list
    for(int j=start+1; j<bief_cnt-1; j++)
    {
        if(!strcmp(bief[j].s,"(None)"))
        {
            for(int i=j; i>0; i--)
                zc_swap(bief[i],bief[i-1]);
                
            break;
        }
    }
}

const char *enetypelist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = bief_cnt;
        return NULL;
    }
    
    return bief[index].s;
}

list_data_struct biea[aMAX];
int biea_cnt=-1;

void build_biea_list()
{
    int start=biea_cnt=0;
    
    for(int i=start; i<aMAX; i++)
    {
        if(eneanim_string[i][0]!='-')
        {
            biea[biea_cnt].s = (char *)eneanim_string[i];
            biea[biea_cnt].i = i;
            ++biea_cnt;
        }
    }
    
    for(int i=start; i<biea_cnt-1; i++)
        for(int j=i+1; j<biea_cnt; j++)
            if(stricmp(biea[i].s,biea[j].s)>0 && strcmp(biea[j].s,""))
                zc_swap(biea[i],biea[j]);
}


const char *eneanimlist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = biea_cnt;
        return NULL;
    }
    
    return biea[index].s;
}

const char *itemsetlist(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,count_item_drop_sets()-1);
        return item_drop_sets[index].name;
        //  return itemset_string[index];
    }
    
    *list_size=count_item_drop_sets();
    return NULL;
}

list_data_struct biew[wMax-wEnemyWeapons];
int biew_cnt=-1;

void build_biew_list()
{
    biew_cnt=0;
    
    for(int i=0; i<wMax-wEnemyWeapons; i++)
    {
        if(eweapon_string[i][0]!='-')
        {
            biew[biew_cnt].s = (char *)eweapon_string[i];
            biew[biew_cnt].i = i;
            ++biew_cnt;
        }
    }
}

const char *eweaponlist(int index, int *list_size)
{
    if(biew_cnt==-1)
        build_biew_list();
        
    if(index>=0)
    {
        bound(index,0,biew_cnt-1);
        return biew[index].s;
    }
    
    *list_size=biew_cnt;
    return NULL;
}


//
// Enemy Misc. Attribute label swapping device
//
struct EnemyNameInfo
{
    int family;
    char const* misc[10];
    void* list[10];
};

const char *walkmisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e1tLAST-1);
        return walkmisc1_string[index];
    }
    
    *list_size = e1tLAST;
    return NULL;
}

const char *walkmisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e2tTRIBBLE);
        return walkmisc2_string[index];
    }
    
    *list_size = e2tTRIBBLE+1;
    return NULL;
}

const char *walkmisc7list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e7tEATHURT);
        return walkmisc7_string[index];
    }
    
    *list_size = e7tEATHURT+1;
    return NULL;
}

const char *walkmisc9list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e9tARMOS);
        return walkmisc9_string[index];
    }
    
    *list_size = e9tARMOS+1;
    return NULL;
}

const char *gleeokmisc3list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Breath" : "1 Shot");
    }
    
    *list_size = 2;
    return NULL;
}

const char *gohmamisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Breath" : index==1 ? "3 Shots" : "1 Shot");
    }
    
    *list_size = 3;
    return NULL;
}

const char *manhandlamisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "2x2 (8 Heads)" : "1x1 (4 Heads)");
    }
    
    *list_size = 2;
    return NULL;
}

const char *aquamisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Left (Facing Right)" : "Right (Facing Left)");
    }
    
    *list_size = 2;
    return NULL;
}

const char *patramisc4list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Oval" : "Big Circle");
    }
    
    *list_size = 2;
    return NULL;
}

const char *patramisc5list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Inner Eyes" : index==1 ? "Center Eye" : "None");
    }
    
    *list_size = 3;
    return NULL;
}

const char *patramisc10list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "2x2 Patra" : "1x1 Patra");
    }
    
    *list_size = 2;
    return NULL;
}

const char *dodongomisc10list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "BS-Zelda" : "NES");
    }
    
    *list_size = 2;
    return NULL;
}

const char *digdoggermisc10list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Kid" : "Digdogger");
    }
    
    *list_size = 2;
    return NULL;
}

const char *walkerspawnlist(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Instant" : index==1 ? "Flicker" : "Puff");
    }
    
    *list_size = 3;
    return NULL;
}

const char *wizzrobemisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Phase" : "Teleport");
    }
    
    *list_size = 2;
    return NULL;
}

const char *wizzrobemisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==3 ? "Summon (Layer)" : index==2 ? "Summon" : index==1 ? "8 Shots" : "1 Shot");
    }
    
    *list_size = 4;
    return NULL;
}

const char *keesemisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Bat" : "Keese");
    }
    
    *list_size = 2;
    return NULL;
}

const char *keesemisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Tribble" : "Normal");
    }
    
    *list_size = 2;
    return NULL;
}

const char *trapmisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Constant" : "Line Of Sight");
    }
    
    *list_size = 2;
    return NULL;
}

const char *trapmisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Vertical" : index==1 ? "Horizontal" : "4-Way");
    }
    
    *list_size = 3;
    return NULL;
}

const char *leevermisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        if(index==0)
            return "Link's path";
        else if(index==1)
            return "In place";
        else if(index==2)
            return "Link's path + second";
    }
    
    *list_size = 3;
    return NULL;
}

const char *rockmisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "2x2" : "1x1");
    }
    
    *list_size = 2;
    return NULL;
}

// 0: no, 1: yes
const char *yesnomisclist(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Yes" : "No");
    }
    
    *list_size = 2;
    return NULL;
}

// 0: yes, 1: no
const char *noyesmisclist(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "No" : "Yes");
    }
    
    *list_size = 2;
    return NULL;
}

static ListData walkmisc1_list(walkmisc1list, &font);
static ListData walkmisc2_list(walkmisc2list, &font);
static ListData walkmisc7_list(walkmisc7list, &font);
static ListData walkmisc9_list(walkmisc9list, &font);

static ListData gleeokmisc3_list(gleeokmisc3list, &font);
static ListData gohmamisc1_list(gohmamisc1list, &font);
static ListData manhandlamisc2_list(manhandlamisc2list, &font);
static ListData aquamisc1_list(aquamisc1list, &font);

static ListData patramisc4_list(patramisc4list, &font);
static ListData patramisc5_list(patramisc5list, &font);
static ListData patramisc10_list(patramisc10list, &font);

static ListData dodongomisc10_list(dodongomisc10list, &font);

static ListData keesemisc1_list(keesemisc1list, &font);
static ListData keesemisc2_list(keesemisc2list, &font);

static ListData digdoggermisc10_list(digdoggermisc10list, &font);

static ListData wizzrobemisc1_list(wizzrobemisc1list, &font);
static ListData wizzrobemisc2_list(wizzrobemisc2list, &font);

static ListData trapmisc1_list(trapmisc1list, &font);
static ListData trapmisc2_list(trapmisc2list, &font);

static ListData leevermisc1_list(leevermisc1list, &font);
static ListData rockmisc1_list(rockmisc1list, &font);

static ListData yesnomisc_list(yesnomisclist, &font);
static ListData noyesmisc_list(noyesmisclist, &font);

static EnemyNameInfo enameinf[]=
{
    {
        eeWALK, { "Shot Type:", "Death Type:", "Death Attr. 1:", "Death Attr. 2:", "Death Attr. 3:", "Extra Shots:", "Touch Effects:", "Effect Strength:", "Walk Style:", "Walk Attr.:" },
        { (void*)&walkmisc1_list, (void*)&walkmisc2_list, NULL, NULL, NULL, NULL, (void*)&walkmisc7_list, NULL, (void*)&walkmisc9_list, NULL }
    },
    {
        eeGLEEOK, { "Heads:", "Head HP:", "Shot Type:", NULL, "Neck Segments:", "Neck Offset 1:", "Neck Offset 2:", "Head Offset:", "Fly Head Offset:", NULL },
        { NULL, NULL, (void*)&gleeokmisc3_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeDIG, { "Enemy 1 ID:", "Enemy 2 ID:", "Enemy 3 ID:", "Enemy 4 ID:", "Enemy 1 Qty:", "Enemy 2 Qty:", "Enemy 3 Qty:", "Enemy 4 Qty:", "Unused:", "Type:" },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&digdoggermisc10_list }
    },
    {
        eePATRA, { "Outer Eyes:", "Inner Eyes:", "Eyes' HP:", "Eye Movement:", "Shooters:", "Pattern Odds:", "Pattern Cycles:", "Eye Offset:", "Eye CSet:", "Type:" },
        { NULL, NULL, NULL, (void*)&patramisc4_list, (void*)&patramisc5_list, NULL, NULL, NULL, NULL, (void*)&patramisc10_list }
    },
    {
        eePROJECTILE, { "Shot Type:",  NULL, "Shot Attr. 1:", "Shot Attr. 2:", NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&walkmisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeGHOMA, { "Shot Type:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&gohmamisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeAQUA, { "Side:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&aquamisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeMANHAN, { "Frame rate:",  "Size:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { NULL, (void*)&manhandlamisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeLANM, { "Segments:",  "Segment Lag:", "Item per segment:", NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { NULL, NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeMOLD, { "Segments:",  "Item per segment:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeWIZZ, { "Walk Style:",  "Shot Type:", "Shot Attr. 1:", "Solid Combos OK:", "Teleport Delay:", NULL, NULL, NULL, NULL, NULL  },
        { (void*)&wizzrobemisc1_list, (void*)&wizzrobemisc2_list, NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeDONGO,{ NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Type :"  },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&dodongomisc10_list }
    },
    {
        eeKEESE, { "Walk Style:",  "Death Type:", "Enemy ID:", NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&keesemisc1_list, (void*)&keesemisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeTEK,  { "1/n jump start:",  "1/n jump cont.:", "Jump Z velocity:", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeLEV,  { "Emerge style:",  "Submerged CSet:", "Emerging step:", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { (void*)&leevermisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeWALLM,{ "Fixed distance:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { (void*)&noyesmisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeTRAP, { "Direction:",  "Move Style:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&trapmisc1_list, (void*)&trapmisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeROCK, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Rock size:" },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&rockmisc1_list }
    },
    {
        eeNONE, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Boss Death Trigger:"  },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&yesnomisc_list }
    },
    {
        eeGUY,  { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Boss Death Trigger:"  },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&yesnomisc_list }
    },
    {
        -1,		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    }
};

static std::map<int, EnemyNameInfo *> *enamemap = NULL;

std::map<int, EnemyNameInfo *> *getEnemyNameMap()
{
    if(enamemap == NULL)
    {
        enamemap = new std::map<int, EnemyNameInfo *>();
        
        for(int i=0;; i++)
        {
            EnemyNameInfo *inf = &enameinf[i];
            
            if(inf->family == -1)
                break;
                
            (*enamemap)[inf->family] = inf;
        }
    }

	walkmisc1_list.font = is_large() ? &lfont_l : &font;
	walkmisc2_list.font = is_large() ? &lfont_l : &font;
	walkmisc7_list.font = is_large() ? &lfont_l : &font;
	walkmisc9_list.font = is_large() ? &lfont_l : &font;

	gleeokmisc3_list.font = is_large() ? &lfont_l : &font;
	gohmamisc1_list.font = is_large() ? &lfont_l : &font;
	manhandlamisc2_list.font = is_large() ? &lfont_l : &font;
	aquamisc1_list.font = is_large() ? &lfont_l : &font;
	
	patramisc4_list.font = is_large() ? &lfont_l : &font;
	patramisc5_list.font = is_large() ? &lfont_l : &font;
	patramisc10_list.font = is_large() ? &lfont_l : &font;
	
	dodongomisc10_list.font = is_large() ? &lfont_l : &font;

	keesemisc1_list.font = is_large() ? &lfont_l : &font;
	keesemisc2_list.font = is_large() ? &lfont_l : &font;

	digdoggermisc10_list.font = is_large() ? &lfont_l : &font;

	wizzrobemisc1_list.font = is_large() ? &lfont_l : &font;
	wizzrobemisc2_list.font = is_large() ? &lfont_l : &font;

	trapmisc1_list.font = is_large() ? &lfont_l : &font;
	trapmisc2_list.font = is_large() ? &lfont_l : &font;
	
	leevermisc1_list.font = is_large() ? &lfont_l : &font;
	rockmisc1_list.font = is_large() ? &lfont_l : &font;

	yesnomisc_list.font = is_large() ? &lfont_l : &font;
	noyesmisc_list.font = is_large() ? &lfont_l : &font;
    
    return enamemap;
}

static ListData itemset_list(itemsetlist, &font);
static ListData eneanim_list(eneanimlist, &font);
static ListData enetype_list(enetypelist, &font);
static ListData eweapon_list(eweaponlist, &font);


static ListData walkerspawn_list(walkerspawnlist, &font);

static ListData sfx__list(sfxlist, &font);


static DIALOG enedata_dlg[] =
{
    {  jwin_win_proc,            0,      0,    320,    240,    vc(14),                 vc(1),                   0,    D_EXIT,      0,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_tab_proc,            4,     24,    312,    192,    0,                      0,                       0,    0,           0,    0, (void *) enedata_tabs,                                          NULL, (void *)enedata_dlg  },
    //2
    {  d_ecstile_proc,          16,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
    {  d_ecstile_proc,          52,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
    {  d_ecstile_proc,          88,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
    //5
    {  jwin_button_proc,        50,    220,     61,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "OK",                                                  NULL,   NULL                 },
    {  jwin_button_proc,       130,    220,     61,     16,    vc(14),                 vc(1),                  27,    D_EXIT,      0,    0, (void *) "Cancel",                                              NULL,   NULL                 },
    //7
    {  jwin_text_proc,          21,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Old",                                                 NULL,   NULL                 },
    {  jwin_text_proc,          44,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Special",                                          NULL,   NULL                 },
    {  jwin_text_proc,          90,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "New",                                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
    //12
    {  jwin_text_proc,          42,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,          42,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,          78,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,          78,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
    //16
    {  jwin_text_proc,         114,     54,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Name:",                                               NULL,   NULL                 },
    {  jwin_text_proc,         114,     72,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HP:",                                                 NULL,   NULL                 },
    {  jwin_text_proc,         114,     90,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Damage:",                                             NULL,   NULL                 },
    {  jwin_text_proc,         114,    108,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W. Damage:",                                          NULL,   NULL                 },
    {  jwin_text_proc,         140,    126,     35,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hunger:",                                             NULL,   NULL                 },
    //21
    {  jwin_text_proc,         210,     72,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Random Rate:",                                        NULL,   NULL                 },
    {  jwin_text_proc,         210,     90,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Halt Rate:",                                          NULL,   NULL                 },
    {  jwin_text_proc,         210,    108,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Homing Factor:",                                      NULL,   NULL                 },
    {  jwin_text_proc,         210,    126,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Step Speed:",                                         NULL,   NULL                 },
    //25
    {  jwin_text_proc,           8,    126,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Weapon:",                                             NULL,   NULL                 },
    {  jwin_text_proc,           8,    144,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Type:",                                               NULL,   NULL                 },
    {  jwin_text_proc,           8,    162,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "O. Anim:",                                            NULL,   NULL                 },
    {  jwin_text_proc,           8,    180,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "E. Anim:",                                            NULL,   NULL                 },
    {  jwin_text_proc,           8,    198,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Item Set:",                                           NULL,   NULL                 },
    //30
    {  jwin_edit_proc,          16,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          16,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          52,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          52,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          88,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          88,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    //36
    {  jwin_edit_proc,         144,     50,    166,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         166,     68,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         166,     86,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         166,    104,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         186,    122,     20,     16,    vc(12),                 vc(1),                   0,    0,           1,    0,  NULL,                                                           NULL,   NULL                 },
    //41
    {  jwin_edit_proc,         280,     68,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,     86,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    104,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    122,     30,     16,    vc(12),                 vc(1),                   0,    0,           4,    0,  NULL,                                                           NULL,   NULL                 },
    //45
    {  jwin_droplist_proc,      51,    122,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eweapon_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    140,    110,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    D_EXIT,      0,    0, (void *) &enetype_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    158,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    176,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    194,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &itemset_list,                                         NULL,   NULL                 },
    //50
    {  d_dummy_proc,           240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0,  NULL,													       NULL,   NULL                 },
    {  jwin_text_proc,         240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Pal CSet:",                                           NULL,   NULL                 },
    {  d_dummy_proc,           280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    //54
    {  jwin_text_proc,           6,     54-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 1:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,     72-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 2:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,     90-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 3:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    108-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 4:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    126-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 5:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    144-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 6:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         160,     54-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 7:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         120+40,     72-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 8:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         120+40,     90-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 9:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         120+40,    108-4,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 10:",                                  NULL,   NULL                 },
    //64 : Misc Attribute boxes. 
    {  jwin_edit_proc,          86,     50-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,     68-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,     86-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,    104-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,    122-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,    140-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,     50-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,     68-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,     86-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,    104-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    //74
    {  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Damaged By Power 0 Weapons",                          NULL,   NULL                 },
    {  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Invisible",                                        NULL,   NULL                 },
    {  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Never Returns After Death",						   NULL,   NULL                 },
    {  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Doesn't Count As Beatable Enemy",					   NULL,   NULL                 },
    {  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",										       NULL,   NULL                 },
    {  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    //82
    {  d_dummy_proc,             6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    //90
    {  d_dummy_proc,             6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    //98
    {  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Front",                                   NULL,   NULL                 },
    {  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Left",                                    NULL,   NULL                 },
    {  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Right",                                   NULL,   NULL                 },
    {  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Back",                                    NULL,   NULL                 },
    {  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Hammer Can Break Shield",                             NULL,   NULL                 },
    {  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shield Is Mirrored",                                  NULL,   NULL                 },
    {  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Only Be Seen By Lens Of Truth",                   NULL,   NULL                 },
    //106
    {  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flashing",                                         NULL,   NULL                 },
    {  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Zora\" Screen Flag",                      NULL,   NULL                 },
    {  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Falling Rocks\" Screen Flag",             NULL,   NULL                 },
    {  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Corner Traps\" Screen Flag",              NULL,   NULL                 },
    {  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Horz Trap\" Combo Type/Flag",            NULL,   NULL                 },
    {  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Vert Trap\" Combo Type/Flag",            NULL,   NULL                 },
    {  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"4-Way Trap\" Combo Type/Flag",           NULL,   NULL                 },
    {  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"LR Trap\" Combo Type/Flag",              NULL,   NULL                 },
    //114
    {  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"UD Trap\" Combo Type/Flag",              NULL,   NULL                 },
    {  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Middle Traps\" Screen Flag",              NULL,   NULL                 },
    {  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Statue Fire\" Screen Flag",               NULL,   NULL                 },
    {  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Armos\" Combo Type",                     NULL,   NULL                 },
    {  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Grave\" Combo Type",                     NULL,   NULL                 },
    {  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Ganon\" Room Type",                      NULL,   NULL                 },
    {  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flickering",                                       NULL,   NULL                 },
    {  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Translucent",                                      NULL,   NULL                 },
    //122
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    //130
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    //138
    {  jwin_text_proc,         240,    162,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
    {  jwin_text_proc,         240,    180,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
    {  jwin_edit_proc,         280,    158,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    176,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_check_proc,        165,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Use Pal CSet",                                        NULL,   NULL                 },
    //144 - note: these are defenses 0-16, 17 is at 191
    {  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Boomerang Defense:",                                  NULL,   NULL                 },
    {  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Bomb Defense:",                                       NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Super Bomb Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Arrow Defense:",                                      NULL,   NULL                 },
    {  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Fire Defense:",                                       NULL,   NULL                 },
    {  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Wand Melee Defense:",                                       NULL,   NULL                 },
    {  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Magic Defense:",                                      NULL,   NULL                 },
    {  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hookshot Defense:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hammer Defense:",                                     NULL,   NULL                 },
    {  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Defense:",                                      NULL,   NULL                 },
    {  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Beam Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Beam Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Magic Defense:",                                NULL,   NULL                 },
    {  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Fireball Defense:",                             NULL,   NULL                 },
    {  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Rock Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Stomp Boots Defense:",                                NULL,   NULL                 },
    {  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Byrna Beam Defense:",                              NULL,   NULL                 },
    //161
    {  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_button_proc,           255,    54-4,     48,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "Set All",                                            NULL,   NULL                 },
    //179
    {  jwin_text_proc,           8,    161-4,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "BG Sound:",                                            NULL,   NULL                 },
    {  jwin_text_proc,           8,    176-4,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hit Sound:",                                           NULL,   NULL                 },
    {  jwin_text_proc,           8,    191-4,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Death Sound:",                                         NULL,   NULL                 },
    //182
    {  jwin_droplist_proc,      86,    157-4,     100,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
    {  jwin_droplist_proc,      86,    172-4,     100,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
    {  jwin_droplist_proc,      86,    187-4,     100,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
    {  jwin_text_proc,          6,     184,     95,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Spawn Animation:",                               NULL,   NULL                 },
    {  jwin_droplist_proc,      86,    180,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &walkerspawn_list,                                     NULL,   NULL                 },
    {  jwin_text_proc,         160,    126-4,     50+30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 11:",                                  NULL,   NULL                 },
    {  jwin_text_proc,         160,    144-4,     50+30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 12:",                                  NULL,   NULL                 },
    {  jwin_edit_proc,         242,    122-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,    140-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    
    // 191
    {  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script Weapon Defense:",                              NULL,   NULL                 },
    {  jwin_droplist_proc,      126, 198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
 //193 - sccript 1
     	{  jwin_text_proc,           6,    51,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 1 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    67,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 2 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    83,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 3 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    99,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 4 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    115,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 5 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    131,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 6 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    147,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 7 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    163,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 8 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    179,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 9 Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    196,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script 10 Weapon Defense:",                              NULL,   NULL                 },
	//203 script 1 pulldown
	{  jwin_droplist_proc,      126, 51-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 67-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 83-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 99-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 115-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 131-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 147-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 163-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 179-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 196-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	//213 HitWidth and Height
	{  jwin_text_proc,         12,      51,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "TileWidth:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    51-4,     30,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
        {  jwin_text_proc,         12,      67,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "TileHeight:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    67-4,     30,     16,    vc(12),                 vc(1),                   0,    0,           6,    0, NULL,                                                           NULL,   NULL                 },
        {  jwin_text_proc,         12,      83,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitWidth:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    83-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0, NULL,                                                           NULL,   NULL                 },
        {  jwin_text_proc,         12,      99,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitHeight:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    99-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//221 HitZHeight
	{  jwin_text_proc,         12,      115,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitZHeight:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    115-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//223 HitXOffset
	{  jwin_text_proc,         12,      131,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitXOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    131-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//225 HitYOffset
	{  jwin_text_proc,         12,      147,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitYOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    147-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//227 HitZOffset
	{  jwin_text_proc,         12,      163,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "DrawZOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    163-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//229 DrawXOffset
	{  jwin_text_proc,         12,      179,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "DrawXOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    179-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//231 DrawYOffset
	{  jwin_text_proc,         12,      195,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "DrawYOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    195-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//233 'Tiles' annotation. 
	{ jwin_text_proc,         96,      51,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "(Tiles)",                                  NULL,   NULL                 },
	{ jwin_text_proc,         96,      67,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "(Tiles)",                                  NULL,   NULL                 },
	//235 : Weapon Sprite Pulldown
	{  jwin_text_proc,          8,    193-4+12,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Weapon Sprite:",                              NULL,   NULL                 },
	{  jwin_button_proc,        86, 189-4+13,    151,     14,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,     0,    0, NULL,                            NULL,   NULL                  },
	//237 HitWidth Override
	 { jwin_check_proc,        94+50,     83,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//238 HitHeight override
	 { jwin_check_proc,        94+50,    99,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//239 HitZHeight Override
	 { jwin_check_proc,        94+50,     115,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//240 HitXOffset override
	 { jwin_check_proc,        94+50,     131,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//241 HitYOffset Override
	 { jwin_check_proc,        94+50,    147,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//242 DrawZOffset Override
	 { jwin_check_proc,        94+50,     163,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//243 DrawXOffset Override
	 { jwin_check_proc,        94+50,    179,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//244 DrawYOffset Overrife
	 { jwin_check_proc,        94+50,     195,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//245 TileWidth Overrife
	 { jwin_check_proc,        94+50,     51,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	//246 TileHeight Overrife
	 { jwin_check_proc,        94+50,     67,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Override",                        NULL,   NULL                  },
	
    {  NULL,                     0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 }
};



void setEnemyLabels(int family)
{
    std::map<int, EnemyNameInfo *> *nmap = getEnemyNameMap();
    std::map<int, EnemyNameInfo *>::iterator it = nmap->find(family);
    EnemyNameInfo *inf = NULL;
    
    if(it != nmap->end())
        inf = it->second;
        
    // Default labels
    enedata_dlg[54].dp = (void*)"Misc. Attr. 1:";
    enedata_dlg[55].dp = (void*)"Misc. Attr. 2:";
    enedata_dlg[56].dp = (void*)"Misc. Attr. 3:";
    enedata_dlg[57].dp = (void*)"Misc. Attr. 4:";
    enedata_dlg[58].dp = (void*)"Misc. Attr. 5:";
    enedata_dlg[59].dp = (void*)"Misc. Attr. 6:";
    enedata_dlg[60].dp = (void*)"Misc. Attr. 7:";
    enedata_dlg[61].dp = (void*)"Misc. Attr. 8:";
    enedata_dlg[62].dp = (void*)"Misc. Attr. 9:";
    enedata_dlg[63].dp = (void*)"Misc. Attr. 10:";
    
    if(family==eeKEESE || family==eeGHINI || family==eePEAHAT || family==eeMANHAN
            || family==eeGLEEOK || family==eePATRA || family==eeDIG)
        enedata_dlg[22].dp = (void*)"Turn Freq.:";
    else
        enedata_dlg[22].dp = (void*)"Halt Rate:";
        
    for(int i = 0; i < 10; i++)
    {
        if(inf != NULL)
        {
            if(inf->misc[i]!=NULL)
            {
                enedata_dlg[54+i].dp = (char*)inf->misc[i];
                //enedata_dlg[54+i].flags = enedata_dlg[64+i].flags = 0;
            }
            else
            {
                //enedata_dlg[64+i].flags = enedata_dlg[54+i].flags = D_DISABLED;
            }
        }
        
        if(inf != NULL && inf->list[i] != NULL)
        {
            enedata_dlg[64+i].proc = jwin_droplist_proc;
            enedata_dlg[64+i].fg = jwin_pal[jcTEXTFG];
            enedata_dlg[64+i].bg = jwin_pal[jcTEXTBG];
            ((ListData*)inf->list[i])->font = (is_large() ? &lfont_l : &font);
            enedata_dlg[64+i].dp = inf->list[i];
            enedata_dlg[64+i].dp2 = NULL;
            enedata_dlg[64+i].h = (is_large() ? 22 : 16);
        }
        else
        {
            enedata_dlg[64+i].proc = jwin_edit_proc;
            enedata_dlg[64+i].fg = vc(14);
            enedata_dlg[64+i].bg = vc(1);
            enedata_dlg[64+i].dp = NULL;
            enedata_dlg[64+i].d1 = 6;
            enedata_dlg[64+i].h = int(16 * (is_large() ? 1.5 : 1));
            enedata_dlg[64+i].dp2 = (is_large() ? lfont_l : font);
        }
    }
    
    bool r = 0 != get_bit(quest_rules,qr_NEWENEMYTILES);
    enedata_dlg[r ? 47 : 48].flags |= D_DISABLED;
    enedata_dlg[r ? 48 : 47].flags &= ~D_DISABLED;
    enedata_dlg[r ? 140 : 141].flags |= D_DISABLED;
    enedata_dlg[r ? 141 : 140].flags &= ~D_DISABLED;
    
    if(family==eeTRAP || family==eeROCK || family==eeDONGO || family==eeGANON)
    {
        for(int j=0; j <= edefBYRNA+1 /* + the Set All button*/; j++) enedata_dlg[j+161].flags |= D_DISABLED;
        
        enedata_dlg[192].flags |= D_DISABLED;
	    
	    enedata_dlg[203].d1 |= D_DISABLED;
     enedata_dlg[204].d1 |= D_DISABLED;
     enedata_dlg[205].d1 |= D_DISABLED;
     enedata_dlg[206].d1 |= D_DISABLED;
     enedata_dlg[207].d1 |= D_DISABLED;
     enedata_dlg[208].d1 |= D_DISABLED;
     enedata_dlg[209].d1 |= D_DISABLED;
     enedata_dlg[210].d1 |= D_DISABLED;
     enedata_dlg[211].d1 |= D_DISABLED;
     enedata_dlg[212].d1 |= D_DISABLED;
    }
    else
    {
        for(int j=0; j <= edefBYRNA+1 /* + the Set All button*/; j++) enedata_dlg[j+161].flags &= ~D_DISABLED;
        
        enedata_dlg[192].flags &= ~D_DISABLED;
	    
	    enedata_dlg[203].d1 &= ~D_DISABLED;
     enedata_dlg[204].d1 &= ~D_DISABLED;
     enedata_dlg[205].d1 &= ~D_DISABLED;
     enedata_dlg[206].d1 &= ~D_DISABLED;
     enedata_dlg[207].d1 &= ~D_DISABLED;
     enedata_dlg[208].d1 &= ~D_DISABLED;
     enedata_dlg[209].d1 &= ~D_DISABLED;
     enedata_dlg[210].d1 &= ~D_DISABLED;
     enedata_dlg[211].d1 &= ~D_DISABLED;
     enedata_dlg[212].d1 &= ~D_DISABLED;
    }
    
    if(!(family==eeWALK || family==eeFIRE || family==eeOTHER))
    {
        enedata_dlg[98].flags |= D_DISABLED;
        enedata_dlg[99].flags |= D_DISABLED;
        enedata_dlg[100].flags |= D_DISABLED;
        enedata_dlg[101].flags |= D_DISABLED;
        enedata_dlg[102].flags |= D_DISABLED;
        enedata_dlg[186].flags |= D_DISABLED;
    }
    else
    {
        enedata_dlg[98].flags &= ~D_DISABLED;
        enedata_dlg[99].flags &= ~D_DISABLED;
        enedata_dlg[100].flags &= ~D_DISABLED;
        enedata_dlg[101].flags &= ~D_DISABLED;
        enedata_dlg[102].flags &= ~D_DISABLED;
        enedata_dlg[186].flags &= ~D_DISABLED;
    }
}

int d_ecstile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    int *tempint=enedata_flags2_list;
    tempint=tempint;
    
    switch(msg)
    {
    case MSG_CLICK:
    {
        int f = -8; // Suppress Flip, but in a way that the lowest 3 bits are 0. (Trust me here.) -L
        int t = d->d1;
        int cs = d->d2;
        
        if(select_tile(t,f,1,cs,true))
        {
            d->d1 = t;
			DIALOG *parent = (DIALOG *)d->dp3;
            parent[2].d2 = cs;
			parent[3].d2 = cs;
			parent[4].d2 = cs;
            return D_REDRAW;
        }
    }
    break;
    
    case MSG_DRAW:
        if(is_large())
        {
            d->w = 36;
            d->h = 36;
        }
        
        BITMAP *buf = create_bitmap_ex(8,20,20);
        BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
        
        if(buf && bigbmp)
        {
            clear_bitmap(buf);
            
            if(d->d1)
                overtile16(buf,d->d1,2,2,d->d2,0);
                
            stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->w-3, d->h-3);
            destroy_bitmap(buf);
            jwin_draw_frame(bigbmp,0,0,d->w,d->h,FR_DEEP);
            blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
            destroy_bitmap(bigbmp);
        }
        
        //    text_mode(d->bg);
        break;
    }
    
    return D_O_K;
}

void edit_enemydata(const EnemyDefinitionRef &index)
{
    char hp[8], dp[8], wdp[8], rat[8], hrt[8], hom[8], grm[8], spd[8],
         frt[8], efr[8], bsp[8];
    char w[8],h[8],sw[8],sh[8],ew[8],eh[8];
    char name[64];
    char ms[12][8];
    char enemynumstr[75];
	char hitx[8], hity[8], hitz[8], tiley[8], tilex[8], hitofsx[8], hitofsy[8], hitofsz[8], drawofsx[8], drawofsy[8];	
    
    //disable the missing dialog items!
    //else they will lurk in the background
    //stealing mouse focus -DD
    for(int i=0; enedata_flags2_list[i] != -1; i++)
    {
        enedata_dlg[enedata_flags2_list[i]].proc = d_dummy_proc;
        enedata_dlg[enedata_flags2_list[i]].x = 0;
        enedata_dlg[enedata_flags2_list[i]].y = 0;
        enedata_dlg[enedata_flags2_list[i]].w = 0;
        enedata_dlg[enedata_flags2_list[i]].h = 0;
    }
    
    sprintf(enemynumstr,"Enemy %d: %s", index.slot, curQuest->getEnemyDefinition(index).name.c_str());
    enedata_dlg[0].dp = enemynumstr;
    enedata_dlg[0].dp2 = lfont;
    enedata_dlg[2].d1 = curQuest->getEnemyDefinition(index).tile;
    enedata_dlg[2].d2 = curQuest->getEnemyDefinition(index).cset;
    enedata_dlg[3].d1 = curQuest->getEnemyDefinition(index).s_tile;
    enedata_dlg[3].d2 = curQuest->getEnemyDefinition(index).cset;
    enedata_dlg[4].d1 = curQuest->getEnemyDefinition(index).e_tile;
    enedata_dlg[4].d2 = curQuest->getEnemyDefinition(index).cset;
    
    // Enemy weapon list
    if(curQuest->getEnemyDefinition(index).weapon==wNone)
    {
        enedata_dlg[45].d1 = 0;
    }
    else
    {
        if(biew_cnt==-1)
        {
            build_biew_list();
        }
        
        for(int j=0; j<biew_cnt; j++)
        {
            if(biew[j].i == curQuest->getEnemyDefinition(index).weapon - wEnemyWeapons)
                enedata_dlg[45].d1 = j;
        }
    }
    
    // Enemy family list
    if(bief_cnt==-1)
    {
        build_bief_list();
    }
    
    for(int j=0; j<bief_cnt; j++)
    {
        if(bief[j].i == curQuest->getEnemyDefinition(index).family)
            enedata_dlg[46].d1 = j;
    }
    
    // Enemy animation list
    if(biea_cnt==-1)
    {
        build_biea_list();
    }
    
    for(int j=0; j<biea_cnt; j++)
    {
        if(biea[j].i == curQuest->getEnemyDefinition(index).anim)
            enedata_dlg[47].d1 = j;
            
        if(biea[j].i == curQuest->getEnemyDefinition(index).e_anim)
            enedata_dlg[48].d1 = j;
    }
    
    
    enedata_dlg[49].d1 = curQuest->getEnemyDefinition(index).item_set;
    
    sprintf(w,"%d",curQuest->getEnemyDefinition(index).width);
    sprintf(h,"%d",curQuest->getEnemyDefinition(index).height);
    sprintf(sw,"%d",curQuest->getEnemyDefinition(index).s_width);
    sprintf(sh,"%d",curQuest->getEnemyDefinition(index).s_height);
    sprintf(ew,"%d",curQuest->getEnemyDefinition(index).e_width);
    sprintf(eh,"%d",curQuest->getEnemyDefinition(index).e_height);
    
    
    enedata_dlg[30].dp = w;
    enedata_dlg[31].dp = h;
    enedata_dlg[32].dp = sw;
    enedata_dlg[33].dp = sh;
    enedata_dlg[34].dp = ew;
    enedata_dlg[35].dp = eh;
    
    sprintf(hp,"%d",curQuest->getEnemyDefinition(index).hp);
    sprintf(dp,"%d",curQuest->getEnemyDefinition(index).dp);
    sprintf(wdp,"%d",curQuest->getEnemyDefinition(index).wdp);
    sprintf(grm,"%d",curQuest->getEnemyDefinition(index).grumble);
    enedata_dlg[37].dp = hp;
    enedata_dlg[38].dp = dp;
    enedata_dlg[39].dp = wdp;
    enedata_dlg[40].dp = grm;
    
    sprintf(rat,"%d",curQuest->getEnemyDefinition(index).rate);
    sprintf(hrt,"%d",curQuest->getEnemyDefinition(index).hrate);
    sprintf(hom,"%d",curQuest->getEnemyDefinition(index).homing);
    sprintf(spd,"%d",curQuest->getEnemyDefinition(index).step);
    enedata_dlg[41].dp = rat;
    enedata_dlg[42].dp = hrt;
    enedata_dlg[43].dp = hom;
    enedata_dlg[44].dp = spd;
    
    sprintf(name,"%s",curQuest->getEnemyDefinition(index).name.c_str());
    enedata_dlg[36].dp = name;
    
    for (int i = 0; i < 12; i++)
    {
        sprintf(ms[i], "%ld", curQuest->getEnemyDefinition(index).miscs[i]);
    }
    
    
    
    for(int j=0; j <= edefBYRNA; j++)
    {
        enedata_dlg[j+161].d1 = curQuest->getEnemyDefinition(index).defense[j];
    }
    
    enedata_dlg[192].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT];
    
    //Script Defences
    enedata_dlg[203].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT01];
     enedata_dlg[204].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT02];
     enedata_dlg[205].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT03];
     enedata_dlg[206].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT04];
     enedata_dlg[207].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT05];
     enedata_dlg[208].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT06];
     enedata_dlg[209].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT07];
     enedata_dlg[210].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT08];
     enedata_dlg[211].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT09];
     enedata_dlg[212].d1 = curQuest->getEnemyDefinition(index).defense[edefSCRIPT10];
    
    //tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
    sprintf(tilex,"%ld",curQuest->getEnemyDefinition(index).txsz);
    sprintf(tiley,"%ld",curQuest->getEnemyDefinition(index).tysz);
    sprintf(hitx,"%ld",curQuest->getEnemyDefinition(index).hxsz);
    sprintf(hity,"%ld",curQuest->getEnemyDefinition(index).hysz);
    sprintf(hitz,"%ld",curQuest->getEnemyDefinition(index).hzsz);
  
    enedata_dlg[214].dp = tilex;
    enedata_dlg[216].dp = tiley;
    enedata_dlg[218].dp = hitx;
    enedata_dlg[220].dp = hity;
    enedata_dlg[222].dp = hitz;
    
    //HitXOffset, HitYOFfset, hitZOffset, DrawXOffsrt, DrawYOffset
    sprintf(hitofsx,"%ld",curQuest->getEnemyDefinition(index).hxofs);
    sprintf(hitofsy,"%ld",curQuest->getEnemyDefinition(index).hyofs);
    sprintf(hitofsz,"%ld",curQuest->getEnemyDefinition(index).zofs);
    sprintf(drawofsx,"%ld",curQuest->getEnemyDefinition(index).xofs);
    sprintf(drawofsy,"%ld",curQuest->getEnemyDefinition(index).yofs); //This seems to be setting to +48 or something with any value set?! -Z
    
    enedata_dlg[224].dp = hitofsx;
    enedata_dlg[226].dp = hitofsy;
    enedata_dlg[228].dp = hitofsz;
    enedata_dlg[230].dp = drawofsx;
    enedata_dlg[232].dp = drawofsy; //This seems to be setting to +48 or something with any value set?! -Z
    
    //Override flags
    enedata_dlg[237].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ? D_SELECTED : 0;
    enedata_dlg[238].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ? D_SELECTED : 0;
    enedata_dlg[239].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) ? D_SELECTED : 0;
    enedata_dlg[240].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) ? D_SELECTED : 0;
    enedata_dlg[241].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) ? D_SELECTED : 0;
    enedata_dlg[242].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) ? D_SELECTED : 0;
    enedata_dlg[243].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) ? D_SELECTED : 0;
    enedata_dlg[244].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) ? D_SELECTED : 0;
    enedata_dlg[245].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_TILE_WIDTH) ? D_SELECTED : 0;
    enedata_dlg[246].flags = (curQuest->getEnemyDefinition(index).SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) ? D_SELECTED : 0;
    
    sprintf(frt,"%d",curQuest->getEnemyDefinition(index).frate);
    sprintf(efr,"%d",curQuest->getEnemyDefinition(index).e_frate);
    enedata_dlg[140].dp = frt;
    enedata_dlg[141].dp = efr;
    
    //sprintf(sfx,"%d",guysbuf[index].bgsfx);
    enedata_dlg[182].d1= (int)curQuest->getEnemyDefinition(index).bgsfx;
    enedata_dlg[183].d1= (int)curQuest->getEnemyDefinition(index).hitsfx;
    enedata_dlg[184].d1= (int)curQuest->getEnemyDefinition(index).deadsfx;
    
   
    
    //2.6 Enemy Weapon Sprite -Z
    
    //Find the default sprite. THe values are offset, somehow. Perhaps a switch statement would be better. 
    
    /*
    byte default_weapon_sprites[]={
	    0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, //15
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,255, //31 script weapons start here
	    255,255,255,255,255,255,255,255, 255,0,0,0,0,0,0,0, //47 last script weapon: 40
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, //63
	    
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, //79
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, //95
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, //111
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, //127
	    17, //fireball 128
	    //spritelist goes here
	    19,	//enemy arrow 130
	    0,
	    4, //enemy brfang 131
	    20, //enemy swordbeam 132
	    0,0,
	    18, //rock 133
	    21, //magic	 134
	    78, //enemy bomb explosion 135
	    79, //enemy Sbomb explosion 136
	    76, //enemy bomb projectile 137
	    77, //enemy Sbomb projectile 138
	   
	    80, //firetrail 139
	    35, //fire flame 140
	    36, //wind magic 141
	    81, //fire2 (fire gleeok flame) 142
	    82, //143 (fire trail 2?
	    83, //144 (ice?)
	    
	    0
    };
    
    */
    
    // I don't know what's going on here; if you want to assign weapon sprites,
    // use curQuest->specialSprites(). not random numerical constants -DD

    //If the weapon sprite is 0, assign it here. 
/*    if ( !curQuest->isValid(guysbuf[index].wpnsprite)) {
	    int wspr = -1;
	    switch(guysbuf[index].weapon)
	    {
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
			wspr = -1;
			break;
		
		case 128: wspr = 17; break;
		case 129: wspr = 0; break;
		case 130: wspr = 19; break;
		case 131: wspr = 4; break;
		case 132: wspr = 20; break;
		case 133: wspr = 18; break;
		case 134: wspr = 21; break;
		case 135: wspr = 78; break;
		case 136: wspr = 79; break;
		case 137: wspr = 76; break;
		case 138: wspr = 77; break;
		case 139: wspr = 80; break;
		case 140: wspr = 35; break;
		case 141: wspr = 36; break;
		case 142: wspr = 81; break;
		case 143: wspr = 82; break;
		case 144: wspr = 83; break;
		case 145: wspr = 0; //fireball (rising)
		
		default: break; //No assign.
		    
		    
	    }
*  
        for ( int q = FIRST_EWEAPON_ID; q < wMAX; q++ ) //read the weapon type of the npc and find its sprite
        {
	    if ( guysbuf[index].weapon == q ) { guysbuf[index].wpnsprite = default_weapon_sprites[q]; break; }
        }
	    
    }*/
    
    //Set the dialogue. 

    const char *nonestr = "(None)";    
    enedata_dlg[236].dp = (void *)(curQuest->isValid(curQuest->getEnemyDefinition(index).wpnsprite) ? curQuest->getSpriteDefinition(curQuest->getEnemyDefinition(index).wpnsprite).name.c_str() : nonestr);
        
    /*
    if ( guysbuf[index].wpnsprite <= 0 ) {
        for ( int q = FIRST_EWEAPON_ID; q < wMAX; q++ ) //read the weapon type of the npc and find its sprite
        {
	    if ( guysbuf[index].weapon == q ) { guysbuf[index].wpnsprite = default_weapon_sprites[q]; itemdata_dlg[236].d1 = default_weapon_sprites[q]; break; }
        }
    }
    else {
    */
    
    /*
        for(int j=0; j<biw_cnt; j++)
        {
	    if(biw[j].i == guysbuf[index].wpnsprite){
	        itemdata_dlg[236].d1 = j;
	        foundwpnsprite = true;
		    al_trace("Found weapon sprite: \n", j);
		    break;
	    }
        }
    //}
    
    */
    
    //We could always make it a text entry box if all else fails. 
    
    
    
    
    //Do enemies use sprite 0 for their weapons? if not then we can override. Hell, we can just read
    //their weapon, and if it is set to sprite 0 and not an arrow, and the quest header is not 2.54+, 
    //we change it to the appropriate sprite HERE. -Z
    
    //Otherwise, add an exception that if the sprite is 0, it is treated as NULL?
    
    //Arrow (Enemy) will always be the first on the list, so we absolutely /must/ prime the enemy weapon sprite somewhere. -Z
    /*
    if ( !foundwpnsprite ) {
	    //set a string "Default" for -1
    }
    */
    
    //We need to set the initial default sprites for all weapons. 
    /* This requires that the default value of guydata.wpnsprite is -1, because sprite 0 is a legal sprite. 
    
    */
    
    //enedata_dlg[236].d1= (int)guysbuf[index].wpnsprite;
    
    sprintf(bsp,"%d",curQuest->getEnemyDefinition(index).bosspal);
    
    if(curQuest->getEnemyDefinition(index).cset == 14)
    {
        enedata_dlg[143].flags = D_SELECTED;
    }
    else
    {
        enedata_dlg[143].flags = 0;
    }
    
    enedata_dlg[53].dp = bsp;
    
    for(int i=0; i<32; i++)
        enedata_dlg[74+i].flags = (curQuest->getEnemyDefinition(index).flags & (1<<i)) ? D_SELECTED : 0;
        
    enedata_dlg[186].d1 = (curQuest->getEnemyDefinition(index).flags & guy_fadeinstant ? 2
                           : curQuest->getEnemyDefinition(index).flags & guy_fadeflicker ? 1 : 0);
                           
    for(int i=0; i<16; i++)
        enedata_dlg[106+i].flags = (curQuest->getEnemyDefinition(index).flags2 & (1<<i)) ? D_SELECTED : 0;

    /*
    enedata_dlg[203].d1 = guysbuf[index].defense[edefSCRIPT01];
     enedata_dlg[204].d1 = guysbuf[index].defense[edefSCRIPT02];
     enedata_dlg[205].d1 = guysbuf[index].defense[edefSCRIPT03];
     enedata_dlg[206].d1 = guysbuf[index].defense[edefSCRIPT04];
     enedata_dlg[207].d1 = guysbuf[index].defense[edefSCRIPT05];
     enedata_dlg[208].d1 = guysbuf[index].defense[edefSCRIPT06];
     enedata_dlg[209].d1 = guysbuf[index].defense[edefSCRIPT07];
     enedata_dlg[210].d1 = guysbuf[index].defense[edefSCRIPT08];
     enedata_dlg[211].d1 = guysbuf[index].defense[edefSCRIPT09];
     enedata_dlg[212].d1 = guysbuf[index].defense[edefSCRIPT10];
    */

    int ret;
    guydata test = curQuest->getEnemyDefinition(index);

	DIALOG *enedata_cpy = resizeDialog(enedata_dlg, 1.5);
    
    setEnemyLabels(curQuest->getEnemyDefinition(index).family);
    
    do
    {
        for (int i = 0; i < 10; i++)
        {
            if (enedata_cpy[64 + i].proc == jwin_droplist_proc)
            {
                int size = 0;
                ((ListData*)enedata_cpy[64 + i].dp)->listFunc(-1, &size);
                // Bound ms[i] as well as enedata_dlg[64+i].d1
                sprintf(ms[i], "%d", vbound(atoi(ms[i]), 0, size));
                enedata_cpy[64 + i].d1 = atoi(ms[i]);
            }
            else
                enedata_cpy[64 + i].dp = ms[i];
        }

        enedata_cpy[189].dp = ms[10];
        enedata_cpy[190].dp = ms[11]; //!

        ret = zc_popup_dialog(enedata_cpy, 3);

        test.tile = enedata_cpy[2].d1;
        test.cset = enedata_cpy[2].d2;
        test.s_tile = enedata_cpy[3].d1;
        test.e_tile = enedata_cpy[4].d1;

        test.width = vbound(atoi(w), 0, 20);
        test.height = vbound(atoi(h), 0, 20);
        test.s_width = vbound(atoi(sw), 0, 20);
        test.s_height = vbound(atoi(sh), 0, 20);
        test.e_width = vbound(atoi(ew), 0, 20);
        test.e_height = vbound(atoi(eh), 0, 20);

        test.weapon = enedata_cpy[45].d1 != 0 ? biew[enedata_cpy[45].d1].i + wEnemyWeapons : wNone;
        test.family = bief[enedata_cpy[46].d1].i;
        test.anim = biea[enedata_cpy[47].d1].i;
        test.e_anim = biea[enedata_cpy[48].d1].i;
        test.item_set = enedata_cpy[49].d1;

        test.hp = vbound(atoi(hp), 0, 32767); //0x7FFF, not 0xFFFF?
        test.dp = vbound(atoi(dp), 0, 32767);
        test.wdp = vbound(atoi(wdp), 0, 32767);
        test.grumble = vbound(atoi(grm), 0, 4);

        test.rate = vbound(atoi(rat), 0, (test.family == eeFIRE || test.family == eeOTHER) ? 32767 : 16);
        test.hrate = vbound(atoi(hrt), 0, (test.family == eeFIRE || test.family == eeOTHER) ? 32767 : 16);
        test.homing = vbound(atoi(hom), 0, (test.family == eeFIRE || test.family == eeOTHER) ? 32767 : 256);
        test.step = vbound(atoi(spd), 0, (test.family == eeFIRE || test.family == eeOTHER) ? 32767 : 1000);

        test.frate = vbound(atoi(frt), 0, 256);
        test.e_frate = vbound(atoi(efr), 0, 256);
        test.bosspal = vbound(atoi(bsp), -1, 29);
        test.bgsfx = enedata_cpy[182].d1;
        test.hitsfx = enedata_cpy[183].d1;
        test.deadsfx = enedata_cpy[184].d1;

        if (ret == 236)
        {
            int status;
            SpriteDefinitionRef ref = select_sprite("Select Weapon Sprite", test.wpnsprite, false, status);
            if (status == 4)
            {
                test.wpnsprite = ref;
                enedata_cpy[236].dp = (void *)(curQuest->isValid(test.wpnsprite) ? curQuest->getSpriteDefinition(test.wpnsprite).name.c_str() : nonestr);
            }
        }

        for (int j = 0; j < 10; j++)
            test.miscs[j] = (enedata_cpy[64 + j].proc == jwin_droplist_proc) ? enedata_cpy[64 + j].d1 : atol(ms[j]);
        test.miscs[10] = atol(ms[10]);
        test.miscs[11] = atol(ms[11]);




        for (int j = 0; j <= edefBYRNA; j++)
        {
            test.defense[j] = enedata_cpy[j + 161].d1;
        }

        test.defense[edefSCRIPT] = enedata_cpy[192].d1;


        for (int i = 0; i < 32; i++)
            test.flags |= (enedata_cpy[74 + i].flags & D_SELECTED) ? (1 << i) : 0;

        test.flags &= ~(guy_fadeinstant | guy_fadeflicker);
        test.flags |= (enedata_cpy[186].d1 == 2 ? guy_fadeinstant : enedata_cpy[186].d1 == 1 ? guy_fadeflicker : 0);

        for (int i = 0; i < 16; i++)
            test.flags2 |= (enedata_cpy[106 + i].flags & D_SELECTED) ? (1 << i) : 0;

        if (enedata_cpy[143].flags & D_SELECTED)
            test.cset = 14;


        test.defense[edefSCRIPT01] = enedata_cpy[203].d1;
        test.defense[edefSCRIPT02] = enedata_cpy[204].d1;
        test.defense[edefSCRIPT03] = enedata_cpy[205].d1;
        test.defense[edefSCRIPT04] = enedata_cpy[206].d1;
        test.defense[edefSCRIPT05] = enedata_cpy[207].d1;
        test.defense[edefSCRIPT06] = enedata_cpy[208].d1;
        test.defense[edefSCRIPT07] = enedata_cpy[209].d1;
        test.defense[edefSCRIPT08] = enedata_cpy[210].d1;
        test.defense[edefSCRIPT09] = enedata_cpy[211].d1;
        test.defense[edefSCRIPT10] = enedata_cpy[212].d1;

        //tilewidth, tileheight, hitwidth, hitheight, 
        test.txsz = atoi(tilex);
        test.tysz = atoi(tiley);
        test.hxsz = atoi(hitx);
        test.hysz = atoi(hity);
        test.hzsz = atoi(hitz);
        test.hxofs = atoi(hitofsx);
        test.hyofs = atoi(hitofsy);
        test.zofs = atoi(hitofsz);
        test.xofs = atoi(drawofsx);
        test.yofs = atoi(drawofsy); //This seems to be setting to +48 or something with any value set?! -Z

        //override flags
        if (enedata_cpy[237].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_HIT_WIDTH;

        if (enedata_cpy[238].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_HIT_HEIGHT;

        if (enedata_cpy[239].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_HIT_Z_HEIGHT;

        if (enedata_cpy[240].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_HIT_X_OFFSET;

        if (enedata_cpy[241].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_HIT_Y_OFFSET;

        if (enedata_cpy[242].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_DRAW_Z_OFFSET;
        if (enedata_cpy[243].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_DRAW_X_OFFSET;

        if (enedata_cpy[244].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_DRAW_Y_OFFSET;
        if (enedata_cpy[245].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_TILE_WIDTH;
        if (enedata_cpy[246].flags & D_SELECTED)
            test.SIZEflags |= guyflagOVERRIDE_TILE_HEIGHT;



        if (ret == 5)
        {
            test.name = std::string(name);
            curQuest->getEnemyDefinition(index) = test;
            saved = false;
        }
        else if (ret == 46)
        {
            setEnemyLabels(bief[enedata_cpy[46].d1].i);
        }
        else if (ret == 178)
        {
            for (int j = 1; j <= edefBYRNA; j++)
            {
                enedata_cpy[j + 161].d1 = enedata_cpy[161].d1;
            }

            enedata_cpy[192].d1 = enedata_cpy[161].d1;
            //Clear to 0
            enedata_cpy[203].d1 = enedata_cpy[161].d1;
            enedata_cpy[204].d1 = enedata_cpy[161].d1;
            enedata_cpy[205].d1 = enedata_cpy[161].d1;
            enedata_cpy[206].d1 = enedata_cpy[161].d1;
            enedata_cpy[207].d1 = enedata_cpy[161].d1;
            enedata_cpy[208].d1 = enedata_cpy[161].d1;
            enedata_cpy[209].d1 = enedata_cpy[161].d1;
            enedata_cpy[210].d1 = enedata_cpy[161].d1;
            enedata_cpy[211].d1 = enedata_cpy[161].d1;
            enedata_cpy[212].d1 = enedata_cpy[161].d1;
        }
    }
    while(ret != 5 && ret != 6 && ret != 0);
    
	delete[] enedata_cpy;
}

int onCustomEnemies()
{
    int foo;
    EnemyDefinitionRef index = select_enemy("Select Enemy", EnemyDefinitionRef(), ESF_BADGUYS | ESF_GOODGUYS, true, foo);
    
    while(curQuest->isValid(index))
    {
        edit_enemydata(index);
        
        index = select_enemy("Select Enemy", index, ESF_BADGUYS | ESF_GOODGUYS, true, foo);
    }
    
    refresh(rMAP+rCOMBOS);
    return D_O_K;
}


int onCustomGuys()
{
    return D_O_K;
}

int d_ltile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    //d1=dir
    //d2=type (determines how to animate)
    //fg=cset (6)
    enum {lt_clock, lt_tile, lt_flip, lt_extend};
    static int bg=makecol(0, 0, 0);
    int *p=(int*)d->dp3;
    int oldtile=0;
    int oldflip=0;
    
    switch(msg)
    {
    case MSG_START:
    {
        d->dp3=(int*)zc_malloc(sizeof(int)*4);
        p=(int*)d->dp3;
        p[lt_clock]=0;
        p[lt_tile]=0;
        p[lt_flip]=0;
        p[lt_extend]=0;
        linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
        break;
    }
    
    case MSG_CLICK:
    {
        int t;
        int f;
        int extend;
        int cs = 6;
        linktile(&t, &f, &extend, d->d2, d->d1, zinit.linkanimationstyle);
        
        switch(extend)
        {
        case 0:
            if(!isinRect(Backend::mouse->getVirtualScreenX(), Backend::mouse->getVirtualScreenY(),d->x+2+8, d->y+2+4, d->x+(16*(is_large() ? 2 : 1))+8+2, d->y+(16+16*(is_large() ? 2 : 1))+2))
            {
                return D_O_K;
            }
            
            break;
            
        case 1:
            if(!isinRect(Backend::mouse->getVirtualScreenX(), Backend::mouse->getVirtualScreenY(),d->x+2+8, d->y+2+4, d->x+(16*(is_large() ? 2 : 1))+8+2, d->y+(4+32*(is_large() ? 2 : 1))+2))
            {
                return D_O_K;
            }
            
            break;
            
        case 2:
            if(!isinRect(Backend::mouse->getVirtualScreenX(), Backend::mouse->getVirtualScreenY(),d->x+2+8, d->y+4, d->x+(32*(is_large() ? 2 : 1))+8+2, d->y+(4+32*(is_large() ? 2 : 1))+2))
            {
                return D_O_K;
            }
            
            break;
        }
        
        if(select_tile(t,f,2,cs,false,extend, true))
        {
            extend=ex;
            setlinktile(t,f,extend,d->d2,d->d1);
            return D_REDRAW;
        }
    }
    break;
    
    case MSG_VSYNC:
        oldtile=p[lt_tile];
        oldflip=p[lt_flip];
        p[lt_clock]++;
        
        switch(zinit.linkanimationstyle)
        {
        case las_original:                                             //2-frame
            switch(d->d2)
            {
            case ls_charge:
            case ls_walk:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=6)
                {
                    if(d->d1==up&&d->d2==ls_walk)
                    {
                        p[lt_flip]=1;                //h flip
                    }
                    else
                    {
                        p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;               //tile++
                    }
                };
                
                if(p[lt_clock]>=11)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_jump:
                if(p[lt_clock]>=24)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=36)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
                }
                
                break;
                
            case ls_slash:
                if(p[lt_clock]<6)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_stab:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_pound:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<30)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=31)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_float:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=12)
                {
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                };
                
                if(p[lt_clock]>=23)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_swim:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=12)
                {
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                };
                
                if(p[lt_clock]>=23)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_dive:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=50)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                };
                
                if((p[lt_clock]/12)&1)
                {
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                };
                
                if(p[lt_clock]>=81)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_landhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_landhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_cast:
                linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]<96)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                };
                
                if(p[lt_clock]>=194)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            default:
                break;
            }
            
            break;
            
        case las_bszelda:                                             //3-frame BS
            switch(d->d2)
            {
            case ls_charge:
            case ls_walk:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=27)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_jump:
                if(p[lt_clock]>=24)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=36)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
                }
                
                break;
                
            case ls_slash:
                if(p[lt_clock]<6)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_stab:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_pound:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<30)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=31)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_float:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=55)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_swim:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=55)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_dive:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=50)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                };
                
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=81)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_landhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_landhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_cast:
                linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]<96)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                };
                
                if(p[lt_clock]>=194)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            default:
                break;
            }
            
            break;
            
        case las_zelda3slow:                                         //multi-frame Zelda 3 (slow)
        case las_zelda3:                                             //multi-frame Zelda 3
            switch(d->d2)
            {
            case ls_charge:
            case ls_walk:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=(64*(link_animation_speed)))
                {
                    p[lt_tile]+=(p[lt_extend]==2?2:1);
                    
                    int l=((p[lt_clock]/link_animation_speed)&15);
                    l-=((l>3)?1:0)+((l>12)?1:0);
                    p[lt_tile]+=(l/2)*(p[lt_extend]==2?2:1);
                    
                    //p[lt_tile]+=(((p[lt_clock]>>2)%8)*(p[lt_extend]==2?2:1));
                    if(p[lt_clock]>=255)
                    {
                        p[lt_clock]=-1;
                    }
                }
                
                break;
                
            case ls_jump:
                if(p[lt_clock]>=24)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=36)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
                }
                
                break;
                
            case ls_slash:
                if(p[lt_clock]>23)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_slash, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=(((p[lt_clock]>>2)%6)*(p[lt_extend]==2?2:1));
                    
                    if(p[lt_clock]>=47)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                
                break;
                
            case ls_stab:
                if(p[lt_clock]>35)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_stab, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
                    
                    if(p[lt_clock]>=47)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                
                break;
                
            case ls_pound:
                if(p[lt_clock]>35)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_pound, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
                    
                    if(p[lt_clock]>=47)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                
                break;
                
            case ls_float:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
                
                if(p[lt_clock]>=23)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_swim:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=((p[lt_clock]/12)%4)<<(p[lt_extend]==2?1:0);
                
                if(p[lt_clock]>=47)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_dive:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=50)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                };
                
                p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
                
                if(p[lt_clock]>=81)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_landhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_landhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_cast:
                linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]<96)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                };
                
                if(p[lt_clock]>=194)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            default:
                break;
            }
            
            break;
            
        default:
            break;
        }
        
        if((p[lt_tile]!=oldtile)||(p[lt_flip]!=oldflip))
        {
            d->flags|=D_DIRTY;
        }
        
        break;
        
    case MSG_DRAW:
    {
        BITMAP *buf=create_bitmap_ex(8,1,1);
        BITMAP *buf2=buf;
        int dummy1, dummy2;
        int extend;
        linktile(&dummy1, &dummy2, &extend, d->d2, d->d1, zinit.linkanimationstyle);
        int w = 16;
        int h = 16;
        
        switch(extend)
        {
        case 0: //16x16
            break;
            
        case 1: //16x32
            w = 16;
            h = 32;
            break;
            
        case 2: //32x32
            w = 32;
            h = 32;
            break;
            
        default:
            break;
        }
        
        buf = create_bitmap_ex(8,w,h);
        
        if(is_large())
        {
            w *= 2;
            h *= 2;
        }
        
        BITMAP *bigbmp = create_bitmap_ex(8,w+4,h+4);
        
        if(buf && bigbmp)
        {
            clear_to_color(buf, bg);
            
            switch(extend)
            {
            case 0:
                overtile16(buf,p[lt_tile],0,0,6,p[lt_flip]);
                stretch_blit(buf, bigbmp, 0,0, 16, 16, 2, 2, w, h);
                jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
                blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
                break;
                
            case 1:
                overtile16(buf,p[lt_tile]-TILES_PER_ROW,0,0,6,p[lt_flip]);
                overtile16(buf,p[lt_tile],0,16,6,p[lt_flip]);
                stretch_blit(buf, bigbmp, 0,0, 16, 32, 2, 2, w, h);
                jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
                blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
                break;
                
            case 2:
                overtile16(buf,p[lt_tile]-TILES_PER_ROW,8,0,6,p[lt_flip]); //top middle
                overtile16(buf,p[lt_tile]-TILES_PER_ROW-(p[lt_flip]?-1:1),-8,0,6,p[lt_flip]); //top left
                overtile16(buf,p[lt_tile]-TILES_PER_ROW+(p[lt_flip]?-1:1),8+16,0,6,p[lt_flip]); //top right
                overtile16(buf,p[lt_tile]-(p[lt_flip]?-1:1),-8,18,6,p[lt_flip]); //bottom left
                overtile16(buf,p[lt_tile]+(p[lt_flip]?-1:1),8+16,18,6,p[lt_flip]); //bottom right
                overtile16(buf,p[lt_tile],8,16,6,p[lt_flip]);              //bottom middle
                stretch_blit(buf, bigbmp, 0,0, 32, 32, 2, 2, w, h);
                jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
                blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
                break;
                
            default:
                break;
            }
            
            destroy_bitmap(bigbmp);
            destroy_bitmap(buf);
            destroy_bitmap(buf2);
        }
    }
    break;
    
    case MSG_END:
    {
        zc_free(d->dp3);
        break;
    }
    }
    
    return D_O_K;
}

static int linktile_land_walk_list[] =
{
    // dialog control number
    11, 12, 13, 14, 15, 16, 17, 18, -1
};

static int linktile_land_slash_list[] =
{
    // dialog control number
    19, 20, 21, 22, 23, 24, 25, 26, -1
};

static int linktile_land_stab_list[] =
{
    // dialog control number
    27, 28, 29, 30, 31, 32, 33, 34, -1
};

static int linktile_land_pound_list[] =
{
    // dialog control number
    35, 36, 37, 38, 39, 40, 41, 42, -1
};

static int linktile_land_hold_list[] =
{
    // dialog control number
    43, 44, 45, 46, -1
};

static int linktile_land_cast_list[] =
{
    // dialog control number
    47, -1
};

static int linktile_land_jump_list[] =
{
    // dialog control number
    77, 78, 79, 80, 81, 82, 83, 84, -1
};

static int linktile_land_charge_list[] =
{
    // dialog control number
    85, 86, 87, 88, 89, 90, 91, 92, -1
};

static TABPANEL linktile_land_tabs[] =
{
    // (text)
    { (char *)"Walk",       D_SELECTED,  linktile_land_walk_list, 0, NULL },
    { (char *)"Slash",      0,           linktile_land_slash_list, 0, NULL },
    { (char *)"Stab",       0,           linktile_land_stab_list, 0, NULL },
    { (char *)"Pound",      0,           linktile_land_pound_list, 0, NULL },
    { (char *)"Jump",       0,           linktile_land_jump_list, 0, NULL },
    { (char *)"Charge",     0,           linktile_land_charge_list, 0, NULL },
    { (char *)"Hold",       0,           linktile_land_hold_list, 0, NULL },
    { (char *)"Cast",       0,           linktile_land_cast_list, 0, NULL },
    { NULL,                 0,           NULL,                   0, NULL }
};

static int linktile_water_float_list[] =
{
    // dialog control number
    48, 49, 50, 51, 52, 53, 54, 55, -1
};

static int linktile_water_swim_list[] =
{
    // dialog control number
    56, 57, 58, 59, 60, 61, 62, 63, -1
};

static int linktile_water_dive_list[] =
{
    // dialog control number
    64, 65, 66, 67, 68, 69, 70, 71, -1
};

static int linktile_water_hold_list[] =
{
    // dialog control number
    72, 73, 74, 75, -1
};

static TABPANEL linktile_water_tabs[] =
{
    // (text)
    { (char *)"Float",      D_SELECTED,  linktile_water_float_list, 0, NULL },
    { (char *)"Swim",       0,           linktile_water_swim_list, 0, NULL },
    { (char *)"Dive",       0,           linktile_water_dive_list, 0, NULL },
    { (char *)"Hold",       0,           linktile_water_hold_list, 0, NULL },
    { NULL,                 0,           NULL,                     0, NULL }
};

static int linktile_land_list[] =
{
    // dialog control number
    9, -1
};

static int linktile_water_list[] =
{
    // dialog control number
    10, -1
};

static TABPANEL linktile_tabs[] =
{
    // (text)
    { (char *)"Land",       D_SELECTED,   linktile_land_list, 0, NULL },
    { (char *)"Water",      0,            linktile_water_list, 0, NULL },
    { NULL,                 0,            NULL,                0, NULL }
};

const char *animationstyles[las_max]= { "Original", "BS-Zelda", "Zelda 3", "Zelda 3 (Slow Walk)" };

const char *animationstylelist(int index, int *list_size)
{
    if(index>=0)
    {
        return animationstyles[index];
    }
    
    *list_size=las_max;
    return NULL;
}

const char *swimspeeds[2]= { "Slow", "Fast" };

const char *swimspeedlist(int index, int *list_size)
{
    if(index>=0)
    {
        return swimspeeds[index];
    }
    
    *list_size=2;
    return NULL;
}

int jwin_as_droplist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_droplist_proc(msg,d,c);
    
    switch(msg)
    {
    case MSG_CHAR:
    case MSG_CLICK:
        zinit.linkanimationstyle=d->d1;
        
        if(zinit.linkanimationstyle==las_zelda3slow)
        {
            link_animation_speed=2;
        }
        else
        {
            link_animation_speed=1;
        }
    }
    
    return ret;
}

static ListData animationstyle_list(animationstylelist, &font);
static ListData swimspeed_list(swimspeedlist, &font);

static DIALOG linktile_dlg[] =
{
    // (dialog proc)                       (x)     (y)     (w)     (h)    (fg)                     (bg)                 (key)     (flags)    (d1)        (d2)              (dp)                             (dp2)   (dp3)
    {  jwin_win_proc,                        0,      0,    320,    240,    vc(14),                 vc(1),                   0,    D_EXIT,     0,          0, (void *) "Link Sprites",         NULL,   NULL                   },
    {  d_vsync_proc,                         0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   },
    {  d_keyboard_proc,                      0,      0,      0,      0,    0,                      0,                       0,    0,          KEY_F1,     0, (void *) onHelp,                 NULL,   NULL                   },
    {  jwin_button_proc,                    90,    215,     61,     21,    vc(14),                 vc(1),                  13,    D_EXIT,     0,          0, (void *) "OK",                   NULL,   NULL                   },
    {  jwin_button_proc,                   170,    215,     61,     21,    vc(14),                 vc(1),                  27,    D_EXIT,     0,          0, (void *) "Cancel",               NULL,   NULL                   },
    // 5
    {  jwin_check_proc,                    217,    200,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Large Link Hit Box",   NULL,   NULL                   },
    {  jwin_text_proc,                       4,    201,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Animation Style:",     NULL,   NULL                   },
    {  jwin_as_droplist_proc,               77,    197,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void *) &animationstyle_list,   NULL,   NULL                   },
    {  jwin_tab_proc,                        4,     25,    312,    150,    0,                      0,                       0,    0,          0,          0, (void *) linktile_tabs,          NULL, (void *)linktile_dlg   },
    // 9
    {  jwin_tab_proc,                        7,     46,    305,    125,    0,                      0,                       0,    0,          0,          0, (void *) linktile_land_tabs,     NULL, (void *)linktile_dlg   },
    {  jwin_tab_proc,                        7,     46,    305,    125,    0,                      0,                       0,    0,          0,          0, (void *) linktile_water_tabs,    NULL, (void *)linktile_dlg   },
    // 11 (walk sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 15 (walk sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_walk,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_walk,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_walk,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_walk,         NULL,                            NULL,   NULL                   },
    // 19 (slash spritetitles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 23 (slash sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_slash,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_slash,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_slash,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_slash,        NULL,                            NULL,   NULL                   },
    // 27 (stab sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 31 (stab sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_stab,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_stab,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_stab,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_stab,         NULL,                            NULL,   NULL                   },
    // 35 (pound sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 39 (pound sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_pound,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_pound,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_pound,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_pound,        NULL,                            NULL,   NULL                   },
    // 43 (hold sprite titles)
    {  jwin_rtext_proc,                     67,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "One Hand",             NULL,   NULL                   },
    {  jwin_rtext_proc,                     67,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Two Hands",            NULL,   NULL                   },
    // 45 (hold sprites)
    {  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_landhold1,    NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        70,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_landhold2,    NULL,                            NULL,   NULL                   },
    // 47 (casting sprites)
    {  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_cast,         NULL,                            NULL,   NULL                   },
    
    // 48 (float sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 52 (float sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_float,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_float,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_float,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_float,        NULL,                            NULL,   NULL                   },
    // 56 (swim sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 60 (swim sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_swim,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_swim,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_swim,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_swim,         NULL,                            NULL,   NULL                   },
    // 64 (dive sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 68 (dive sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_dive,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_dive,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_dive,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_dive,         NULL,                            NULL,   NULL                   },
    // 72 (hold sprite titles)
    {  jwin_rtext_proc,                     67,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "One Hand",             NULL,   NULL                   },
    {  jwin_rtext_proc,                     67,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Two Hands",            NULL,   NULL                   },
    // 74 (hold sprites)
    {  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_waterhold1,   NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        70,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_waterhold2,   NULL,                            NULL,   NULL                   },
    {  jwin_check_proc,                    217,    186,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Diagonal Movement",    NULL,   NULL                   },
    // 77 (jump sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 81 (jump sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_jump,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_jump,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_jump,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_jump,         NULL,                            NULL,   NULL                   },
    // 85 (charge sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 89 (charge sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_charge,       NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_charge,       NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_charge,       NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_charge,       NULL,                            NULL,   NULL                   },
    // 93
    {  d_timer_proc,                         0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   },
    {  jwin_text_proc,                       4,    183,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Swim Speed:",          NULL,   NULL                   },
    {  jwin_droplist_proc,                  77,    179,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void *) &swimspeed_list,        NULL,   NULL                   },
    {  NULL,                                 0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   }
};



int onCustomLink()
{
    //setuplinktiles(zinit.linkanimationstyle);
    if(zinit.linkanimationstyle==las_zelda3slow)
    {
        link_animation_speed=2;
    }
    else
    {
        link_animation_speed=1;
    }
    
    linktile_dlg[0].dp2=lfont;
    linktile_dlg[5].flags = get_bit(quest_rules, qr_LTTPCOLLISION)? D_SELECTED : 0;
    linktile_dlg[76].flags = get_bit(quest_rules, qr_LTTPWALK)? D_SELECTED : 0;
    linktile_dlg[95].d1=(zinit.link_swim_speed<60)?0:1;
    linktile_dlg[7].d1=zinit.linkanimationstyle;
    
	DIALOG *linktile_cpy = resizeDialog(linktile_dlg, 2.0);
        
    int oldWalkSpr[4][3];
    int oldStabSpr[4][3];
    int oldSlashSpr[4][3];
    int oldFloatSpr[4][3];
    int oldSwimSpr[4][3];
    int oldDiveSpr[4][3];
    int oldPoundSpr[4][3];
    int oldJumpSpr[4][3];
    int oldChargeSpr[4][3];
    int oldCastSpr[3];
    int oldHoldSpr[2][2][3];
    memcpy(oldWalkSpr, walkspr, 4*3*sizeof(int));
    memcpy(oldStabSpr, stabspr, 4*3*sizeof(int));
    memcpy(oldSlashSpr, slashspr, 4*3*sizeof(int));
    memcpy(oldFloatSpr, floatspr, 4*3*sizeof(int));
    memcpy(oldSwimSpr, swimspr, 4*3*sizeof(int));
    memcpy(oldDiveSpr, divespr, 4*3*sizeof(int));
    memcpy(oldPoundSpr, poundspr, 4*3*sizeof(int));
    memcpy(oldJumpSpr, jumpspr, 4*3*sizeof(int));
    memcpy(oldChargeSpr, chargespr, 4*3*sizeof(int));
    memcpy(oldCastSpr, castingspr, 3*sizeof(int));
    memcpy(oldHoldSpr, holdspr, 2*2*3*sizeof(int));
    
    
    int ret = popup_dialog_through_bitmap(screen2, linktile_cpy,3);
    
    if(ret==3)
    {
        saved=false;
        set_bit(quest_rules, qr_LTTPCOLLISION, (linktile_cpy[5].flags&D_SELECTED)?1:0);
        set_bit(quest_rules, qr_LTTPWALK, (linktile_cpy[76].flags&D_SELECTED)?1:0);
        zinit.link_swim_speed=(linktile_cpy[95].d1==0)?50:67;
    }
    else
    {
        memcpy(walkspr, oldWalkSpr, 4*3*sizeof(int));
        memcpy(stabspr, oldStabSpr, 4*3*sizeof(int));
        memcpy(slashspr, oldSlashSpr, 4*3*sizeof(int));
        memcpy(floatspr, oldFloatSpr, 4*3*sizeof(int));
        memcpy(swimspr, oldSwimSpr, 4*3*sizeof(int));
        memcpy(divespr, oldDiveSpr, 4*3*sizeof(int));
        memcpy(poundspr, oldPoundSpr, 4*3*sizeof(int));
        memcpy(jumpspr, oldJumpSpr, 4*3*sizeof(int));
        memcpy(chargespr, oldChargeSpr, 4*3*sizeof(int));
        memcpy(castingspr, oldCastSpr, 3*sizeof(int));
        memcpy(holdspr, oldHoldSpr, 2*2*3*sizeof(int));
    }

	delete[] linktile_cpy;
    
    ret=ret;
    return D_O_K;
}

void center_zq_custom_dialogs()
{
    jwin_center_dialog(itemdata_dlg);
    jwin_center_dialog(linktile_dlg);
    jwin_center_dialog(wpndata_dlg);
    jwin_center_dialog(enedata_dlg);
}

