#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"
#include "esp_jpeg_dec.h"
#include "app_lcd.h"
#include "bsp.h"

#define RLCD_BYTES ((BSP_LCD_H_RES * BSP_LCD_V_RES) / 8)
#define RGB565_BYTES (BSP_LCD_H_RES * BSP_LCD_V_RES * 2)
static const char *TAG = "rlcd_st7305";
static esp_lcd_panel_io_handle_t s_io;
static uint8_t *s_mono;
static uint8_t *s_rgb;
static SemaphoreHandle_t s_color_done;

static bool IRAM_ATTR on_color_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *event, void *user_ctx) {
    BaseType_t need_yield = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)user_ctx, &need_yield);
    return need_yield == pdTRUE;
}

static void cmd(uint8_t c) { ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(s_io, c, NULL, 0)); }
static void data(uint8_t d) { ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(s_io, -1, &d, 1)); }
static void data_n(const uint8_t *p, size_t n) { ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(s_io, -1, p, n)); }
static void init_cmd(uint8_t c, const uint8_t *p, size_t n) { cmd(c); if(n) data_n(p,n); }

static void rlcd_init(void) {
    const uint8_t d6[]={0x17,0x02}, c1[]={0x69,0x69,0x69,0x69}, c2[]={0x19,0x19,0x19,0x19};
    const uint8_t c4[]={0x4B,0x4B,0x4B,0x4B}, c5[]={0x19,0x19,0x19,0x19}, d8[]={0x80,0xE9};
    const uint8_t b3[]={0xE5,0xF6,0x05,0x46,0x77,0x77,0x77,0x77,0x76,0x45};
    const uint8_t b4[]={0x05,0x46,0x77,0x77,0x77,0x77,0x76,0x45}, b62[]={0x32,0x03,0x1F};
    const uint8_t col[]={0x12,0x2A}, row[]={0x00,0xC7};
    gpio_set_level(BSP_LCD_RST,1); vTaskDelay(pdMS_TO_TICKS(50)); gpio_set_level(BSP_LCD_RST,0); vTaskDelay(pdMS_TO_TICKS(20)); gpio_set_level(BSP_LCD_RST,1); vTaskDelay(pdMS_TO_TICKS(50));
    init_cmd(0xD6,d6,2); init_cmd(0xD1,(uint8_t[]){1},1); init_cmd(0xC0,(uint8_t[]){0x11,4},2);
    init_cmd(0xC1,c1,4); init_cmd(0xC2,c2,4); init_cmd(0xC4,c4,4); init_cmd(0xC5,c5,4); init_cmd(0xD8,d8,2);
    init_cmd(0xB2,(uint8_t[]){2},1); init_cmd(0xB3,b3,10); init_cmd(0xB4,b4,8); init_cmd(0x62,b62,3);
    init_cmd(0xB7,(uint8_t[]){0x13},1); init_cmd(0xB0,(uint8_t[]){0x64},1); cmd(0x11); vTaskDelay(pdMS_TO_TICKS(200));
    init_cmd(0xC9,(uint8_t[]){0},1); init_cmd(0x36,(uint8_t[]){0x48},1); init_cmd(0x3A,(uint8_t[]){0x11},1);
    init_cmd(0xB9,(uint8_t[]){0x20},1); init_cmd(0xB8,(uint8_t[]){0x29},1); cmd(0x21); init_cmd(0x2A,col,2); init_cmd(0x2B,row,2); init_cmd(0x35,(uint8_t[]){0},1); init_cmd(0xD0,(uint8_t[]){0xFF},1); cmd(0x38); cmd(0x29);
}
static void rlcd_present(void) {
    // tx_color is asynchronous.  Waiting for completion before reusing the
    // single DMA buffer prevents partial-frame mixing and localized flicker.
    xSemaphoreTake(s_color_done, portMAX_DELAY);
    cmd(0x2A); data(0x12); data(0x2A); cmd(0x2B); data(0); data(0xC7); cmd(0x2C);
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_color(s_io,-1,s_mono,RLCD_BYTES));
}
static void put_pixel(unsigned x,unsigned y,bool white) { if(x>=400||y>=300)return; unsigned iy=299-y, i=(x>>1)*75+(iy>>2), b=7-(((iy&3)<<1)|(x&1)); if(white)s_mono[i]|=1u<<b;else s_mono[i]&=~(1u<<b); }
/* Ordered dithering preserves thin antialiased glyph strokes on the 1-bit RLCD. */
static bool light(const uint8_t *p, unsigned x, unsigned y) { static const uint8_t bayer[4][4]={{0,8,2,10},{12,4,14,6},{3,11,1,9},{15,7,13,5}}; unsigned v=((unsigned)p[0]<<8)|p[1], r=(v>>11)&31,g=(v>>5)&63,b=v&31; unsigned l=(r*255/31*77+g*255/63*150+b*255/31*29)/256; return l > (bayer[y&3][x&3]*16+8); }
static esp_err_t decode_jpeg(const uint8_t *in,size_t len,uint8_t *out) { jpeg_dec_config_t cfg=DEFAULT_JPEG_DEC_CONFIG(); cfg.output_type=JPEG_PIXEL_FORMAT_RGB565_BE; jpeg_dec_handle_t h=NULL; jpeg_dec_io_t io={.inbuf=(uint8_t*)in,.inbuf_len=len,.outbuf=out}; jpeg_dec_header_info_t info={0}; if(jpeg_dec_open(&cfg,&h)!=JPEG_ERR_OK)return ESP_FAIL; esp_err_t r=jpeg_dec_parse_header(h,&io,&info); if(r>=0)r=jpeg_dec_process(h,&io); jpeg_dec_close(h); return r; }

