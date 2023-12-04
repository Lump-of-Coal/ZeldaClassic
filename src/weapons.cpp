//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>

#include "weapons.h"
#include "zelda.h"
#include "zsys.h"
#include "maps.h"
#include "tiles.h"
#include "pal.h"
#include "link.h"
#include "mem_debug.h"
#include "backend/AllBackends.h"
#include "guys.h"

extern LinkClass *Link;
extern zinitdata zinit;

/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

byte boomframe[16] = {0,0,1,0,2,0,1,1,0,1,1,3,2,2,1,2};
byte bszboomflip[4] = {0,2,3,1};

int EwpnsIdCount(int id)
{
    int ret = 0;
    for (int i = 0; i < Ewpns.Count(); i++)
    {
        if (((weapon *)Ewpns.spr(i))->id == id)
            ret++;
    }
    return ret;
}

int LwpnsIdCount(int id)
{
    int ret = 0;
    for (int i = 0; i < Lwpns.Count(); i++)
    {
        if (((weapon *)Lwpns.spr(i))->id == id)
            ret++;
    }
    return ret;
}
//light up the screen if there is at least one "lit" weapon
//otherwise darken the screen
void checkLightSources()
{
    for(int i=0; i<Lwpns.Count(); i++)
    {
        if(((weapon *)Lwpns.spr(i))->isLit)
        {
            lighting(true);
            return;
        }
    }
    
    for(int i=0; i<Ewpns.Count(); i++)
    {
        if(((weapon *)Ewpns.spr(i))->isLit)
        {
            lighting(true);
            return;
        }
    }
    
    lighting(false);
}

void getdraggeditem(int j)
{
    item *it=(item*)items.spr(j);
    
    if(it==NULL)
        return;
        
    it->x = LinkX();
    it->y = LinkY();
    it->z = LinkZ();
    LinkCheckItems();
}

void weapon::seekLink()
{
    angular = true;
    angle = atan2(double(LinkY()-y),double(LinkX()-x));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>(PI/2))   dir=l_down;
    else                  dir=r_down;
    
    if(z>LinkZ()) z--;
    else if(z<LinkZ()) z++;
}

void weapon::seekEnemy(int j)
{
    angular = true;
    fix mindistance=(fix)1000000;
    fix tempdistance;
    
    if((j==-1)||(j>=GuyCount()))
    {
        j=-1;
        
        for(int i=0; i<GuyCount(); i++)
        {
            //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
            tempdistance=distance(x,y,GuyX(i),GuyY(i));
            
            if((tempdistance<mindistance)
                &&
                curQuest->getEnemyDefinition(GuyID(i)).family != eeGUY && !GuySuperman(i))
            {
                mindistance=tempdistance;
                j=i;
            }
        }
    }
    
    if(j==-1)
    {
        return;
    }
    
    angle = atan2(double(GuyY(j)-y),double(GuyX(j)-x));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>PI/2)   dir=l_down;
    else                  dir=r_down;
}

int weapon::seekEnemy2(int j)
{
    angular = true;
    fix mindistance=(fix)1000000;
    fix tempdistance;
    
    if((j==-1)||(j>=GuyCount()))
    {
        j=-1;
        
        for(int i=0; i<GuyCount(); i++)
        {
            //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
            tempdistance=distance(dummy_fix[0],dummy_fix[1],GuyX(i),GuyY(i));
            
            if((tempdistance<mindistance)
                && curQuest->getEnemyDefinition(GuyID(i)).family != eeGUY && !GuySuperman(i))
            {
                mindistance=tempdistance;
                j=i;
            }
        }
    }
    
    if(j==-1)
    {
        return j;
    }
    
    //al_trace("Guy: %d, gx: %f, gy: %f, x: %f, y: %f\n", j, float(GuyX(j)), float(GuyY(j)), float(dummy_fix[0]), float(dummy_fix[1]));
    angle = atan2(double(GuyY(j)-dummy_fix[1]),double(GuyX(j)-dummy_fix[0]));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>PI/2)   dir=l_down;
    else                  dir=r_down;
    
    return j;
}

weapon::weapon(weapon const & other):

    //Struct Element			Type		Purpose
    sprite(other),
    power(other.power), 		//int
    type(other.type), 			//int
    dead(other.dead),			//int
    clk2(other.clk2),			//int
    misc2(other.misc2),			//int
    ignorecombo(other.ignorecombo),	//int
    id(other.id),
    isLit(other.isLit),			//bool		Does it light the screen?
    parentid(other.parentid),		//int		Enemy that created it. -1 for none. This is the Enemy POINTER, not the Enemy ID. 
    parentitem(other.parentitem),	//int		Item that created it. -1 for none. 
    dragging(other.dragging),		//int draggong		?
    step(other.step),			//fix		Speed of movement
    bounce(other.bounce),		//bool		Boomerang, or hookshot bounce. 
    ignoreLink(other.ignoreLink),	//bool		?
    flash(other.flash),			//word		Is it flashing?
    wid(other.wid),			//word		ID
    aframe(other.aframe),		//word		Anim frame
    csclk(other.csclk),			//word		CSet flash clk (?)
    o_tile(other.o_tile),		//int		The base item tile
    o_cset(other.o_cset),		//int		The CSet		
    o_speed(other.o_speed),		//int		Original anim (?) speed.
    o_type(other.o_type),		//int		The weapon ID (type)
    frames(other.frames),		//int		Frames of the anim cycle
    o_flip(other.o_flip),		//int		The original flip/orientationn
    temp1(other.temp1),			//int		Misc var.
    behind(other.behind),		//bool		Should it be drawn behind Link, NPC, and other sprites?
    minX(other.minX),			//int		How close can the weapon get tot he edge of the screen
    maxX(other.maxX),			//int		...before being deleted or bouncing
    minY(other.minY),			//int		...
    maxY(other.maxY),			//int		...
	
    //! Dimentio Wand
    /*
    //!Dimentio: These 5 exist both here and in the header file. If you remove these, don't forget to
    remove them over there as well.
    */
    count1(other.count1), 		//int		Dimentio Wand 
    count2(other.count2), 		//int		Dimentio Wand 
    count3(other.count3), 		//int		Dimentio Wand
    count4(other.count4), 		//int		Dimentio Wand
    count5(other.count5), 		//int		Dimentio Wand
	
    //Weapon Editor -Z
    useweapon(other.useweapon),		//byte		The weapon editor weapon type.
    usedefence(other.usedefence),	//byte		The defence type to evaluate in do_enemy_hit()
    weaprange(other.weaprange),		//int		The range or distance of the weapon before removing it. 
    weapduration(other.weapduration),	//int		The number of frames that must elapse before removing it
    weaponscript(other.weaponscript),	//word		The weapon action script. 
    tilemod(other.tilemod),		//long		The LTM to use when the weapon is active. 
    drawlayer(other.drawlayer),		//byte		The layer onto which we draw the weapon.
    family_class(other.family_class),	//byte		Item Class
    family_level(other.family_level),	//byte		Item Level
    flags(other.flags),			//word		A misc flagset. 
    collectflags(other.collectflags)	//long		A flagset that determines of the weapon can collect an item.
    
	
	//End Weapon editor non-arrays. 

     
{
	//weapname = other.weapname;		//char[128]	The name of the weapon. 
    for(int i=0; i<10; ++i)
    {
        dummy_int[i]=other.dummy_int[i];
        dummy_fix[i]=other.dummy_fix[i];
        dummy_float[i]=other.dummy_float[i];
        dummy_bool[i]=other.dummy_bool[i];
    }
    
    //Weapon Editor Arrays
    for ( int q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) 
    {
	weap_pattern[q] = other.weap_pattern[q];	//int	The movement pattern and args.
    }
    for ( int q = 0; q < WEAPON_CLOCKS; q++ ) 
    {
	clocks[q] = other.clocks[q];		//long	An array of misc clocks. 
    }
    for ( int q = 0; q < INITIAL_A; q++ )
    {
	initiala[q] = other.initiala[q];		//byte	InitA[]
    }
    for ( int q = 0; q < INITIAL_D; q++ ) 
    {
	initiald[q] = other.initiald[q];		//long	InitD[]
    }
    for ( int q = 0; q < FFSCRIPT_MISC; q++ ) 
    {
	ffmisc[q] = other.ffmisc[q];		//long -The base wpn->Misc[32] set from the editor
    }
    
   
    
	//! END Weapon Editor
    
    /*for (int i=0; i<8; ++i)
    {
      d[i]=other.d[i];
    }
    for (int i=0; i<2; ++i)
    {
      a[i]=other.a[i];
    }*/
    
    //Enemy Editor Weapon Sprite
    wpnsprite = other.wpnsprite;
    //if ( parentid > 0 ) wpnsprite = guysbuf[parentid].wpnsprite;
    //else wpnsprite  = -1;
}

// Let's dispose of some sound effects!
weapon::~weapon()
{
    // First, check for the existence of weapons that don't have parentitems
    // but make looping sounds anyway.
    if(!curQuest->isValid(parentitem) && get_bit(quest_rules, qr_MORESOUNDS))
    {
        //I am reasonably confident that I fixed these expressions. ~pkmnfrk
        if(id==ewBrang && EwpnsIdCount(ewBrang) > 0)
            return;
            
        if(id==wWind && LwpnsIdCount(wWind) > 0)
            return;
    }
    
    // Check each Lwpn to see if this weapon's sound is also allocated by it.
    if(curQuest->isValid(parentitem))
    {
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = ((weapon *)Lwpns.spr(i));
            
            if(w->getUID() == getUID())  // Itself!
            {
                continue;
            }
            
            ItemDefinitionRef wparent = w->parentitem;
            
            if(curQuest->isValid(wparent) && 
                (curQuest->getItemDefinition(wparent).family == itype_brang || curQuest->getItemDefinition(wparent).family == itype_nayruslove
                              || curQuest->getItemDefinition(wparent).family == itype_hookshot || curQuest->getItemDefinition(wparent).family == itype_cbyrna))
            {
                if(curQuest->getItemDefinition(wparent).usesound == curQuest->getItemDefinition(parentitem).usesound)
                    return;
            }
        }
    }
    
    switch(id)
    {
    case wWind:
        Backend::sfx->stop(WAV_ZN1WHIRLWIND);
        break;
        
    case ewBrang:
        //stop_sfx(WAV_BRANG); //causes a bug -L
        break;
        
    case wBrang:
    case wCByrna:
        if(curQuest->isValid(parentitem))
        {
            Backend::sfx->stop(curQuest->getItemDefinition(parentitem).usesound);
        }
        
        break;
        
    case wSSparkle:
    case wFSparkle:
        if(curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).family==itype_cbyrna)
        {
            Backend::sfx->stop(curQuest->getItemDefinition(parentitem).usesound);
        }
        
        break;
    }
}

// This should be done elsewhere
void setScreenLimits(weapon& w)
{
	bool isDgn = isdungeon() != 0;
	int border = get_bit(quest_rules, qr_NOBORDER) ? 16 : 0;

	if (w.id>wEnemyWeapons && w.id != ewBrang)
	{
		w.minY = isDgn ? 32 : (16 - border);
		w.maxY = isDgn ? 128 : (144 + border);
		w.minX = isDgn ? 32 : (16 - border);
		w.maxX = isDgn ? 208 : (224 + border);
	}
	else if (w.id == wHookshot || w.id == wHSChain)
	{
		w.minY = isDgn ? 8 : 0;
		w.maxY = isDgn ? 152 : 160;
		w.minX = isDgn ? 8 : 0;
		w.maxX = isDgn ? 248 : 256;
	}
	else
	{
		w.minY = isDgn ? 18 : 2;
		w.maxY = isDgn ? 144 : 160;
		w.minX = isDgn ? 20 : 4;
		w.maxX = isDgn ? 220 : 236;
	}

	if (w.id == wSSparkle || w.id == wFSparkle)
	{
		w.minY = 0;
		w.maxY = 176;
		w.minX = 0;
		w.maxX = 256;
	}
	else if (w.id == ewFlame)
	{
		w.minY = isDgn ? 32 : (16 - border);
		w.maxY = isDgn ? 128 : (144 + border);
		w.minX = isDgn ? 32 : (16 - border);
		w.maxX = isDgn ? 208 : (224 + border);
	}
	else if (w.id == ewFireTrail)
	{
		w.minY = isDgn ? 32 : (16 - border);
		w.maxY = isDgn ? 128 : (144 + border);
		w.minX = isDgn ? 32 : (16 - border);
		w.maxX = isDgn ? 208 : (224 + border);
	}

	else if (w.id == ewWind)
	{
		w.minY = isDgn ? 32 : (16 - border);
		w.maxY = isDgn ? 128 : (144 + border);
		w.minX = isDgn ? 32 : (16 - border);
		w.maxX = isDgn ? 208 : (224 + border);
	}
}

