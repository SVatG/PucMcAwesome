#include <nds.h>
#include <nds/fifocommon.h>
#include <stdlib.h>
#include <maxmod9.h>

extern const u8 soundbank_bin_end[];
extern const u8 soundbank_bin[];
extern const u32 soundbank_bin_size;

#include "gfx/pucmcawesome.h"
#include "gfx/bgbottom.png.h"
#include "gfx/bgtop.png.h"
#include "gfx/bgtop_menu.png.h"
#include "gfx/girder.h"
#include "gfx/png_shared.h"
#include "gfx/ghosties.h"
#include "soundbank.h"

// gfx
u16* puc;
u16* puc_sub;
u16* gird;
u16* gird_sub;
u16* ghosties;

// girder positions
float girdx[4];
float girdy[4];

void drawGirds(){
	for( int i = 0; i < 4; i++ ) {
		if( girdy[i] <= 193 ) {
			oamSet(
				&oamMain,
				2*(i+1)-1, girdx[i], girdy[i], 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				gird,
				-1, false, false, false, false, false
			);
			oamSet(
				&oamMain,
				2*(i+1), girdx[i]+28, girdy[i], 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				gird,
				-1, false, false, false, false, false
			);
			oamClear(&oamSub,2*(i+1)-1,2);
		}
		else {
			oamSet(
				&oamSub,
				2*(i+1)-1, girdx[i], girdy[i]-198, 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				gird_sub,
				-1, false, false, false, false, false
			);
			oamSet(
				&oamSub,
				2*(i+1), girdx[i]+28, girdy[i]-198, 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				gird_sub,
				-1, false, false, false, false, false
			);
			oamClear(&oamMain,2*(i+1)-1,2);

		}
	}
}

int main()
{
// 	irqInit();
// 	irqEnable(IRQ_VBLANK);
// 	fifoInit();

	int mode = 1;

	mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLoad( MOD_OH_SCHEISSE_MP );
	mmStart( MOD_OH_SCHEISSE_MP , MM_PLAY_LOOP );
	
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);

	// Load puc
	puc = oamAllocateGfx(&oamMain, SpriteSize_32x32*6, SpriteColorFormat_256Color);
	dmaCopy((u8*)pucmcawesomeTiles, puc, 32*32*6);
	puc_sub = oamAllocateGfx(&oamSub, SpriteSize_32x32*6, SpriteColorFormat_256Color);
	dmaCopy((u8*)pucmcawesomeTiles, puc_sub, 32*32*6);


	// Load girder
	gird = oamAllocateGfx(&oamMain, SpriteSize_32x64, SpriteColorFormat_256Color);
	dmaCopy((u8*)girderTiles, gird, 32*64*2);
	gird_sub = oamAllocateGfx(&oamSub, SpriteSize_32x64, SpriteColorFormat_256Color);
	dmaCopy((u8*)girderTiles, gird_sub, 32*64*2);

	// Load ghosties
	ghosties = oamAllocateGfx(&oamSub, SpriteSize_32x32*6, SpriteColorFormat_256Color);
	dmaCopy((u8*)ghostiesTiles, ghosties, 32*32*6);

	// Palette
	dmaCopy(png_sharedPal, SPRITE_PALETTE, 512);
	dmaCopy(png_sharedPal, SPRITE_PALETTE_SUB, 512);

	int bg = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	bgSetPriority(bg, 2);
	dmaCopy(bgtop_pngBitmap, bgGetGfxPtr(bg), 256*256);
	dmaCopy(bgtop_pngPal, BG_PALETTE, 256*2);

	int bg2 = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 1,0);
	bgSetPriority(bg2, 2);
	dmaCopy(bgbottom_pngBitmap, bgGetGfxPtr(bg2), 256*256);
	dmaCopy(bgbottom_pngPal, BG_PALETTE_SUB, 256*2);

	int x = (256-32)/2.0;
	int y = 0;
	bool nose_right = 0;
	float acc_x = 0;
	float acc_y = 0;
	int frame = 0;
	int framedelay = 0;
	float gird_accel=0.2;
	bool grounded = false;
	float t;
	float s;
	// Girdpos init
	for(int i = 0; i < 4; i++ ) {
		girdy[i] = (384/4) * i;
		girdx[i] = rand() % 200;
	}

