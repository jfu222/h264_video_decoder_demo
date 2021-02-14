#pragma once

#include <vector>
#include "MyMenu.h"
#include "MyVideoPlayer.h"

#define WM_LOOK_PICTURE_OPEN (WM_USER + 301)


typedef struct tagJIPS_COLORREF
{
    int r;
    int g;
    int b;
}JIPS_COLORREF;


//设置KRectScale(...)的参数结构体
typedef struct tagKRectScale
{
    RECT dst; //客户区矩形
    RECT src; //原始位图中，需要放大的矩形
    POINT org_left_top; //整张原始位图放大后的矩形的左上角左边
}KRectScale;


enum KDraw_Mode
{
    DRAW_MODE_NONE, //没有画线
    DRAW_MODE_BEGIN, //开始画线
    DRAW_MODE_DRAWING, //正在画线
    DRAW_MODE_END //结束画线
};


enum KDraw_Shape
{
    DRAW_SHAPE_NONE, //没有形状
    DRAW_SHAPE_POINTS, //画一组点
    DRAW_SHAPE_LINE, //画一条线段
    DRAW_SHAPE_LINES, //画一组首尾相接的线
    DRAW_SHAPE_RECT, //画一个矩形
    DRAW_SHAPE_ELLIPSE, //画一个椭圆
    DRAW_SHAPE_POLYGON, //画一个封闭多边形
};


typedef struct tagDrawShapes
{
    KDraw_Shape draw_shape;
    std::vector<POINT> vector_point; //存放用于画线的点坐标
    std::vector<JIPS_COLORREF> vector_color; //线段的颜色
}DrawShapes;


// CMyStatic

class CMyStatic : public CStatic
{
    DECLARE_DYNAMIC(CMyStatic)

public:
    CMyStatic();
    virtual ~CMyStatic();
    
public:
    void *m_ppvBits;
    int m_frame_num_now;
    int m_total_num_frames;

    CMyVideoPlayer m_player;

    CH264Picture m_outPicture;

    //-------------------
    int m_guid_index; //保存的图片格式，0-ImageFormatBMP,1-ImageFormatJPEG,2-ImageFormatPNG,3-ImageFormatGIF
    BITMAP m_bitmap;
    HBITMAP m_hbitmap;
    BITMAP m_bitmap_copy; //位图的一个拷贝

    char m_temp_filename[260]; //临时文件

    BOOL m_bLButtonDown; //鼠标左键是否按下

    KRectScale m_st_rect_scale;

    float m_zoom_scale; //目前画面缩放的倍数
    float m_zoom_scale_will; //即将缩放的倍数

    int m_mouse_move_last_x;
    int m_mouse_move_last_y;
    int m_mouse_move_delta_x; //鼠标移动偏移量
    int m_mouse_move_delta_y; //鼠标移动偏移量

    int m_mouse_scale_point_x; //鼠标滚轮滚动时的点
    int m_mouse_scale_point_y; //鼠标滚轮滚动时的点

    BOOL m_Is_Popup_Right_Menu; //控制是否弹出右键菜单
    KDraw_Mode m_Draw_Mode; //画线状态
    KDraw_Shape m_Draw_Shape; //画线形状

    POINT m_pt_mouse_move; //鼠标移动时的点
    std::vector<DrawShapes> m_vector_draw_shapes; //存放多种模式用于画线的点坐标
    std::vector<POINT> m_vector_point; //存放用于画线的点坐标
    int m_max_points_num; //画折线最多3个点

    CMyStatic * m_pAnotherClass; //用于关联另一个类指针

    RECT m_rect_render; //需要渲染的矩形区域(默认不渲染)
    BOOL m_IsAcceptDropFiles; //是否接受拖曳文件(默认是)

    CString m_file_path_open; //保存打开的文件名称

    JIPS_COLORREF m_color_background; //整个视图的背景颜色(默认黑色)
    JIPS_COLORREF m_color_draw_shape; //画各种线的颜色(默认红色)
    
    CMyMenu m_menu;
    int m_currMbAddr;

public:
    int SetHwndParent(HWND hwnd_parent);

    int start_play(const char *url);
    int stop_play();
    int pause_play(bool isPause);

    static int H264VideoPlayerFrameCallback(CH264Picture *outPicture, void *userData, int errorCode);

