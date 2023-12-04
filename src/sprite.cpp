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

#include "zdefs.h"
#include "sprite.h"
#include "tiles.h"
#include "quest/Quest.h"

extern Quest *curQuest;
extern bool get_debug();
extern bool halt;
extern bool show_sprites;
extern bool show_hitboxes;
extern bool is_zquest();
extern void debugging_box(int x1, int y1, int x2, int y2);
extern ObjectPool *pool;

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

sprite::sprite(ObjectPool &pool) : GameObject(pool)
{
    x=y=z=tile=shadowtile=cs=flip=c_clk=clk=xofs=yofs=zofs=hxofs=hyofs=fall=0;
    txsz=1;
    tysz=1;
    hxsz=hysz=16;
    hzsz=1;
	
	//Enemy and Weapon Editor Hit Sizing
	//if ( txsz == 0 ) txsz = 1; 
	//if ( tysz == 0 ) tysz = 1;
	//if ( hxsz == 0 ) hxsz = 16; 
	//if ( hysz == 0 ) hysz = 16;
    yofs=playing_field_offset;
    dir=down;
    angular=canfreeze=false;
    drawstyle=0;
    extend=0;
    
    /*ewpnclass=0;
    lwpnclass=0;
    guyclass=0;*/ //Not implemented
    //ewpnref=0;
    //lwpnref=0;
    //guyref=0;
    //itemref=0;
    lasthitclk=0;
    lasthit=0;
    angle=0;
    misc=0;
    
    for(int i=0; i<10; i++)
    {
        dummy_int[i]=0;
        dummy_fix[i]=0;
        dummy_float[i]=0;
        dummy_bool[i]=0;
    }
    
    /*for(int i=0;i<8;i++)
    {
      if(i<2) a[i]=0;
      d[i]=0;
    }
    scriptflag=0;
    pc=0;
    sp=0;
    itemclass=0;
    ffcref=0; */
    //scriptData.Clear(); //when we have npc scripts we'll need this again, for now not.
    //doscript=1;
    for(int i=0; i<16; i++) miscellaneous[i] = 0;
    
    scriptcoldet = 1;
}

sprite::sprite(sprite const & other) : GameObject(other),
    x(other.x),
    y(other.y),
    z(other.z),
    fall(other.fall),
    tile(other.tile),
    shadowtile(other.shadowtile),
    cs(other.cs),
    flip(other.flip),
    c_clk(other.c_clk),
    clk(other.clk),
    misc(other.misc),
    xofs(other.xofs),
    yofs(other.yofs),
    zofs(other.zofs),
    hxofs(other.hxofs),
    hyofs(other.hyofs),
    hxsz(other.hxsz),
    hysz(other.hysz),
    hzsz(other.hzsz),
    txsz(other.txsz),
    tysz(other.tysz),
    dir(other.dir),
    angular(other.angular),
    canfreeze(other.canfreeze),
    angle(other.angle),
    lasthit(other.lasthit),
    lasthitclk(other.lasthitclk),
    drawstyle(other.drawstyle),
    extend(other.extend)
{    
    
    for(int i=0; i<10; ++i)
    {
        dummy_int[i]=other.dummy_int[i];
        dummy_fix[i]=other.dummy_fix[i];
        dummy_float[i]=other.dummy_float[i];
        dummy_bool[i]=other.dummy_bool[i];
    }
    
    /*for (int i=0; i<8; ++i)
    {
      d[i]=other.d[i];
    }
    for (int i=0; i<2; ++i)
    {
      a[i]=other.a[i];
    }*/
    for(int i=0; i<16; i++) miscellaneous[i] = other.miscellaneous[i];
    
    scriptcoldet = other.scriptcoldet;
}

