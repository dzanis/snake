/*
 *      This Code Was Created By Jeff Molofee 2000
 *      A HUGE Thanks To Fredric Echols For Cleaning Up
 *      And Optimizing This Code, Making It More Flexible!
 *      If You've Found This Code Useful, Please Let Me Know.
 *      Visit My Site At nehe.gamedev.net
 */

#include <windows.h>        // Header File For Windows
#include <gl\gl.h>          // Header File For The OpenGL32 Library
#include <gl\glu.h>         // Header File For The GLu32 Library
#include <gl\glaux.h>       // Header File For The Glaux Library
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#define WINDOWS_WIDTH 640
#define WINDOWS_HEIGHT 640
int w_height,w_width;
#define WINDOWS_TITLE "Snake by dzanis "
char tmp_programName[50]; // MAKE THIS BIG ENOUGH!

HDC         hDC=NULL;       // Private GDI Device Context
HGLRC       hRC=NULL;       // Permanent Rendering Context
HWND        hWnd=NULL;      // Holds Our Window Handle
HINSTANCE   hInstance;      // Holds The Instance Of The Application

// http://glasnost.itcarlow.ie/~powerk/technology/opengl/displaylistsandarrays/fonts.htm
typedef struct {int base;int size;} Font;
void fontInit(Font *font, int size, const char *fontName);
void drawText(Font font,float x, float y, const char *text);
void drawTextCenter(Font font,float x,float y,const char * text);
void drawNum(Font font,float x,float y,int num);

bool    keys[256];          // Array Used For The Keyboard Routine
bool    active=TRUE;        // Window Active Flag Set To TRUE By Default
bool    fullscreen=TRUE;    // Fullscreen Flag Set To Fullscreen Mode By Default
int getFPS();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);   // Declaration For WndProc

void resizeGL(int width, int height);
int initGL();
void updateGame(float deltaTime);
int drawGL(float delta);
void keyDown(int key);
void keyUp(int key);
void hide();
void show();
void destroy();

#define KEY_LEFT VK_LEFT
#define KEY_RIGHT VK_RIGHT
#define KEY_UP VK_UP
#define KEY_DOWN VK_DOWN
#define KEY_SPACE VK_SPACE
#define KEY_M 0x4D

int _initGL()                                      // All Setup For OpenGL Goes Here
{
    glShadeModel(GL_SMOOTH);                            // Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);               // Black Background
    glClearDepth(1.0f);                                 // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                            // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                             // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Really Nice Perspective Calculations
	initGL();
    return TRUE;                                        // Initialization Went OK
}

void set2DMode(float width, float height)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void _resizeGL(int width, int height)     // Resize And Initialize The GL Window
{
/*
    if (height==0)                                      // Prevent A Divide By Zero By
    {
        height=1;                                       // Making Height Equal One
    }
    //printf("w %d h %d \n" , width,height);
    glViewport(0,0,width,height);                       // Reset The Current Viewport
    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glLoadIdentity();                                   // Reset The Projection Matrix
    // Calculate The Aspect Ratio Of The Window
    float ratio = width / (float) height;
    //gluPerspective(45.0f,ratio,0.1f,100.0f);
    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
    glLoadIdentity();                                   // Reset The Modelview Matrix
    */
    w_height = height;
    w_width = width;
    glViewport(0,0,width,height);
    set2DMode(width,  height);
	resizeGL(width,height);
}


GLvoid KillGLWindow(GLvoid)                             // Properly Kill The Window
{
    if (fullscreen)                                     // Are We In Fullscreen Mode?
    {
        ChangeDisplaySettings(NULL,0);                  // If So Switch Back To The Desktop
        ShowCursor(TRUE);                               // Show Mouse Pointer
    }

    if (hRC)                                            // Do We Have A Rendering Context?
    {
        if (!wglMakeCurrent(NULL,NULL))                 // Are We Able To Release The DC And RC Contexts?
        {
            MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }

        if (!wglDeleteContext(hRC))                     // Are We Able To Delete The RC?
        {
            MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }
        hRC=NULL;                                       // Set RC To NULL
    }

    if (hDC && !ReleaseDC(hWnd,hDC))                    // Are We Able To Release The DC
    {
        MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hDC=NULL;                                       // Set DC To NULL
    }

    if (hWnd && !DestroyWindow(hWnd))                   // Are We Able To Destroy The Window?
    {
        MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hWnd=NULL;                                      // Set hWnd To NULL
    }

    if (!UnregisterClass("OpenGL",hInstance))           // Are We Able To Unregister Class
    {
        MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hInstance=NULL;                                 // Set hInstance To NULL
    }
}