    int mb_hit_test(POINT pt, CH264Picture *Pic, CH264MacroBlock &mb); //鼠标击中测试

    //----------------------------
    int image_set_render_rect(int left, int top, int right, int bottom); //设置渲染区域(即感兴趣区域)
    int image_set_render_rect_by_ROI(); //设置渲染区域(即感兴趣区域)
    int SetAnotherClass(CMyStatic * p);
    int GetAnotherClass(CMyStatic * p);
    int OpenImage(char* filename, BITMAP &bitmap, HBITMAP &hBitmap); //打开图片
    int SaveImage(char* filename, HBITMAP &hBitmap); //保存图片,针对位图句柄
    int SaveImage(char* filename, BITMAP &bitmap); //保存图片，针对内存中位图结果
    int CreateEmptyImage(BITMAP &bitmap, int width, int height, int bmBitsPixel); //在内存中创建一幅空白位图
    int ReleaseHandle(); //主动释放资源
    int ReleaseBitmap(BITMAP &bitmap); //主动释放资源
    int Convert_Bitmap_BitsPixel_8_to_24(BITMAP &bitmap_24, BITMAP &bitmap_8); //将8位位图转换成24位
    int Cut_Image(BITMAP &bitmap_src, BITMAP &bitmap_dst, int x, int y, int w, int h); //裁剪位图的一个矩形区域

    //---------直接在内存DC中操作-------------------
    int Draw_Rect_DC(HDC hdc, int x, int y, int w, int h, int r, int g, int b, int line_width/*=1*/); //在位图的指定位置画指定颜色的矩形框
    int Draw_Ellipse_DC(HDC hdc, int x, int y, int w, int h, int r, int g, int b, int line_width/*=1*/); //在位图的指定位置画指定颜色的椭圆
    int Draw_Line_DC(HDC hdc, POINT pt1, POINT pt2, int r, int g, int b, int line_width/*=1*/); //画直线
    int Draw_Polygon_DC(HDC hdc, std::vector<POINT> &vector_point, int r, int g, int b, int line_width/*=1*/); //画多边形
    int Draw_Triangle_DC(HDC hdc, POINT pt1, POINT pt2, POINT pt3, int r, int g, int b, int line_width/*=1*/); //在位图的指定位置画指定颜色的三角形
    int Draw_Points_DC(HDC hdc, std::vector<POINT> &vector_point, int r, int g, int b, int line_width/*=1*/); //画点

    int picture_open(CString filename);
    int picture_open_bitmap(BITMAP bitmap);
    int picture_open_bitmap_fresh(BITMAP &bitmap); //专门用于图片播放刷新
    int picture_close();
    int picture_copy(BITMAP bitmap_src, BITMAP &bitmap_dst); //复制一张位图
    int picture_paint(HDC hdc, int dst_x, int dst_y, int dst_width, int dst_height, BITMAP &bitmap); //视频画图
    virtual int picture_draw_callback(HDC hdc); //供继承类调用
    int picture_draw_shape(HDC hdc); //画各种图形

public:
    int CalcIntersectRect(); //计算矩形交集
    int CenterVideo(); //将视频画面调整到屏幕中央(必要时缩放到屏幕窗口大小)
    int ConvertPt2Pt(POINT in, POINT &out); //将窗口客户区坐标转换成原始图片坐标
    int SetDrawShapeParam(KDraw_Shape draw_shape, std::vector<POINT> vector_point); //设置画线的参数
    int GetDrawShapeParam(KDraw_Shape &draw_shape, std::vector<POINT> &vector_point); //获取画线的参数
    int GetBitampFromDC(BITMAP &Bitmap, BOOL isDrawShape = FALSE);
    int GetMergeBitmap(BITMAP &Bitmap, BOOL isDrawShape = FALSE); //相当于合并图层
    int Set_Is_Accept_Drop_Files(BOOL isAcceptDropFiles);
    int Set_Background_Color(int r, int g, int b); //设置背景颜色
    int Set_Draw_Shape_Line_Color(int r, int g, int b); //设置画线的颜色
    int Set_Draw_Shape_Mode(KDraw_Shape draw_shape); //设置画线形状

    DECLARE_MESSAGE_MAP()

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual void PreSubclassWindow();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg UINT OnGetDlgCode();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};