sprite::sprite(ObjectPool &pool, fix X,fix Y,int T,int CS,int F,int Clk,int Yofs) :
	GameObject(pool),
    x(X),y(Y),tile(T),cs(CS),flip(F),clk(Clk),yofs(Yofs)
{    	
    hxsz=hysz=16;
    hxofs=hyofs=xofs=0;
	//Enemy and Weapon Editor Hit Sizing
	//if ( txsz == 0 ) txsz = 1; 
	//if ( tysz == 0 ) tysz = 1;
	//if ( hxsz == 0 ) hxsz = 16; 
	//if ( hysz == 0 ) hysz = 16;
    txsz=1;
    tysz=1;
    dir=down;
    angular=canfreeze=false;
    extend=0;
    
    /*for(int i=0;i<8;i++)
    {
      if(i<2) a[i]=0;
      d[i]=0;
    }
    scriptflag=0;
    pc=0;
    sp=0;
    ffcref=0;
    doscript=1;*/
    //itemclass=0;
    for(int i=0; i<16; i++) miscellaneous[i] = 0;
    
    scriptcoldet = 1;
    
    //scriptData.Clear();
    /*ewpnclass=0;
    lwpnclass=0;
    guyclass=0;*/ //Not implemented
    /*ewpnref=0;
    lwpnref=0;
    guyref=0;
    itemref=0;*/
    drawstyle=0;
    lasthitclk=0;
    lasthit=0;
    angle=0;
    hzsz=1;
    misc=0;
    c_clk=0;
    shadowtile=0;
}

sprite::~sprite()
{
}

void sprite::draw2(BITMAP *)                            // top layer for special needs
{
}

void sprite::drawcloaked2(BITMAP *)                     // top layer for special needs
{
}

bool sprite::animate(int)
{
    ++c_clk;
    return false;
}
int sprite::real_x(fix fx)
{
    int rx=fx.v>>16;
    
    switch(dir)
    {
    case 9:
    case 13:
        if(fx.v&0xFFFF)
            ++rx;
            
        break;
    }
    
    return rx;
}

int sprite::real_y(fix fy)
{
    return fy.v>>16;
}

int sprite::real_z(fix fz)
{
    return fz.v>>16;
}

bool sprite::hit(sprite *s)
{
    if(!(scriptcoldet&1)) return false;
    
    if(clk<0) return false;
    
    if(halt)
    {
    }
    
    return hit(s->x+s->hxofs,s->y+s->hyofs,s->z+s->zofs,s->hxsz,s->hysz,s->hzsz);
}

bool sprite::hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
{
    if(!(scriptcoldet&1)) return false;
    
    if(clk<0) return false;
    
    return tx+txsz2>x+hxofs &&
           ty+tysz2>y+hyofs &&
           tz+tzsz2>z+zofs &&
           
           tx<x+hxofs+hxsz &&
           ty<y+hyofs+hysz &&
           tz<z+zofs+hzsz;
}

int sprite::hitdir(int tx,int ty,int txsz2,int tysz2,int dir2)
{
    if(!(scriptcoldet&1)) return 0xFF;
    
    int cx1=x+hxofs+(hxsz>>1);
    int cy1=y+hyofs+(hysz>>1);
    int cx2=tx+(txsz2>>1);
    int cy2=ty+(tysz2>>1);
    
    if(dir2>=left && abs(cy1-cy2)<=8)
        return (cx2-cx1<0)?left:right;
        
    return (cy2-cy1<0)?up:down;
}

void sprite::move(fix dx,fix dy)
{
    x+=dx;
    y+=dy;
}

void sprite::move(fix s)
{
    if(angular)
    {
        x += cos(angle)*s;
        y += sin(angle)*s;
        return;
    }
    
    switch(dir)
    {
    case 8:
    case up:
        y-=s;
        break;
        
    case 12:
    case down:
        y+=s;
        break;
        
    case 14:
    case left:
        x-=s;
        break;
        
    case 10:
    case right:
        x+=s;
        break;
        
    case 15:
    case l_up:
        x-=s;
        y-=s;
        break;
        
    case 9:
    case r_up:
        x+=s;
        y-=s;
        break;
        
    case 13:
    case l_down:
        x-=s;
        y+=s;
        break;
        
    case 11:
    case r_down:
        x+=s;
        y+=s;
        break;
        
    case -1:
        break;
    }
}

