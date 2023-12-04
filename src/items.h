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

#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "sprite.h"
#include "zdefs.h"


extern int fairy_cnt;
void movefairy(fix &x,fix &y,int misc);
void killfairy(int misc);
bool addfairy(fix x, fix y, int misc3, int id);
bool can_drop(fix x, fix y);
ItemDefinitionRef select_dropitem(int item_set, int x, int y);
bool is_side_view();

class item : public sprite
{
public:
    int pickup,clk2;
    int aclk,aframe;
    char PriceIndex;
    bool flash,twohand,anim, subscreenItem;
    int o_tile,o_cset, o_speed, o_delay, frames;
    ItemDefinitionRef itemDefinition;
    item(fix X,fix Y,fix Z, ItemDefinitionRef ref,int p,int c, bool isDummy = false);
    virtual ~item();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

// easy way to draw an item
void putitem(BITMAP *dest,int x,int y,const ItemDefinitionRef &itemref);
void putitem2(BITMAP *dest,int x,int y, const ItemDefinitionRef &itemref, int &aclk, int &aframe, int flash);
#endif
/*** end of sprite.cc ***/