weapon::weapon(fix X,fix Y,fix Z,int Id,int Type,int pow,int Dir, const ItemDefinitionRef &Parentitem, int prntid, bool isDummy) : sprite(*pool), parentid(prntid)
{
    x=X;
    y=Y;
    z=Z;
    id=Id;
    type=Type;
    power=pow;
    parentitem=Parentitem;
    dir=zc_max(Dir,0);
    clk=clk2=flip=misc=misc2=0;
    frames=flash=aframe=csclk=0;
    wid = SpriteDefinitionRef();
    ignorecombo=-1;
    step=0;
    dead=-1;
    bounce=ignoreLink=false;
    yofs=playing_field_offset - 2;
    dragging=-1;
    hxsz=15;
    hysz=15;
    hzsz=8;
    isLit = false;
	setScreenLimits(*this);
	
	//2.6 ZScript
    
    //We might need to clear scriptrange. -Z
    scriptrange = -1; //we'll try -1 so that 0 does not interfere.
    //Pewrhaps check the weapon editor properties, and see if it has a range; only for weapons
    //that use misc1 for this.
	
	//Weapon Editor -Z
	if ( curQuest->isValid(parentitem)) { 
		useweapon = curQuest->getItemDefinition(parentitem).useweapon;
		usedefence = curQuest->getItemDefinition(parentitem).usedefence;
		weaprange = curQuest->getItemDefinition(parentitem).weaprange;
		weapduration = curQuest->getItemDefinition(parentitem).weapduration;
		duplicates = curQuest->getItemDefinition(parentitem).duplicates;
		family_class = curQuest->getItemDefinition(parentitem).family;
		family_level = curQuest->getItemDefinition(parentitem).fam_type;
		//flags = 
		collectflags = curQuest->getItemDefinition(parentitem).collectflags;
		tilemod = curQuest->getItemDefinition(parentitem).ltm;
		
		for ( int q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ )  
		{
			weap_pattern[q] = curQuest->getItemDefinition(parentitem).weap_pattern[q];
		}
		
		for ( int q = 0; q < FFSCRIPT_MISC; q++ )
		{
			wpn_misc_d[q] = curQuest->getItemDefinition(parentitem).wpn_misc_d[q];
			
		}
		//hxofs = itemsbuf[parentitem].weap_hxofs; //hit x offset
		//hyofs = itemsbuf[parentitem].weap_yxofs; //hit y offset
		//hxsz = (itemsbuf[parentitem].weap_hxsz > 0 ? itemsbuf[parentitem].weap_hxsz : 15); //hitwidth
		//hysz = (itemsbuf[parentitem].weap_hysz > 0 ? itemsbuf[parentitem].weap_hysz : 15);  //hitheight
		//hzsz = (itemsbuf[parentitem].weap_hzsz > 0 ? itemsbuf[parentitem].weap_hzsz : 8);   //HitZHeight / HitDepth
		//xofs = itemsbuf[parentitem].weap_xofs; //drawxoffset
		//yofs = itemsbuf[parentitem].weap_yofs; //drawyoffset
		//weaponscript = itemsbuf[parentitem].weaponscript;
		
		for ( int q = 0; q < INITIAL_D; q++ )
		{
			initiald[q] = curQuest->getItemDefinition(parentitem).weap_initiald[q];
		}
		for ( int q = 0; q < INITIAL_A; q++ )
		{
			initiala[q] = curQuest->getItemDefinition(parentitem).weap_initiala[q];
		}
			
			
	}
    
    SpriteDefinitionRef defaultw;
    ItemDefinitionRef itemid = parentitem;
    
    if(id>wEnemyWeapons)
    {
        canfreeze=true;
        
        if(id!=ewBrang)
        {
            if(Type&2)
            {
                misc=(Type>>3)-1;
                type &= ~2;
            }
            else
                misc=-1;
        }
    }
    
    //! Dimentio Wand
    
    if (curQuest->isValid(parentitem))
    {
        if (curQuest->getItemDefinition(parentitem).family == itype_wand && (id != wWand || (curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG3)))
        { //!Dimentio: This calculates the move effects. These are modifiers to normal wand weapon's movement. 
            //Turn on flag 3 to include the wand with it.
            switch (curQuest->getItemDefinition(parentitem).misc5)
            {
            case 1:
            case 2:
            case 3:
            {
                switch (dir)
                {
                case up: this->count1 = 90;
                    break;
                case down: this->count1 = 270;
                    break;
                case left: this->count1 = 180;
                    break;
                case right: this->count1 = 0;
                    break;
                }
                this->count3 = dir;
            }
            default: break;
            }
        }
        if (type == 1 && curQuest->getItemDefinition(parentitem).misc4 > 0 && curQuest->getItemDefinition(parentitem).family == itype_book)
        {
            switch (curQuest->getItemDefinition(parentitem).misc4)
            {
            case 5:
            {
                if (GuyCount() > 0) seekEnemy(-1);
                break;
            }
            case 6:
            {
                if (GuyCount() > 0) seekEnemy(-1);
                break;
            }
            case 7:
            {
                seekLink();
                break;
            }
            case 8:
            {
                seekLink();
                break;
            }
            }
        }

        if (type == 1 && curQuest->getItemDefinition(parentitem).misc4 > 0 && curQuest->getItemDefinition(parentitem).family == itype_book)
        {
            switch (curQuest->getItemDefinition(parentitem).misc4)
            {
            case 5:
            {
                if (GuyCount() > 0)
                {
                    fix StepSaving = curQuest->getItemDefinition(parentitem).misc5 / (fix)100.0;
                    step = StepSaving;
                }
                break;
            }
            case 6:
            {
                if (GuyCount() > 0)
                {
                    fix StepSaving = curQuest->getItemDefinition(parentitem).misc5 / (fix)100.0;
                    step = StepSaving;
                }
                break;
            }
            case 7:
            {
                fix StepSaving = curQuest->getItemDefinition(parentitem).misc5 / (fix)100.0;
                step = StepSaving;
                seekLink();
                break;
            }
            case 8:
            {
                fix StepSaving = curQuest->getItemDefinition(parentitem).misc5 / (fix)100.0;
                step = StepSaving;
                seekLink();
                break;
            }
            }
        }
    }
    
    switch(id)
    {
    case wSword: // Link's sword
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_sword);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        hxsz=hysz=15;
        LOADGFX(defaultw);
        break;
    }
    
    case wWand: // Link's wand, as well as the Cane itself
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_wand);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        hxsz=15;
        hysz=15; //hysz=24;
        LOADGFX(defaultw);
        break;
    }
    
    case wHammer:
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_hammer);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        hxsz=15;
        hysz=24;
        LOADGFX(defaultw);
        break;
    }
    
    case wCByrna: // The Cane's beam
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_cbyrna);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[2];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        int speed = curQuest->isValid(parentitem) ? zc_max(curQuest->getItemDefinition(parentitem).misc1,1) : 1;
        int qty = curQuest->isValid(parentitem) ? zc_max(curQuest->getItemDefinition(parentitem).misc3,1) : 1;
        clk = (int)((((2*type*PI)/qty)
                     // Appear on top of the cane's hook
                     + (dir==right? 3*PI/2 : dir==left? PI/2 : dir==down ? 0 : PI))*speed);
        type = 0;
        
        if(curQuest->isValid(parentitem))
        {
            Backend::sfx->loop(curQuest->getItemDefinition(parentitem).usesound,128);
        }
        
        break;
    }
    
    case wWhistle:
        xofs=1000;                                            // don't show
        x=y=hxofs=hyofs=0;
        hxsz=hysz=255;                                        // hit the whole screen
        break;
        
    case wWind:
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_whistle);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        
        if(get_bit(quest_rules,qr_MORESOUNDS))
            Backend::sfx->loop(WAV_ZN1WHIRLWIND,128);
            
        clk=-14;
        step=2;
        break;
    }
    
    case wBeam:
    case wRefBeam:
        step = 3;
        
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_sword);
        }
        
        if (curQuest->isValid(itemid) && id != wRefBeam)
            defaultw = curQuest->getItemDefinition(itemid).wpns[2];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        flash = 1;
        cs = 6;
        
        switch(dir)
        {
        case down:
            flip=get_bit(quest_rules,qr_SWORDWANDFLIPFIX)?3:2;
            
        case up:
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            hxofs=2;
            hxsz=12;
            yofs = playing_field_offset+(BSZ ? 3 : 1);
            break;
        }
        
        if(id==wRefBeam)
        {
            ignorecombo=(((int)y&0xF0)+((int)x>>4));
        }
        
        break;
        
    case wArrow:
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_arrow);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        step=3;
        
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset + 1;
            hyofs=2;
            hysz=14;
            hxofs=2;
            hxsz=12;
            break;
        }
        
        if(curQuest->isValid(itemid))
            misc = curQuest->getItemDefinition(itemid).misc1;
            
        break;
        
    case wSSparkle:
    {
        SpriteDefinitionRef ssparkles = curQuest->specialSprites().silverSparkle;
        LOADGFX(ssparkles);
        step = 0;
        break;
    }
        
    case wFSparkle:
    {
        SpriteDefinitionRef fsparkles = curQuest->specialSprites().fireSparkle;
        LOADGFX(fsparkles);
        step = 0;
        break;
    }
        
    case wFire:
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_candle);
        }
        
        switch(type)
        {
        case 3: // Din's Fire. This uses magicitem rather than itemid
            if(curQuest->isValid(magicitem) && !isDummy)
                defaultw = curQuest->getItemDefinition(magicitem).wpns[4];
            else defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
            break;
            
        case 2: // Wand
            if(curQuest->isValid(itemid) && !isDummy)
                defaultw = curQuest->getItemDefinition(itemid).wpns[1];
            else defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
            break;
            
        case 0:
        case 1: // Candles
            if(curQuest->isValid(itemid) && !isDummy)
            {
                defaultw = curQuest->getItemDefinition(itemid).wpns[2];
                break;
            }
            
        default:
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
        }
        
        LOADGFX(defaultw);
        step = (type<2)?.5:0;
        hxofs = hyofs=1;
        hxsz = hysz = 14;
        
        if(BSZ)
            yofs+=2;
            
        break;
        
    case wLitBomb:
    case wBomb:
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            if (curQuest->getItemDefinition(parentitem).family == itype_wand || curQuest->getItemDefinition(parentitem).family == itype_book) itemid = parentitem;
		//!Dimentio: Bomb exceptions, to prevent them from being naughty with the new wand.
            else itemid = curQuest->getCanonicalItemID(itype_bomb);
        }
        
        if (curQuest->isValid(itemid))
        {
            if (curQuest->getItemDefinition(parentitem).family == itype_wand)
            {
                defaultw = curQuest->getItemDefinition(itemid).wpns[2]; //!Dimentio: Here too.
                //! ZoriaRPG: I need to know what value wpn3 is doing here. 
                misc = (id == wBomb ? 1 : curQuest->getItemDefinition(itemid).misc2);
            }
            else if (curQuest->getItemDefinition(parentitem).family == itype_book)
            {
                defaultw = curQuest->getItemDefinition(itemid).wpns[1]; //!Dimentio: Here too.
                //! ZoriaRPG: I need to know what value wpn3 is doing here. 
                misc = (id == wBomb ? 1 : curQuest->getItemDefinition(itemid).misc2);
            }
            else {
                defaultw = curQuest->getItemDefinition(itemid).wpns[0];
                misc = (id == wBomb ? 1 : curQuest->getItemDefinition(itemid).misc1);
            }
        }
        else
        {
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            misc = (id==wBomb ? 1 : 50);
        }
        
        LOADGFX(defaultw);
        hxofs=hyofs=4;
        hxsz=hysz=8;
        break;
    }
    
    case wLitSBomb:
    case wSBomb:
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
	    if (curQuest->getItemDefinition(parentitem).family == itype_wand) itemid = parentitem; 
		//!Dimentio: Bomb exceptions, to prevent them from being naughty with the new wand.
            else itemid = curQuest->getCanonicalItemID(itype_sbomb);
        }
        
        if(curQuest->isValid(itemid))
        {
			if (curQuest->getItemDefinition(parentitem).family == itype_wand){ 
				defaultw = curQuest->getItemDefinition(itemid).wpns[2]; 
				//!Dimentio: Here too.
				//! ZoriaRPG: I need to know what value wpn3 is doing here. 
				misc = (id==wSBomb ? 1 : curQuest->getItemDefinition(itemid).misc2);
			}
            else {
				defaultw = curQuest->getItemDefinition(itemid).wpns[0];
				misc = (id==wSBomb ? 1 : curQuest->getItemDefinition(itemid).misc1);
			}
        }
        else
        {
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            misc = (id==wSBomb ? 1 : 50);
        }
        
        LOADGFX(defaultw);
        hxofs=hyofs=4;
        hxsz=hysz=8;
        break;
    }
    
    case wBait:
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_bait);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        break;
        
    case wMagic:
    {
        itemid = current_item_id(itype_book);
        bool book = true;
        
        if(!curQuest->isValid(itemid))
        {
            itemid = current_item_id(itype_wand);
            book = false;
        }
        
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_wand);
            book = false;
        }
        
        if (curQuest->isValid(itemid))
        {
            // Book Magic sprite is wpn, Wand Magic sprite is wpn3.
            if (curQuest->getItemDefinition(parentitem).family == itype_book && type == 1) defaultw = curQuest->getItemDefinition(parentitem).wpns[2];
            //!Dimentio: Okay, is it created by the book?
            else defaultw = book ? curQuest->getItemDefinition(itemid).wpns[0] : curQuest->getItemDefinition(itemid).wpns[2];
        }
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        step = (BSZ ? 3 : 2.5);
        
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            hxofs=2;
            hxsz=12;
            break;
        }
        
        break;
    }
    
    case wBrang:
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_brang);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultBoomerangSprites[0];
            
        LOADGFX(defaultw);
        hxofs=4;
        hxsz=7;
        hyofs=2;
        hysz=11;
        dummy_bool[0]=false;                                  //grenade armed?
        break;
        
    case wHookshot:
        hookshot_used=true;
        
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_hookshot);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[0];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        step = 4;
        clk2=256;
        
        switch(dir)
        {
        case down:
            flip=2;
            xofs+=4;
            yofs+=1;
            hyofs=2;
            hysz=12;
            break;
            
        case up:
            yofs+=3;
            xofs-=5;
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1; /*tile=o_tile+((frames>1)?frames:1)*/update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs+=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs-=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
        }
        
        break;
        
    case wHSHandle:
        step = 0;
        
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_hookshot);
        }
        
        if(curQuest->isValid(itemid))
            defaultw = curQuest->getItemDefinition(itemid).wpns[3];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        LOADGFX(defaultw);
        
        switch(dir)
        {
        case down:
            flip=2;
            xofs+=4;
            yofs+=1;
            hyofs=2;
            hysz=12;
            break;
            
        case up:
            yofs+=3;
            xofs-=5;
            hyofs=2;
            hysz=12;
            break;
            
        case left:
            flip=1; /*tile=o_tile+((frames>1)?frames:1)*/update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs+=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            xofs-=2;
            yofs=playing_field_offset+4;
            hxofs=2;
            hxsz=12;
            break;
        }
        
        break;
        
    case wHSChain:
    {
        if(isDummy || !curQuest->isValid(itemid))
        {
            itemid = curQuest->getCanonicalItemID(itype_hookshot);
        }
        
        if (curQuest->isValid(itemid))
            defaultw = (dir < left) ? curQuest->getItemDefinition(itemid).wpns[2] : curQuest->getItemDefinition(itemid).wpns[1];
        else
            defaultw = curQuest->specialSprites().defaultLinkWeaponSprite;
            
        step = 0;
        
        switch(dir)
        {
        case down:
            LOADGFX(defaultw);
            xofs+=4;
            yofs-=7;
            break;
            
        case up:
            LOADGFX(defaultw);
            xofs-=5;
            yofs+=11;
            break;
            
        case left:
            LOADGFX(defaultw);
            xofs+=10;
            yofs=playing_field_offset+4;
            break;
            
        case right:
            LOADGFX(defaultw);
            xofs-=10;
            yofs=playing_field_offset+4;
            break;
        }
    }
    break;
    
    case ewLitBomb:
    case ewBomb:
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) )
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef ebombs = curQuest->specialSprites().defaultEnemyBomb;
            LOADGFX(ebombs);
        }
	}
	
    else
    {
        SpriteDefinitionRef ebombs = curQuest->specialSprites().defaultEnemyBomb;
        LOADGFX(ebombs);
    }

        hxofs=0;
        hxsz=16;
        
		if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hyofs=0;
			hysz=16;
		}
        
        if(id==ewBomb)
            misc=2;
        else
            step=3;
            
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            break;
        }
        
        break;
        
    case ewLitSBomb:
    case ewSBomb:
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite)) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef esbombs = curQuest->specialSprites().defaultEnemySuperBomb;
            LOADGFX(esbombs);
        }
	}
    else
    {
        SpriteDefinitionRef esbombs = curQuest->specialSprites().defaultEnemySuperBomb;
        LOADGFX(esbombs);
    }
        hxofs=0;
        hxsz=16;
		if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hyofs=0;
			hysz=16;
		}
        
        if(id==ewSBomb)
            misc=2;
        else
            step=3;
            
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            break;
        }
        
        break;
        
    case ewBrang:
    {
        if (get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs = 0;
            hxsz = 16;
            hyofs = 0;
            hysz = 16;
        }
        else
        {
            hxofs = 4;
            hxsz = 8;
        }

        int branglevels = zc_min(zc_max(currentItemLevel(itype_brang), 1), 3) - 1;
        SpriteDefinitionRef wid = curQuest->specialSprites().defaultBoomerangSprites[branglevels];

        if (parentid > -1)
        {
            sprite *e = (sprite *)pool->getFromUID(parentid);
            if (curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite)) 
                LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
            else 
                LOADGFX(wid);
        }
        else 
            LOADGFX(wid);
        break;
    }

    case ewFireball2:
        if(dir < 8)
            misc=dir;
        else
        {
            misc = -1;
        }
        
        //fallthrough
    case ewFireball:
    case wRefFireball:
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite) ) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef fireballs = curQuest->specialSprites().defaultEnemyFireball;
            LOADGFX(fireballs);
        }
	}
    else
    {
        SpriteDefinitionRef fireballs = curQuest->specialSprites().defaultEnemyFireball;
        LOADGFX(fireballs);
    }
 
        step=1.75;
        
        if(Type&2)
        {
            seekLink();
        }
        else misc=-1;
        
        break;
        
    case ewRock:
	    
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite)) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef rocks = curQuest->specialSprites().defaultEnemyRock;
            LOADGFX(rocks);
        }
	}
    {
        SpriteDefinitionRef rocks = curQuest->specialSprites().defaultEnemyRock;
        LOADGFX(rocks);
    }

        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        step=3;
        break;
        
    case ewArrow:
	    
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite )) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef arrows = curQuest->specialSprites().defaultEnemyArrow;
            LOADGFX(arrows);
        }
	}
    {
        SpriteDefinitionRef arrows = curQuest->specialSprites().defaultEnemyArrow;
        LOADGFX(arrows);
    }

        step=2;
        
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            xofs=-4;
            hxsz=8;
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset+1;
            break;
        }
        
        break;
        
    case ewSword:
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) ) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef swords = curQuest->specialSprites().defaultEnemySwordBeamSprite;
            LOADGFX(swords);
        }
	}
    {
        SpriteDefinitionRef swords = curQuest->specialSprites().defaultEnemySwordBeamSprite;
        LOADGFX(swords);
    }

        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        step=3;
        
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset+1;
            break;
        }
        
        break;
        
    case wRefMagic:
    case ewMagic:
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) ) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef magics = curQuest->specialSprites().defaultEnemyMagic;
            LOADGFX(magics);
        }
	}
    {
        SpriteDefinitionRef magics = curQuest->specialSprites().defaultEnemyMagic;
        LOADGFX(magics);
    }

        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
        {
            hxofs=0;
            hxsz=16;
			hyofs=0;
			hysz=16;
        }
        else
        {
            hxofs=4;
            hxsz=8;
        }
        
        step=3;
        
        switch(dir)
        {
        case down:
            flip=2;
            
        case up:
            break;
            
        case left:
            flip=1;
            
        case right: /*tile=o_tile+((frames>1)?frames:1)*/
            update_weapon_frame(((frames>1)?frames:1),o_tile);
            yofs=playing_field_offset+1;
            break;
        }
        
        if(id==wRefMagic)
        {
            ignorecombo=(((int)y&0xF0)+((int)x>>4));
        }
        
        break;
        
    case ewFlame:
    case ewFlame2:
        if(id==ewFlame)
	{
		if ( parentid > -1 )
		{
			sprite *e = (sprite *)pool->getFromUID(parentid);
			if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) ) 
                LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
            else
            {
                SpriteDefinitionRef flames = curQuest->specialSprites().flickeringFlame;
                LOADGFX(flames);
            }
		}
        else
        {
            SpriteDefinitionRef flames = curQuest->specialSprites().flickeringFlame;
            LOADGFX(flames);
        }
        }
        else
	{
		if ( parentid > -1 )
		{
			sprite *e = (sprite *)pool->getFromUID(parentid);
			if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) ) 
                LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
            else
            {
                SpriteDefinitionRef flames = curQuest->specialSprites().flickeringFlame2;
                LOADGFX(flames);
            }
		}
        else
        {
            SpriteDefinitionRef flames = curQuest->specialSprites().flickeringFlame2;
            LOADGFX(flames);
        }
        }
            
        if(dir==255)
        {
            step=2;
            seekLink();
        }
        else
        {
            /*if(Dir<0)
            {
            	step = 0;
            }*/
            if(dir>right)
            {
                step = .707;
            }
            else
            {
                step = 1;
            }
        }
        
		if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hxofs=hyofs=0;
			hxsz=hysz=16;
		}
		else
		{
			hxofs = hyofs=1; // hof of 1 means that link can use the 'half-tile trick'.
			hxsz = hysz = 14;
		}
        
        if(BSZ)
            yofs+=2;
            
        break;
        
    case ewFireTrail:
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) ) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef firetrails = curQuest->specialSprites().flickeringFireTrail;
            LOADGFX(firetrails);
        }
	}
	else
    {
        SpriteDefinitionRef firetrails = curQuest->specialSprites().flickeringFireTrail;
        LOADGFX(firetrails);
    }

        step=0;
        dir=-1;
        
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hxofs=hyofs=0;
			hxsz=hysz=16;
		}
		else
		{
			hxofs = hyofs=1; // hof of 1 means that link can use the 'half-tile trick'.
			hxsz = hysz = 15;
		}
        
        if(BSZ)
            yofs+=2;
            
        break;
        
    case ewWind:
        if(get_bit(quest_rules, qr_OFFSETEWPNCOLLISIONFIX))
		{
			hxofs=hyofs=0;
			hxsz=hysz=16;
		}
	if ( parentid > -1 )
	{
		sprite *e = (sprite *)pool->getFromUID(parentid);
		if ( curQuest->isValid(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite ) ) 
            LOADGFX(curQuest->getEnemyDefinition(((enemy *)e)->enemyDefinition).wpnsprite);
        else
        {
            SpriteDefinitionRef winds = curQuest->specialSprites().defaultEnemyWind;
            LOADGFX(winds);
        }
	}
    else
    {
        SpriteDefinitionRef winds = curQuest->specialSprites().defaultEnemyWind;
        LOADGFX(winds);
    }

        clk=0;
	if (power > 0) step = curQuest->getItemDefinition(parentitem).misc2; //!Dimentio: Add a check here.
		//! ZoriaRPG: Explain why this is here. 
        else step=3;
        break;
        
    case wPhantom:
        switch(type)
        {
        case pDINSFIREROCKET:
            if(get_bit(quest_rules,qr_MORESOUNDS))
                Backend::sfx->play(WAV_ZN1ROCKETUP,(int)x);
                
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[0]);
            step = 4;
            break;
            
        case pDINSFIREROCKETRETURN:
            if(get_bit(quest_rules,qr_MORESOUNDS))
                Backend::sfx->play(WAV_ZN1ROCKETDOWN,(int)x);
                
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[1]);
            step = 4;
            break;
            
        case pDINSFIREROCKETTRAIL:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[2]);
            break;
            
        case pDINSFIREROCKETTRAILRETURN:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[3]);
            break;
            
        case pMESSAGEMORE:
        {
            SpriteDefinitionRef mores = curQuest->specialSprites().messageMoreIndicator;
            LOADGFX(mores);
            break;
        }
            
        case pNAYRUSLOVEROCKET1:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[0]);
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
                Backend::sfx->play(WAV_ZN1ROCKETUP,(int)x);
                
            step = 4;
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETRETURN1:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[1]);
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
                Backend::sfx->play(WAV_ZN1ROCKETDOWN,(int)x);
                
            step = 4;
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAIL1:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[2]);
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN1:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[3]);
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKET2:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[5]);
            step = 4;
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETRETURN2:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[6]);
            step = 4;
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAIL2:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[7]);
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        case pNAYRUSLOVEROCKETTRAILRETURN2:
            LOADGFX(curQuest->getItemDefinition(parentitem).wpns[8]);
            drawstyle=curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2 ? 1 : 0;
            break;
            
        default:
            tile=0;
            break;
        }
        
        break;
        
    default:
        break;
    }
    
    if(id>wEnemyWeapons && id!=ewBrang && (Type&4)!=0)  // Increase speed of Aquamentus 2 fireballs
    {
        step *=2;
    }
}

