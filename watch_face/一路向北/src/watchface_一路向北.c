#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "maibu_sdk.h"
#include "maibu_res.h"

//<静态变量声明>

//屏幕坐标与尺寸相关
#define WATCHFACE_BG_ORIGIN_X 	0
#define WATCHFACE_BG_ORIGIN_Y 	0
#define WATCHFACE_BG_SIZE_H	128
#define WATCHFACE_BG_SIZE_W 	128

#define WATCHFACE_ANI_ORIGIN_X 	0
#define WATCHFACE_ANI_ORIGIN_Y 	0
#define WATCHFACE_ANI_SIZE_H	78
#define WATCHFACE_ANI_SIZE_W 	128

#define WATCHFACE_TIME_SIZE_H  23
#define WATCHFACE_TIME_SIZE_W  13

#define WATCHFACE_DATE_SIZE_H  13
#define WATCHFACE_DATE_SIZE_W  8


static GRect bmp_ani_rect = {
	{WATCHFACE_ANI_ORIGIN_X,WATCHFACE_ANI_ORIGIN_Y},
	{WATCHFACE_ANI_SIZE_H,WATCHFACE_ANI_SIZE_W}
};

static uint32_t bmp_bg[]=
{
	RES_BITMAP_WATCHFACE_METER
};

//动画帧数组
static uint32_t bmp_ani_frames[] =
{
	RES_BITMAP_WATCHFACE_ANI_1,
	RES_BITMAP_WATCHFACE_ANI_2,
	RES_BITMAP_WATCHFACE_ANI_3,
	RES_BITMAP_WATCHFACE_ANI_4,
	RES_BITMAP_WATCHFACE_ANI_5
};

//星期图片数组
static uint32_t bmp_week_icons[] =
{
	RES_BITMAP_WATCHFACE_WEEK_SUN,
	RES_BITMAP_WATCHFACE_WEEK_MON,
	RES_BITMAP_WATCHFACE_WEEK_TUE,
	RES_BITMAP_WATCHFACE_WEEK_WED,
	RES_BITMAP_WATCHFACE_WEEK_THU,
	RES_BITMAP_WATCHFACE_WEEK_FRI,
	RES_BITMAP_WATCHFACE_WEEK_SAT
};

//中号数字图片数组
static uint32_t bmp_number_mid_icons[] = 
{
	RES_BITMAP_WATCHFACE_NUMBER_MID_0,
	RES_BITMAP_WATCHFACE_NUMBER_MID_1,
	RES_BITMAP_WATCHFACE_NUMBER_MID_2,
	RES_BITMAP_WATCHFACE_NUMBER_MID_3,
	RES_BITMAP_WATCHFACE_NUMBER_MID_4,
	RES_BITMAP_WATCHFACE_NUMBER_MID_5,
	RES_BITMAP_WATCHFACE_NUMBER_MID_6,
	RES_BITMAP_WATCHFACE_NUMBER_MID_7,
	RES_BITMAP_WATCHFACE_NUMBER_MID_8,
	RES_BITMAP_WATCHFACE_NUMBER_MID_9
	
};

static uint32_t bmp_num_lit_icons[] =
{
	RES_BITMAP_WATCHFACE_NUMBER_LIT_0,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_1,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_2,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_3,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_4,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_5,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_6,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_7,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_8,
	RES_BITMAP_WATCHFACE_NUMBER_LIT_9
};

/*动画图层ID*/
static int8_t g_layer_ani_id = -1;
static int8_t g_layer_sec_id = -1;

/*窗口ID, 通过该窗口ID获取窗口句柄*/
static int32_t g_window_id = -1;

static int8_t g_ani_index= 0;
static int8_t g_ani_frame_count= 5;//sizeof(bmp_ani_frames)/sizeof(bmp_ani_frames[0]);

/*表盘应显示数据*/
static uint8_t watch_data[11] = {0};

