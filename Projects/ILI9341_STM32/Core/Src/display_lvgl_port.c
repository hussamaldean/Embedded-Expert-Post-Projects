/**
 * @file display_lvgl_port.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include <display_lvgl_port.h>
#include <stdbool.h>

#include "ILI9341.h"


#define MY_DISP_HOR_RES    240
#define MY_DISP_VER_RES    320

static uint8_t buf_2_1[240*32*2]__attribute__((aligned(4)));;
static uint8_t buf_2_2[240*32*2]__attribute__((aligned(4)));;

lv_display_t  *disp;

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
     disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);


    lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    // Change the color format to RGB565_SWAPPED
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);



}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
	ILI9341_Init(ROTATE_0);
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{
	if(disp_flush_enabled)
	    {
	        // Simply pass the area coordinates directly to your DMA function
	        ILI9341_Draw_Bitmap_DMA(area->x1, area->y1, area->x2, area->y2, px_map);
	    }
	    else
	    {
	        // If flushing is disabled, we must still inform LVGL to prevent a deadlock
	        lv_display_flush_ready(disp_drv);
	    }

}

void ILI9341_Transfer_Completed(void)
{
	lv_display_flush_ready(disp);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