void weapon::LOADGFX(const SpriteDefinitionRef &wpn)
{
    if(!curQuest->isValid(wpn))
        return;
        
    wid = wpn;
    flash = curQuest->getSpriteDefinition(wid).misc&3;
    tile  = curQuest->getSpriteDefinition(wid).tile;
    cs = curQuest->getSpriteDefinition(wid).csets&15;
    o_tile = curQuest->getSpriteDefinition(wid).tile;
    o_cset = curQuest->getSpriteDefinition(wid).csets;
    o_flip=(curQuest->getSpriteDefinition(wid).misc>>2)&3;
    o_speed = curQuest->getSpriteDefinition(wid).speed;
    o_type = curQuest->getSpriteDefinition(wid).type;
    frames = curQuest->getSpriteDefinition(wid).frames;
    temp1 = curQuest->specialSprites().defaultFireSprite;
    behind = (curQuest->getSpriteDefinition(wid).misc & wpndata::WF_BEHIND)!=0;
}

bool weapon::Dead()
{
    return dead!=-1;
}

bool weapon::clip()
{
    if(x < minX)
        if(dir==left || dir==l_up || dir==l_down)
            return true;
            
    if(x > maxX)
        if(dir==right || dir==r_up || dir==r_down)
            return true;
            
    if(y < minY)
        if(dir==up || dir==l_up || dir==r_up)
            return true;
            
    if(y > maxY)
        if(dir==down || dir==l_down || dir==r_down)
            return true;
            
    if(id>wEnemyWeapons)
    {
		int border = get_bit(quest_rules, qr_NOBORDER) ? 8 : 0;
		if ((x<(8 - border) && dir == left) || // Not checking diagonals?
			(y<(8 - border) && dir == up) ||
			(x>(232 + border) && dir == right) ||
			(y>(168 + border) && dir == down))
            return true;
    }
    
	// This doesn't do anything useful, does it?
    if(x<0||y<0||x>240||y>176)
        return true;
        
    return false;
}

bool weapon::blocked()
{
    return blocked(8, 8);
}

bool weapon::blocked(int xOffset, int yOffset)
{
    int wx = x+xOffset;
    int wy = y+yOffset;
    
    if(id == wPhantom || id == wHSHandle || id == wHSChain)  // Sanity check
    {
        return false;
    }
    
    if(get_bit(combo_class_buf[COMBOTYPE(wx,wy)].block_weapon,id)
            || get_bit(combo_class_buf[FFCOMBOTYPE(wx,wy)].block_weapon, id))
    {
        if(!curQuest->isValid(parentitem) || (combo_class_buf[COMBOTYPE(wx,wy)].block_weapon_lvl >=
            curQuest->getItemDefinition(parentitem).fam_type))
        {
            return true;
        }
    }
    
    switch(dir)
    {
    case up:
        wy-=8;
        break;
        
    case down:
        wy+=8;
        break;
        
    case left:
        wx-=8;
        break;
        
    case right:
        wx+=8;
        break;
    }
    
    if(get_bit(combo_class_buf[COMBOTYPE(wx,wy)].block_weapon,id)
            || get_bit(combo_class_buf[FFCOMBOTYPE(wx,wy)].block_weapon, id))
    {
        if(!curQuest->isValid(parentitem) || (combo_class_buf[COMBOTYPE(wx,wy)].block_weapon_lvl >=
            curQuest->getItemDefinition(parentitem).fam_type))
        {
            return true;
        }
    }
    
    return false;
}

