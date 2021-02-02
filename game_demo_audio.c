/******************************************************
 *    Filename:     game_demo_audio.c 
 *     Purpose:     test app for game demo with WAV audio
 *  Target Plf:     ZYBO (azplf)
 *  Created on: 	2021/01/31
 *      Author: 	atsupi.com
 *     Version:		1.0
 ******************************************************/

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "azplf_bsp.h"
#include "azplf_hal.h"
#include "azplf_util.h"
#include "azplf_audio.h"

// Game definitions
#define INITIAL_SCENE				0			// initial game scene number

//extern int azplf_start_game_thread(int scene, fb_render_handler fb_renderer);

// Frame buffer addresses
static u32 WriteFrameAddr[NUMBER_OF_FRAMES];
static u32 ResourceAddr; // resource buffer
static u32 mappedResAddr; // logical address
static u32 CamFrameAddr; // dummy camera buffer

// driver instances
static VdmaInstance vdmaInst_0;
static VdmaInstance vdmaInst_1;  /* dummy for testing */
static LQ070outInstance lq070Inst;
static GfxaccelInstance gfxaccelInst;

// frame buffer usage flags
static int fbActive;
static int fbBackgd;

// flag for wav file playback
static int wavfile_played = 0;

static int quit = 0;

static Sprite Sprite1 = {
	448, // x;
	224, // y;
	32, // dx;
	32, // dy;
	0, // src_x;
	0, // src_y;
	{ // anim
		0, // used;
		0, // num_anim;
		0, // curr_num;
		0, // reserved;
		0, // duration;
		0, // prev_time;
		{ 0 }, // src_x[8];
		{ 0 }, // src_y[8];
		{ 0 }, // msk_x[8];
		{ 0 }, // msk_y[8];
	}
};

static Sprite Sprite2 = {
	384, // x;
	224, // y;
	32, // dx;
	32, // dy;
	0, // src_x;
	0, // src_y;
	{ // anim
		1, // used;
		2, // num_anim;
		0, // curr_num;
		0, // reserved;
		30, // duration; 1/60s unit
		0, // prev_time;
		{ 0, 1, -1, -1, -1, -1, -1, -1 }, // src_x[8]; put -1 if not used
		{ 2, 2, -1, -1, -1, -1, -1, -1 }, // src_y[8]; put -1 if not used
		{ 0, 1, -1, -1, -1, -1, -1, -1 }, // msk_x[8]; put -1 if not used
		{ 1, 1, -1, -1, -1, -1, -1, -1 }, // msk_y[8]; put -1 if not used
	}
};

// Map Data
static u8 mapData[] = { // 15x15
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 
	0x02, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x02, 
	0x02, 0x01, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x10, 0x01, 0x02, 
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 

	0x02, 0x01, 0x01, 0x01, 0x01, 0x06, 0x05, 0x06, 0x05, 0x06, 0x01, 0x01, 0x01, 0x01, 0x02, 
	0x02, 0x01, 0x07, 0x07, 0x07, 0x05, 0x06, 0x05, 0x06, 0x05, 0x07, 0x07, 0x07, 0x01, 0x02, 
	0x02, 0x01, 0x07, 0x01, 0x07, 0x06, 0x05, 0x06, 0x05, 0x06, 0x07, 0x01, 0x07, 0x01, 0x02, 
	0x02, 0x01, 0x07, 0x01, 0x07, 0x05, 0x06, 0x05, 0x06, 0x05, 0x07, 0x01, 0x07, 0x01, 0x02, 
	0x02, 0x01, 0x01, 0x01, 0x01, 0x06, 0x05, 0x06, 0x05, 0x06, 0x01, 0x01, 0x01, 0x01, 0x02, 

	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 
	0x02, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x02, 
	0x02, 0x01, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x10, 0x01, 0x02, 
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};

/******************* Function Prototypes ************************************/

static int ReadSetup(VdmaInstance *inst);

static u32 mapResourceFBtoMem(u32 baseAddr)
{
	int fd;
	u32 result;

	printf("In mapResourceFBtoMem()\n");
	printf("File page size=0x%08x (%dKB)\n", frame_page, frame_page>>10);

	fd = open("/dev/mem", O_RDWR | O_SYNC); // no cache used
	result = (u32)mmap(NULL, frame_page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, baseAddr);
	printf("Mapping I/O: 0x%08x to vmem: 0x%08x\n", baseAddr, result);
	close(fd);

	if (result == 0 || result == 0xFFFFFFFF)
	{
		printf("Error: Mapping failure\n");
		return 0;
	}

	return (result);
}

