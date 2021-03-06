/**
 * @brief 测试各类事件
 * 
 * 编译：gcc simple_event.c -lX11 -lXcursor
 * 参考：窗口最大化、悬浮等：http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#URGENCY
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xcursor/Xcursor.h>

#define MWM_DECOR_ALL           (1L << 0)
#define MWM_FUNC_ALL            (1L << 0)
#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define PROP_MOTIF_WM_HINTS_ELEMENTS	5


#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

/** 事件名称 */
const char* const X11_EVENT_STRINGS[] =
{
	"", "",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify",
	"GenericEvent",
};

typedef struct _test_context {
    Display *display;
    int screen_number;
    Screen *screen;
    int depth;
    Atom _NET_WM_STATE;
    Atom _NET_WM_STATE_SKIP_PAGER;
    Atom _NET_WM_STATE_SKIP_TASKBAR;
    Atom _MOTIF_WM_HINTS;
    Atom _NET_WM_MOVERESIZE;
	XSetWindowAttributes winAttrib;
    
}testContext;

struct _PropMotifWmHints
{
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long inputMode;
	unsigned long status;
};
typedef struct _PropMotifWmHints PropMotifWmHints;

/**
 * @brief 设置后任务栏没有图表
 */
void xf_SetWindowUnlisted(testContext *cxt, Window window)
{
	Atom window_state[2];

	window_state[0] = cxt->_NET_WM_STATE_SKIP_PAGER;
	window_state[1] = cxt->_NET_WM_STATE_SKIP_TASKBAR;

	XChangeProperty(cxt->display, window, cxt->_NET_WM_STATE,
			XA_ATOM, 32, PropModeReplace, (char *) &window_state, 2);
}

/**
 * @breif 创建实例
 */
testContext *test_context_new() {
    testContext *cxt = (testContext *)malloc(sizeof(testContext));
    cxt->display = XOpenDisplay(NULL);
	cxt->_NET_WM_STATE = XInternAtom(cxt->display, "_NET_WM_STATE", False);
	cxt->_NET_WM_STATE_SKIP_PAGER = XInternAtom(cxt->display, "_NET_WM_STATE_SKIP_PAGER", False);
	cxt->_NET_WM_STATE_SKIP_TASKBAR = XInternAtom(cxt->display, "_NET_WM_STATE_SKIP_TASKBAR", False);
	cxt->_MOTIF_WM_HINTS = XInternAtom(cxt->display, "_MOTIF_WM_HINTS", False);
    cxt->_NET_WM_MOVERESIZE = XInternAtom(cxt->display, "_NET_WM_MOVERESIZE", False);
    cxt->screen_number = DefaultScreen(cxt->display);
    cxt->screen = ScreenOfDisplay(cxt->display, cxt->screen_number);
    cxt->depth = DefaultDepthOfScreen(cxt->screen);
    return cxt;
}

/**
 * 设置完后，建立的窗口就没有Ubuntu自带的关闭、最小化和最大化这三个键了
 */