bool weapon::animate(int)
{
    // do special timing stuff
    bool hooked=false;
    
    // fall down
    switch(id)
    {
    case wFire:
    
        // Din's Fire shouldn't fall
        if(curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).family==itype_dinsfire && !(curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG3))
        {
            break;
        }
        
    case wLitBomb:
    case wLitSBomb:
    case wBait:
    case ewFlame:
    case ewFireTrail:
        if(tmpscr->flags7&fSIDEVIEW)
        {
            if(!_walkflag(x,y+16,0))
            {
                y+=fall/100;
                
                if(fall <= (int)zinit.terminalv)
                {
                    fall += zinit.gravity;
                }
            }
            else
            {
                if(fall!=0 && !(step>0 && dir==up))  // Don't fix pos if still moving through solidness
                    y-=(int)y%8; // Fix position
                    
                fall = 0;
            }
            
            if(y>192) dead=0;  // Out of bounds
        }
        else
        {
            z-=fall/100;
            
            if(z<=0)
            {
                z = fall = 0;
            }
            else if(fall <= (int)zinit.terminalv)
            {
                fall += zinit.gravity;
            }
        }
    }
    
    if(id>wEnemyWeapons && id!=ewBrang && id != ewFireball2 && id != ewBomb && id != ewSBomb)
        switch(misc)
        {
        case up:
            y-=.5;
            break;
            
        case down:
            y+=.5;
            break;
            
        case left:
            x-=.5;
            break;
            
        case right:
            x+=.5;
            break;
            
        case 4:
            y-=1;
            break;
            
        case 5:
            y+=1;
            break;
            
        case 6:
            x-=1;
            break;
            
        case 7:
            x+=1;
            break;
            //case l_up:  y-=.354; x-=.354; break;
            //case r_up:  y-=.354; x+=.354; break;
            //case l_down:y+=.354; x-=.354; break;
            //case r_down:y+=.354; x+=.354; break;
        }
	
	//! Dimentio Wand Stuff
    if (curQuest->isValid(parentitem))
    {
        if (curQuest->getItemDefinition(parentitem).family == itype_wand && (id != wWand || curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2))
        { //!Dimentio: So this handles the movement of the wand's movement effect. Uses offsets to do this, so unfortunately,
          //!Dimentio: no setting offsets via script while one of these is active. Could change this if need be, though it'd
          //!Dimentio: be a bit odd.
            //! ZoriaRPG: Scripts MUST NOT conflict. Please look into this. 

            switch (curQuest->getItemDefinition(parentitem).misc5)
            {
            case 1:
            {
                this->count1 += curQuest->getItemDefinition(parentitem).misc7;
                this->count1 %= 360;
                switch (this->count3)
                {
                case up:
                case down:
                {
                    hxofs = curQuest->getItemDefinition(parentitem).misc6*cos(this->count1 * 0.0174);
                    xofs = curQuest->getItemDefinition(parentitem).misc6*cos(this->count1 * 0.0174);
                    break;
                }
                case left:
                case right:
                {
                    hyofs = curQuest->getItemDefinition(parentitem).misc6*sin(this->count1 * 0.0174);
                    yofs = (curQuest->getItemDefinition(parentitem).misc6*sin(this->count1 * 0.0174)) + playing_field_offset;
                    break;
                }
                }
                break;
            }
            case 2:
            {
                this->count1 += curQuest->getItemDefinition(parentitem).misc7;
                this->count1 %= 360;
                switch (this->count3)
                {
                case up:
                case down:
                {
                    hyofs = curQuest->getItemDefinition(parentitem).misc6*sin(this->count1 * 0.0174);
                    yofs = (curQuest->getItemDefinition(parentitem).misc6*sin(this->count1 * 0.0174)) + playing_field_offset;
                    break;
                }
                case left:
                case right:
                {
                    hxofs = curQuest->getItemDefinition(parentitem).misc6*cos(this->count1 * 0.0174);
                    xofs = curQuest->getItemDefinition(parentitem).misc6*cos(this->count1 * 0.0174);
                    break;
                }
                }
                break;
            }
            case 3:
            {
                this->count1 += curQuest->getItemDefinition(parentitem).misc7;
                this->count1 %= 360;
                xofs = curQuest->getItemDefinition(parentitem).misc6*cos(this->count1 * 0.0174);
                yofs = (curQuest->getItemDefinition(parentitem).misc6*sin(this->count1 * 0.0174)) + playing_field_offset;
                hxofs = curQuest->getItemDefinition(parentitem).misc6*cos(this->count1 * 0.0174);
                hyofs = curQuest->getItemDefinition(parentitem).misc6*sin(this->count1 * 0.0174);
                break;
            }
            default: break;
            }
        }

        if (curQuest->getItemDefinition(parentitem).family == itype_book && (curQuest->getItemDefinition(parentitem).misc4 >= 3 && curQuest->getItemDefinition(parentitem).misc4 <= 8) && type == 2) //Is this the weapon?
        {
            if ((this->count4 - 1) % 4 == 0) //If the death counter's evenly divideable by 4...
            {
                Lwpns.add(new weapon(x + ((rand() % 32) - 16), y + ((rand() % 32) - 16), z, curQuest->getItemDefinition(parentitem).misc2, 1, curQuest->getItemDefinition(parentitem).misc1*DAMAGE_MULTIPLIER, 0, parentitem, -1)); //Make 3 randomly placed weapons around this weapon
                Lwpns.add(new weapon(x + ((rand() % 32) - 16), y + ((rand() % 32) - 16), z, curQuest->getItemDefinition(parentitem).misc2, 1, curQuest->getItemDefinition(parentitem).misc1*DAMAGE_MULTIPLIER, 0, parentitem, -1));
                Lwpns.add(new weapon(x + ((rand() % 32) - 16), y + ((rand() % 32) - 16), z, curQuest->getItemDefinition(parentitem).misc2, 1, curQuest->getItemDefinition(parentitem).misc1*DAMAGE_MULTIPLIER, 0, parentitem, -1));
                //sfx(itemsbuf[parentitem].usesound,pan(x)); //Play sound
            }
            if (this->count4 >= ((curQuest->getItemDefinition(parentitem).misc6 * 4) - 3))
            {
                dead = 0;
            }
            ++this->count4;
        }
    }
        
    switch(id)
    {
        // Link's weapons
    case wSword:
    case wWand:
    case wHammer:
        if(LinkAction()!=attacking && LinkAction()!=ischarging && !LinkCharged())
        {
            dead=0;
        }
        
        break;
        
    case wCByrna:
    {
        if(blocked())
        {
            dead=0;
        }
        
        int speed = curQuest->isValid(parentitem) ? zc_max(curQuest->getItemDefinition(parentitem).misc1,1) : 1;
        int radius = curQuest->isValid(parentitem) ? zc_max(curQuest->getItemDefinition(parentitem).misc2,8) : 8;
        double xdiff = -(sin((double)clk/speed) * radius);
        double ydiff = (cos((double)clk/speed) * radius);
        
        double ddir=atan2(double(ydiff),double(xdiff));
        
        if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
            dir=r_up;
        else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
            dir=right;
        else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
            dir=r_down;
        else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
            dir=down;
        else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
            dir=l_down;
        else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
            dir=left;
        else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
            dir=l_up;
        else
            dir=up;
            
        x = (fix)((double)LinkX() + xdiff);
        y = (fix)((double)LinkY() + ydiff);
        z = LinkZ();
        
        if(curQuest->isValid(parentitem))
            Backend::sfx->loop(curQuest->getItemDefinition(parentitem).usesound,int(x));
    }
    break;
    
    case wBeam:
    case wRefBeam: //More Dimentio Wand stuff
		if (clk==94 && curQuest->getItemDefinition(parentitem).family == itype_book && (dir < 0 || step <= 0))
		{
			dead = 23;
		}
        for(int i2=0; ((i2<=zc_min(type-1,3) && curQuest->getItemDefinition(parentitem).family != itype_wand) || (curQuest->getItemDefinition(parentitem).family == itype_wand && i2 <= zc_min(curQuest->getItemDefinition(parentitem).fam_type - 1, 3))) && dead!=23; i2++)
        {
            if(findentrance(x,y,mfSWORDBEAM+i2,true)) dead=23; //!Dimentio: Alright, now this checks to see if it's fired from a wand.
        }
        
        if(blocked())
        {
            dead=23;
        }
        
    case ewSword:
        if(blocked())
        {
            dead=0;
        }
        
        if(get_bit(quest_rules,qr_SWORDMIRROR))
        {
            int checkx=0, checky=0;
            
            switch(dir)
            {
            case up:
                checkx=x+7;
                checky=y+8;
                break;
                
            case down:
                checkx=x+7;
                checky=y;
                break;
                
            case left:
                checkx=x+8;
                checky=y+7;
                break;
                
            case right:
                checkx=x;
                checky=y+7;
                break;
            }
            
            if(ignorecombo==(((int)checky&0xF0)+((int)checkx>>4)))
                break;
                
            if(hitcombo(checkx,checky,cMIRROR))
            {
                id = wRefBeam;
                dir ^= 1;
                
                if(dir&2)
                    flip ^= 1;
                else
                    flip ^= 2;
                    
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)y&0xF0;
                x=(int)x&0xF0;
            }
            
            if(hitcombo(checkx,checky,cMIRRORSLASH))
            {
                id = wRefBeam;
                dir = 3-dir;
                {
                    if(dir==right)
                        flip &= ~1; // not horiz
                    else if(dir==left)
                        flip |= 1;  // horiz
                    else if(dir==up)
                        flip &= ~2; // not vert
                    else if(dir==down)
                        flip |= 2;  // vert
                }
                tile=o_tile;
                
                if(dir&2)
                {
                    if(frames>1)
                    {
                        tile+=frames;
                    }
                    else
                    {
                        ++tile;
                    }
                }
                
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)y&0xF0;
                x=(int)x&0xF0;
            }
            
            if(hitcombo(checkx,checky,cMIRRORBACKSLASH))
            {
                id = wRefBeam;
                dir ^= 2;
                {
                    if(dir==right)
                        flip &= ~1; // not horiz
                    else if(dir==left)
                        flip |= 1;  // horiz
                    else if(dir==up)
                        flip &= ~2; // not vert
                    else if(dir==down)
                        flip |= 2;  // vert
                }
                tile=o_tile;
                
                if(dir&2)
                {
                    if(frames>1)
                    {
                        tile+=frames;
                    }
                    else
                    {
                        ++tile;
                    }
                }
                
                ignoreLink=false;
                ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                y=(int)y&0xF0;
                x=(int)x&0xF0;
            }
            
            if(hitcombo(checkx,checky,cMAGICPRISM))
            {
                int newx, newy;
                newy=(int)y&0xF0;
                newx=(int)x&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
                        w->dir=tdir;
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefBeam;
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        w->hyofs = w->hxofs = 0;
                        //also set up the magic's correct animation -DD
                        w->flip=0;
                        
                        switch(w->dir)
                        {
                        case down:
                            w->flip=2;
                            
                        case up:
                            w->tile = w->o_tile;
                            w->hyofs=2;
                            w->hysz=12;
                            break;
                            
                        case left:
                            w->flip=1;
                            
                        case right:
                            w->tile=w->o_tile+((w->frames>1)?w->frames:1);
                            w->hxofs=2;
                            w->hxsz=12;
                            break;
                        }
                        
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo(checkx,checky,cMAGICPRISM4))
            {
                int newx, newy;
                newy=(int)y&0xF0;
                newx=(int)x&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
                    w->dir=tdir;
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefBeam;
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    //also set up the magic's correct animation -DD
                    w->flip=0;
                    
                    switch(w->dir)
                    {
                    case down:
                        w->flip=2;
                        
                    case up:
                        w->tile = w->o_tile;
                        w->hyofs=2;
                        w->hysz=12;
                        break;
                        
                    case left:
                        w->flip=1;
                        
                    case right:
                        w->tile=w->o_tile+((w->frames>1)?w->frames:1);
                        w->hxofs=2;
                        w->hxsz=12;
                        break;
                    }
                    
                    Lwpns.add(w);
                }
                
                dead=0;
            }
        }
        
        break;
        
    case wWhistle:
        if(clk)
        {
            dead=1;
        }
        
        break;
        
    case wWind:
    {
        if(blocked())
        {
            dead=1;
        }
        
        int wrx;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wrx=tmpscr->warpreturnx[0];
        else wrx=tmpscr->warparrivalx;
        
        int wry;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wry=tmpscr->warpreturny[0];
        else wry=tmpscr->warparrivaly;
        
        if(type==1 && dead==-1 && x==(int)wrx && y==(int)wry)
        {
            dead=2;
        }
        else if(LinkAction() !=inwind && ((dir==right && x>=240) || (dir==down && y>=160) || (dir==left && x<=0) || (dir==up && y<=0)))
        {
            Backend::sfx->stop(WAV_ZN1WHIRLWIND);
            dead=1;
        }
        else if(get_bit(quest_rules,qr_MORESOUNDS))
            Backend::sfx->loop(WAV_ZN1WHIRLWIND,int(x));
            
        if(get_bit(quest_rules,qr_WHIRLWINDMIRROR))
            goto mirrors;
            
        break;
    }
    
    case wFire:
    {
        if(blocked())
        {
            dead=1;
        }
        
        if(!curQuest->isValid(parentitem) || (curQuest->getItemDefinition(parentitem).family!=itype_book))
        {
            if(clk==32)
            {
                step=0;
                
                if(!curQuest->isValid(parentitem) || !(curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2))
                {
                    isLit = true;
                    checkLightSources();
                }
            }
            
            if(clk==94)
            {
                dead=1;
                
                if((!curQuest->isValid(parentitem) || !(curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2)) &&
                   get_bit(quest_rules,qr_TEMPCANDLELIGHT) &&
                   (LwpnsIdCount(wFire) + EwpnsIdCount(ewFlame))==1)
                {
                    isLit = false;
                    checkLightSources();
                }
            }
            
            if(clk==94 || get_bit(quest_rules,qr_INSTABURNFLAGS))
            {
                findentrance(x,y,mfBCANDLE,true);
                
		    //Dimentio wand
                if((type>0 && curQuest->getItemDefinition(parentitem).family!=itype_wand) || (curQuest->getItemDefinition(parentitem).family==itype_wand && curQuest->getItemDefinition(parentitem).fam_type>=2)) //!Dimentio: Blue Fire fired from wand was triggering red fire, so have to make this check more specific.
                {
                    findentrance(x,y,mfRCANDLE,true);
                }
                
                if((type>2 && curQuest->getItemDefinition(parentitem).family!=itype_wand) || (curQuest->getItemDefinition(parentitem).family==itype_wand && curQuest->getItemDefinition(parentitem).fam_type>=4))   
                {
                    findentrance(x,y,mfDINSFIRE,true); 
                }
            }
        }                                                     //wand fire
        else
        {
            if(clk==1)
            {
                isLit = true;
                checkLightSources();
            }
            
            if(clk==80)
            {
                dead=1;
                findentrance(x,y,mfBCANDLE,true);
                findentrance(x,y,mfRCANDLE,true);
                findentrance(x,y,mfWANDFIRE,true);
                
                if(get_bit(quest_rules,qr_TEMPCANDLELIGHT) && (LwpnsIdCount(wFire) + EwpnsIdCount(ewFlame))==1)
                {
                    isLit=false;
                    checkLightSources();
                }
            }
        }
        
        // Killed by script?
        if(dead==0 && get_bit(quest_rules,qr_TEMPCANDLELIGHT) && (LwpnsIdCount(wFire) + EwpnsIdCount(ewFlame))==1)
        {
            isLit=false;
            checkLightSources();
        }
        
        break;
    }
    
    case wLitBomb:
    case wBomb:
    case ewLitBomb:
    case ewBomb:
    case ewLitSBomb:
    case ewSBomb:
    case wLitSBomb:
    case wSBomb:
    {
        if(!misc)
        {
            break;
        }
        
        // Naaah.
        /*if (blocked())
        {
          dead=1;
        }*/
        if(clk==(misc-2) && step==0)
        {
            id = (id>wEnemyWeapons ? (id==ewLitSBomb||id==ewSBomb ? ewSBomb : ewBomb)
                      : curQuest->isValid(parentitem) ? ((curQuest->getItemDefinition(parentitem).family==itype_sbomb) ? wSBomb:wBomb)
                      : (id==wLitSBomb||id==wSBomb ? wSBomb : wBomb));
            hxofs=2000;
        }
        
        if(clk==(misc-1) && step==0)
    {
            Backend::sfx->play((id>=wEnemyWeapons || !curQuest->isValid(parentitem)) ? WAV_BOMB :
                curQuest->getItemDefinition(parentitem).usesound,int(x));
                
            if(id==wSBomb || id==wLitSBomb || id==ewSBomb || id==ewLitSBomb)
            {
                hxofs=hyofs=-16;
                hxsz=hysz=48;
            }
            else
            {
                hxofs=hyofs=-8;
                hxsz=hysz=32;
            }
            
            hzsz=16;
        }
        
        int boomend = (misc+(((id == wBomb || id == wSBomb || id == wLitBomb || id == wLitSBomb) &&
                              (curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG1)) ? 35 : 31));
                              
        if(clk==boomend && step==0)
        {
            hxofs=2000;
        }
        
        if(id<wEnemyWeapons)
        {
            if(clk==(misc-1))
            {
                int f1 = (id==wSBomb || id==wLitSBomb) ? 16 : 0; // Large SBomb triggerbox
                
                for(int tx=-f1; tx<=f1; tx+=8)  // -16,-8,0,8,16
                {
                    int f2 = 0;
                    
                    if(tx==-8 || tx==8)
                        f2 = f1;
                        
                    for(int ty=-f2; ty<=f2; ty+=32)
                    {
                        findentrance(x+tx,y+ty+(tmpscr->flags7&fSIDEVIEW?2:-3),mfBOMB,true);
                        
                        if(id==wSBomb || id==wLitSBomb)
                        {
                            findentrance(x+tx,y+ty+(tmpscr->flags7&fSIDEVIEW?2:-3),mfSBOMB,true);
                        }
                        
                        findentrance(x+tx,y+ty+(tmpscr->flags7&fSIDEVIEW?2:-3),mfSTRIKE,true);
                    }
                }
            }
            
            if(!get_bit(quest_rules,qr_NOBOMBPALFLASH))
            {
                if(!usebombpal)
                {
                    if(clk==misc || clk==misc+5)
                    {
                    
                        usebombpal=true;
                        memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
                        
                        //grayscale entire screen
                        if(get_bit(quest_rules,qr_FADE))
                        {
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                int g = zc_min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
                                g = (g >> 1) + 32;
                                RAMpal[i] = _RGB(g,g,g);
                            }
                            
                        }
                        else
                        {
                            // this is awkward. NES Z1 converts colors based on the global
                            // NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
                            for(int i=CSET(0); i < CSET(15); i++)
                            {
                                RAMpal[i] = NESpal(reverse_NESpal(RAMpal[i]) & 0x30);
                            }
                        }
                        
                        refreshpal = true;
                    }
                }
                
                if((clk==misc+4 || clk==misc+9) && usebombpal)
                {
                    // undo grayscale
                    usebombpal=false;
                    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
                    refreshpal = true;
                }
            }
            
            if(clk==misc+30)
            {
                bombdoor(x,y);
            }
        }
        
        if(clk==misc+34)
        {
            if(step==0)
            {
                dead=1;
            }
        }
        
        break;
    }
    
    case wArrow:
    {
        if(dead>0)
        {
            break;
        }
        
        if(misc>0 && clk > misc)
        {
            dead=4;
        }
	
	//2.6 ZScript lw->Range -Z
	if(scriptrange>0 && clk > scriptrange)
        {
            dead=4; //if it exceeds its range, kill it. 
        }
        
        if(findentrance(x,y,mfSTRIKE,true))
        {
            dead=4;
        }
        
        if(findentrance(x,y,mfARROW,true))
        {
            dead=4;
        }
        
	//Dimentio Wand stuff
        if((currentItemLevel(itype_arrow)>1 && curQuest->getItemDefinition(parentitem).family != itype_wand) || (curQuest->getItemDefinition(parentitem).family == itype_wand && curQuest->getItemDefinition(parentitem).fam_type > 1)) //!Dimentio: Okay, the wand's level can also serve as the level of the arrow.
        {
            if(findentrance(x,y,mfSARROW,true))
            {
                dead=4;
            }
        }
        
        if((currentItemLevel(itype_arrow)>=3 && curQuest->getItemDefinition(parentitem).family != itype_wand) || (curQuest->getItemDefinition(parentitem).family == itype_wand && curQuest->getItemDefinition(parentitem).fam_type >= 3)) //!Dimentio: So now Arrows fired from the wand can trigger higher leveled arrow secrets.
        {
            if(findentrance(x,y,mfGARROW,true))
            {
                dead=4;
            }
        }
        
        if(blocked())
        {
            dead=4;
        }
        
        break;
    }
    
    case wSSparkle:
        if(blocked())  //no spakle area?
        {
            dead=1;
        }
        
        if(clk>=frames*o_speed-1) //(((wpnsbuf[wSSPARKLE].frames) * (wpnsbuf[wSSPARKLE].speed))-1))
        {
            dead=1;
        }
        
        break;
        
    case wFSparkle:
        if(blocked())  //no sparkle area?
        {
            dead=1;
        }
        
        if(clk>=frames*o_speed-1) //(((wpnsbuf[wFSPARKLE].frames) * (wpnsbuf[wFSPARKLE].speed))-1))
        {
            dead=1;
        }
        
        break;
        
    case wBait:
        if(blocked())  //no bait area?
        {
            dead=23;
        }
        
	//!Dimentio: What's this? Hardcoded bait values? Nononono, screw this. 
        if(((curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).family != itype_wand) && clk>=curQuest->getItemDefinition(parentitem).misc1) || (curQuest->getItemDefinition(parentitem).family == itype_wand && clk>=curQuest->getItemDefinition(parentitem).power))
        { //!Dimentio: Ah, much better.
            dead=1;
        }
        
        break;
        
    case wBrang:
    {
        if(dead==0)  // Set by ZScript
        {
            Backend::sfx->stop(curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).usesound);
            break;
        }
        
        else if(dead==1) // Set by ZScript
        {
            onhit(false);
        }
        
        int deadval=(curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).flags & itemdata::IF_FLAG3)?-2:1;
        
        for(int i=0; ((i<=zc_min(currentItemLevel(itype_brang)-1,2) && curQuest->getItemDefinition(parentitem).family != itype_wand) || (curQuest->getItemDefinition(parentitem).family == itype_wand && i <= zc_min(curQuest->getItemDefinition(parentitem).fam_type - 1, 3))); i++)
        { //!Dimentio: If you have a wand, use misc 4. Otherwise, use the canon boomerang.
            if(findentrance(x,y,mfBRANG+i,true)) dead=deadval;
        }
        
        if(findentrance(x,y,mfSTRIKE,true)) dead=deadval;
        
        if(blocked())
        {
            dead=deadval;
            onhit(false);
        }
        
        ++clk2;
	
        //scriptrange is now -1
        int range = curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).misc1;
	if ( scriptrange < 0 ) scriptrange =  curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).misc1;
	else if ( scriptrange >= 0 ) {
		range = scriptrange; 
	}
	
	//This sets zscript l->Range to the editor value if it is still the default (-1)
	//If the user write to l->Range, it updates.
        
        if(range && clk2>=range)
        {
            if(deadval==1)
            {
                misc=1;
            }
            else
            {
                dead=deadval;
            }
        }
        
        if(range && clk2>range-18 && clk2<range+16)
        {
            step=1;
        }
        else if(misc)
        {
            step=2;
        }
        else
        {
            step=3;
        }
        
        if(clk==0)                                            // delay a frame
        {
            ++clk;
            Backend::sfx->loop(curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).usesound,int(x));
            return false;
        }
        
        if(clk==1)                                            // then check directional input
        {
            if(Up())
            {
                dir=up;
                
                if(Left())  dir=l_up;
                
                if(Right()) dir=r_up;
            }
            else if(Down())
            {
                dir=down;
                
                if(Left())  dir=l_down;
                
                if(Right()) dir=r_down;
            }
            else if(Left())
            {
                dir=left;
            }
            else if(Right())
            {
                dir=right;
            }
        }
        
        if(dead==1)
        {
            dead=-1;
            misc=1;
        }
        
        if(dead==-2)
        {
            misc=1;
        }
        
        if(misc==1)                                           // returning
        {
            if((abs(LinkY()-y)<7 && abs(LinkX()-x)<7)||dead==-2)
            {
                if(dead!=-2)
                {
                    CatchBrang();
                }
                
                if (LwpnsIdCount(wBrang) <= 1 && (!get_bit(quest_rules, qr_MORESOUNDS) || !EwpnsIdCount(ewBrang)))
                    Backend::sfx->stop(curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).usesound);
                    
                /*if (dummy_bool[0])
                {
                    add_grenade(x,y,z,0,-1); //TODO: Super bombs as well?
                    dummy_bool[0]=false;
                }*/
                if(dragging!=-1)
                {
                    getdraggeditem(dragging);
                }
                
                return true;
            }
            
            seekLink();
        }
        
        Backend::sfx->loop(curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).usesound,int(x));
        
        break;
    }
    
    case wHookshot:
    {
	    //for lw->Range -Z
        int  maxlength = curQuest->isValid(parentitem) ? 2 * curQuest->getItemDefinition(parentitem).misc1 : 0; //2* value seems to match the desired range. 
        if(dead==0)  // Set by ZScript
        {
            hookshot_used = false;
            
            bool found = true;
            while (found)
            {
                found = false;
                for (int i = 0; i < chainlinks.Count(); i++)
                {
                    if (((weapon *)chainlinks.spr(i))->id == wHSChain)
                    {
                        chainlinks.del(i);
                        found = true;
                        break;
                    }
                }
            }
        }
        
	//Set the range based on defaults, item editor, and script ofverrides. -Z
	if ( scriptrange < 0 ) scriptrange =  maxlength; //scriptrange inits at -1
	else if ( scriptrange > -1 ) { maxlength = scriptrange; } //20JAN17, was >= 0 before. 
		//If the user sets it, change it.
	
	//Diagonal hookshot set-up. -Z
	
	//Check the item editor flag to see if it is allowed to be diagonal. 
	if ( (curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_hookshot)).flags & itemdata::IF_FLAG2) && misc2 == 0 ) {
	    if(Up())
	    {
		dir=up;
		
		if(Left() )  dir=l_up;
		
		if(Right() ) dir=r_up;
		    misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
	    }
	    else if(Down())
	    {
		dir=down;
		
		if(Left() )  dir=l_down;
		
		if(Right() ) dir=r_down;
		     misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
	    }
	    else if(Left())
	    {
		dir=left;
		if(Up() )  dir=l_up;
		if(Down() )  dir=l_down;
		     misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
		
	    }
	    else if(Right())
	    {
		dir=right;
		if(Up() )  dir=r_up;
		if(Down() )  dir=r_down;
		     misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
	    }
	}
	
        // Hookshot grab and retract code
        if(misc==0)
        {
		
	    //If the hookshot has extended to maxlength, retract it.
            if((abs(LinkX()-x)>maxlength)||(abs(LinkY()-y)>maxlength))
            {
                dead=1;  //causes it to retract.
            }
            
	    //If it hits a block object, retract it.
            if(findentrance(x,y,mfSTRIKE,true)) dead=1;
            
            if(findentrance(x,y,mfHOOKSHOT,true)) dead=1;
            
	    
	    //Look for grab combos based on direction.
	    //...but where is the support for diagonal and angular? Oh, right, no-one ever thought of this.
	    // /sarcasm -Z
	    
	    //This is the issue with weapons that predate angular movement, in general.
	    
	    /*
	    int tempdir = dir;
	    
	    if ( key[KEY_LEFT] && key[KEY_UP] ) tempdir = l_up;
	    if ( key[KEY_LEFT] && key[KEY_DOWN] ) tempdir = l_down;
	    if ( key[KEY_RIGHT] && key[KEY_UP] ) tempdir = r_up;
	    if ( key[KEY_RIGHT] && key[KEY_DOWN] ) tempdir = r_down;
	    */
	    
            if(dir==up)
            {
		    //It would have been nice to comment these coordinates... 
		    //...so that we know *why* the values were used.
                if((combobuf[MAPCOMBO(x+2,y+7)].type==cHSGRAB))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || (combobuf[MAPCOMBO2(0,x+2,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+2,y+7)].type==cHSGRAB);
                    
		//Retracts if the hookshot hits illegal spaces, solid spaces, and such.
                if(!hooked && _walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7))
                {
                    dead=1;
                }
            }
            
            if(dir==down)
            {
                if((combobuf[MAPCOMBO(x+12,y+12)].type==cHSGRAB))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || (combobuf[MAPCOMBO2(0,x+12,y+12)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+12,y+12)].type==cHSGRAB);
                    
                if(!hooked && _walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12))
                {
                    dead=1;
                }
            }
            
            if(dir==left)
            {
		    //this is why it would be nice to know the specific reasons for the values
		    //.., old rules grabbed at a higher y position
                if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
                {
                    if(combobuf[MAPCOMBO(x+6,y+7)].type==cHSGRAB)
                    {
                        hooked=true;
                    }
                }
                else if(combobuf[MAPCOMBO(x+6,y+13)].type==cHSGRAB)
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || (combobuf[MAPCOMBO2(0,x+6,y+13)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+6,y+13)].type==cHSGRAB);
                    
                if(!hooked && _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13))
                {
                    dead=1;
                }
            }
            
            if(dir==right)
            {
		    
                if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
                {
                    if(combobuf[MAPCOMBO(x+9,y+7)].type==cHSGRAB)
                    {
                        hooked=true;
                    }
                }
                else if((combobuf[MAPCOMBO(x+9,y+13)].type==cHSGRAB))
                {
                    hooked=true;
                }
                
                if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
                    hooked = hooked || (combobuf[MAPCOMBO2(0,x+9,y+13)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+9,y+13)].type==cHSGRAB);
                    
                if(!hooked && _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13))
                {
                    dead=1;
                }
            }
	    
	    /*
	    if ( dir == angular ) {} //Diagonal or angular
	    
	    */ 

	    //in 2.50.x, if the user sets the hookshot direction to a diagonal, it flat-out refuses to grab. -Z
	    
		//! -Z Diagonal Hookshot Movement and Grab Code: Will need bugtesting galore. 
		if ( dir == r_down ) //Diagonal hookshot, right-down -Z
		{
			if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//right						//down
				if( (combobuf[MAPCOMBO(x+9,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO(x+12,y+12)].type==cHSGRAB) )
				{
					hooked=true;
				}
			}

					//right						//down
			else if( ( (combobuf[MAPCOMBO(x+9,y+13)].type==cHSGRAB)) || (combobuf[MAPCOMBO(x+12,y+12)].type==cHSGRAB) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//right
			{
				hooked = hooked || (combobuf[MAPCOMBO2(0,x+9,y+13)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+9,y+13)].type==cHSGRAB ) || 
				//down
				(combobuf[MAPCOMBO2(0,x+12,y+12)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+12,y+12)].type==cHSGRAB);
			}
                    
			//right
			if(!hooked &&  ( ( ( _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13)) ) ||
				//down
				(_walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12)) ) )
			{
			    dead=1;
			}
		    
		    
		}
		if ( dir == l_down ) //Diagonal hookshot, left-down -Z
		{
			if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//left						//down
				if( (combobuf[MAPCOMBO(x+6,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO(x+12,y+12)].type==cHSGRAB) )
				{
					hooked=true;
				}
			}
			

					//left						//down
			else if( ( (combobuf[MAPCOMBO(x+6,y+13)].type==cHSGRAB)) || (combobuf[MAPCOMBO(x+12,y+12)].type==cHSGRAB) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))
			{
							//left
				hooked = hooked || (combobuf[MAPCOMBO2(0,x+6,y+13)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+6,y+13)].type==cHSGRAB) || 
					//down
				(combobuf[MAPCOMBO2(0,x+12,y+12)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+12,y+12)].type==cHSGRAB);
			}
			//left
			if(!hooked && ( ( ( _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13)) ) ||
				//down
				(_walkflag(x+12,y+12,1) && !ishookshottable((int)x+12,(int)y+12)) ) )
			{
				dead=1;
			}
		    
		    
		}
		if ( dir == r_up ) //Diagonal hookshot, right-up -Z
		{
		        if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//right						//up
			    if( (combobuf[MAPCOMBO(x+9,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO(x+2,y+7)].type==cHSGRAB) )
				{
					hooked=true;
				}
			}

					//right						//up
			else if( ( (combobuf[MAPCOMBO(x+9,y+13)].type==cHSGRAB)) || (combobuf[MAPCOMBO(x+2,y+7)].type==cHSGRAB) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//right
			{
				hooked = hooked || (combobuf[MAPCOMBO2(0,x+9,y+13)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+9,y+13)].type==cHSGRAB ) || 
				//up
				(combobuf[MAPCOMBO2(0,x+2,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+2,y+7)].type==cHSGRAB);
			}
			//right
			if(!hooked &&  ( ( ( _walkflag(x+9,y+13,1) && !ishookshottable((int)x+9,(int)y+13)) ) ||
				//up
				(_walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7)) ) )
			{
				dead=1;
			}
		}
		if ( dir == l_up ) //Diagonal hookshot, left-up -Z
		{
		        if(get_bit(quest_rules, qr_OLDHOOKSHOTGRAB))
			{	//left						//up
				if( (combobuf[MAPCOMBO(x+6,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO(x+2,y+7)].type==cHSGRAB) )
				{
					hooked=true;
				}
			}

					//left						//up
			else if( ( (combobuf[MAPCOMBO(x+6,y+13)].type==cHSGRAB)) || (combobuf[MAPCOMBO(x+2,y+7)].type==cHSGRAB) )
			{
				hooked=true;
			}

			if(get_bit(quest_rules, qr_HOOKSHOTLAYERFIX))			//left
			{
				hooked = hooked || (combobuf[MAPCOMBO2(0,x+6,y+13)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+6,y+13)].type==cHSGRAB) || 
				//up
				(combobuf[MAPCOMBO2(0,x+2,y+7)].type==cHSGRAB) || (combobuf[MAPCOMBO2(1,x+2,y+7)].type==cHSGRAB);
			}
							//left
			if(!hooked && ( ( ( _walkflag(x+6,y+13,1) && !ishookshottable((int)x+6,(int)y+13)) ) ||
				//up
				(_walkflag(x+2,y+7,1) && !ishookshottable((int)x+2,(int)y+7)) ) )
			{
				dead=1;
			}
		}
        }
        
        if(hooked==true)
        {
            misc=1;
            pull_link=true;
            step=0;
        }
        
        ++clk2;
        
        if(clk==0)                                            // delay a frame
        {
            ++clk;
            
            if(curQuest->isValid(parentitem))
            {
                Backend::sfx->loop(curQuest->getItemDefinition(parentitem).usesound,int(x));
            }
            
            return false;
        }
        
        if(dead==1)
        {
            dead=-1;
            
            if(step > 0)
                step = -step;
                
            misc=1;
        }
        
        if(misc==1)                                           // returning
        {
            if((dir<left && abs(LinkY()-y)<9) || (dir >= left && abs(LinkX()-x)<9))
            {
                hookshot_used=false;
                
                if(pull_link)
                {
                    hs_fix=true;
                }
                
                pull_link=false;
                chainlinks.clear();
                CatchBrang();
                
                if(curQuest->isValid(parentitem))
                {
                    Backend::sfx->stop(curQuest->getItemDefinition(parentitem).usesound);
                }
                
                if(dragging!=-1)
                {
                    getdraggeditem(dragging);
                }
                
                return true;
            }
        }
        
        if(curQuest->isValid(parentitem))
        {
            Backend::sfx->loop(curQuest->getItemDefinition(parentitem).usesound,int(x));
        }
        
        if(blocked())
        {
            //not on the return!! -DD
            if(dead != -1)
                dead=1;
        }
    }
    break;
    
    case wHSHandle:
        if(hookshot_used==false)
        {
            dead=0;
        }
        
        if(blocked())  //no hookshot handle area?
        {
            hookshot_used=false;
            dead=0;
        }
        
        break;
        
    case wPhantom:
        switch(type)
        {
        case pDINSFIREROCKET:
            if(y <= -200)
                dead = 1;
                
            break;
            
        case pDINSFIREROCKETRETURN:                                             //Din's Fire Rocket return
            if(y>=casty)
            {
                dead=1;
                castnext=true;
            }
            
            break;
            
        case pDINSFIREROCKETTRAIL:                                             //Din's Fire Rocket trail
        {            
            SpriteDefinitionRef dinsa = curQuest->specialSprites().dinsRocketTrail;
            if (clk >= (((curQuest->getSpriteDefinition(dinsa).frames) * (curQuest->getSpriteDefinition(dinsa).speed)) - 1))
            {
                dead = 0;
            }

            break;
        }
        case pDINSFIREROCKETTRAILRETURN:      
        {
            SpriteDefinitionRef dinsb = curQuest->specialSprites().dinsRocketTrailReturn;
            //Din's Fire Rocket return trail
            if (clk >= (((curQuest->getSpriteDefinition(dinsb).frames) * (curQuest->getSpriteDefinition(dinsb).speed)) - 1))
            {
                dead = 0;
            }

            break;
        }
        case pNAYRUSLOVEROCKETRETURN1:                                             //Nayru's Love Rocket return
            if(x>=castx)
            {
                dead=1;
                castnext=true;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAIL1:                                             //Nayru's Love Rocket trail
        {
            SpriteDefinitionRef nayrua = curQuest->specialSprites().nayruRocketTrail;
            if (clk >= (((curQuest->getSpriteDefinition(nayrua).frames) * (curQuest->getSpriteDefinition(nayrua).speed)) - 1))
            {
                dead = 0;
            }

            break;
        }

        case pNAYRUSLOVEROCKETTRAILRETURN1:                                             //Nayru's Love Rocket return trail
        {
            SpriteDefinitionRef nayrub = curQuest->specialSprites().nayruRocketTrailReturn;
            if (clk >= (((curQuest->getSpriteDefinition(nayrub).frames) * (curQuest->getSpriteDefinition(nayrub).speed)) - 1))
            {
                dead = 0;
            }

            break;
        }
            
        case pNAYRUSLOVEROCKETRETURN2:                                             //Nayru's Love Rocket return
            if(x<=castx)
            {
                dead=0;
                castnext=true;
            }
            
            break;
            
        case pNAYRUSLOVEROCKETTRAIL2:
        {
            //Nayru's Love Rocket trail
            SpriteDefinitionRef nayru2a = curQuest->specialSprites().nayruRocketTrail2;
            if (clk >= (((curQuest->getSpriteDefinition(nayru2a).frames) * (curQuest->getSpriteDefinition(nayru2a).speed)) - 1))
            {
                dead = 0;
            }

            break;
        }
            
        case pNAYRUSLOVEROCKETTRAILRETURN2:
        {
            //Nayru's Love Rocket return trail
            SpriteDefinitionRef nayru2b = curQuest->specialSprites().nayruRocketTrailReturn2;
            if (clk >= (((curQuest->getSpriteDefinition(nayru2b).frames) * (curQuest->getSpriteDefinition(nayru2b).speed)) - 1))
            {
                dead = 0;
            }

            break;
        }
        }
        
        if(blocked()) //not really sure this is needed
        {
            dead=1;
        }
        
        break;
        
    case wRefMagic:
    case wMagic:
    case ewMagic:
    {
        if((id==wMagic)&&(findentrance(x,y,mfWANDMAGIC,true))) dead=0;
        
        if((id==wRefMagic)&&(findentrance(x,y,mfREFMAGIC,true))) dead=0;
        
        if((id!=ewMagic)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
        
        if((id==wMagic && currentItemLevel(itype_book) &&
            curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_book)).flags & itemdata::IF_FLAG1) && get_bit(quest_rules,qr_INSTABURNFLAGS))
        {
            findentrance(x,y,mfBCANDLE,true);
            findentrance(x,y,mfRCANDLE,true);
            findentrance(x,y,mfWANDFIRE,true);
        }
        
mirrors:
        int checkx=0, checky=0;
        
        switch(dir)
        {
        case up:
            checkx=x+7;
            checky=y+8;
            break;
            
        case down:
            checkx=x+7;
            checky=y;
            break;
            
        case left:
            checkx=x+8;
            checky=y+7;
            break;
            
        case right:
            checkx=x;
            checky=y+7;
            break;
        }
        
        if(ignorecombo!=(((int)checky&0xF0)+((int)checkx>>4)))
        {
            if(hitcombo(checkx,checky,cMIRROR))
            {
                weapon *w=new weapon(*this);
                
                if(id==ewMagic)
                {
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                
                w->dir ^= 1;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic;
                    
                    if(w->dir&2)
                        w->flip ^= 1;
                    else
                        w->flip ^= 2;
                }
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo(checkx,checky,cMIRRORSLASH))
            {
                weapon *w=new weapon(*this);
                
                if(id==ewMagic)
                {
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir = 3-w->dir;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic;
                    
                    if((w->dir==1)||(w->dir==2))
                        w->flip ^= 3;
                }
                
                w->tile=w->o_tile;
                
                if(w->dir&2)
                {
                    if(w->frames>1)
                    {
                        w->tile+=w->frames;
                    }
                    else
                    {
                        ++w->tile;
                    }
                }
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo(checkx,checky,cMIRRORBACKSLASH))
            {
                weapon *w=new weapon(*this);
                
                if(id==ewMagic)
                {
                    Lwpns.add(w);
                    dead=0;
                }
                else
                {
                    w=this;
                }
                
                w->dir ^= 2;
                
                if(w->id != wWind)
                {
                    w->id = wRefMagic;
                    
                    if(w->dir&1)
                        w->flip ^= 2;
                    else
                        w->flip ^= 1;
                }
                
                w->tile=w->o_tile;
                
                if(w->dir&2)
                {
                    if(w->frames>1)
                    {
                        w->tile+=w->frames;
                    }
                    else
                    {
                        ++w->tile;
                    }
                }
                
                w->ignoreLink=false;
                w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                w->y=checky&0xF0;
                w->x=checkx&0xF0;
            }
            
            if(hitcombo(checkx,checky,cMAGICPRISM) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    if(dir!=(tdir^1))
                    {
                        weapon *w=new weapon(*this);
                        w->dir=tdir;
                        w->x=newx;
                        w->y=newy;
                        w->z=z;
                        w->id=wRefMagic;
                        w->parentid=parentid;
                        w->parentitem=parentitem;
                        w->flip = 0;
                        w->ignoreLink = false;
                        w->hyofs = w->hxofs = 0;
                        w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                        
                        //also set up the magic's correct animation -DD
                        switch(w->dir)
                        {
                        case down:
                            w->flip=2;
                            
                        case up:
                            w->tile = w->o_tile;
                            w->hyofs=2;
                            w->hysz=12;
                            break;
                            
                        case left:
                            w->flip=1;
                            
                        case right:
                            w->tile=w->o_tile+((w->frames>1)?w->frames:1);
                            w->hxofs=2;
                            w->hxsz=12;
                            break;
                        }
                        
                        Lwpns.add(w);
                    }
                }
                
                dead=0;
            }
            
            if(hitcombo(checkx,checky,cMAGICPRISM4) && (id != wWind))
            {
                int newx, newy;
                newy=checky&0xF0;
                newx=checkx&0xF0;
                
                for(int tdir=0; tdir<4; tdir++)
                {
                    weapon *w=new weapon(*this);
                    w->dir=tdir;
                    w->x=newx;
                    w->y=newy;
                    w->z=z;
                    w->id=wRefMagic;
                    w->parentid=parentid;
                    w->parentitem=parentitem;
                    w->flip = 0;
                    w->ignoreLink = false;
                    w->hyofs = w->hxofs = 0;
                    w->ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
                    
                    //also set up the magic's correct animation -DD
                    switch(w->dir)
                    {
                    case down:
                        w->flip=2;
                        
                    case up:
                        w->tile = w->o_tile;
                        w->hyofs=2;
                        w->hysz=12;
                        break;
                        
                    case left:
                        w->flip=1;
                        
                    case right:
                        w->tile=w->o_tile+((w->frames>1)?w->frames:1);
                        w->hxofs=2;
                        w->hxsz=12;
                        break;
                    }
                    
                    Lwpns.add(w);
                }
                
                dead=0;
            }
            
            if(blocked(0, 0))
            {
                dead=0;
            }
        }
    }
    break;
    
    // enemy weapons
    case ewFireball2:
        switch(misc)
        {
        case up:
            y-=.5;
            break;
            
        case down:
            y+=.5;
            break;
            
        case left:
            x-=.5;
            break;
            
        case right:
            x+=.5;
            break;
            
        case l_up:
            y-=.354;
            x-=.354;
            break;
            
        case r_up:
            y-=.354;
            x+=.354;
            break;
            
        case l_down:
            y+=.354;
            x-=.354;
            break;
            
        case r_down:
            y+=.354;
            x+=.354;
            break;
        }
        
        //fallthrough
    case wRefFireball:
    case ewFireball:
    {
        if((id==wRefFireball)&&(findentrance(x,y,mfREFFIREBALL,true))) dead=0;
        
        if((id==wRefFireball)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
        
        if(blocked())
        {
            dead=0;
        }
        
        if(clk<16)
        {
            ++clk;
            
            if(dead>0)
                --dead;
                
            return dead==0;
        }
        
        break;
    }
    
    case ewFlame:
    {
        if(clk==32)
        {
            step=0;
            misc = -1; // Don't drift diagonally anymore
            isLit=true;
            checkLightSources();
        }
        
        if(clk==126)
        {
            dead=1;
            
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
            {
                isLit=false;
                checkLightSources();
            }
        }
        
        if(blocked())
        {
            dead=1;
        }
        
        // Killed by script?
        if(dead==0 && get_bit(quest_rules,qr_TEMPCANDLELIGHT) && (LwpnsIdCount(wFire) + EwpnsIdCount(ewFlame))==1)
        {
            isLit=false;
            checkLightSources();
        }
        
        break;
    }
    
    case ewFireTrail:
    {
        if(clk==32)
        {
            step=0;  //should already be 0, but still...
            isLit=true;
            checkLightSources();
        }
        
        if(clk==640)
        {
            dead=1;
            
            if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
            {
                isLit=false;
                checkLightSources();
            }
        }
        
        if(blocked())
        {
            dead=1;
        }
        
        break;
    }
    
    case ewBrang:
    {
        if(clk==0)
        {
            misc2=(dir<left)?y:x;                               // save home position
            
            if(dummy_bool[0]==true)
            {
                seekLink();
            }
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
            {
                //if (step!=0)
                Backend::sfx->loop(WAV_BRANG, int(x));
                //else
                ;//stop_sfx(WAV_BRANG);
            }
        }
        
        ++clk2;
        
        if(clk2==45&&!dummy_bool[0])
        {
            misc=1;
            dir^=1;
        }
        
        if(dummy_bool[0])
        {
            step=5;
        }
        else
        {
            if(clk2>27 && clk2<61)
            {
                step=1;
            }
            else if(misc)
            {
                step=2;
            }
            else
            {
                step=3;
            }
        }
        
        bool killrang = false;
        
        if(misc==1)                                           // returning
        {
            if(dummy_bool[0]==true)
            {
                //no parent
                if(parentid < 0)
                    return true;
                    
                //find parent
                int index = -1;
                
                for(int i=0; i<guys.Count(); i++)
                    if(guys.spr(i)->getUID()==parentid)
                    {
                        index = i;
                        break;
                    }
                    
                //parent is not a guy
                if(index < 0)
                    return true;
                    
                seekEnemy(index);
                
                if((abs(x-GuyX(index))<7)&&(abs(y-GuyY(index))<7))
                {
                    if(get_bit(quest_rules,qr_MORESOUNDS) && !LwpnsIdCount(wBrang) && EwpnsIdCount(ewBrang)<=1)
                    {
                        Backend::sfx->stop(WAV_BRANG);
                    }
                    
                    return true;
                }
            }
            else
            {
                switch(dir)
                {
                case up:
                    if(y<misc2) killrang=true;
                    
                    break;
                    
                case down:
                    if(y>misc2) killrang=true;
                    
                    break;
                    
                case left:
                    if(x<misc2) killrang=true;
                    
                    break;
                    
                case right:
                    if(x>misc2) killrang=true;
                    
                    break;
                }
            }
        }
        else if(blocked())
        {
            //only obey block flags before the bounce
            dead=1;
        }
        
        //if the boomerang is not on its way home, it bounces
        if(dead==1 && misc != 1)
        {
            dead=-1;
            misc=1;
            dir^=1;
            clk2=zc_max(46,90-clk2);
        }
        
        //otherwise it disappears
        if(killrang || dead==1)
        {
            if(get_bit(quest_rules,qr_MORESOUNDS) && !LwpnsIdCount(wBrang) && EwpnsIdCount(ewBrang)<=1)
            {
                Backend::sfx->stop(WAV_BRANG);
            }
            
            dead = 1;
        }
    }
    }
    
    // move sprite, check clipping
    if(dead==-1 && clk>=0)
    {
        move(step);
        
        if(clip())
        {
            onhit(true);
        }
        else if(id==ewRock)
        {
            if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
            {
                onhit(true);
            }
        }
    }
    else if(dead==-10) // Scripting hack thing related to weapon->DeadState
    {
        if(clip())
        {
            onhit(true);
        }
    }
    
    if(bounce)
    {
        switch(dir)
        {
        case up:
            x-=1;
            y+=2;
            break;
            
        case down:
            x+=1;
            y-=2;
            break;
            
        case left:
            x+=2;
            y-=1;
            break;
            
        case right:
            x-=2;
            y-=1;
            break;
        }
    }
    
    // update clocks
    ++clk;
    
    if(dead>0)
    {
        --dead;
    }
    
    return dead==0;
}