void unmapResourceVirAddress(u32 virtAddress)
{
	if (virtAddress)
		munmap((void *)virtAddress, frame_page);
}

void drawTrianglePolygons(int fbNum)
{
	pos points[] = {
		{ 100, 100 },
		{ 160, 100 },
		{ 130, 150 },
		{ 190, 150 },
		{ 160, 200 },
		{ 220, 200 },
		{ 190, 250 },
		{ 250, 250 },
		{ 220, 200 },
		{ 280, 200 },
		{ 250, 250 },
		{ 310, 250 },
		{ 280, 300 },
		{ 340, 300 },
		{ 310, 350 },
		{ 370, 350 },
		{ 340, 300 },
		{ 400, 300 },
		{ 370, 250 },
		{ 430, 250 },
		{ 400, 200 },
		{ 460, 200 },
		{ 430, 150 },
		{ 490, 150 },
		{ 460, 100 },
		{ 520, 100 },
	};
	int i;

	gfxaccel_draw_line(&gfxaccelInst, WriteFrameAddr[fbNum], 
		points[0].x, points[0].y, points[1].x, points[1].y, 
		0xffffffff);
	for (i = 0; i < sizeof(points)/sizeof(points[0]) - 2; i++)
	{
		gfxaccel_draw_line(&gfxaccelInst, WriteFrameAddr[fbNum], 
			points[i+1].x, points[i+1].y, points[i+2].x, points[i+2].y, 
			0xffffffff);
		gfxaccel_draw_line(&gfxaccelInst, WriteFrameAddr[fbNum], 
			points[i+2].x, points[i+2].y, points[i].x, points[i].y, 
			0x3ff00000);
	}
}

static void fillColorTiles(u32 baseAddr)
{
	int i, j;
	unsigned int col, data;

	for (i = 0; i < 480 / 16; i++) {
		for (j = 0; j < 800 / 16; j++) {
			col = ((i) << 2) | (j);
			data = RGB1(col >> 2, col >> 1, col);
			gfxaccel_fill_rect(&gfxaccelInst, baseAddr, 
				j * 16, i * 16, j * 16 + 15, i * 16 + 15, 
				data);
		}
	}
}

static void copyBitmapToFramebuffer(u32 *dst, u32 *src, int width, int height, int stride)
{
	int x, y;
	u32 pix;
	u32 *ptr = src;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pix = ptr[x];
			// Shift RGB bitmap to VDMA bitmap pixel data
			pix = ((pix & 0xFF0000) << 6) | ((pix & 0xFF00) << 4) | ((pix & 0xFF) << 2);
			*dst++ = pix;
		}
		ptr += stride;
	}
}

static void loadBitmapFiletoFB(u32 baseAddr)
{
	Bitmap bmp;
	printf("load reource file res/resource.png ...\n");
	loadPngFile2Bitmap(&bmp, "./res/resource.png");
	printf("done.\n");
	printf("copy bitmap to resource frame buffer ...\n");
	copyBitmapToFramebuffer((u32 *)baseAddr, bmp.data, 
		DISP_WIDTH, DISP_HEIGHT, DISP_WIDTH);
	printf("done.\n");
	if (bmp.data) free(bmp.data);
}

static int parse_argument(int argc, char *argv[])
{
	int mode = 0;
	int i;

	for (i = 0; i < argc; i++)
	{
		if (*argv[i] == '-' && *(argv[i]+1) == 's')
		{
			printf("  Shot mode enabled.\n");
			mode = 1;
			break;
		}
		else if (*argv[i] == '-' && *(argv[i]+1) == 'd')
		{
			printf("  Debug mode enabled.\n");
			mode = 2;
			break;
		}
	}

	return (mode);
}

static void DrawDebugInfo(int fbNum)
{
	u8 info[16];
	u32 systime = game_get_systemtime();
	sprintf(info, "%04d:%06d", (int)(systime / 60), (int)systime);
	drawText(WriteFrameAddr[fbNum], 606, 448, info);
}