/*文字缓存*/
char text_buf[20]= "";
/*矩形区域 注意顺序！！X-Y 高-宽!!*/
GRect frame_bg = {{0,78}, {50,128}};
GRect frame_date[] = {
	{
		{10, 103},
		{WATCHFACE_DATE_SIZE_H, WATCHFACE_DATE_SIZE_W}
	},
	{
		{17, 103},
		{WATCHFACE_DATE_SIZE_H, WATCHFACE_DATE_SIZE_W}
	},
	{
		{31, 103},
		{WATCHFACE_DATE_SIZE_H, WATCHFACE_DATE_SIZE_W}
	},
	{
		{38, 103},
		{WATCHFACE_DATE_SIZE_H, WATCHFACE_DATE_SIZE_W}
	}
};
GRect frame_wday = {{101, 89}, {5, 15}};
GRect frame_hm[] = {
	{
		{53, 98},
		{WATCHFACE_TIME_SIZE_H, WATCHFACE_TIME_SIZE_W}
	},
	{
		{67, 98},
		{WATCHFACE_TIME_SIZE_H, WATCHFACE_TIME_SIZE_W}
	},
	{
		{83, 98},
		{WATCHFACE_TIME_SIZE_H, WATCHFACE_TIME_SIZE_W}
	},
	{
		{97, 98},
		{WATCHFACE_TIME_SIZE_H, WATCHFACE_TIME_SIZE_W}
	},
};
GRect frame_sec = {{70, 75}, {40, 50}};
GRect frame_bat = {{106, 107}, {15,20}};

//</静态变量声明>

//<静态函数声明>
static P_Window init_window(void);
//</静态函数声明>


/*
 *--------------------------------------------------------------------------------------
 *     function:  get_watch_data
 *    parameter:  0为watch_data所有元素值，1仅为watch_data中的秒
 *       return:
 *  description:  将数组参数赋值为当前表盘应显示值
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static void get_watch_data(uint8_t sec_refresh)
{
	struct date_time datetime;
	app_service_get_datetime(&datetime);

	watch_data[4] = datetime.sec/10;
	watch_data[5] = datetime.sec%10;
	if(sec_refresh == 1)
	{
		return;
	}
	watch_data[0] = datetime.hour/10;
	watch_data[1] = datetime.hour%10;
	watch_data[2] = datetime.min/10;
	watch_data[3] = datetime.min%10;

	watch_data[6] = datetime.mon/10;
	watch_data[7] = datetime.mon%10;
	watch_data[8] = datetime.mday/10;
	watch_data[9] = datetime.mday%10;

	watch_data[10] = datetime.wday;

}

/*创建并显示图片图层，需要坐标值，得到icon_key的数组，数组需要的参数值，P_Window*/
int32_t display_target_layer(P_Window p_window,GRect *temp_p_frame,enum GAlign how_to_align,enum GColor black_or_white,int32_t bmp_array_name[],int bmp_id_number)
{
	GBitmap bmp_point;
	P_Layer temp_P_Layer = NULL;

	res_get_user_bitmap(bmp_array_name[bmp_id_number], &bmp_point);
	LayerBitmap layer_bitmap_struct_l = {bmp_point, *temp_p_frame, how_to_align};
 	temp_P_Layer = app_layer_create_bitmap(&layer_bitmap_struct_l);

	if(temp_P_Layer != NULL)
	{
		app_layer_set_bg_color(temp_P_Layer, black_or_white);
		return app_window_add_layer(p_window, temp_P_Layer);
	}

	return 0;
}

int32_t display_text_layer(P_Window p_window, GRect *temp_p_frame, enum GAlign how_to_align,enum GColor black_or_white, char* text_buf, uint8_t font_type)
{
    /*生成文本结构体*/
    LayerText text = {text_buf, *temp_p_frame, how_to_align, font_type};
    /*创建文本图层*/
    P_Layer temp_P_Layer = app_layer_create_text(&text);

    if(temp_P_Layer != NULL)
    {
        app_layer_set_bg_color(temp_P_Layer, black_or_white);
        return app_window_add_layer(p_window, temp_P_Layer);
    }

    return 0;
}

static void sec_callback()
{
    P_Window p_window = NULL;
    P_Layer p_layer = NULL;
    GBitmap bitmap = {0};
    uint8_t i;

    /*根据窗口ID获取窗口句柄*/
    p_window = app_window_stack_get_window_by_id(g_window_id);
    if (p_window == NULL)
    {
        return;
    }

    /*获取数据图层句柄*/
    p_layer = app_window_get_layer_by_id(p_window, g_layer_sec_id);
    if (p_layer != NULL)
    {
        sprintf(text_buf, "%d%d", watch_data[4], watch_data[5]);
        app_layer_set_text_text(p_layer, text_buf);
    }

    /*窗口显示*/
    app_window_update(p_window);
}