void weapon::onhit(bool clipped)
{
    onhit(clipped, 0, -1);
}

void weapon::onhit(bool clipped, int special, int linkdir)
{
    if((scriptcoldet&1) == 0)
    {
        // These won't hit anything, but they can still go too far offscreen...
        // Unless the compatibility rule is set.
        if(get_bit(quest_rules, qr_OFFSCREENWEAPONS) || !clipped)
            return;
        goto offscreenCheck;
    }
    
    if(special==2)                                            // hit Link's mirror shield
    {
        switch(id)
        {
        case ewFireball2:
        case ewFireball:
            id = wRefFireball;
            ignoreLink=true;
            goto reflect;
            
        case ewRock:
        case ewSword:
        case wRefBeam:
        case ewMagic:
        case wRefMagic:
            //otherwise he can get hit by the newly-created projectile if he's walking into it fast enough -DD
            ignoreLink=true;
            id = ((id==ewMagic || id==wRefMagic) ? wRefMagic : id==ewRock ? wRefRock : wRefBeam);
            goto reflect;
            
        case wScript1:
        case wScript2:
        case wScript3:
        case wScript4:
        case wScript5:
        case wScript6:
        case wScript7:
        case wScript8:
        case wScript9:
        case wScript10:
            // If this isn't set, the weapon may reflect repeatedly
            ignoreLink=true;
reflect:

            if(angular) switch(linkdir)
                {
                case up:
                    angle += (PI - angle) * 2.0;
                    break;
                    
                case down:
                    angle = -angle;
                    break;
                    
                case left:
                    angle += ((-PI/2) - angle) * 2.0;
                    break;
                    
                case right:
                    angle += ((PI/2) - angle) * 2.0;
                    break;
                    
                default:
                    angle += PI;
                    break;
                }
            else
            {
                dir ^= 1;
                
                if(dir&2)
                    flip ^= 1;
                else
                    flip ^= 2;
            }
            
            return;
        }
    }
    
    if(special>=1)                                            // hit Link's shield
    {
        switch(id)
        {
        case ewRock:
        case ewMagic:
        case ewArrow:
        case ewSword:
            bounce=true;
            dead=16;
            return;
            
        case ewBrang:
            if(misc==0)
            {
                clk2=256;
                misc=1;
                dir^=1;
            }
            
            return;
        }
    }
    
offscreenCheck:
    switch(id)
    {
    case wSword:
    case wWand:
    case wHammer:
        break;
        
    case ewBomb:
        step=0;
        break;
        
    case ewLitBomb:
        step=0;
        misc=50;
        clk=misc-3;
        hxofs=hyofs=-7;
        hxsz=hysz=30;
        break;
        
    case ewSBomb:
        step=0;
        break;
        
    case ewLitSBomb:
        step=0;
        misc=50;
        clk=misc-3;
        hxofs=hyofs=-16;
        hxsz=hysz=48;
        break;
        
    case wLitBomb:
        if(!clipped) dead=1;
        
    case wLitSBomb:
        if(!clipped) dead=1;
        
    case wWhistle:
    case wBomb:
    case wSBomb:
    case wBait:
    case wFire:
    case wHSHandle:
    case wPhantom:
        break;                                   // don't worry about clipping or hits with these
        
    case ewFireTrail:
        if(!clipped) dead=1;
        
        break;
        
    case ewFlame:
        if(!clipped) dead=1;
        
        break;
        
    case wRefBeam:
    case wBeam:
        dead=23;
        break;
        
    case wArrow:
        dead=4;
        break;                           //findentrance(x,y,mfARROW,true); break;
        
    case ewArrow:
        dead=clipped?4:1;
        break;
        
    case wCByrna:
    
        // byrna keeps going
        if(!curQuest->isValid(parentitem) || !(curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG1))
            dead=0;
            
        break;
        
    case wWind:
        if(x>=240)
            dead=2;
            
        break;
        
    case wBrang:
        if(misc==0)
        {
            clk2=256;
            int deadval=(curQuest->getItemDefinition(curQuest->isValid(parentitem) ? parentitem : current_item_id(itype_brang)).flags & itemdata::IF_FLAG3)?-2:4;
            
            if(clipped)
            {
                dead=deadval;
            }
            else
            {
                if(deadval==-2)
                {
                    dead=deadval;
                }
                
                misc=1;
                /*
                  if (current_item(itype_brang,true)>1) {
                  if (dummy_bool[0]) {
                  add_grenade(x,y,z,current_item(itype_brang,true)>2);
                  dummy_bool[0]=false;
                  }
                  }
                  */
            }
        }
        
        break;
        
    case wHookshot:
        if(misc==0)
        {
            clk2=256;
            
            if(clipped)
                dead=4;
            else
                dead=1;
        }
        
        break;
        
    case ewBrang:
        if(misc==0)
        {
            clk2=256;
            dead=4;
        }
        
        break;
        
    case wRefMagic:
    case wMagic:
    //!Dimentio: Hey, let's add changing the book fire to other weapons, too!
    {
	if (curQuest->getItemDefinition(parentitem).family == itype_book && (x<272 && x>-17 && y<272 && y>-17)) break;
	dead=1; //remove the dead part to make the wand only die when clipped
		bookfirecreate();

	break; //!Over already? What a shame.
    }
        
        if(((id==wMagic && currentItemLevel(itype_book) &&
                (curQuest->getItemDefinition(current_item_id(itype_book)).flags & itemdata::IF_FLAG1))) && LwpnsIdCount(wFire)<2)
        {
            Lwpns.add(new weapon(x,y,z,wFire,2,1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
            Backend::sfx->play(WAV_FIRE,x);
        }
        
        break;
        
    case ewWind:
        if(clipped)
        {
            if(misc==999)                                       // in enemy wind
            {
                ewind_restart=true;
            }
            
            dead=1;
        }
        
        break;
        
    default:
        dead=1;
    }
}

// override hit detection to check for invicibility, etc
bool weapon::hit(sprite *s)
{
    if(!(scriptcoldet&1)) return false;
    
    if(id==ewBrang && misc)
        return false;
        
    return (dead!=-1&&dead!=-10) ? false : sprite::hit(s);
}

bool weapon::hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
{
    if(!(scriptcoldet&1)) return false;
    
    if(id==ewBrang && misc)
        return false;
        
    return (dead!=-1&&dead!=-10) ? false : sprite::hit(tx,ty,tz,txsz2,tysz2,tzsz2);
}

void weapon::update_weapon_frame(int change, int orig)
{
    if(extend > 2)
    {
        byte extra_rows=(tysz-1)*(((orig%TILES_PER_ROW)+(txsz*change))/TILES_PER_ROW);
        tile=orig+(txsz*change)+(extra_rows*TILES_PER_ROW);
    }
    else
        tile=orig+change;
}

void weapon::draw(BITMAP *dest)
{
    if(flash==1)
    {
        if(!BSZ)
        {
            cs = (id==wBeam || id==wRefBeam) ? 6 : o_cset&15;
            cs += frame&3;
        }
        else
        {
            if(id==wBeam || id==wRefBeam)
                cs = ((frame>>2)&1)+7;
            else
            {
                cs = o_cset&15;
                
                if(++csclk >= 12)
                    csclk = 0;
                    
                cs += csclk>>2;
            }
        }
    }
    
    if(flash>1)
    {
        if(++csclk >= (o_speed<<1))
            csclk = 0;
            
        cs = o_cset&15;
        
        if(csclk >= o_speed)
            cs = o_cset>>4;
    }
    
    if(frames)
    {
        if(id!=wBrang && id!=ewBrang)  // Brangs use clk2 for adjusting boomerang's flight range.
        {
            if(++clk2 >= o_speed)
            {
                clk2 = 0;
                
                if(frames>1 && ++aframe >= frames)
                {
                    aframe = 0;
                }
            }
        }
        else
        {
            if((clk2 % zc_max(1, o_speed))==0)
            {
                if(frames>1 && ++aframe >= frames)
                {
                    aframe = 0;
                }
            }
        }
        
        //tile = o_tile+aframe;
        update_weapon_frame(aframe,o_tile);
    }
    
    // do special case stuff
    switch(id)
    {
    case wSword:
    case wHammer:
        if(get_bit(quest_rules,qr_LINKFLICKER)&&((getClock()||LinkHClk())&&(frame&1)) ||
                Link->getDontDraw() || tmpscr->flags3&fINVISLINK)
            return;
            
    case wBeam:
    case wRefBeam:
    {
        if(dead==-1) break;
        
        // draw the beam shards
        int ofs=23-dead;
        int f = frame&3;
        int type2 = o_type;
        //tile = o_tile+((frames?frames:1)*2);
        update_weapon_frame(((frames?frames:1)*2),o_tile);
        
        if(type2)
            cs = o_cset>>4;
            
        if(type2==3 && (f&2))
            ++tile;
            
        if(!type2 || f==0 || (type2>1 && f==3)) overtile16(dest,tile,x-2-ofs,y+playing_field_offset-2-ofs-(z+zofs),cs,0);
        
        if(!type2 || f==2 || (type2>1 && f==1)) overtile16(dest,tile,x+2+ofs,y+playing_field_offset-2-ofs-(z+zofs),cs,1);
        
        if(!type2 || f==1 || (type2>1 && f==2)) overtile16(dest,tile,x-2-ofs,y+playing_field_offset+2+ofs-(z+zofs),cs,2);
        
        if(!type2 || f==3 || (type2>1 && f==0)) overtile16(dest,tile,x+2+ofs,y+playing_field_offset+2+ofs-(z+zofs),cs,3);
    }
    
    return;                                               // don't draw sword
    
    case wBomb:
    case wSBomb:
    case ewBomb:
    case ewSBomb:
    {
        if(!misc || clk<misc-2) break;
        
        // draw the explosion
        SpriteDefinitionRef id2;
        int boomframes = 0;
        
        switch(id)
        {
        case wBomb:
        {
            id2 = curQuest->specialSprites().defaultBombExplosion;

            if (curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).family != itype_wand) //!Dimentio: Yet another exception, for bombs.
            {
                id2 = curQuest->getItemDefinition(parentitem).wpns[1]; //This sets the explosion sprite, but because it is linked to itemdata
                        //it may not work with script generated explosions. -Z
            //Perhaps it would be better to add a special case to createlweapon and createeweapon to 
            //use a usesprite value with wpns.add
            }
            else if (curQuest->getItemDefinition(parentitem).family == itype_wand) id2 = curQuest->getItemDefinition(parentitem).wpns[3];


            break;
        }
        
        case wSBomb:
        {            
            id2 = curQuest->specialSprites().defaultSuperBombExplosion;
            
            if(curQuest->isValid(parentitem))
            {
                id2=curQuest->getItemDefinition(parentitem).wpns[1];
            }
            
            break;
        }
        
        case ewBomb:
        {            
            id2 = curQuest->specialSprites().defaultEnemyBombExplosion;
            break;
        }
            
        case ewSBomb:
            id2 = curQuest->specialSprites().defaultEnemySuperBombExplosion;
            break;
        }
        
        tile = curQuest->getSpriteDefinition(id2).tile;
        cs = curQuest->getSpriteDefinition(id2).csets&15;
        boomframes = curQuest->getSpriteDefinition(id2).frames;
        
        if(boomframes != 0)
        {
            //equally divide up the explosion time among the frames
            int perframe = (boomframes >= 34 ? 1 : (boomframes + 34)/boomframes);
            
            if(clk > misc)
                tile += (clk-misc)/perframe;
                
            //update_weapon_frame((clk-misc)/perframe,tile);
        }
        else if(clk>misc+22)
        {
            ++tile;
            //update_weapon_frame(1,tile);
        }
        
        overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-13-(z+zofs),cs,0);
        overtile16(dest,tile,x,y+yofs-(z+zofs),cs,0);
        overtile16(dest,tile,x+((clk&1)?-14:14),y+yofs-(z+zofs),cs,0);
        overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+14-(z+zofs),cs,0);
        
        if(id==wSBomb||id==ewSBomb)
        {
            overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-27-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?-21:21),y+yofs-13-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?-28:28),y+yofs-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?21:-21),y+yofs+14-(z+zofs),cs,0);
            overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+28-(z+zofs),cs,0);
        }
        else
        {
            ;
        }
        
        if(get_debug() && key[KEY_O])
            rectfill(dest,x+hxofs,y+hyofs+yofs-(z+zofs),
                     x+hxofs+hxsz-1,y+hyofs+hysz-1+yofs,vc(id));
                     
        return;                                               // don't draw bomb
    }
    
    case wArrow:
    case ewArrow:
        if(dead>0 && !bounce)
        {
            cs=7;
            tile=54;
            flip=0;
        }
        
        break;
        
    case ewFireTrail:
    case ewFlame:
    case wFire:
    
        //if no animation, flip tile
        if(frames==0)
            flip = o_flip & (clk>>2);
            
        break;
        
    case ewBrang:
    case wBrang:
        cs = o_cset&15;
        
        if(!curQuest->isValid(parentitem) || !(curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG1))
        {
            tile = o_tile;
            
            if(BSZ)
                flip = bszboomflip[(clk>>2)&3];
            else
            {
                //tile = boomframe[clk&0xE] + o_tile;
                update_weapon_frame(boomframe[clk&0xE],o_tile);
                flip = boomframe[(clk&0xE)+1];
            }
            
            if(curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2)
            {
                update_weapon_frame((BSZ?1:4)*dir,tile);
            }
        }
        else
        {
            if(curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2)
            {
                update_weapon_frame(zc_max(frames,1)*dir,tile);
            }
        }
        
        if(dead>0)
        {
            if(curQuest->isValid(parentitem) && curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG1)
            {
                tile=o_tile+(frames*(curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG2)?8:1);
            }
            else
            {
                cs=7;
                tile=54;
                flip=0;
            }
        }
        
        break;
        
    case wHookshot:
        break;
        
    case wWind:
        if(frames==0)
            flip ^= o_flip;
            
        if((dead!=-1) && !BSZ)
            tile = curQuest->getSpriteDefinition(temp1).tile;//wpnsbuf[wFIRE].tile;
            
        break;
        
    case ewWind:
        /*
          if(wpnsbuf[wid].frames==0)
          flip ^= (wpnsbuf[wid].misc>>2)&3;
          */
        break;
        
    case wPhantom:
        switch(type)
        {
        case pNAYRUSLOVEROCKET1:
        case pNAYRUSLOVEROCKETRETURN1:
        case pNAYRUSLOVEROCKETTRAIL1:
        case pNAYRUSLOVEROCKETTRAILRETURN1:
        case pNAYRUSLOVEROCKET2:
        case pNAYRUSLOVEROCKETRETURN2:
        case pNAYRUSLOVEROCKETTRAIL2:
        case pNAYRUSLOVEROCKETTRAILRETURN2:
            if(curQuest->isValid(parentitem) && (curQuest->getItemDefinition(parentitem).flags & itemdata::IF_FLAG1 ? 1 : 0)&&!(frame&1))
            {
                return;
            }
            
            break;
        }
        
        break;
    }
    
    // draw it
    sprite::draw(dest);
}