start:
	s = 0;
	dmaCopy(bgtop_pngBitmap, bgGetGfxPtr(bg), 256*256);
	dmaCopy(bgtop_pngPal, BG_PALETTE, 256*2);
	
	while( mode == 0 ) {
// 		iprintf("\n Lets print pretty text on the\n NDS screen ");
// 		iprintf("\x1b[32musing devkitpro!\n\n");
// 		iprintf("\x1b[32;1m All we need now is actual game\n code!\x1b[39m");
		scanKeys();
		int keys = keysHeld();
		t+=0.05;
		s += 1/60.0;
		
		if( keys & KEY_A) {
			// Should be: "on platform"
			if(grounded) {
				acc_y = 4;
				grounded = false;
			}
		}
		if( keys & KEY_RIGHT) {
			acc_x = -2;
			nose_right = true;
		} else if( keys & KEY_LEFT) {
			acc_x = 2;
			nose_right = false;
		} else {
			acc_x = 0;
		}

		// Move puc
		for(int i = 0; i < 4; i++ ) {
			bool found_ground = 0;
			if( x > girdx[i]-16 && x < girdx[i]+50 && y < (int)girdy[i]-26 && y>(int)girdy[i] - 33){
				acc_y = gird_accel;
				grounded = true;
			}
		}
		y -= acc_y - gird_accel/2.0;
		x -= acc_x;
		if( x < -32 ) {
			x = 256+32;
		}
		if( x > 256+32 ) {
			x = -31;
		}
		if(acc_y > -5.0) {
			acc_y-= 0.1;
		}
		
		if( y >= 384-32 ) {
			acc_y = 0;
			y = 384-32;
			// DIE
			mode = 1;
		}
		framedelay = (framedelay + 1)%3;
		if(framedelay == 0) {
			frame = (frame + 1) % 6;
		}

		// Move girds
		for(int i = 0; i < 4; i++ ) {
			girdy[i] += gird_accel;
			if(girdy[i] > 384) {
				girdy[i] = 0;
				girdx[i] = rand() % 200;
			}
		}
		gird_accel+=0.001;
		
		// Draw things on screen.
		if( y <= 193 ) {
			oamSet(
				&oamMain,
				0, x, y, 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				puc + (frame * 32 * 32)/2,
				-1, false, false, nose_right, false, false
			);
		}
		if( y >= 193-32 ) {
			oamSet(
				&oamSub,
				0, x, y-198, 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				puc_sub + (frame * 32 * 32)/2,
				-1, false, false, nose_right, false, false
			);
		}

		// Ghosties
		for( int i = 0; i < 20; i++ ) {
			oamSet(
				&oamSub,
				10+i, i*20-32*4+(int)(t*20.0)%(20*3), 167+sin(t*2.0)*5.0+sin((i*20-32*4+(int)(t*20.0)%(20*3))/20.0)*10.0, 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				ghosties + ((i%3) * 32 * 32)/2,
				-10, false, false, true, false, false
			);
			oamSet(
				&oamSub,
				30+i, 256-i*20+32*4-(int)(t*25.0)%(20*3), 175+cos(t*2.3)*5.0+cos((256-i*20+32*4-(int)(t*25.0)%(20*3))/20.0)*10.0, 0, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				ghosties + ((i%3) * 32 * 32)/2,
				-10, false, false, false, false, false
			);
		}
		
		drawGirds();

		swiWaitForVBlank();
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}
	dmaCopy(bgtop_menu_pngBitmap, bgGetGfxPtr(bg), 256*256);
	dmaCopy(bgtop_menu_pngPal, BG_PALETTE, 256*2);

	while( mode == 1 ) {
// 		iprintf("\n Lets print pretty text on the\n NDS screen ");
// 		iprintf("\x1b[32musing devkitpro!\n\n");
// 		iprintf("\x1b[32;1m All we need now is actual game\n code!\x1b[39m");
		scanKeys();
		int keys = keysHeld();
		if( keys & KEY_START ) {
			mode = 0;
		}
		framedelay = (framedelay + 1)%3;
		if(framedelay == 0) {
			frame = (frame + 1) % 6;
		}
		
		// Reset.
		x = (256-32)/2.0;
		y = 0;
		nose_right = 0;
		acc_x = 0;
		acc_y = 0;
		frame = 0;
		framedelay = 0;
		gird_accel=0.2;
		grounded = false;

		// Girdpos init
		for(int i = 0; i < 4; i++ ) {
			girdy[i] = (384/4) * i;
			girdx[i] = rand() % 200;
		}


		// Ghosties
		t+=0.05;
		for( int i = 0; i < 20; i++ ) {
			oamSet(
				&oamSub,
				10+i, i*20-32*4+(int)(t*20.0)%(20*3), 167+sin(t*2.0)*5.0+sin((i*20-32*4+(int)(t*20.0)%(20*3))/20.0)*10.0, 1, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				ghosties + ((i%3) * 32 * 32)/2,
				-10, false, false, true, false, false
			);
			oamSet(
				&oamSub,
				50+i, 256-i*20+32*4-(int)(t*25.0)%(20*3), 175+cos(t*2.3)*5.0+cos((256-i*20+32*4-(int)(t*25.0)%(20*3))/20.0)*10.0, 0, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				ghosties + ((i%3) * 32 * 32)/2,
				-10, false, false, false, false, false
			);
			oamSet(
				&oamSub,
				30+i, i*20-32*4+(int)(t*10.0)%(20*3), 185+sin(t*2.0)*5.0+sin((i*20-32*4+(int)(t*10.0)%(20*3))/20.0)*4.0, 0, 0,
				SpriteSize_32x32,
				SpriteColorFormat_256Color,
				ghosties + ((i%3) * 32 * 32)/2,
				-10, false, false, true, false, false
			);
		}
		
		swiWaitForVBlank();
		oamClear(&oamMain,0,0);
		oamClear(&oamSub,0,10);
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}

	goto start;
	return 0;
}
	