static void DrawMap(int fbNum)
{
	int i;
	int x, y;
	int src_x, src_y;
	u8 data;

	for (i = 0; i < sizeof(mapData); i++) {
		x = (i % 15) * 32;
		y = (i / 15) * 32;
		data = mapData[i];
		src_x = (data & 0x07) * 32;
		src_y = (data >> 4)   * 32;
		gfxaccel_bitblt(&gfxaccelInst, 
			ResourceAddr, src_x, src_y, 32, 32, 
			WriteFrameAddr[fbNum], x + 160, y, 
			GFXACCEL_BB_NONE);
	}
}

static void DrawTestFrame(int fbNum)
{
	int i, j;

    for (i = 0; i < 480 / 16; i++)
    {
    	for (j = 0; j < 800 / 16; j++)
    	{
    		gfxaccel_bitblt(&gfxaccelInst, 
				ResourceAddr, j * 16, (i /*% 15*/) * 16, 16, 16, 
				WriteFrameAddr[fbNum], j * 16, i * 16, 
				GFXACCEL_BB_NONE);
    	}
    }

    // Invoke fill rectangle accelerator
    gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbNum],
		 80,  80, 719, 399, RGB8(255, 255, 255));
    // Invoke fill rectangle accelerator
    gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbNum],
		 83,  83, 716, 396, RGB8(0, 64, 255));
    // Invoke fill rectangle accelerator
    gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbNum],
		 80, 240, 719, 241, RGB8(255, 255, 255));
    // Invoke fill rectangle accelerator
    gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbNum],
		480, 240, 481, 399, RGB8(255, 255, 255));
}

static char mml[] = "cegefdggcegefdgrcegefdggcegefdcr";
static float freq[8] = { 261.626, 293.665, 329.628, 349.228, 391.995, 440.0, 493.883, 523.251 };
int sound_pos = 0;

void PlayMusic(int pos)
{
	int i, j;
	float len;
	char note = mml[pos];
	int note_p;

	if (note == 'r') return;

	if (note < 'c') note += 7; // a or b
	note_p = note - 'c';

	if (note_p >= sizeof(freq)/sizeof(freq[0])) return;

	len = 48000.0 / freq[note_p];
	len /= 2;

	if (len > 1745) return;

	for (j = 0; j < 50; j++) {
		for (i = 0; i < (int)len; i++)
			i2sout_senddata(4, 0x10001000); // L=0x2000; R=0x2000
		for (i = 0; i < (int)len; i++)
			i2sout_senddata(4, 0x00000000); // L=0x0000; R=0x0000
   	}
}

static void UpdateAudio(int scene)
{
	static int mode = 0;
	int len;

	len = sizeof(mml) - 1;

	if (scene == 0) {
		if (!wavfile_played) PlayWavFile("res/test.wav");
		wavfile_played = 1;
	} else if (scene == 1) {
		switch (mode) {
		case 0:
			PlayMusic(sound_pos);
			sound_pos = (sound_pos + 1) % len;
			break;
		default:
			break;
		}
	} else {
	}

	mode = (mode + 1) % 15;
}

static void UpdateFrame(int scene)
{
	static int x = 0;
	static int y = 0;
	int i, j;
	int status;
	u32 systime;

	UpdateAudio(scene);

	// draw backfround frame as per scene number
	switch (scene) {
	case 0:
		DrawTestFrame(fbBackgd);
		drawTrianglePolygons(fbBackgd);
		drawText(WriteFrameAddr[fbBackgd], 176, 448, "\x80\x80\x80 2021 (c) ATSUPI.COM \x80\x80\x80");
		break;

	case 1:
		gfxaccel_bitblt(&gfxaccelInst, 
			ResourceAddr, 0, 0, 800, 128, 
			WriteFrameAddr[fbBackgd], 0, 0, GFXACCEL_BB_NONE);
		DrawMap(fbBackgd);
		gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbBackgd], 
			  0, 128, 159, 479, 0);
		gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbBackgd], 
			640, 128, 799, 479, 0);
		DrawDebugInfo(fbBackgd);
		gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbBackgd], 
			x * 32, y * 32, x * 32 + 63, y * 32 + 63, 
			RGB8(255, 255, 255));
	    if (++x == 24) {
	    	x = 0;
	    	if (++y == 14) y = 0;
	    }
		systime = game_get_systemtime();
		drawSprite(&Sprite1, WriteFrameAddr[fbBackgd], systime);
		drawSprite(&Sprite2, WriteFrameAddr[fbBackgd], systime);
		Sprite2.y += 2;
		if (Sprite2.y > 448) Sprite2.y = 0;
		break;
	case 2:
	    gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[fbBackgd], 0, 0, 799, 479, 0x0);
		drawText(WriteFrameAddr[fbBackgd], 240, 240, "APPLICATION CLOSED.");
		break;
	}

	// switch display frame on double buffer
	fbActive ^= 1;
	fbBackgd ^= 1;
	status = vdma_start_parking(&vdmaInst_0, VDMA_READ, fbActive);
	if (status != PST_SUCCESS) {
		printf("Start Park failed\r\n");
		return;
	}