/*  This Code Creates Our OpenGL Window.  Parameters Are:                   *
 *  title           - Title To Appear At The Top Of The Window              *
 *  width           - Width Of The GL Window Or Fullscreen Mode             *
 *  height          - Height Of The GL Window Or Fullscreen Mode            *
 *  bits            - Number Of Bits To Use For Color (8/16/24/32)          *
 *  fullscreenflag  - Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)   */

BOOL CreateGLWindow(char* title, int width, int height,int bits, bool fullscreenflag)
{
    GLuint      PixelFormat;            // Holds The Results After Searching For A Match
    WNDCLASS    wc;                     // Windows Class Structure
    DWORD       dwExStyle;              // Window Extended Style
    DWORD       dwStyle;                // Window Style
    RECT        WindowRect;             // Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left=(long)0;            // Set Left Value To 0
    WindowRect.right=(long)width;       // Set Right Value To Requested Width
    WindowRect.top=(long)0;             // Set Top Value To 0
    WindowRect.bottom=(long)height;     // Set Bottom Value To Requested Height

    fullscreen=fullscreenflag;          // Set The Global Fullscreen Flag

    hInstance           = GetModuleHandle(NULL);                // Grab An Instance For Our Window
    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc      = (WNDPROC) WndProc;                    // WndProc Handles Messages
    wc.cbClsExtra       = 0;                                    // No Extra Window Data
    wc.cbWndExtra       = 0;                                    // No Extra Window Data
    wc.hInstance        = hInstance;                            // Set The Instance
    wc.hIcon            = LoadIcon(NULL, IDI_WINLOGO);          // Load The Default Icon
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
    wc.hbrBackground    = NULL;                                 // No Background Required For GL
    wc.lpszMenuName     = NULL;                                 // We Don't Want A Menu
    wc.lpszClassName    = "OpenGL";                             // Set The Class Name

    if (!RegisterClass(&wc))                                    // Attempt To Register The Window Class
    {
        MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                                           // Return FALSE
    }

    if (fullscreen)                                             // Attempt Fullscreen Mode?
    {
        DEVMODE dmScreenSettings;                               // Device Mode
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));   // Makes Sure Memory's Cleared
        dmScreenSettings.dmSize=sizeof(dmScreenSettings);       // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth    = width;                // Selected Screen Width
        dmScreenSettings.dmPelsHeight   = height;               // Selected Screen Height
        dmScreenSettings.dmBitsPerPel   = bits;                 // Selected Bits Per Pixel
        dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
        {
            // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
            if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                fullscreen=FALSE;       // Windowed Mode Selected.  Fullscreen = FALSE
            }
            else
            {
                // Pop Up A Message Box Letting User Know The Program Is Closing.
                MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
                return FALSE;                                   // Return FALSE
            }
        }
    }

    if (fullscreen)                                             // Are We Still In Fullscreen Mode?
    {
        dwExStyle=WS_EX_APPWINDOW;                              // Window Extended Style
        dwStyle=WS_POPUP;                                       // Windows Style
        ShowCursor(FALSE);                                      // Hide Mouse Pointer
    }
    else
    {
        dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;           // Window Extended Style
        dwStyle=WS_OVERLAPPEDWINDOW;                            // Windows Style
    }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);     // Adjust Window To True Requested Size

    // Create The Window
    if (!(hWnd=CreateWindowEx(  dwExStyle,                          // Extended Style For The Window
                                "OpenGL",                           // Class Name
                                title,                              // Window Title
                                dwStyle |                           // Defined Window Style
                                WS_CLIPSIBLINGS |                   // Required Window Style
                                WS_CLIPCHILDREN,                    // Required Window Style
                                0, 0,                               // Window Position
                                WindowRect.right-WindowRect.left,   // Calculate Window Width
                                WindowRect.bottom-WindowRect.top,   // Calculate Window Height
                                NULL,                               // No Parent Window
                                NULL,                               // No Menu
                                hInstance,                          // Instance
                                NULL)))                             // Dont Pass Anything To WM_CREATE
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }

    static  PIXELFORMATDESCRIPTOR pfd=              // pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),              // Size Of This Pixel Format Descriptor
        1,                                          // Version Number
        PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
        PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                           // Must Support Double Buffering
        PFD_TYPE_RGBA,                              // Request An RGBA Format
        16,                                       // Select Our Color Depth //FIXME  здесь был bits
        0, 0, 0, 0, 0, 0,                           // Color Bits Ignored
        0,                                          // No Alpha Buffer
        0,                                          // Shift Bit Ignored
        0,                                          // No Accumulation Buffer
        0, 0, 0, 0,                                 // Accumulation Bits Ignored
        16,                                         // 16Bit Z-Buffer (Depth Buffer)
        0,                                          // No Stencil Buffer
        0,                                          // No Auxiliary Buffer
        PFD_MAIN_PLANE,                             // Main Drawing Layer
        0,                                          // Reserved
        0, 0, 0                                     // Layer Masks Ignored
    };

    if (!(hDC=GetDC(hWnd)))                         // Did We Get A Device Context?
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }

    if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd))) // Did Windows Find A Matching Pixel Format?
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }

    if(!SetPixelFormat(hDC,PixelFormat,&pfd))       // Are We Able To Set The Pixel Format?
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }

    if (!(hRC=wglCreateContext(hDC)))               // Are We Able To Get A Rendering Context?
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }

    if(!wglMakeCurrent(hDC,hRC))                    // Try To Activate The Rendering Context
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }

    ShowWindow(hWnd,SW_SHOW);                       // Show The Window
    SetForegroundWindow(hWnd);                      // Slightly Higher Priority
    SetFocus(hWnd);                                 // Sets Keyboard Focus To The Window
    _resizeGL(width, height);                   // Set Up Our Perspective GL Screen

    if (!_initGL())                                  // Initialize Our Newly Created GL Window
    {
        KillGLWindow();                             // Reset The Display
        MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;                               // Return FALSE
    }


    return TRUE;                                    // Success
}