esp_err_t app_lcd_init(void) {
    spi_bus_config_t bus={.mosi_io_num=BSP_LCD_SPI_MOSI,.miso_io_num=GPIO_NUM_NC,.sclk_io_num=BSP_LCD_SPI_CLK,.quadwp_io_num=GPIO_NUM_NC,.quadhd_io_num=GPIO_NUM_NC,.max_transfer_sz=RLCD_BYTES};
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM,&bus,SPI_DMA_CH_AUTO),TAG,"spi");
    esp_lcd_panel_io_spi_config_t io={.dc_gpio_num=BSP_LCD_DC,.cs_gpio_num=BSP_LCD_SPI_CS,.pclk_hz=BSP_LCD_PIXEL_CLOCK_HZ,.lcd_cmd_bits=8,.lcd_param_bits=8,.spi_mode=0,.trans_queue_depth=1};
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM,&io,&s_io),TAG,"io");
    gpio_config_t rst={.pin_bit_mask=1ULL<<BSP_LCD_RST,.mode=GPIO_MODE_OUTPUT}; ESP_RETURN_ON_ERROR(gpio_config(&rst),TAG,"rst");
    s_color_done=xSemaphoreCreateBinary(); ESP_RETURN_ON_FALSE(s_color_done,ESP_ERR_NO_MEM,TAG,"semaphore");
    esp_lcd_panel_io_callbacks_t cbs={.on_color_trans_done=on_color_done}; ESP_RETURN_ON_ERROR(esp_lcd_panel_io_register_event_callbacks(s_io,&cbs,s_color_done),TAG,"callbacks"); xSemaphoreGive(s_color_done);
    s_mono=heap_caps_malloc(RLCD_BYTES,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL); s_rgb=heap_caps_malloc(RGB565_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(s_mono&&s_rgb,ESP_ERR_NO_MEM,TAG,"buffers"); memset(s_mono,0xFF,RLCD_BYTES); rlcd_init(); rlcd_present(); ESP_LOGI(TAG,"ST7305 ready: 400x300, %d bytes",RLCD_BYTES); return ESP_OK;
}
void app_lcd_draw(uint8_t *buf,uint32_t len,uint16_t width,uint16_t height) {
    if(!s_mono||!s_rgb||decode_jpeg(buf,len,s_rgb)!=ESP_OK){ESP_LOGW(TAG,"jpeg decode failed");return;} if(width!=400||height!=300)ESP_LOGW(TAG,"frame %ux%u scaled/cropped to 400x300",width,height);
    for(unsigned y=0;y<300;y++){unsigned sy=(unsigned)y*height/300;for(unsigned x=0;x<400;x++){unsigned sx=(unsigned)x*width/400; put_pixel(x,y,light(&s_rgb[(sy*width+sx)*2],x,y));}} rlcd_present();
}
void app_lcd_draw_mono1(const uint8_t *buf, uint32_t len, uint16_t width, uint16_t height) {
    if (!s_mono || width != 400 || height != 300 || len != RLCD_BYTES) {
        ESP_LOGW(TAG, "drop mono frame: %ux%u, %lu bytes", width, height, (unsigned long)len);
        return;
    }
    memcpy(s_mono, buf, RLCD_BYTES);
    rlcd_present();
}