#ifdef DEBUG
	printf("current frame = %d\r\n", fbActive);
#endif
}

static void TestPngFileConversion(void)
{
	Bitmap bmp;
	printf("load bitmap file res/resource.png ...\n");
	loadPngFile2Bitmap(&bmp, "./res/resource.png");
	printf("done.\n");
	printf("save bitmap data to png file ...\n");
	saveBitmap2PngFile(&bmp, "resource.png");
	printf("done.\n");
	if (bmp.data) free(bmp.data);
}

int main(int argc, char *argv[])
{
	int i, j;
	u32 size = FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN;
	int status;
	int mode;
	u32 ReadAddr;
	pthread_t pt;
	char ch = ' ';
	pos basePos;
	u32 time;

	mode = parse_argument(argc, argv);

	WriteFrameAddr[0] = WRITE_ADDRESS_BASE;
	WriteFrameAddr[1] = WRITE_ADDRESS_BASE + frame_page;
	ResourceAddr      = WriteFrameAddr[1]  + frame_page;
	CamFrameAddr      = ResourceAddr + frame_page;

	fbActive = 0;
	fbBackgd = 1;

	printf("\r\n--- Entering main() --- \r\n");

	/* The information of the XAxiVdma_Config comes from hardware build.
	 * The user IP should pass this information to the AXI DMA core.
	 */
	printf("Step 0\n");
	vdma_init(&vdmaInst_0, VDMA_0_BASEADDRESS, WriteFrameAddr[0]);
	if (VDMA_INSTANCE_NUM > 1) {
//		vdma_init(&vdmaInst_1, VDMA_1_BASEADDRESS, CamFrameAddr);
	}

	printf("Step 1\n");
	/* Setup the read channel
	 */
	status = ReadSetup(&vdmaInst_0);
	if (status != PST_SUCCESS) {
		printf("Read channel setup failed %d\r\n", status);
		if(status == PST_VDMA_MISMATCH_ERROR)
			printf("DMA Mismatch Error\r\n");
		return PST_FAILURE;
	}
	printf("Step 2\n");

	// Start DMA engine to transfer
	status = vdma_start(&vdmaInst_0, VDMA_READ);
	if (status != PST_SUCCESS) {
		printf("Start read transfer failed %d\r\n", status);
		return PST_FAILURE;
	}

	status = vdma_start_parking(&vdmaInst_0, VDMA_READ, fbActive);
	if (status != PST_SUCCESS) {
		printf("Start Park failed\r\n");
		vdma_deinit(&vdmaInst_0);
		return PST_FAILURE;
	}

	// map Resource frambuffer address
	mappedResAddr = mapResourceFBtoMem(ResourceAddr);
	if (!mappedResAddr) {
		vdma_deinit(&vdmaInst_0);
		return PST_FAILURE;
	}
	printf("Test passed\r\n");

	lq070out_init(&lq070Inst, LQ070OUT_BASE_ADDR);
    lq070out_setmode(&lq070Inst, LQ070_PG_NONE);
    printf("Hello World\n\r");

    printf("Initialize gfxaccel instance\r\n");
	if (gfxaccel_init(&gfxaccelInst, GFXACCEL_BASE_ADDR) == PST_FAILURE) {
		lq070out_deinit(&lq070Inst);
		unmapResourceVirAddress(mappedResAddr);
		vdma_deinit(&vdmaInst_0);
		return PST_FAILURE;
	}

    // Clear frame buffer
    printf("Clear frame buffer\r\n");
    gfxaccel_fill_rect(&gfxaccelInst, WriteFrameAddr[0], 0, 0, 799, 479, 0x0);

    // Setup Resource frame
    printf("Setup Resource frame buffer\r\n");
    fillColorTiles(ResourceAddr);

    printf("BitBlt Resource frame buffer to main frame buffer\r\n");
	DrawTestFrame(fbActive);

    // Output results
	ReadAddr = vdma_get_frame_address(&vdmaInst_0, 0);
    printf("Read Frame address: 0x%x", ReadAddr);
    for (i = 0; i < 8; i++)
    {
    	printf("\r\nfb(%03d): ", i);
    	for (j = 0; j < 16; j++)
		{
    		printf("%02x ", FB(ReadAddr, ((i+76)*800+(j+76))) & 0xFF);
		}
    }
	printf("\r\n");

	drawTrianglePolygons(fbActive);

	// configure sprite drawing module
	printf("loadBitmapFiletoFB()\n");
	loadBitmapFiletoFB(mappedResAddr);

	printf("configure Sprite Resource\n");
	// start position on resource frame buffer
	basePos.x = 512;
	basePos.y = 0;
	configSpriteResouce(&gfxaccelInst, ResourceAddr, &basePos);

	printf("configure Font Resource\n");
	// start position on resource frame buffer
	basePos.x = 256;
	basePos.y = 0;
	configFontResouce(&gfxaccelInst, ResourceAddr, &basePos);

	printf("Initialize audio component\n");
	azplf_audio_init();
	azplf_audio_initSSM2603();
	i2sout_senddata(0, 0xFE000000); // debug=254 (no invert data), mute=0, lr_mode=R only

	// Test conversion from bitmap to png
//	TestPngFileConversion();

	sleep(1); // 1sec wait before starting game work thread

	status = azplf_start_game_thread(INITIAL_SCENE, UpdateFrame);
	if (status != PST_SUCCESS) {
		printf("Error: Game worker thread cannot start.\n");
		quit = 1;
	}

	while (!quit) {
		do {
			scanf("%c", &ch);
		} while (ch == '\r' || ch == '\n');

		switch (game_get_scene()) {
		case 0:
			if (ch == 'x') {
				game_set_next_scene(2);
				usleep(50000);
				azplf_terminate_game_thread();
				quit = 1; // quit thread
				sleep(1); // wait until thread terminates
				break;
			} else if (ch == ' ' || ch == 'n') {
				game_set_next_scene(1);
			} else {
				printf("Command not found\r\n");
			}
			break;
		case 1:
			if (ch == 0x1b || ch == 'e') { // ESC
				game_set_next_scene(0);
			} else {
				printf("Command not found\r\n");
			}
			break;
		default:
			break;
		}
		if (quit) break;
		time = game_get_systemtime();
		printf("system time=%d\n", time);
		sleep(1);
	}
	// draw ending screen
    printf("--- Exiting main() --- \r\n");

	azplf_game_deinit();
	azplf_audio_deinit();
	gfxaccel_deinit(&gfxaccelInst);
	lq070out_deinit(&lq070Inst);
	unmapResourceVirAddress(mappedResAddr);
	vdma_deinit(&vdmaInst_0);

	return 0;
}

/*****************************************************************************
 *
 * This function sets up the read channel
 *
 *****************************************************************************/
static int ReadSetup(VdmaInstance *inst)
{
	int Index;
	u32 Addr;
	int status;

	printf("Step 2.1\n");
	status = vdma_config(inst, VDMA_READ);
	if (status != PST_SUCCESS) {
		printf("Read channel config failed %d\r\n", status);
		return PST_FAILURE;
	}

	printf("Step 2.2\n");
	// Initialize buffer addresses (physical addresses)
	Addr = READ_ADDRESS_BASE;
	for(Index = 0; Index < NUMBER_OF_FRAMES; Index++) {
		vdma_set_frame_address(inst, Index, Addr);
		Addr += frame_page;
	}

	printf("Step 2.3\n");
	// Set the buffer addresses for transfer in the DMA engine
	status = vdma_dma_setbuffer(inst, VDMA_READ);
	if (status != PST_SUCCESS) {
		printf("Read channel set buffer address failed %d\r\n", status);
		return PST_FAILURE;
	}

	printf("Step 2.4\n");
	return PST_SUCCESS;
}