LRESULT CALLBACK WndProc(   HWND    hWnd,           // Handle For This Window
                            UINT    uMsg,           // Message For This Window
                            WPARAM  wParam,         // Additional Message Information
                            LPARAM  lParam)         // Additional Message Information
{
    switch (uMsg)                                   // Check For Windows Messages
    {
        case WM_ACTIVATE:                           // Watch For Window Activate Message
        {
            if (!HIWORD(wParam))                    // Check Minimization State
            {
                active=TRUE;                        // Program Is Active
                show();
            }
            else
            {
                active=FALSE;                       // Program Is No Longer Active
                hide();
            }

            return 0;                               // Return To The Message Loop
        }

        case WM_SYSCOMMAND:                         // Intercept System Commands
        {
            switch (wParam)                         // Check System Calls
            {
                case SC_SCREENSAVE:                 // Screensaver Trying To Start?
                case SC_MONITORPOWER:               // Monitor Trying To Enter Powersave?
                return 0;                           // Prevent From Happening
            }
            break;                                  // Exit
        }

        case WM_CLOSE:                              // Did We Receive A Close Message?
        {
            PostQuitMessage(0);                     // Send A Quit Message
            return 0;                               // Jump Back
        }

        case WM_KEYDOWN:                            // Is A Key Being Held Down?
        {
            keys[wParam] = TRUE;                    // If So, Mark It As TRUE
            keyDown(wParam);
            return 0;                               // Jump Back
        }

        case WM_KEYUP:                              // Has A Key Been Released?
        {
            keys[wParam] = FALSE;                   // If So, Mark It As FALSE
            keyUp(wParam);
            return 0;                               // Jump Back
        }

        case WM_SIZE:                               // Resize The OpenGL Window
        {
            _resizeGL(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
            return 0;                               // Jump Back
        }
    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}



void fontInit(Font * font,int size, const char * fontName)
{
    static int base_inc=0;
    font->base = base_inc+=1000;
    font->size = size;
    HFONT hfont = CreateFont(size, 0, 0, 0,
                               FW_NORMAL, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, 0,
                                0,0,0,fontName); // can only use true type fonts
                                                        //  this one is 40 points in size
    SelectObject(hDC, hfont);

    //wglUseFontOutlines(hDC,0,127,font_list_base_2d, 0.0f, 0.1f,WGL_FONT_POLYGONS,NULL);
            // generate a display list for every character (up to ascii 127)
            // generate as accurately as possible (0.0f deviation)
            // extrude by 0.1 units
            // generate polygons (not lines) to render the fonts

    wglUseFontBitmaps(hDC,0,127,font->base); // create a display list for each character (0-127)
                                                        // start numbering the display lists at font_list_base_2d
}

void drawText(Font font,float x,float y,const char * text)
{
    glRasterPos3f(x,y,0); // set start position
    glListBase(font.base); //start of our font display list numbers
    GLint len=strlen(text);
    glCallLists(len, GL_UNSIGNED_BYTE, text);
}

void drawTextCenter(Font font,float x,float y,const char * text)
{
    GLint len=strlen(text);
    x = x - len*(font.size/4); // good for Consolas font
    x = x < 0 ? 0:x;
    glRasterPos3f(x,y,0); // set start position
    glListBase(font.base); //start of our font display list numbers
    glCallLists(len, GL_UNSIGNED_BYTE, text);
}

void drawNum(Font font,float x,float y,int num)
{
    char str[10];
    sprintf(str, "%d", num);
    drawText(font,x,y,str);
}


int WINAPI WinMain( HINSTANCE   hInstance,          // Instance
                    HINSTANCE   hPrevInstance,      // Previous Instance
                    LPSTR       lpCmdLine,          // Command Line Parameters
                    int         nCmdShow)           // Window Show State
{
    MSG     msg;                                    // Windows Message Structure
    BOOL    exit=FALSE;                             // Bool Variable To Exit Loop

    // Ask The User Which Screen Mode They Prefer
    //if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
    {
        fullscreen=FALSE;                           // Windowed Mode
    }


    sprintf(tmp_programName, "%s (%s)", WINDOWS_TITLE,__DATE__ );

    // Create Our OpenGL Window
    if (!CreateGLWindow(tmp_programName,WINDOWS_WIDTH,WINDOWS_HEIGHT,16,fullscreen))
    {
        return 0;                                   // Quit If Window Was Not Created
    }


    double lastTime = GetTickCount();
    while(!exit)                                    // Loop That Runs While done=FALSE
    {
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))   // Is There A Message Waiting?
        {
            if (msg.message==WM_QUIT)               // Have We Received A Quit Message?
            {
                exit=TRUE;                          // If So done=TRUE
            }
            else                                    // If Not, Deal With Window Messages
            {
                TranslateMessage(&msg);             // Translate The Message
                DispatchMessage(&msg);              // Dispatch The Message
            }
        }
        else                                        // If There Are No Messages
        {           
            // Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
            if (active)                             // Program Active?
            {
                if (keys[VK_ESCAPE])                // Was ESC Pressed?
                {
                    exit=TRUE;                      // ESC Signalled A Quit
                }
                else                                // Not Time To Quit, Update Screen
                {

                    // Compute time step
                    double currentTime =  GetTickCount();
                    float deltaTime = (currentTime - lastTime)/ 1000.f;
                    lastTime = currentTime;
                    //updateGame(deltaTime);
                    drawGL(deltaTime);                  // Draw The Scene
                    SwapBuffers(hDC);               // Swap Buffers (Double Buffering)
                    Sleep(10);                
                }
            }
            else
            {
                Sleep(20);
            }

            if (keys[VK_F1])                        // Is F1 Being Pressed?
            {
                keys[VK_F1]=FALSE;                  // If So Make Key FALSE
                KillGLWindow();                     // Kill Our Current Window
                fullscreen=!fullscreen;             // Toggle Fullscreen / Windowed Mode
                // Recreate Our OpenGL Window
                if (!CreateGLWindow(tmp_programName,640,640,16,fullscreen))
                {
                    return 0;                       // Quit If Window Was Not Created
                }
            }
        }
    }

    destroy();
    // Shutdown
    KillGLWindow();                                 // Kill The Window
    return (msg.wParam);                            // Exit The Program
}

int getFPS()
{
    static int fps,fps_count;
    static int fps_time ;
    fps_count++;
    if(clock() - fps_time > CLOCKS_PER_SEC )
    {
        fps_time = clock();
        fps = fps_count;
        fps_count = 0;
    }
    return  fps;
}