void sprite::draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,0,0,cs,flip);
                overtile16(temp,tile,0,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,0,0,cs,flip,128);
                overtiletranslucent16(temp,tile,0,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,0,0,flip);
                overtilecloaked16(temp,tile,0,16,flip);
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,16,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip);
                overtile16(temp,tile,16,16,cs,flip);
                overtile16(temp,tile-(flip?-1:1),0,16,cs,flip);
                overtile16(temp,tile+(flip?-1:1),32,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,16,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip,128);
                overtiletranslucent16(temp,tile,16,16,cs,flip,128);
                overtiletranslucent16(temp,tile-(flip?-1:1),0,16,cs,flip,128);
                overtiletranslucent16(temp,tile+(flip?-1:1),32,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,16,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,flip);
                overtilecloaked16(temp,tile,16,16,flip);
                overtilecloaked16(temp,tile-(flip?-1:1),0,16,flip);
                overtilecloaked16(temp,tile+(flip?-1:1),32,16,flip);
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            
            switch(flip)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,flip);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,flip);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,tile,sx,sy,cs,flip);
                else if(drawstyle==1)
                    overtiletranslucent16(dest,tile,sx,sy,cs,flip,128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,tile,sx,sy,flip);
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            SpriteDefinitionRef spawns = curQuest->specialSprites().enemySpawnCloud;
            int t  = curQuest->getSpriteDefinition(spawns).tile;
            int cs2 = curQuest->getSpriteDefinition(spawns).csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            //TODO: wtf??
            SpriteDefinitionRef ref("CORE", extend);
            sprite w(*pool, (fix)sx,(fix)sy,curQuest->getSpriteDefinition(ref).tile,curQuest->getSpriteDefinition(ref).csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            
            if(BSZ)
            {
                if(clk>=-10)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-5)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            else
            {
                if(clk>=-12)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-6)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            
            w.draw(dest);
        }
    }
    
    // changed the color modulation to be based on UID -DD
    if (show_hitboxes && !is_zquest())
        rect(dest, x + hxofs, y + playing_field_offset + hyofs - (z + zofs), x + hxofs + hxsz - 1, (y + playing_field_offset + hyofs + hysz - (z + zofs)) - 1, vc((getUID()) % 255));
}

void sprite::draw8(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(clk>=0)
    {
        switch(drawstyle)
        {
        case 0:                                               //normal
            overtile8(dest,tile,sx,sy,cs,flip);
            break;
            
        case 1:                                               //phantom
            overtiletranslucent8(dest,tile,sx,sy,cs,flip,128);
            break;
        }
    }
}

void sprite::drawcloaked(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(clk>=0)
    {
        overtilecloaked16(dest,tile,sx,sy,flip);
    }
    else
    {
        SpriteDefinitionRef spawns = curQuest->specialSprites().enemySpawnCloud;
        int t  = curQuest->getSpriteDefinition(spawns).tile;
        int cs2 = curQuest->getSpriteDefinition(spawns).csets&15;
        
        if(BSZ)
        {
            if(clk>=-10) ++t;
            
            if(clk>=-5) ++t;
        }
        else
        {
            if(clk>=-12) ++t;
            
            if(clk>=-6) ++t;
        }
        
        overtile16(dest,t,x,sy,cs2,0);
    }
    
    // changed color modulation to base on UID -DD
    if(get_debug() && key[KEY_O])
        rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(getUID()));
}

void sprite::drawshadow(BITMAP* dest,bool translucent)
{
    if(extend == 4 || shadowtile==0)
    {
        return;
    }
    
    SpriteDefinitionRef shadows = curQuest->specialSprites().smallShadow;

    int shadowcs = curQuest->getSpriteDefinition(shadows).csets & 0xFFFF;
    int shadowflip = curQuest->getSpriteDefinition(shadows).misc & 0xFF;
    
    int sx = real_x(x+xofs)+(txsz-1)*8;
    int sy = real_y(y+yofs+(tysz-1)*16);
    
    if(clk>=0)
    {
        if(translucent)
        {
            overtiletranslucent16(dest,shadowtile,sx,sy,shadowcs,shadowflip,128);
        }
        else
        {
            overtile16(dest,shadowtile,sx,sy,shadowcs,shadowflip);
        }
    }
}