void putweapon(BITMAP *dest,int x,int y,int weapon_id, int type, int dir, int &aclk, int &aframe, int parentid)
{
    weapon temp((fix)x,(fix)y,(fix)0,weapon_id,type,0,dir,ItemDefinitionRef(),parentid,-1);
    temp.ignorecombo=((dir==up?y+8:y)&0xF0)+((dir==left?x+8:x)>>4); // Lens hints can sometimes create real weapons without this
    temp.ignoreLink=true;
    temp.yofs=0;
    temp.clk2=aclk;
    temp.aframe=aframe;
    temp.animate(0);
    temp.draw(dest);
    aclk=temp.clk2;
    aframe=temp.aframe;
}

void weapon::bookfirecreate()
{
	int bookmagicmaxcount = 2; //!Dimentio: BOOKMAGICHANDLER
	if (curQuest->getItemDefinition(current_item_id(itype_book)).misc3 > 0)
	{
		bookmagicmaxcount = curQuest->getItemDefinition(current_item_id(itype_book)).misc3;
	}
	else if (curQuest->getItemDefinition(parentitem).misc8 > 0)
	{
		bookmagicmaxcount = (curQuest->getItemDefinition(parentitem).misc8 + 1) * 2;
	}
	else bookmagicmaxcount = 2;
	if(((curQuest->getItemDefinition(parentitem).family == itype_wand && currentItemLevel(itype_book) && (curQuest->getItemDefinition(current_item_id(itype_book)).flags & itemdata::IF_FLAG1))) && LwpnsIdCount(curQuest->getItemDefinition(current_item_id(itype_book)).misc2)<bookmagicmaxcount)
	{
		switch(curQuest->getItemDefinition(current_item_id(itype_book)).misc4)
		{
			
			case 3:
			{
				Lwpns.add(new weapon(x,y,z,wBomb,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1)); //Let's make an explosion...
				Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1)); //And here's the dummy info holder
				break;
			}
			case 4:
			{
				Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
				break;
			}
			case 5:
			{
				Lwpns.add(new weapon(x,y,z,wBomb,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1)); //Let's make an explosion...
				Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1)); //And here's the dummy info holder
				break;
			}
			case 6:
			{
				Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
				break;
			}
			case 7:
			{
				Lwpns.add(new weapon(x,y,z,wBomb,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1)); //Let's make an explosion...
				Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1)); //And here's the dummy info holder
				break;
			}
			case 8:
			{
				Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
				break;
			}
			default:
			{
				//!Dimentio: Making sure this isn't null
				//! ZoriaRPG: I need a detailed explanantion of what this is doing as a code comment. 
				if (curQuest->getItemDefinition(current_item_id(itype_book)).misc2 <= 0) Lwpns.add(new weapon(x,y,z,wFire,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
				//!Dimentio: here's the default behavior.
				else Lwpns.add(new weapon(x,y,z,curQuest->getItemDefinition(current_item_id(itype_book)).misc2,2,curQuest->getItemDefinition(current_item_id(itype_book)).misc1*DAMAGE_MULTIPLIER,0,current_item_id(itype_book),-1));
				break;
			}
		}
		Backend::sfx->play(WAV_FIRE,x); // sfx() is deprecated. -Z
	}
}

/*** end of weapons.cpp ***/