void setWindowDecorations(testContext *cxt, Window window, int show)
{
	PropMotifWmHints hints;
	
	hints.decorations = (show) ? MWM_DECOR_ALL : 0;
	hints.functions = MWM_FUNC_ALL ;
	hints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
	hints.inputMode = 0;
	hints.status = 0;

	XChangeProperty(cxt->display, window, cxt->_MOTIF_WM_HINTS, cxt->_MOTIF_WM_HINTS, 32,
			PropModeReplace, (char*) &hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
}

/**
 * 设置override redirect后窗口一直显示在屏幕上，不会被覆盖，
 * 并且不受窗口管理器控制，任务栏中没有图标
 */
void setWindowOverrideRedirect(testContext *cxt, Window window) {
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(cxt->display, window, CWOverrideRedirect, &attrs);
}

/**
 * @breif 模拟发送事件
 *
 * @param atom  _NET_WM_MOVERESIZE 发送这个后就可以按住鼠标拖动窗体
 *              _NET_WM_STATE 窗口状态，如最大化
 */
void sendClientEvent(testContext *cxt, Window window, Atom atom, unsigned int numArgs, ...) {
    XEvent xevent;
    unsigned int i;
    va_list argp;
    va_start(argp, numArgs);
    memset(&xevent, 0, sizeof(XEvent));

    xevent.xclient.type = ClientMessage;
    xevent.xclient.serial = 0;
    xevent.xclient.send_event = False;
    xevent.xclient.display = cxt->display;
    xevent.xclient.window = window;
    xevent.xclient.message_type = atom;
    xevent.xclient.format = 32;

    fprintf(stdout, "%s numArgs:%d\n", __func__, numArgs);
    for (i=0; i<numArgs; i++) {
        xevent.xclient.data.l[i] = va_arg(argp, int);
    }
    XSendEvent(cxt->display, RootWindowOfScreen(cxt->screen), False,
             SubstructureRedirectMask | SubstructureNotifyMask, &xevent);
    XSync(cxt->display, False);
    va_end(argp);
}

/**
 * @brief 设置窗口悬浮，设置后窗口一直显示在屏幕上
 *
 * _NET_WM_STATE_ABOVE indicates that the window should be on top of most windows
 * 参考：http://stackoverflow.com/questions/20733215/how-to-make-a-window-always-on-top
 *      http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#URGENCY
 */
Status x11_set_window_on_top(testContext *cxt, Window window) {
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = cxt->display;
    event.xclient.window  = window;
    event.xclient.message_type = XInternAtom (cxt->display, "_NET_WM_STATE", False);
    event.xclient.format = 32;

    event.xclient.data.l[0] = _NET_WM_STATE_ADD;
    event.xclient.data.l[1] = XInternAtom (cxt->display, "_NET_WM_STATE_ABOVE", False);
    event.xclient.data.l[2] = 0; //unused.
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    return XSendEvent (cxt->display, DefaultRootWindow(cxt->display), False,
                       SubstructureRedirectMask|SubstructureNotifyMask, &event);
}

/**
 * @breif 创建窗口
 */
Window create_window(testContext *cxt) {
	Window window;
	int x, y;
	int width, height;
	
	x = 30;
	y = 60;
	width = 200;
	height = 300;
	window = XCreateWindow(cxt->display, RootWindowOfScreen(cxt->screen),
						x, y,
						width, height,
						0,
						cxt->depth,
						InputOutput, DefaultVisual(cxt->display, 0),
						0, &cxt->winAttrib);
	setWindowDecorations(cxt, window, 0);	//设置后没有Ubuntu自带的关闭、最小和最大这三个键
#if 1
    x11_set_window_on_top(cxt, window);
#else
    setWindowOverrideRedirect(cxt, window); //设置override redirect，设置后不受窗口管理器控制也没有任务栏图标
//    xf_SetWindowUnlisted(cxt, window);      //设置后任务栏中没有图表
#endif
	return window;
}

int main(int argc, char *argv[]) {
	Window window;
	int input_mask;
	XEvent event;
	
    testContext *cxt = test_context_new();
    window = create_window(cxt);
	
	input_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
				 ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
				 PointerMotionMask | Button1MotionMask | Button2MotionMask |
				 Button3MotionMask | Button4MotionMask | Button5MotionMask |
				 ButtonMotionMask | KeymapStateMask | ExposureMask |
				 VisibilityChangeMask | StructureNotifyMask | SubstructureNotifyMask |
				 SubstructureRedirectMask | FocusChangeMask | PropertyChangeMask |
				 ColormapChangeMask | OwnerGrabButtonMask;
	XSelectInput(cxt->display, window, input_mask); //设置要响应的事件
	
	XMapWindow(cxt->display, window);
	XFlush(cxt->display);
    int x=0, y=0;
    int width=200, height=300;
    KeySym keysym;
    int num = 0;
	while (1) {
		XNextEvent(cxt->display, &event); //获取事件
		if (event.type!=MotionNotify)
			fprintf(stdout, "event type:%d;%s\n", event.type, X11_EVENT_STRINGS[event.type]);
        switch (event.type) {
        case ButtonPress:
#if 0
            /*点一次按键就移动窗口并修改其尺寸*/
            x+=30;
            y+=20;
            width += 40;
            height += 60;
            XMoveResizeWindow(cxt->display, window, x, y, width, height);
#else
            /* 在窗口区域内按住鼠标左键进行拖动 */
            XUngrabPointer(cxt->display, CurrentTime); //必须要有这句，没有这句不会产生拖动的效果。
            Window child_window;
            XTranslateCoordinates(cxt->display, window, 
                    RootWindowOfScreen(cxt->screen), event.xbutton.x, event.xbutton.y,
                    &x, &y, &child_window); /**< 加上这句解决鼠标拖动窗体时鼠标位置不对的问题 */
            fprintf(stdout, "x:%d;y:%d;%d;%d\n", x, y, event.xbutton.x, event.xbutton.y);
            sendClientEvent(cxt, window, cxt->_NET_WM_MOVERESIZE, 5, x, y, 8, 1, 1);
#endif
            break;
        case KeyPress:
            keysym = XLookupKeysym (&event.xkey, 0);
            if (XK_h == keysym) {
                fprintf(stdout, "h key press;try to hide\n");
                XWithdrawWindow(cxt->display, window, cxt->screen_number);
            } else if (XK_m == keysym) {
                /* 窗口移动 */
                XMoveWindow(cxt->display, window, 200, 100);
            } else if (XK_i == keysym) {
                /* 最小化 */
                XIconifyWindow(cxt->display, window, cxt->screen_number);
            } else if (XK_a == keysym) {
                // 最大化
                //_NET_WM_STATE_MAXIMIZED_{VERT,HORZ} indicates that the window is {vertically,horizontally} maximized
                sendClientEvent(cxt, window, cxt->_NET_WM_STATE, 4, 1, 
                        XInternAtom(cxt->display, "_NET_WM_STATE_MAXIMIZED_VERT", False),
                        XInternAtom(cxt->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False), 0);
            } else if (XK_c == keysym) {
            	fprintf(stdout, "c press\n");
            	// Cursor c = XcursorLibraryLoadCursor(cxt->display, "sb_v_double_arrow");
            	Cursor c = XcursorLibraryLoadCursor(cxt->display, "pencil");
            	XDefineCursor(cxt->display, window, c);
            } else if (XK_q == keysym) {
                fprintf(stdout, "try to quit\n");
                exit(0);
            }
            break;
        case ConfigureNotify:
            if (num++>100) {
                int x, y;
                int child_x;
                int child_y;
                unsigned int mask;
                Window root_window;
                Window child_window;
                XQueryPointer(cxt->display, window,
                    &root_window, &child_window, &x, &y, &child_x, &child_y, &mask); // 鼠标当前位置
                fprintf(stdout, "button position-----------------%d %d %d %d \n", x, y, child_x, child_y);
                fprintf(stdout, "test to stop move=================\n");
                // 停止窗口拖动
                sendClientEvent(cxt, window, cxt->_NET_WM_MOVERESIZE, 5, 0, 0, 11, 1, 1);
                num = 0;
            }
            break;
        }
	}
	
	XDestroyWindow(cxt->display, window);
	XCloseDisplay(cxt->display);
    free(cxt);
    return 0;
}

