#include "../../inc/freetype.h"
#include "../../../lvgl/src/libs/freetype/lv_freetype.h"
extern mutex mtx;
Freetype::Freetype(string ttf_path, int size)
{
    cout<<"ttf_path: "<<ttf_path<<endl;
    font = lv_freetype_font_create(ttf_path.c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, size,
                                   LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }else{
        cout<<"font create success."<<endl;
    }
    /*Create style with the new font*/
    lv_style_init(&style);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);
}

void Freetype::set_font_style(lv_obj_t * obj, string txt)
{
    /*Create a label with the new style*/
    
    lv_obj_add_style(obj, &style, 0);
    lv_label_set_text(obj, txt.c_str());
}