static void ani_index_update()
 {
    g_ani_index++;
    if(g_ani_index>=g_ani_frame_count)
    {
        g_ani_index= 0;
    }
 }

static void ani_callback(date_time_t tick_time, uint32_t millis,void *context)
{
  	P_Window p_window = NULL;
	P_Layer p_layer = NULL;
	GBitmap bitmap = {0};
	uint8_t i;

	/*根据窗口ID获取窗口句柄*/
	p_window = app_window_stack_get_window_by_id(g_window_id);
	if (p_window == NULL)
	{
		return;
	}
	ani_index_update();
	
	/*获取数据图层句柄*/
    p_layer = app_window_get_layer_by_id(p_window, g_layer_ani_id);
    if (p_layer != NULL)
    {
        /*更新数据图层图片*/
        res_get_user_bitmap(bmp_ani_frames[g_ani_index], &bitmap);
        app_layer_set_bitmap_bitmap(p_layer, &bitmap);
    }

	/*窗口显示*/
	app_window_update(p_window);
}

static P_Window init_window(void)
{
	P_Window p_window = NULL;

	/*创建一个窗口*/
	p_window = app_window_create();
	if (NULL == p_window)
	{
		return NULL;
	}

    /*创建背景层*/
    display_target_layer(p_window,&frame_bg,GAlignCenter,GColorWhite,bmp_bg,0);

    uint8_t i = 0;
    /*创建月日图层*/
    for(i = 0;i <= 3;i++)
	{
		display_target_layer(p_window,&frame_date[i],GAlignLeft,GColorWhite,bmp_num_lit_icons,watch_data[i+6]);
	}

    /*创建星期图层*/
    display_target_layer(p_window,&frame_wday,GAlignCenter,GColorWhite,bmp_week_icons, watch_data[10]);    

    /*创建时分图层*/
	for(i = 0;i <= 3;i++)
	{
		display_target_layer(p_window,&frame_hm[i],GAlignLeft,GColorWhite,bmp_number_mid_icons,watch_data[i]);
	}

    // sprintf(text_buf, "%d%d:%d%d", watch_data[0], watch_data[1], watch_data[2], watch_data[3]);
    // display_text_layer(p_window,&frame_hm,GAlignLeft,GColorBlack,text_buf,U_ASCII_ARIAL_24);

	/*创建电池图层*/
	char battery_temp = 0;
	maibu_get_battery_percent(&battery_temp);
	int battery= (int)battery_temp;
	if(battery>=100)
	{
		sprintf(text_buf,"%s", "满");
	}
	else
	{
		sprintf(text_buf,"%02d", battery);
	}
	display_text_layer(p_window,&frame_bat,GAlignRight,GColorBlack,text_buf,U_ASCII_ARIAL_12);


	/*创建动画图层*/
	g_layer_ani_id = display_target_layer(p_window,&bmp_ani_rect,GAlignCenter,GColorWhite,bmp_ani_frames,g_ani_index);

    /*定义窗口定时器，用于动作刷新*/
    app_window_timer_subscribe(p_window, 1000, ani_callback, NULL);

	return p_window;
}

//重新载入并刷新窗口所有图层
void window_reloading(void)
{
	/*根据窗口ID获取窗口句柄*/
	P_Window p_old_window = app_window_stack_get_window_by_id(g_window_id);

	if (NULL != p_old_window)
	{
	    ani_index_update();
		P_Window p_window = init_window();
		if (NULL != p_window)
		{
			g_window_id = app_window_stack_replace_window(p_old_window, p_window);
		}
	}

}

/*
 *--------------------------------------------------------------------------------------
 *     function:  app_watch_time_change
 *    parameter:
 *       return:
 *  description:  系统时间有变化时，更新时间图层
 * 	      other:
 *--------------------------------------------------------------------------------------
 */
static void app_watch_time_change(enum SysEventType type, void *context)
{

	/*时间更改*/
	if (type == SysEventTypeTimeChange)
	{
		get_watch_data(0);
		window_reloading();
	}
}

int main()
{
//simulator_init();

    get_watch_data(0);
	/*创建显示表盘窗口*/
	P_Window p_window = init_window();
	if (p_window != NULL)
	{
		/*放入窗口栈显示*/
		g_window_id = app_window_stack_push(p_window);

		/*注册一个事件通知回调，当有时间改变时，立即更新时间*/
        maibu_service_sys_event_subscribe(app_watch_time_change);
	}
//simulator_wait();
	return 0;
}