/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/

#define SLMAX 255

//class enemy;

sprite_list::sprite_list() : count(0) {}
void sprite_list::clear()
{
    while(count>0) del(0);
}

sprite *sprite_list::spr(int index)
{
    if(index<0 || index>=count)
        return NULL;
        
    return sprites[index];
}

bool sprite_list::swap(int a,int b)
{
    if(a<0 || a>=count || b<0 || b>=count)
        return false;
        
    sprite *c = sprites[a];
    sprites[a] = sprites[b];
    sprites[b] = c;    
// checkConsistency();
    return true;
}

bool sprite_list::add(sprite *s)
{
    if(count>=SLMAX)
    {
        delete s;
        return false;
    }
    
    sprites[count++]=s;
    //checkConsistency();
    return true;
}

bool sprite_list::remove(sprite *s)
// removes pointer from list but doesn't delete it
{
    int j=0;
    
    for(; j<count; j++)
        if(sprites[j] == s)
            goto gotit;
            
    return false;
    
gotit:

    for(int i=j; i<count-1; i++)
    {
        sprites[i]=sprites[i+1];
    }
    
    --count;
    //checkConsistency();
    return true;
}

fix sprite_list::getX(int j)
{
    if((j>=count)||(j<0))
    {
        return (fix)1000000;
    }
    
    return sprites[j]->x;
}

fix sprite_list::getY(int j)
{
    if((j>=count)||(j<0))
    {
        return (fix)1000000;
    }
    
    return sprites[j]->y;
}

int sprite_list::getMisc(int j)
{
    if((j>=count)||(j<0))
    {
        return -1;
    }
    
    return sprites[j]->misc;
}

bool sprite_list::del(int j)
{
    if(j<0||j>=count)
        return false;
        
    delete sprites[j];
    
    for(int i=j; i<count-1; i++)
    {
        sprites[i]=sprites[i+1];
    }
    
    --count;
    //checkConsistency();
    return true;
}

void sprite_list::draw(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
        {
            sprites[i]->draw(dest);
        }
        
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
        {
            sprites[i]->draw(dest);
        }
        
        break;
    }
}

void sprite_list::drawshadow(BITMAP* dest,bool translucent, bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->drawshadow(dest,translucent);
            
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
            sprites[i]->drawshadow(dest,translucent);
            
        break;
    }
}

void sprite_list::draw2(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->draw2(dest);
            
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
            sprites[i]->draw2(dest);
            
        break;
    }
}

void sprite_list::drawcloaked2(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->drawcloaked2(dest);
            
        break;
        
    case false:
    
        for(int i=count-1; i>=0; i--)
            sprites[i]->drawcloaked2(dest);
            
        break;
    }
}

void sprite_list::animate()
{
    int i=0;
    
    while(i<count)
    {
        if(!(freeze_guys && sprites[i]->canfreeze))
        {
            if(sprites[i]->animate(i))
            {
                del(i);
                --i;
            }
        }
        
        ++i;
    }
}

void sprite_list::check_conveyor()
{
    int i=0;
    
    while(i<count)
    {
        sprites[i]->check_conveyor();
        ++i;
    }
}

int sprite_list::Count()
{
    return count;
}

int sprite_list::hit(sprite *s)
{
    for(int i=0; i<count; i++)
        if(sprites[i]->hit(s))
            return i;
            
    return -1;
}

int sprite_list::hit(int x,int y,int z, int xsize, int ysize, int zsize)
{
    for(int i=0; i<count; i++)
        if(sprites[i]->hit(x,y,z,xsize,ysize,zsize))
            return i;
            
    return -1;
}

/**********************************/
/********** Moving Block **********/
/**********************************/

movingblock::movingblock() : sprite(*pool)
{
}

void movingblock::draw(BITMAP *dest)
{
    if(clk)
    {
        //    sprite::draw(dest);
        overcombo(dest,real_x(x+xofs),real_y(y+yofs),bcombo ,cs);
    }
}

/*** end of sprite.cc ***/

