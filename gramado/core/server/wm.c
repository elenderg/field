/*
 * File: wm.c 
 *     The Window Manager.
 * History:
 *     2020 - Create by Fred Nora.
 */


#include <gws.h>




#define TB_HEIGHT  40
#define TB_BUTTON_PADDING  4
#define TB_BUTTON_HEIGHT  (TB_HEIGHT - (TB_BUTTON_PADDING*2))
#define TB_BUTTON_WIDTH  TB_BUTTON_HEIGHT
#define TB_BUTTONS_MAX  8
int tb_buttons[TB_BUTTONS_MAX];
unsigned long tb_windows[TB_BUTTONS_MAX];  // ponteiros de estrutura de janelas.
// Quantos botões ja temos.
static int tb_buttons_count=0;  

// ---------

#define WM_DEFAULT_BACKGROUND_COLOR   COLOR_GRAY

int mousehover_window=0;


static long old_x=0;
static long old_y=0;

//#todo
//GetWindowRect
//GetClientRect

// refresh rate of the whole screen.
static unsigned long fps=0;

// refresh rate for all dirty objects. In one round.
static unsigned long frames_count=0;
//static unsigned long frames_count_in_this_round;

static unsigned long ____old_time=0;
static unsigned long ____new_time=0;


void __set_default_background_color( int color )
{
    WindowManager.default_background_color = (unsigned int) color;
}

unsigned int __get_default_background_color(void)
{
    return (unsigned int) WindowManager.default_background_color;
}

void __set_custom_background_color( int color )
{
    WindowManager.custom_background_color = (unsigned int) color;
    WindowManager.has_custom_background_color = TRUE;
}

unsigned int __get_custom_background_color(void)
{
    return (unsigned int) WindowManager.custom_background_color;
}

int __has_custom_background_color(void)
{
    if ( WindowManager.has_custom_background_color == TRUE )
        return TRUE;

    return FALSE;
}

int __has_wallpaper(void)
{
    if ( WindowManager.has_wallpaper == TRUE )
        return TRUE;

    return FALSE;
}

void __init_wm_structure(void)
{

// Clear the structure.
    WindowManager.mode = 1;  //tiling

//orientation
    WindowManager.vertical = TRUE;   //default
    //WindowManager.vertical = FALSE;
    
    WindowManager.root = NULL;
    WindowManager.taskbar = NULL;
    WindowManager.wa_left = 0;
    WindowManager.wa_top = 0;
    WindowManager.wa_width=0;
    WindowManager.wa_height=0;

// Default background color.
    __set_default_background_color(WM_DEFAULT_BACKGROUND_COLOR);

// Default background color.
    __set_custom_background_color(COLOR_GREEN);
    WindowManager.has_custom_background_color = FALSE;

// Wallpaper
    WindowManager.has_wallpaper = FALSE;

// Not initialized yet.
// We need to setup the windows elements.

    WindowManager.initialized = FALSE;
}


// Internal
// Called by wm_process_windows().

void __update_fps(void)
{
    unsigned long dt=0;
    char rate_string[32];

    debug_print ("__update_fps:\n");

// counter
    frames_count++;

//
// == time =========================================
//

    // #bugbug
    // We have a HUGE problem here.
    // We can't properly get the data inside the structures. 
    // The value is not the same when we enter inside the kernel via
    // keyboard interrupt or via system interrupt.


// get current time.

// #bugbug
// A variável do indice 120 não esta sendo usada.
// Vamos tentar a variável do indice 118, que é a jiffies.

    //____new_time = rtl_get_progress_time();
    //____new_time = (unsigned long) rtl_get_system_metrics (120);
    ____new_time = (unsigned long) rtl_get_system_metrics (118);

// delta
    dt = (unsigned long) (____new_time - ____old_time);

    ____old_time = ____new_time;


    fps = (1000/dt);

// mostra 
    //if ( show_fps_window == TRUE )
    //{
        //itoa(____new_time,rate_string);
        //itoa(dt,rate_string);
        itoa(fps,rate_string);
        yellow_status(rate_string);
    //}

    return;
    

    //if(dt<8)
        //return;

//=============================================================
// ++  End

    //t_end = rtl_get_progress_time();
    //__refresh_rate =  t_end - t_start;
    //__refresh_rate = __refresh_rate/1000;
    //printf ("@ %d %d %d \n",__refresh_rate, t_now, t_old);


    //====================================
    // fps++
    // conta quantos frames. 

    // se passou um segundo.
    //if ( dt > 1000 )
    if ( dt > 8 )
    {
        // Save old time.
        ____old_time = ____new_time;
        
        //fps = frames_count; // quantos frames em 1000 ms aproximadamente?
        //itoa(fps,rate_string); 

        itoa(dt,rate_string); // mostra o delta.

        //if ( show_fps_window == TRUE ){
            yellow_status(rate_string);
        //}

        // Clean for next round.
        frames_count=0;
        fps=0;
        dt=0;
    }
    //fps--
    //=======================

    debug_print ("__update_fps: done\n");
}


// WORKER
// Paint button borders.
// Called by xxxCreateWindow
// >>> No checks
// IN: window, color, color, color, color.
void 
__draw_button_borders(
    struct gws_window_d *w,
    unsigned int color1,
    unsigned int color2,
    unsigned int color2_light,
    unsigned int outer_color )
{

    debug_print("__draw_button_borders:\n");


// This is the window for relative positions.
    if ( (void*) w == NULL ){
        return;
    }

//  ____
// |
//
// board1, borda de cima e esquerda.

// Cima
    rectBackbufferDrawRectangle ( 
        w->left+1, w->top, 
        w->width-2, 1, 
        outer_color, TRUE,0 );
    rectBackbufferDrawRectangle ( 
        w->left+1, w->top+1, 
        w->width-2, 1, 
        color1, TRUE,0 );
    rectBackbufferDrawRectangle ( 
        w->left+1+1, w->top+1+1,
        w->width-4, 1, 
        color1, TRUE,0 );

// Esq
    rectBackbufferDrawRectangle ( 
        w->left, w->top+1, 
        1, w->height-2,
        outer_color, TRUE,0 );
    rectBackbufferDrawRectangle ( 
        w->left+1, w->top+1, 
        1, w->height-2,
        color1, TRUE,0 );
    rectBackbufferDrawRectangle ( 
        w->left+1+1, w->top+1+1, 
        1, w->height-4,
        color1, TRUE,0 );

//  
//  ____|
//
// board2, borda direita e baixo.

// Dir
    rectBackbufferDrawRectangle ( 
        ((w->left) + (w->width) -1 -1 -1), 
        w->top+1+1, 
        1, 
        w->height-4, 
        color2_light, TRUE, 0 );
    rectBackbufferDrawRectangle ( 
        ((w->left) + (w->width) -1 -1), 
        w->top+1, 
        1, 
        w->height-2, 
        color2, TRUE, 0 );
    rectBackbufferDrawRectangle ( 
        ((w->left) + (w->width) -1), 
        w->top+1, 
        1, 
        w->height-2, 
        outer_color, TRUE, 0 );

// Baixo
    rectBackbufferDrawRectangle ( 
        w->left+1+1, ( (w->top) + (w->height) -1 -1 -1),  
        w->width-4, 1, 
        color2_light, TRUE, 0 );
    rectBackbufferDrawRectangle ( 
        w->left+1, ( (w->top) + (w->height) -1 -1),  
        w->width-2, 1, 
        color2, TRUE, 0 );
    rectBackbufferDrawRectangle ( 
        w->left+1, ( (w->top) + (w->height) -1 ),  
        w->width-2, 1, 
        outer_color, TRUE, 0 );

    debug_print("__draw_button_borders: done\n");
}


// worker:
// no checks
// Draw the border of an edit box.
void 
__draw_window_border( 
    struct gws_window_d *parent, 
    struct gws_window_d *window )
{

    debug_print("__draw_window_border:\n");

    if ( (void*) parent == NULL )
        return;

    if ( (void*) window == NULL )
        return;


    if( window->type == WT_EDITBOX )
    {
        // board1, borda de cima e esquerda.
        rectBackbufferDrawRectangle( 
            window->left, window->top, 
            window->width, window->border_size, 
            window->border_color, TRUE,0 );

        rectBackbufferDrawRectangle( 
            window->left, window->top, 
            window->border_size, window->height, 
            window->border_color, TRUE,0 );

        // board2, borda direita e baixo.
        rectBackbufferDrawRectangle( 
            (window->left + window->width - window->border_size), window->top,  
            window->border_size, window->height, 
            window->border_color, TRUE,0 );

        rectBackbufferDrawRectangle ( 
            window->left, (window->top + window->height - window->border_size), 
            window->width, window->border_size, 
            window->border_color, TRUE,0 );
    }

    if( window->type == WT_OVERLAPPED )
    {
        // board1, borda de cima e esquerda.
        rectBackbufferDrawRectangle( 
            parent->left + window->left, parent->top + window->top, 
            window->width, window->border_size, 
            window->border_color, TRUE,0 );

        rectBackbufferDrawRectangle( 
            parent->left + window->left, parent->top + window->top, 
            window->border_size, window->height, 
            window->border_color, TRUE,0 );

        //board2, borda direita e baixo.
        rectBackbufferDrawRectangle( 
            (parent->left + window->left + window->width - window->border_size), (parent->top + window->top), 
            window->border_size, window->height, 
            window->border_color, TRUE,0 );

        rectBackbufferDrawRectangle ( 
            (parent->left + window->left), (parent->top + window->top + window->height - window->border_size), 
            window->width, window->border_size, 
            window->border_color, TRUE,0 );
    }

    debug_print("__draw_window_border: done\n");
}


/*
 ********************************
 * wmDrawFrame:
 * 
 */

// Called by CreateWindow in createw.c

// #importante:
// Essa rotina será chamada depois que criarmos uma janela básica,
// mas só para alguns tipos de janelas, pois nem todos os tipos 
// precisam de um frame. Ou ainda, cada tipo de janela tem um 
// frame diferente. Por exemplo: Podemos considerar que um checkbox 
// tem um tipo de frame.
// Toda janela criada pode ter um frame.
// Durante a rotina de criação do frame para uma janela que ja existe
// podemos chamar a rotina de criação da caption bar, que vai criar os
// botões de controle ... mas nem toda janela que tem frame precisa
// de uma caption bar (Title bar).
// Estilo do frame:
// Dependendo do estilo do frame, podemos ou nao criar a caption bar.
// Por exemplo: Uma editbox tem um frame mas não tem uma caption bar.

// IN:
// parent = parent window ??
// window = The window where to build the frame.
// x
// y
// width
// height
// style = Estilo do frame.

// OUT:
// 0   = ok, no erros;
// < 0 = not ok. something is wrong.

int 
wmCreateWindowFrame ( 
    struct gws_window_d *parent,
    struct gws_window_d *window,
    unsigned long border_size,
    unsigned int border_color1,
    unsigned int border_color2,
    unsigned int border_color3,
    unsigned int ornament_color1,
    unsigned int ornament_color2,
    int style ) 
{

    int useFrame       = FALSE;
    int useTitleBar    = FALSE;
    int useTitleString = FALSE;
    int useBorder      = FALSE;
    int useIcon        = FALSE;
    // ...

// #bugbug
// os parâmetros 
// parent, 
// x,y,width,height 
// não estão sendo usados.


// Overlapped.
// Janela de aplicativos.

// Title bar and status bar.
    struct gws_window_d  *tbWindow;
    struct gws_window_d  *sbWindow;

    int id=-1;  //usado pra registrar janelas filhas.

    int Type=0;

// Border color.
    unsigned long BorderSize   = (border_size & 0xFFFF);
    unsigned int  BorderColor1 = border_color1;
    unsigned int  BorderColor2 = border_color2;
    unsigned int  BorderColor3 = border_color3;
    unsigned int  BorderColor  = border_color1;

// Ornament color.
    unsigned int OrnamentColor1 = ornament_color1;
    unsigned int OrnamentColor2 = ornament_color2;

// Title bar color.
    unsigned int TitleBarColor = COLOR_BLUE1;   // Light blue (Um pouco fosco) 
    //unsigned long TitleBarColor = 0x001473E6;  // Claro.  
    //unsigned long TitleBarColor = 0x00000E80;  // Dark blue
    // ...

    //unsigned long X = (x & 0xFFFF);
    //unsigned long Y = (y & 0xFFFF);
    //unsigned long Width = (width & 0xFFFF);
    //unsigned long Height = (height & 0xFFFF);

// #debug
    //gwssrv_debug_print ("wmCreateWindowFrame:\n");

// #todo
// Se estamos minimizados ou a janela mãe está minimizada,
// então não temos o que pintar.
// #todo
// O estilo de frame é diferente se estamos em full screen ou maximizados.
// não teremos bordas laterais
// #todo
// Cada elemento da frame que incluimos, incrementa
// o w.top do retângulo da área de cliente.

// check parent

    if ( (void*) parent == NULL ){
        gwssrv_debug_print ("wmCreateWindowFrame: [FAIL] parent\n");
        return -1;
    }

    if (parent->used != TRUE || parent->magic != 1234)
        return -1;

// check window

    if ( (void*) window == NULL ){
        gwssrv_debug_print ("wmCreateWindowFrame: [FAIL] window\n");
        return -1;
    }

    if (window->used != TRUE || window->magic != 1234)
        return -1;

// #bugbug
// Estamos mascarando pois os valores anda corrompendo.

    window->left   = (window->left   & 0xFFFF);
    window->top    = (window->top    & 0xFFFF);
    window->width  = (window->width  & 0xFFFF);
    window->height = (window->height & 0xFFFF);

// #todo
// Desenhar o frame e depois desenhar a barra de títulos
// caso esse estilo de frame precise de uma barra.
// Editbox
// EDITBOX NÃO PRECISA DE BARRA DE TÍTULOS.
// MAS PRECISA DE FRAME ... QUE SERÃO AS BORDAS.
    
//
// Type
//

// Qual é o tipo da janela em qual precisamos
// criar o frame. Isso indica o tipo de frame.

    Type = window->type;

    switch (Type){
    
    case WT_EDITBOX:     
        useFrame=TRUE; 
        useIcon=FALSE;
        useBorder=TRUE;
        break;
    
    case WT_OVERLAPPED:  
        useFrame=TRUE; 
        useIcon=TRUE;
        useTitleBar=TRUE;  // Normalmente uma janela tem a barra de t[itulos.
        if ( window->style & 0x0001 ){ useTitleBar=FALSE; }    //maximized
        //if ( window->style & 0x0004 ){ useTitleBar=FALSE; }  //fullscreen
        useTitleString=TRUE;
        useBorder=TRUE;
        break;
    
    case WT_BUTTON:      
        useFrame=TRUE;
        useIcon=FALSE; 
        break;

    //default: break;
    };

    if ( useFrame == FALSE ){
        gwssrv_debug_print ("wmCreateWindowFrame: [ERROR] This type does not use a frame.\n");
        return -1;
    }

// ===============================================
// editbox

    if ( Type == WT_EDITBOX )
    {

        // #todo
        // The window structure has a element for border size
        // and a flag to indicate that border is used.
        // It also has a border style.

        // Se tiver o foco.
        if ( window->focus == TRUE ){
            BorderColor = COLOR_BLUE;
            BorderSize = 4;
        }else{
            BorderColor = COLOR_BLACK;  // COLOR_INACTIVEBORDER;
            BorderSize = 2;
        };
        
        window->border_size = 0;
        window->borderUsed = FALSE;
        if (useBorder==TRUE){
            window->border_color = BorderColor;
            window->border_size  = BorderSize;
            window->borderUsed   = TRUE;
        }

        // Draw the border of an edit box.
        __draw_window_border(parent,window);

        /*
        // board1, borda de cima e esquerda.
        rectBackbufferDrawRectangle( 
            window->left, window->top, 
            window->width, window->border_size, 
            window->border_color, TRUE,0 );

        rectBackbufferDrawRectangle( 
            window->left, window->top, 
            window->border_size, window->height, 
            window->border_color, TRUE,0 );

        // board2, borda direita e baixo.
        rectBackbufferDrawRectangle( 
            (window->left + window->width - window->border_size), window->top,  
            window->border_size, window->height, 
            window->border_color, TRUE,0 );

        rectBackbufferDrawRectangle ( 
            window->left, (window->top + window->height - window->border_size), 
            window->width, window->border_size, 
            window->border_color, TRUE,0 );
        */
        
        // ok
        return 0;
    }


// ===============================================
// overlapped?
// Draw border, titlebar and status bar.

    // String support.
    // String at center?

    size_t tmp_size = (size_t) strlen ( (const char *) window->name );
    if (tmp_size > 64){
        tmp_size=64;
    }
    unsigned long offset = 
        ( ( (unsigned long) window->width - ( (unsigned long) tmp_size * (unsigned long) gcharWidth) ) / 2 );



    if ( Type == WT_OVERLAPPED )
    {

        // #todo
        // Maybe we nned border size and padding size.
        
        // Consistente para overlapped.
        BorderSize = METRICS_BORDER_SIZE;
        // ...
        
        // #todo
        // The window structure has a element for border size
        // and a flag to indicate that border is used.
        // It also has a border style.

        // Se tiver o foco.
        if ( window->focus == TRUE ){
            BorderColor = COLOR_BLUE1;
        }else{
            BorderColor = COLOR_INACTIVEBORDER;
        };

        window->border_size = 0;
        window->borderUsed = FALSE;
        if (useBorder==TRUE){
            window->border_color = BorderColor;
            window->border_size  = BorderSize;
            window->borderUsed   = TRUE;
        }

        // Quatro bordas de uma janela overlapped.
        __draw_window_border(parent,window);

        // #important:
        // The border in an overlapped window will affect
        // the top position of the client area rectangle.
        window->rcClient.top += window->border_size;

        //
        // Title bar
        //

        // #todo
        // The window structure has a flag to indicate that
        // we are using titlebar.
        // It also has a title bar style.
        // Based on this style, we can setup some
        // ornaments for this title bar.
        // #todo
        // Simple title bar.
        // We're gonna have a wm inside the window server.
        // The title bar will be very simple.
        // We're gonna have a client area.
        // #bugbug
        // Isso vai depender da resolução da tela.
        // Um tamanho fixo pode fica muito fino em uma resolução alta
        // e muito largo em uma resolução muito baixa.
        
        // Title bar
        // #todo: Essa janela foi registrada?
        if (useTitleBar == TRUE)
        {
            window->titlebar_height = 32;
            window->titlebar_color = TitleBarColor; //0x00AC81

            tbWindow = (void *) xxxCreateWindow ( 
                                    WT_SIMPLE, 0, 1, 1, "TitleBar", 
                                    BorderSize, 
                                    BorderSize, 
                                    (window->width - BorderSize - BorderSize), 
                                    window->titlebar_height, 
                                    (struct gws_window_d *) window, 
                                    0, window->titlebar_color, window->titlebar_color, 
                                    0 );   // rop_flags  

            if ( (void *) tbWindow == NULL ){
                gwssrv_debug_print ("wmCreateWindowFrame: tbWindow fail \n");
                return -1;
            }
            tbWindow->type = WT_SIMPLE;
            window->titlebar = tbWindow;  // Window pointer!
            // Register window
            id = RegisterWindow(tbWindow);
            if (id<0){
                gwssrv_debug_print ("wmCreateWindowFrame: Couldn't register window\n");
                return -1;
            }

            // #important:
            // The Titlebar in an overlapped window will affect
            // the top position of the client area rectangle.
            window->rcClient.top += window->titlebar_height;

            // Ornamento:
            // Ornamento na parte de baixo da title bar.
            // #important:
            // O ornamento é pintado dentro da barra, então isso
            // não afetará o positionamento da área de cliente.
            // border on bottom.
            // Usado para explicitar se a janela é ativa ou não
            // e como separador entre a barra de títulos e a segunda
            // área da janela de aplicativo.
            // Usado somente por overlapped window.
        
            window->frame.ornament_color1   = OrnamentColor1;  //COLOR_BLACK;
            window->titlebar_ornament_color = OrnamentColor1;  //COLOR_BLACK;
        
            rectBackbufferDrawRectangle ( 
                tbWindow->left, 
                ( (tbWindow->top) + (tbWindow->height) - METRICS_TITLEBAR_ORNAMENT_SIZE ),  
                tbWindow->width, 
                METRICS_TITLEBAR_ORNAMENT_SIZE, 
                OrnamentColor1, 
                TRUE,
                0 );  // rop_flags

            //
            // Icon (Titlebar)
            //

            // O posicionamento em relação
            // à janela é consistente por questão de estilo.
            // See: bmp.c
            // IN: index, left, top.

            window->titlebarHasIcon = FALSE;
            window->frame.icon_id = 1;
            if( useIcon == TRUE )
            {
                // Decode the bmp that is in a buffer
                // and display it directly into the framebuffer. 
                // IN: index, left, top
                gwssrv_display_system_icon( 
                    (int) window->frame.icon_id, 
                    (tbWindow->left + METRICS_ICON_LEFT), 
                    (tbWindow->top  + METRICS_ICON_TOP) 
                    );
                
                window->titlebarHasIcon = TRUE;
             }

            //
            // String (title bar)
            //
        
            window->titlebar_text_color = COLOR_WHITE;
        
            // #todo
            // Temos que gerenciar o posicionamento da string.
        
            // #bugbug: Use 'const char *'
            tbWindow->name = (char *) strdup ( (const char *) window->name );
        
            //#todo: validation
            //if ( (void*) tbWindow->name == NULL ){}
        
            if ( useTitleString == TRUE ){
                grDrawString ( 
                    (tbWindow->left) + offset, 
                    (tbWindow->top)  + 8, 
                    COLOR_WHITE, 
                    tbWindow->name );
            }

        }  //--use title bar.
        // ooooooooooooooooooooooooooooooooooooooooooooooo


        //
        // Status bar (bottom)
        // 
        
        // Se for maximized ou fullscreen
        // #todo: Essa janela foi registrada?
        if ( window->style & 0x0008 )
        {
            window->statusbar_height = 32;
            window->statusbar_color = 0x00AC81;

            sbWindow = (void *) xxxCreateWindow ( 
                                WT_SIMPLE, 0, 1, 1, "Statusbar", 
                                BorderSize,                                                //left
                                (window->height - window->statusbar_height - BorderSize),  //top
                                (window->width - BorderSize - BorderSize),                 //width 
                                window->statusbar_height,                                  //height
                                (struct gws_window_d *) window, 
                                0, window->statusbar_color, window->statusbar_color, 
                                0 );   // rop_flags  
            if ( (void *) sbWindow == NULL ){
                gwssrv_debug_print ("wmCreateWindowFrame: sbWindow fail \n");
                return -1;
            }
            sbWindow->type = WT_SIMPLE;
            window->statusbar = sbWindow;  // Window pointer.
            // Register window
            id = RegisterWindow(tbWindow);
            if (id<0){
                gwssrv_debug_print ("wmCreateWindowFrame: Couldn't register window\n");
                return -1;
            }
        }

        // ok
        return 0;
    }

// ===============================================
// button

    //button
    if ( Type == WT_BUTTON )
    {
        gwssrv_debug_print ("wmCreateWindowFrame: [TODO] frame for button\n");
        
        //todo frame or not
        //just like the edit box.   
        
        // ok     
        return 0;
    }

// ===============================================
// more ... ??


//done:
    // ok
    return 0;
}


void wm_flush_rectangle(struct gws_rect_d *rect)
{
    if( (void*) rect != NULL ){
        gwssrv_refresh_this_rect(rect);
    }
}


void wm_flush_window(struct gws_window_d *window)
{
    if( (void*) window == NULL )
        return;

    if(window->used != TRUE)
        return;

    if(window->magic != 1234)
        return;

// flush
    gws_show_window_rect(window);
}


void wm_flush_screen(void)
{
    gwssrv_show_backbuffer();
}


// Refresh screen via kernel.
// Copy the backbuffer in the frontbuffer(lfb).
// #??
// It uses the embedded window server in the kernel.
//#define	SYSTEMCALL_REFRESHSCREEN        11
// #todo
// trocar o nome dessa systemcall.
// refresh screen será associado à refresh all windows.

void gwssrv_show_backbuffer (void)
{
    gramado_system_call(11,0,0,0);
}



// Called by the main routine for now.
// Its gonne be called by the timer.
// See: comp.c
void wmCompose(void)
{
    // #todo: Working on this thing.
    //__update_fps();
    
    compose();
}


/*
 * wmRefreshDirtyRectangles: 
 */

// O compositor deve ser chamado para compor um frame 
// logo após uma intervenção do painter, que reaje às
// ações do usuário.
// Ele não deve ser chamado X vezes por segundo.
// Quem deve ser chamado X vezes por segundo é a rotina 
// de refresh, que vai efetuar refresh dos retângulos sujos e
// dependendo da ação do compositor, o refresh pode ser da tela toda.

void wmRefreshDirtyRectangles(void)
{
    register int i=0;
    struct gws_window_d *tmp;

// #debug
    //gwssrv_debug_print("wmRefreshDirtyRectangles:\n");

//==========================================================
// ++  Start

    //t_start = rtl_get_progress_time();

//
// == Update screen ====================
//

// Redrawing all the windows.
// redraw using zorder.
// refresh using zorder.
// Invalidating all the windows ... 
// and it will be flushed into the framebuffer for the ring0 routines.

//
// Update
//

/*
// #debug 
// Bar for debug.
// update message/hour/date ...
    if ( (void*) __root_window != NULL ) 
    {
        if ( __root_window->magic == 1234 )
        {
                //redraw_window(tmp,FALSE);  //bugbug
                grDrawString(
                    __root_window->left, 
                    __root_window->top,
                    COLOR_WHITE,
                    "Gramado OS");
               invalidate_window(__root_window);
               //__root_window->redraw = FALSE;
        }
    }
*/

// ======================================================
// Flush
// #todo #bugbug
// Flush all the dirty windows into the framebuffer.
// It will lookup the main window list.
// This is a very slow way of doing this.
// But it is just a test.


// #test

    //int UpdateScreenFlag=FALSE;
    int UpdateScreenFlag=TRUE;

    if(UpdateScreenFlag == TRUE)
    {
        // Lookup the main window list.

        for (i=0; i<WINDOW_COUNT_MAX; ++i)
        {
            tmp = (struct gws_window_d  *) windowList[i];

            if ( (void*) tmp != NULL )
            {
                // It is a valid window and
                // it is a dirty window.
                // Flush the window's rectangle.
                if ( tmp->used == TRUE && 
                     tmp->magic == 1234 )
                {
                    if ( tmp->dirty == TRUE )
                    {
                        wm_flush_window(tmp);
                        validate_window(tmp);
                    }
                }
            }
        };
    }
// =======================
}


void flush_frame(void)
{
    wm_flush_screen();
}


// Starting with the first window of the list,
// create a stack o windows in the top/left corner
// of the screen.

void __Tile(void)
{

// #todo:
// only overlapped windows?

    struct gws_window_d *w;

    int cnt=0;
    int c=0;
    int i=0;

    debug_print("__Tile:\n");

    w=(struct gws_window_d *)first_window;
    if((void*)w==NULL)
    { 
        debug_print("__Tile: w==NULL\n");
        return; 
    }


// Get the size of the list.

    while((void*)w != NULL)
    {
        w = (struct gws_window_d *) w->next;
        cnt++;
    };

// Starting with the forst window of the list,
// create a stack o windows in the top/left corner
// of the screen.

    w = (struct gws_window_d *) first_window;

    if ((void*)w==NULL)
    { 
        debug_print("__Tile: w==NULL\n");
        return; 
    }

    i=0;
    while ((void*)w != NULL)
    {
        if(i>=cnt)
            break;

        //#bugbug: limite provisorio
        if(i>4){
            cnt=4;
            break;
        }
        
        /*
         //original
        w->left = (i*10);
        w->top  = (i*10);
        w->width  = 240;
        w->height = 120;
        */
        
        // #test
        // Empilhando verticalmente.
        if( WindowManager.initialized == TRUE )
        {
            if(WindowManager.mode == 1)
            {
                //VERTICAL
                if( WindowManager.vertical==TRUE)
                {
                    w->width  = WindowManager.wa_width;
                    w->height = (WindowManager.wa_height / cnt); 
                    w->left   = 0;
                    w->top    = (w->height * i);
                }

                //NOT VERTICAL
                if( WindowManager.vertical!=TRUE)
                {
                    w->height = WindowManager.wa_height; 
                    w->width  = (WindowManager.wa_width/cnt);
                    w->left   = (w->width * i);
                    w->top    = 0;
                }
            }
        }
        
        w = (struct gws_window_d *) w->next;
        i++;
    };

    debug_print("__Tile: done\n");
}


void wm_update_window_by_id(int wid)
{
    struct gws_window_d *w;

// Redraw and show the root window.
    redraw_window(__root_window,TRUE);

    if(active_window<0)
        return;

    if(active_window>=WINDOW_COUNT_MAX)
        return;  

    w = (struct gws_window_d *) windowList[active_window];

    if((void*)w==NULL){ return; }

    if ( w->used != TRUE )
        return;

    if ( w->magic != 1234 )
        return;

    if (w->type != WT_OVERLAPPED)
        return;

    // #test
    // Empilhando verticalmente.
    if( WindowManager.initialized != TRUE )
        return;
    
    
    if(WindowManager.mode == 1)
    {
        //maximized
        w->left=0;
        w->top=0;
        w->width  = WindowManager.wa_width;
        w->height = WindowManager.wa_height; 
    }
    
    redraw_window(w,FALSE);
    invalidate_window(w);
    set_focus(w);

    wm_Update_TaskBar("Active");
}


void wm_update_active_window(void)
{
    wm_update_window_by_id(active_window);
}


// #danger: We are testing this funcion.
// Repinta todas as janelas seguindo a ordem da lista
// que está em last_window.
// No teste isso é chamado pelo kernel através do handler.
// Mas também será usado por rotinas internas.
void wm_update_desktop(void)
{
    struct gws_window_d *w;

// #test
// Starting with the first window of the list,
// create a stack o windows in the top/left corner
// of the screen.

   __Tile();

// Redraw and show the root window.
    redraw_window(__root_window,TRUE);


// Redraw the whole stack of windows,
// but do not show yet them.
// Only for app windows. (overlapped).
// Set focus on the last window of the stack.

    w = (struct gws_window_d *) first_window;

    if((void*)w==NULL){ return; }

    while(1){

        if((void*)w==NULL){ break; }

        if ( (void*) w != NULL )
        {
            //only overlapped windows.
            if (w->type == WT_OVERLAPPED)
            {
                redraw_window(w,FALSE);
                invalidate_window(w);
                set_focus(w);
                set_active_window(w->id);
            }
        }

        w = (struct gws_window_d *) w->next; 
    }; 

// Update the taskbar at the bottom of the screen.
    wm_Update_TaskBar("Desktop");
}


// Set focus on a window.
// Change the foreground thread.
// The new foreground thread will be 
// the thread associated with this given window.
void set_focus(struct gws_window_d *window)
{
    if( (void*) window == NULL ){
        return;
    }

    if ( window->used != TRUE ){
        return;
    }

    if ( window->magic != 1234 ){
        return;
    }

// global variable

    window_with_focus = window->id;

// Pede para o kernel mudar a foreground thread.
    sc82 (
        10011,
        window->client_tid,
        window->client_tid,
        window->client_tid);
}




void set_status_by_id( int wid, int status )
{
    struct gws_window_d *w;

    if(wid<0)
        return;

    if(wid>=WINDOW_COUNT_MAX)
        return;  

    w = (struct gws_window_d *) windowList[wid];

    if((void*)w==NULL){ return; }
    
    if ( w->used != TRUE )
        return;

    if ( w->magic != 1234 )
        return;

// Set 
    w->status = status;
}


void set_focus_by_id( int wid )
{
    struct gws_window_d *w;

    if(wid<0)
        return;

    if(wid>=WINDOW_COUNT_MAX)
        return;  

    w = (struct gws_window_d *) windowList[wid];

    if((void*)w==NULL){ return; }
    
    if ( w->used != TRUE )
        return;

    if ( w->magic != 1234 )
        return;

//ok
    set_focus(w);
}


void set_first_window( struct gws_window_d *window)
{
    first_window = (struct gws_window_d *) window;
}


struct gws_window_d *get_first_window(void)
{
    return (struct gws_window_d *) first_window;
}


void set_last_window( struct gws_window_d *window )
{
    if( (void*) window == NULL )
         return;

    wm_add_window_into_the_list(window);
}


struct gws_window_d *get_last_window(void)
{
    return (struct gws_window_d *) last_window;
}


void activate_first_window(void)
{
    if( (void*) first_window == NULL )
        return;

    if ( first_window->used != TRUE )
        return;

    if ( first_window->magic != 1234 )
        return;

    if ( first_window->type != WT_OVERLAPPED )
        return;

    set_active_window(first_window->id);
}


void activate_last_window(void)
{
    if( (void*) last_window == NULL )
        return;

    if ( last_window->used != TRUE )
        return;

    if ( last_window->magic != 1234 )
        return;

    if ( last_window->type != WT_OVERLAPPED )
        return;

// Activate
    set_active_window(last_window->id);
}



// not tested yet
void wm_add_window_into_the_list( struct gws_window_d *window)
{
    struct gws_window_d  *Next;


// ========================

    //if( window == __root_window )
        //return;

// ========================

    if( (void*) window == NULL )
        return;

    if ( window->used != TRUE )
        return;

    if ( window->magic != 1234 )
        return;

    if ( window->type != WT_OVERLAPPED )
        return;


// =====================================
// Se não existe uma 'primeira da fila'.
// Então somos a primeira e a última.
    if( (void*) first_window == NULL )
    {
        first_window = window;
        last_window  = window;
        goto done;
    }

// Invalid first window.
    if ( first_window->magic != 1234 )
    {
        first_window = window;
        last_window  = window;
        goto done;
    }


// ===================================
// Se exite uma 'primeira da fila'.
    Next = first_window;

    while( (void*) Next->next != NULL )
    {
        Next = Next->next;
    };

// Agora somos a última da fila.
    Next->next  = (struct gws_window_d *) window;

done:
    last_window  = (struct gws_window_d *) window;
    window->next = NULL;
    set_active_window(window->id);
}



// not tested yet
void wm_remove_window_from_list_and_kill( struct gws_window_d *window)
{
    struct gws_window_d *w;
    struct gws_window_d *pick_this_one;

    if( (void*) window == NULL )
        return;

    w = (struct gws_window_d *) first_window;

    if( (void*) w == NULL )
        return;

    while(1)
    {
        if( (void*) w == NULL )
            break;

        if(w==window)
        {
            // remove
            pick_this_one = (struct gws_window_d *) w;
            
            // Glue the list.
            w = w->next;
            
            // Kill
            pick_this_one->used = FALSE;
            pick_this_one->magic = 0;
            pick_this_one = NULL;
            break;
        }
        w = w->next;
    };
}


// ====================

// Local worker
// The color is red.
// We need to create a parameter for that.
void 
__draw_char_into_the_window(
    struct gws_window_d *window, 
    int ch )
{
// draw char support.
    unsigned char _string[4];

// Invalid window
    if( (void*)window == NULL)
        return;
    if(window->magic!=1234)
        return;


// Invalid char
    if(ch<0)
        return;


// Invalid char
// UP, LEFT, RIGHT, DOWN
// #todo
// Update input pointer for this window.
    if( ch==0x48 || 
        ch==0x4B || 
        ch==0x4D || 
        ch==0x50 )
    {
        // #todo: 
        // Update input pointers for this window.
        
        // right
        if(ch==0x4D)
            window->ip_x++;

        // down
        if(ch==0x50)
            window->ip_y++;

        return;
    }

// #todo: 
// Isso tem que voltar apagando.
    if(ch==VK_BACK)
    {
        window->ip_x--;
        if(window->ip_x < 0)
            window->ip_x = 0;
        
        return;
    }

    if(ch==VK_TAB)
    {
        window->ip_x += 8;
        //#todo limits
        if(window->ip_x >= window->width_in_bytes)
        {
            window->ip_x = 0;
            if(window->type == WT_EDITBOX_MULTIPLE_LINES)
            {
                window->ip_y++;
                //#todo
                //if(window->ip_y >= window->height_in_bytes)
            }
        }
        return;
    }

   _string[0] = (unsigned char) ch;
   _string[1] = 0;


// types

    if( window->type == WT_OVERLAPPED )
        return;

    if( window->type == WT_SCROLLBAR )
        return;

    if( window->type == WT_STATUSBAR )
        return;

    if( window->type == WT_CHECKBOX )
        return;

    if( window->type == WT_BUTTON )
        return;

// #todo
// Isso pode receber char se tiver em modo de edição.
    if( window->type == WT_ICON )
        return;

// Editbox
    if( window->type == WT_EDITBOX ||
        window->type == WT_EDITBOX_MULTIPLE_LINES )
    {
        // draw char
        dtextDrawText ( 
            (struct gws_window_d *) window,
            (window->ip_x*8), (window->ip_y*8), 
            COLOR_RED, (unsigned char *) &_string[0] );

        // refresh rectangle
        // x,y,w,h
        gws_refresh_rectangle ( 
            (window->left + (window->ip_x*8)), 
            (window->top  + (window->ip_y*8)), 
            8, 
            8 );

        window->ip_x++;
        if(window->ip_x >= window->width_in_bytes)
        {
            if (window->type == WT_EDITBOX_MULTIPLE_LINES)
                window->ip_y++; 
            
            window->ip_x=0;
        }   
    }
}


/*
//local worker
void __switch_window(void)
{
}
*/


// #todo: explain it better.
void __switch_focus(void)
{
// Switch focus support.
    struct gws_window_d *next;

    if( window_with_focus < 0 )
    {
        window_with_focus=0;
        goto do_select;
    }

// Walk
    window_with_focus++;

    if ( window_with_focus >= WINDOW_COUNT_MAX )
    { 
        window_with_focus=0; 
    }

do_select: 

    next = (struct gws_window_d *) windowList[window_with_focus];

// NULL
    if( (void*) next == NULL )
    {
        // Volta para a primeira.
        window_with_focus = 0;
        // Seleciona a janela root.
        next = __root_window;
    }

    if( (void*) next == NULL )
        return;

    if( next->used != TRUE )
        return;

    if( next->magic != 1234 )
        return;

// + Pede para o kernel mudar a foreground thread.
// + Seleciona o próximo input reponder.

    if (next->client_tid < 0)
        return;

    sc82 (
        10011,
        next->client_tid,
        next->client_tid,
        next->client_tid);

// ==============================================
// prepara o nome

    int name_len = strlen(next->name);
    if(name_len > 32){ name_len = 32; }
    char w_name[64];
    sprintf(w_name,"| ");
    strncat(w_name,next->name,name_len);
    w_name[63]=0;

    // notifica na barra de tarefas
    wm_Update_TaskBar((char *) w_name);

//=================================================

do_redraw: 

// redraw

    redraw_window(next,TRUE);

    // Activate app window
    if( next->type == WT_OVERLAPPED )
    {
        active_window = next->id;
        return;
    }

    // focus
    next->focus = TRUE;
}


// local worker
// Colocaremos uma mensagem na fila de mensagens 
// da janela indicada via argumento.
// :::: Coloca em tail.
int
__add_message_to_into_the_queue(
    struct gws_window_d *window,
    int msg,
    unsigned long long1,
    unsigned long long2 )
{

// Send message to the thread associated with this window.

    unsigned long message_buffer[8];

    //debug_print("__add_message_to_into_the_queue:\n");

// Invalid window

    if( (void*) window == NULL ){
        debug_print("__add_message_to_into_the_queue: window\n");
        return -1;
    }

    if( window->used != TRUE ){
        debug_print("__add_message_to_into_the_queue: used\n");
        return -1;
    }

    if( window->magic != 1234 ){
        debug_print("__add_message_to_into_the_queue: magic\n");
        return -1;
    }


//
// event
//

// standard
    message_buffer[0] = (unsigned long) (window->id & 0xFFFF);  // wid
    message_buffer[1] = (unsigned long) (msg        & 0xFFFF);  // message code
    message_buffer[2] = (unsigned long) long1;
    message_buffer[3] = (unsigned long) long2;

// extra
    message_buffer[4] = 0;
    message_buffer[5] = 0;


// Invalid client tid.
    if( window->client_tid < 0 ){
        return -1;
    }

//
// Send
//

// IN:
// tid, message buffer address

    rtl_post_system_message( 
        (int) window->client_tid, 
        (unsigned long) &message_buffer[0] );


// Pede para o kernel mudar a foreground thread.
// Seleciona o próximo 'input reponder'.
// #danger: 
// Lembrando que vamos fazer um interrupção estando dentro 
// do handler da interrupção de teclado.

    sc82 (
        10011,
        window->client_tid,
        window->client_tid,
        window->client_tid );

//done:
    //debug_print("__add_message_to_into_the_queue: done\n");
    return 0;
}


unsigned long 
mainmenuDialog(
    struct gws_window_d *window,
    int msg,
    unsigned long long1,
    unsigned long long2 )
{
    int Status=FALSE;
    
    // ok, it is working
    Status = __is_inside_menu(MainMenu,long1,long2);
    if(Status==FALSE)
        yellow_status("Out ");
    if(Status==TRUE)
        yellow_status("In ");
    return 0;
}


//local
void __probe_tb_botton_hover(long long1, long long2)
{

    int Status=0;
    
    int i=0;
    struct gws_window_d *tmp_window_button;
    
    for(i=0; i<4; i++){
    
        // pega um ponteiro da lista
        tmp_window_button = (struct gws_window_d *) tb_windows[i];
        
        // checa a validade do ponteiro.
        if ( (void*) tmp_window_button != NULL )
        {
            //paranoia
            if(tmp_window_button->magic == 1234)
            {
                Status = is_within(
                    (struct gws_window_d *) tmp_window_button,
                    long1, long2 );
                    
                if(Status==FALSE){ mousehover_window=0; }

                if(Status==TRUE)
                {
                    yellow_status("oops");
                    //rtl_reboot();
                    // Register the hover window.
                    mousehover_window = 
                        (int) tmp_window_button->id;
                    
                    //ok.
                    return;
                }
            }
        }
    };
}


// Talvez precisaremos de mais parametros.
unsigned long 
wmProcedure(
    struct gws_window_d *window,
    int msg,
    unsigned long long1,
    unsigned long long2 )
{
    int Status=FALSE;
    unsigned long r=0;

// #debug
    //printf("wmProcedure: w=? m=%d l1=%d l2=%d\n", 
        //msg, long1, long2 );

// See:
// globals.h

    switch(msg){

    case GWS_Create:
        printf("wmProcedure: [1] GWS_Create\n");
        break;

    case GWS_Destroy:
        printf("wmProcedure: [2] GWS_Destroy\n");
        break;

    case GWS_Move:
        printf("wmProcedure: [3] GWS_Move\n");
        break;

    case GWS_Size: //get size?
        printf("wmProcedure: [4] GWS_Size\n");
        break;

    case GWS_Resize: //set size ?
        printf("wmProcedure: [5] GWS_Resize\n");
        break;

    // ...

    case GWS_Close:
        printf("wmProcedure: [7] GWS_Close\n");
        if (long1==0){
            printf("Closing root window.\n");
            //exit(0);
        }
        break;

    case GWS_Paint:
        printf("wmProcedure: [8] GWS_Paint\n");
        break;

    case GWS_SetFocus: // set focus
        printf("wmProcedure: [9] GWS_SetFocus\n");
        break;

    case GWS_KillFocus: //kill focus
        printf("wmProcedure: [10] GWS_KillFocus\n");
        break;

    case GWS_Activate:
        printf("wmProcedure: [11] GWS_Activate\n");
        break;

    case GWS_ShowWindow:
        printf("wmProcedure: [12] GWS_ShowWindow\n");
        break;

    case GWS_SetCursor:
        printf("wmProcedure: [13] GWS_SetCursor\n");
        break;

    case GWS_Hide:
        printf("wmProcedure: [14] GWS_Hide\n");
        break;

    case GWS_Maximize:
        printf("wmProcedure: [15] GWS_Maximize\n");
        break;

    case GWS_Restore:
        printf("wmProcedure: [16] GWS_Restore\n");
        break;

    case GWS_ShowDefault:
        printf("wmProcedure: [17] GWS_ShowDefault\n");
        break;

    case GWS_SetFocus2:
        printf("wmProcedure: [18] GWS_SetFocus2\n");
        break;

    case GWS_GetFocus2:
        printf("wmProcedure: [19] GWS_GetFocus2\n");
        break;


// #todo
// Esse eh o momento de exibirmos o cursor do mouse,
// e nao no kernel como estamos fazendo.
// Precisamos fazer refresh para apagar o cursor antigo
// depois pintarmos o cursor novo direto no lfb.
// Mas nao temos aqui a rotina de pintarmos direto no
// lfb.
// #todo: Trazer as rotinas de exibiçao de cursor
// para ca, depois deixar de usar
// as rotinas de pintura de cursor que estao no kernel.

    case GWS_MouseMove:

        // The compositor is doing this job at the
        // and of it's routine. See: comp.c
        comp_set_mouse_position(long1,long2);
        
        //------
        //#dangerdanger
        //#todo: show the backbuffer
        //gws_refresh_rectangle(old_x,old_y,8,8);
        //old_x = long1;
        //old_y = long2;
        //#todo: print directly into the lfb.
        //frontbuffer_draw_rectangle( 
        //    long1, long2, 8, 8, COLOR_YELLOW, 0 );
        //------        

        // Em qual botão o mouse esta passando por cima.
        __probe_tb_botton_hover(long1,long2);

        //========
        //o ponteiro do mouse esta dentro do main menu?
        if( (void*)MainMenu!=NULL)
        {
            if(MainMenu->in_use == TRUE)
            {
                // Call the main menu dialog
                r = (unsigned long) mainmenuDialog(
                    (struct gws_window_d *) 0,
                    (int) msg,
                    (unsigned long) long1,
                    (unsigned long) long2 ); 
                return r;
            }
        }
        //========
        
        return 0;
        break;

    case GWS_MousePressed:
        if(long1==0){ yellow_status("P0"); }
        if(long1==1){ yellow_status("P1"); }
        if(long1==2){ yellow_status("P2"); }

        // #test
        // em qual botão.
        if(mousehover_window == tb_buttons[0] ||
           mousehover_window == tb_buttons[1] ||
           mousehover_window == tb_buttons[2] ||
           mousehover_window == tb_buttons[3] )
        { 
            set_status_by_id(mousehover_window,BS_PRESSED);
            redraw_window_by_id(mousehover_window,TRUE);
            return 0;
        }
        
        return 0;
        break;

    case GWS_MouseReleased:
        if(long1==0){ yellow_status("R0"); }
        //if(long1==1){ yellow_status("R1"); wm_update_desktop(); return 0; }
        if(long1==1){ 
            yellow_status("R1"); 
            create_main_menu(8,8);
            return 0; 
        }
        if(long1==2){ yellow_status("R2"); return 0; }
        //if(long1==1){ create_main_menu(mousex,mousey); return 0; }
        //if(long1==1){ create_main_menu(mousex,mousey); return 0; }

        //tb_button[0]
        if(mousehover_window == tb_buttons[0])
        {
            set_status_by_id(mousehover_window,BS_RELEASED);
            redraw_window_by_id(mousehover_window,TRUE);
            create_main_menu(8,8);
            //wm_update_active_window();
        }

        //tb_button[1]
        if(mousehover_window == tb_buttons[1])
        {
            set_status_by_id(mousehover_window,BS_RELEASED);
            redraw_window_by_id(mousehover_window,TRUE);
            //#fail rtl_clone_and_execute("editor.bin");
            yellow_status("1");
        }

        //tb_button[2]
        if(mousehover_window == tb_buttons[2])
        {
            set_status_by_id(mousehover_window,BS_RELEASED);
            redraw_window_by_id(mousehover_window,TRUE);
            //#fail rtl_clone_and_execute("fileman.bin");
            yellow_status("2");
        }

        //tb_button[3]
        if(mousehover_window == tb_buttons[3])
        {
            set_status_by_id(mousehover_window,BS_RELEASED);
            redraw_window_by_id(mousehover_window,TRUE);
            //#fail rtl_clone_and_execute("terminal.bin");
            yellow_status("3");
        }

        return 0;
        break;


    // #bugbug
    // Quando imprimir na tela e quando enviar para o cliente?
    // Uma flag deve indicar se o sistema deve ou nao imprimir 
    // previamente o char.
    // Caixas de edição podem deixar todo o trabalho
    // de teclas de digitação para o sistema, liberando o aplicativo
    // desse tipo de tarefa. Mas editores de texto querem 
    // processar cada tecla digitada.

    case GWS_KeyDown:
        // Imprime o char na janela indicada.
        // Essa é a janela com foco de entrada.
        //if( pre_print === TRUE)
        __draw_char_into_the_window(window,(int)long1);
        // Enfileirar a mensagem na fila de mensagens
        // da janela com foco de entrada.
        // O cliente vai querer ler isso.
        __add_message_to_into_the_queue(
            (struct gws_window_d *)window,
            (int)msg,
            (unsigned long)long1,
            (unsigned long)long2);
        return 0;
        break;

    case GWS_SysKeyDown:
        //printf("wmProcedure: [?] GWS_SysKeyDown\n");
        // Enfileirar a mensagem na fila de mensagens
        // da janela com foco de entrada.
        // O cliente vai querer ler isso.
        __add_message_to_into_the_queue(
            (struct gws_window_d *) window,
            (int)msg,
            (unsigned long)long1,
            (unsigned long)long2);
        //wm_update_desktop(); // 
        return 0;
        break;

    //
    case GWS_SwitchFocus:
        //printf("Switch "); fflush(stdout);
        __switch_focus();
        //printf("wmProcedure: [?] GWS_SwitchFocus\n");
        //next = window->next;
        //window->focus = TRUE;
        //redraw_window(window,1);
        return 0;
        break;
    
    default:
        return 0;
        break;
    };

    return 0;
}


// Entry point
// Called by the kernel.
// Order: rdi, rsi, rdx, rcx, r8, r9.
unsigned long 
wmHandler(
    unsigned long arg1_rdi,
    unsigned long arg2_rsi,
    unsigned long arg3_rdx,
    unsigned long arg4_rcx )
{

    unsigned long r=0;

// Final message
    struct gws_window_d *w;
    int msg=0;
    unsigned long long1=0;
    unsigned long long2=0;

// #debug
//    debug_print ("wmHandler:-----------------------------\n");

// #debug
    //printf("wmHandler: %x %x %x %x\n", 
        //arg1_rdi, arg2_rsi, arg3_rdx, arg4_rcx );

// wid
// Ignoring this parameter
// We ware called by the kernel, and the kernel has no information
// about the windows. So, the messages sent by the kernel are able
// to act on the active window, on the window with focus and
// on the server itself.

    //int wid=-1;
    //wid = (int) (arg1_rdi & 0xFFFF);

// Message
    msg = (int) (arg2_rsi & 0xFFFF);


// #special
// Refresh rectangles and exit.
// GWS_RefreshDirtyRectangles

// #todo:
// This way the kernel is able to call the
// compositor at a given timer.
// Called by the kernel in pit.c

    if ( msg == 9091 )
    {
        // debug_print ("wmHandler: 9091\n");
        wmCompose();
        return 0;
    }


// #test
// Redraw all the windows. Back to front.
// GWS_UpdateDesktop

    if ( msg == 9092 )
    {
        //debug_print ("wmHandler: 9092\n");
        wm_update_desktop();
        return 0;  //important: We need to return.
    }


// ==============================================
// #test
// Testing some random functions.

    if (msg == 9093)
    {
        debug_print ("wmHandler: 9093\n");

        // #test
        // drawing a rect using kernel and ws routines.
        // TRUE = use kgws ; FALSE =  do not use kgws.

        rectBackbufferDrawRectangle0(
            10, 10, 40, 40,
            COLOR_RED,
            TRUE,      // fill? 
            0,         // rop falgs
            FALSE );   // TRUE = use kgws. (kernel service)
        //refresh_rectangle_via_kgws(10, 10, 40, 40);

        return 0;
    }
// ==============================================

// Data

    long1 = (unsigned long) arg3_rdx;
    long2 = (unsigned long) arg4_rcx;

//
// Calling wmProcedure()
//

    switch (msg){

    case GWS_MouseMove:
    case GWS_MousePressed:
    case GWS_MouseReleased:
        //#debug
        //if( msg == GWS_MousePressed ){ printf("%d\n",long1); }
        r = (unsigned long) wmProcedure(
                (struct gws_window_d *) 0,
                (int) msg,
                (unsigned long) long1,
                (unsigned long) long2 ); 
        return r;
        break;

// #important:
// Mandaremos input de teclado somente para 
// a janela com foco de entrada,
// seja ela de qualquer tipo.

    case GWS_KeyDown:
    case GWS_SysKeyDown:
    case GWS_SwitchFocus:
        goto do_process_message;
        break;

    //case 9091:
        //wmCompositor();
        //return 0;  //important
        //break;

    // Mensagens que atuam sobre o window server.
    default:
        printf("wmHandler: default message\n");
        return 0;
        break;
    };

do_process_message:

// wid
// window with focus

    if( window_with_focus < 0 || 
            window_with_focus >= WINDOW_COUNT_MAX )
    { 
        return 0; 
    }

// window structure

    w = (struct gws_window_d *) windowList[window_with_focus];

    if ( (void *) w == NULL ){
        printf ("wmHandler: GWS_KeyDown w\n");
        return 0;
    }

    if ( w->used != TRUE || w->magic != 1234 )
    {
        printf ("wmHandler: w validation\n");
        return 0;
    }

// Procedure
// IN: A valid window with focus.

    r = (unsigned long) wmProcedure(
                            (struct gws_window_d *) w,
                            (int) msg,
                            (unsigned long) long1,
                            (unsigned long) long2 ); 

done:

// #debug
    // debug_print ("wmHandler: done\n");
    
    return (unsigned long) r;
}


// yellow bar. (rectangle not window)
// developer status.
void yellow_status( char *string )
{
    //methods. get with the w.s., not with the system.
    unsigned long w = gws_get_device_width();
    unsigned long h = gws_get_device_height();

    unsigned long offset_string1 = 8;  //( 8*1 );
    unsigned long offset_string2 = ( 8*5 );
    unsigned long bar_size = w;

    debug_print ("yellow_status:\n");
    
    //#todo
    //if ( (void*) string == NULL ){ return; }
    //if ( *string == 0 ){ return; }


    // Desenha a barra no backbuffer

    if ( current_mode == GRAMADO_JAIL ){
        //bar_size = w;
        bar_size = (w>>1);
        rectBackbufferDrawRectangle ( 
            0, 0, bar_size, 24, COLOR_YELLOW, 1,0 );
    }else{

        //bar_size = (offset_string2 + (4*8) );
        bar_size = (offset_string2 + (100) );
        rectBackbufferDrawRectangle ( 
            0, 0, bar_size, 24, COLOR_YELLOW, 1,0 );
    };

// Escreve as strings
    grDrawString ( offset_string1, 8, COLOR_BLACK, string );
    grDrawString ( offset_string2, 8, COLOR_BLACK, "FPS" );
    
    // Mostra o retângulo.
     
    if (bar_size == 0)
        bar_size = 32;
 
    gws_refresh_rectangle(0,0,bar_size,24);

    debug_print ("yellow_status: done\n");
}



int 
is_within2 ( 
    struct gws_window_d *window, 
    unsigned long x, 
    unsigned long y )
{
    struct gws_window_d *pw;
    struct gws_window_d *w;

// #bugbug
// E se a janela tem janela mae?

// window validation

    if( (void*) window == NULL )
        return FALSE;

    if ( window->used != TRUE && 
         window->magic != 1234 )
    {
        return FALSE;
    }


// ====

// parent
// The parent window.
    pw = window->parent;

    if( (void*) pw == NULL )
        return FALSE;

    if ( pw->used != TRUE && 
         pw->magic != 1234 )
    {
        return FALSE;
    }

// window
// the window itself
    w = window;

    if( (void*) w == NULL )
        return FALSE;

    if ( w->used != TRUE && 
         w->magic != 1234 )
    {
        return FALSE;
    }

//relative to the parent.
    int x1= pw->left + w->left; 
    int x2= x1 + w->width;
    int y1= pw->top  + w->top;
    int y2= y1 + w->height;

    if( x > x1 && 
        x < x2 &&
        y > y1 && 
        y < y2 )
    {
        return TRUE;
    }

    return FALSE;
}


//#todo: Explain it.
int 
is_within ( 
    struct gws_window_d *window, 
    unsigned long x, 
    unsigned long y )
{

// #bugbug
// E se a janela tem janela mae?

    if ( (void*) window != NULL )
    {
        if ( window->used == TRUE && window->magic == 1234 )
        {
            // yes!
            if ( x >= window->left   && 
                 x <= window->right  &&
                 y >= window->top    &&
                 y <= window->bottom )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}


// validate
void validate_window (struct gws_window_d *window)
{
    if ( (void*) window != NULL )
    {
        if ( window->used == TRUE && window->magic == 1234 )
        {
            window->dirty = FALSE;
        }
    }
}


void invalidate_window_by_id( int wid )
{
    struct gws_window_d *w;

    if(wid<0)
        return;
    if(wid>=WINDOW_COUNT_MAX)
        return;
    w=(struct gws_window_d *)windowList[wid];
    if((void*)w==NULL)
        return;
    if(w->used!=TRUE)
        return;
    if(w->magic!=1234)
        return;

    invalidate_window(w);    
}


// Invalidate
void invalidate_window (struct gws_window_d *window)
{
    if ( (void*) window != NULL )
    {
        if ( window->used == TRUE && window->magic == 1234 )
        {
            window->dirty = TRUE;
        }
    }
}


void invalidate_root_window(void)
{
    invalidate_window ( (struct gws_window_d *) __root_window );
}


void invalidate_taskbar_window(void)
{
    invalidate_window ( (struct gws_window_d *) __taskbar_window );
}


void __begin_paint(struct gws_window_d *window)
{
    if( (void*) window == NULL )
        return;

    validate_window(window);
}


void __end_paint(struct gws_window_d *window)
{
    if( (void*) window == NULL )
        return;

    invalidate_window(window);
}

/*
void destroy_window (struct gws_window_d *window);
void destroy_window (struct gws_window_d *window)
{
    // #todo
    // if( window == __root_window)
        // return;
  
    if ( (void*) window != NULL )
    {
        if ( window->used == TRUE && window->magic == 1234 )
        {
            // ...
        }
    }
}
*/



int serviceChangeWindowPosition(void)
{
	//o buffer é uma global nesse documento.
    unsigned long *message_address = (unsigned long *) &__buffer[0];


    struct gws_window_d *window;
    int window_id = -1;
    
    unsigned long x = 0;
    unsigned long y = 0;


    // #debug
    gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition\n");


    // Get
    
    window_id = message_address[0];  //wid
    // msg
    x         = message_address[2];  
    y         = message_address[3];  


    //
    // Window ID
    //
   
    // Limits
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window_id\n");
        return -1;
    }

    //#todo
    // Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
   
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window\n");
        return -1;
    }

    if ( window->used != TRUE || window->magic != 1234 ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition validation\n");
        return -1;
    }

    gwssrv_change_window_position ( 
        (struct gws_window_d *) window, 
        (unsigned long) x, 
        (unsigned long) y );

    return 0;
}



int serviceResizeWindow(void)
{

	//o buffer é uma global nesse documento.
    unsigned long *message_address = (unsigned long *) &__buffer[0];


    struct gws_window_d *window;
    int window_id = -1;
    
    unsigned long w = 0;
    unsigned long h = 0;


    // #debug
    gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition\n");

    // #todo
    // Check all the header.


    // Get
    
    window_id = message_address[0];  //wid
    // msg
    w         = message_address[2];  
    h         = message_address[3];  


    //
    // Window ID
    //
   
    // Limits
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window_id\n");
        return -1;
    }

    //#todo
    // Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
   
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window\n");
        return -1;
    }
    
    if ( window->used != 1 || window->magic != 1234 ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition validation\n");
        return -1;
    }

//do!

    gws_resize_window ( 
        (struct gws_window_d *) window, 
        (unsigned long) w, 
        (unsigned long) h );

    return 0;
}



// #bugbug
// Usaremos a função create window para desenhar botões.
// #deletar !!!

int serviceDrawButton(void)
{
    // Deprecated !!
    gwssrv_debug_print("serviceDrawButton: deprecated\n");
    printf            ("serviceDrawButton: deprecated\n");
    exit(1);
    return -1;
}


// Redraw window.
// It will invalidate the window, and it will need to be flushed
// into the frame buffer.

int serviceRedrawWindow (void)
{
    //O buffer é uma global nesse documento.
    unsigned long *message_address = (unsigned long *) &__buffer[0];

    struct gws_window_d *window;

    // parameters
    int window_id = -1;
    int msg_code  = 0;
    unsigned long flags = 0;


// #debug
    gwssrv_debug_print ("serviceRedrawWindow:\n");


// Get wid and flag.
    window_id  = message_address[0];  // window id 
    msg_code   = message_address[1];  // message code
    flags      = message_address[2];  // flags
    //??       = message_address[3];  // nothing 

    // #todo
    //  Not tested yet.
    //if ( msg_code <= 0 ){
    //    gwssrv_debug_print ("serviceRedrawWindow:\n");
    //    goto fail;
    //}


// Window ID

    window_id = (int) (window_id & 0xFFFF);
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        gwssrv_debug_print ("serviceRefreshWindow: [FAIL] window_id\n");
        goto fail;
    }

// Get the window structure given the id.

    window = (struct gws_window_d *) windowList[window_id];

    if ( (void *) window == NULL ){
        gwssrv_debug_print ("serviceRefreshWindow: [FAIL] window\n");
        goto fail;
    }

    if ( window->used != TRUE || window->magic != 1234 )
    {
        gwssrv_debug_print ("serviceRefreshWindow: [FAIL] window validation\n");
        goto fail;
    }

// Validate
    window->dirty = FALSE;

// Redraw

    redraw_window (
        (struct gws_window_d *) window, 
        (unsigned long) flags );

// Invalidate
    window->dirty = TRUE;

//done:
    return 0;

fail:
    return (int) (-1);
}


// 2021
// Flush a given area into the framebuffer.

int serviceRefreshRectangle (void)
{
	//o buffer é uma global nesse documento.
    unsigned long *message_address = (unsigned long *) &__buffer[0];

    unsigned long left=0;
    unsigned long top=0;
    unsigned long width=0;
    unsigned long height=0;

// #todo
// Check all the header.

    unsigned long msg_code = message_address[1];
    if( msg_code != GWS_RefreshRectangle )
        return -1;

// #todo
// Check if the message code is right.

    left   = message_address[4];
    top    = message_address[5];
    width  = message_address[6];
    height = message_address[7];

    left   = (left   & 0xFFFF);
    top    = (top    & 0xFFFF);
    width  = (width  & 0xFFFF);
    height = (height & 0xFFFF);

// #todo
// Maybe we can test some limits here.

// Flush it into the framebuffer.
// See: rect.c
    gws_refresh_rectangle ( left, top, width, height );
    return 0;
}


// 1006
// Flush a given window into the backbuffer.
// #todo: Here we can simple mark this window as 'dirty'
// and let the compositor do its job.

int serviceRefreshWindow (void){

    unsigned long *message_address = (unsigned long *) &__buffer[0];

    struct gws_window_d *window;
    int window_id = -1;
    
    //unsigned long x;
    //unsigned long y;
    //unsigned long color;
    //int __char;
    //char *text_buffer;    // #todo


    // #debug
    gwssrv_debug_print ("serviceRefreshWindow:\n");


    // #todo
    // Check all the header.

    //
    // == Window ID ============================
    //

    // Get it
    window_id = (int) message_address[0];   
   
    // #extra
    // Special case.
    // Will be used in the ghost frame routines.
    
    if ( window_id == (-4) )
    {
        gwssrv_debug_print("serviceRefreshWindow:\n");  //debug
        gwssrv_debug_print("== R (extra) ==\n");  //debug
        refresh_device_screen();
        return 0;
    }

 
    // Limits
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        //printf("%d\n",window_id);
        gwssrv_debug_print ("serviceRefreshWindow: [FAIL] window_id\n");
        return -1;
    }

    //#todo
    // Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
   
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("serviceRefreshWindow: [FAIL] window\n");
        return -1;
    }
    
    if ( window->used != 1 || window->magic != 1234 ){
        gwssrv_debug_print ("serviceRefreshWindow: [FAIL] window validation\n");
        return -1;
    }



// #todo
// simply invalidate the window and let the compositor do its job.

    //invalidate_window(window);

//
// Flush
//

    gws_show_window_rect(window);
    
    return 0;
}


//
//===================================================================
//

// Let's redraw the window.
// Called by serviceRedrawWindow().

// #todo
// devemos repintar as janelas filhas, caso existam.

// IN: 
// window pointer, show or not.

int 
redraw_window ( 
    struct gws_window_d *window, 
    unsigned long flags )
{
    unsigned int __tmp_color=0;

    //gwssrv_debug_print ("redraw_window:\n");

    if ( (void *) window == NULL ){ return -1; }

    if (window->used!=TRUE || window->magic!=1234)
        return -1;

// Shadow
// A sombra pertence à janela e ao frame.
// A sombra é maior que a própria janela.
// ?? Se estivermos em full screen não tem sombra ??

    //gwssrv_debug_print ("redraw_window: Shadow\n");
    if ( window->shadowUsed == 1 )
    {

		//CurrentColorScheme->elements[??]
		
		//@todo: 
		// ?? Se tiver barra de rolagem a largura da 
		// sombra deve ser maior. ?? Não ...
		//if()
		
        // @todo: Adicionar a largura das bordas verticais 
		// e barra de rolagem se tiver.
		// @todo: Adicionar as larguras das 
		// bordas horizontais e da barra de títulos.
		// Cinza escuro.  CurrentColorScheme->elements[??] 
		// @TODO: criar elemento sombra no esquema. 

        if ( (unsigned long) window->type == WT_OVERLAPPED )
        {
            if (window->focus == 1){ __tmp_color = xCOLOR_GRAY1; }
            if (window->focus == 0){ __tmp_color = xCOLOR_GRAY2; }

            //ok funciona
            //rectBackbufferDrawRectangle ( 
            //    window->left +1, window->top +1, 
            //    window->width +1 +1, window->height +1 +1, 
            //    __tmp_color, 1, 0 ); 
            
            //test
            //remeber: the first window do not have a parent.
            //if ( (void*) window->parent == NULL ){
 
                //gwssrv_debug_print ("redraw_window: [Shadow] Parent"); 
                //exit(1); 
                rectBackbufferDrawRectangle ( 
                    (window->left +1), (window->top +1), 
                    (window->width +1 +1), (window->height +1 +1), 
                    __tmp_color, 1, 0 ); 
            //}
            
            //if ( (void*) window->parent != NULL ){
            //    rectBackbufferDrawRectangle ( 
            //        (window->left +1), (window->top +1), 
            //        (window->width +1 +1), (window->height +1 +1), 
            //        __tmp_color, 1, 0 ); 
            //}
        }

        // ??
        // E os outros tipos, não tem sombra ??
        // Os outros tipos devem ter escolha para sombra ou não ??
        // Flat design pode usar sombra para definir se o botão 
        // foi pressionado ou não.

       // ...
    } //fim do shadow
  

// ## Background ##
// Background para todo o espaço ocupado pela janela e pelo seu frame.
// O posicionamento do background depende do tipo de janela.
// Um controlador ou um editbox deve ter um posicionamento relativo
// à sua janela mãe. Já uma overlapped pode ser relativo a janela 
// gui->main ou relativo à janela mãe.

    //gwssrv_debug_print ("redraw_window: Background\n");
    if ( window->backgroundUsed == 1 )
    {

        //window->bg_color = COLOR_PINK;
        //window->bg_color = CurrentColorScheme->elements[csiWindowBackground]; 

        // O argumento 'color' será a cor do bg para alguns tipos.
        // Talvez não deva ser assim. Talvez tenha que se respeitar o tema instalado.
        //if ( (unsigned long) window->type == WT_SIMPLE ) { window->bg_color = color; }
        //if ( (unsigned long) window->type == WT_POPUP )  { window->bg_color = color; }
        //if ( (unsigned long) window->type == WT_EDITBOX) { window->bg_color = color; }
        //if ( (unsigned long) window->type == WT_CHECKBOX){ window->bg_color = color; }
        //if ( (unsigned long) window->type == WT_SCROLLBAR){ window->bg_color = color; }
        //if ( (unsigned long) window->type == WT_ICON )   { window->bg_color = color; }
        //if ( (unsigned long) window->type == WT_BUTTON ) { window->bg_color = color; }
        // ...

		// Pintar o retângulo.
		// #todo: 
		// ?? width Adicionar a largura da bordas bordas verticais.
		// #todo: 
		// ?? height Adicionar as larguras das bordas horizontais e da barra de títulos.

        /*
        if ( (unsigned long) type == WT_STATUSBAR )
        {
            drawDataRectangle ( window->left, window->top, 
                window->width -1, window->height, window->bg_color ); 

            grDrawString ( window->left +8, window->top +8, 
                COLOR_TEXT, window->name ); 
            goto done;
        }
        */

        // 
        // Draw background!
        //

        //#bugbug
        //Remember: The first window do not have a parent.
        //if ( (void*) window->parent == NULL ){ 
            //gwssrv_debug_print ("redraw_window: [Background] Parent\n"); 
            //exit(1); 
            rectBackbufferDrawRectangle ( 
                window->left, window->top, 
                window->width, window->height, 
                window->bg_color, 1, 0 );
        //}  
        
        //if ( (void*) window->parent != NULL ){
        //    rectBackbufferDrawRectangle ( 
        //        window->left, window->top, 
        //        window->width, window->height, 
        //        window->bg_color, 1, 0 );
        //}
        //?? More ...
    }  //fim do background

    //
    // botao
    //
    
    //Termina de desenhar o botão, mas não é frame
    //é só o botão...
    //caso o botão tenha algum frame, será alguma borda extra.
    int Focus=0;    //(precisa de borda)
    int Selected=0;
    unsigned int border1=0;
    unsigned int border2=0;

    //gwssrv_debug_print ("redraw_window: Type Button\n");
    if ( (unsigned long) window->type == WT_BUTTON )
    {
        //if ( (void*) window->parent == NULL )
            //printf("redraw_window: [FAIL] window->parent\n");
        
        
        /*
        if ( (void*) window->parent != NULL )
        {
           wmDrawFrame ( 
            (struct gws_window_d *) window->parent,  //parent.
            (struct gws_window_d *) window,      //bg do botão em relação à sua parent. 
            METRICS_BORDER_SIZE,       //border size
            (unsigned int)COLOR_BLACK, //border color 1
            (unsigned int)COLOR_BLACK, //border color 2
            (unsigned int)COLOR_BLACK, //border color 3
            (unsigned int)COLOR_BLACK, //ornament color 1
            (unsigned int)COLOR_BLACK, //ornament color 2
            1 );  //style
        }
        */
        
        //border color
        //o conceito de status e state
        //está meio misturado. ja que estamos usando
        //a função de criar janela para criar botão.
        //#bugbug
        
        switch( window->status )
        {
            case BS_FOCUS:
                border1 = COLOR_BLUE;
                border2 = COLOR_BLUE;
                break;

            //case BS_PRESS:
            case BS_PRESSED:
                Selected = 1;
                border1 = GWS_COLOR_BUTTONSHADOW3;
                border2 = GWS_COLOR_BUTTONHIGHLIGHT3;
                break;

            case BS_HOVER:
                break;
                    
            case BS_DISABLED:
                border1 = COLOR_GRAY;
                border2 = COLOR_GRAY;
                break;

            case BS_PROGRESS:
                break;

            case BS_DEFAULT:
            default: 
                Selected = 0;
                border1 = GWS_COLOR_BUTTONHIGHLIGHT3;
                border2 = GWS_COLOR_BUTTONSHADOW3;
                break;
        };
        

        
        size_t tmp_size = (size_t) strlen ( (const char *) window->name );

        if(tmp_size>64)
            tmp_size=64;

        unsigned long offset = 
        ( ( (unsigned long) window->width - ( (unsigned long) tmp_size * (unsigned long) gcharWidth) ) / 2 );


            // redraw the button border.
            // #todo:
            // as cores vao depender do etado do botao.
            // #todo: veja como foi feito na hora da criaçao do botao.
            __draw_button_borders(
                (struct gws_window_d *)window,
                (unsigned int) border1, //buttonBorderColor1,
                (unsigned int) border2, //buttonBorderColor2,
                (unsigned int) xCOLOR_GRAY5, //buttonBorderColor2_light,
                (unsigned int) COLOR_BLACK );  //buttonBorder_outercolor );
             
            // Button label

            //gwssrv_debug_print ("redraw_window: [FIXME] Button label\n"); 
            
            if (Selected == TRUE)
            {
                grDrawString ( 
                    (window->left) +offset, 
                    (window->top)  +8, 
                    COLOR_WHITE, window->name );
            }
            
            if (Selected == FALSE)
            {
                grDrawString ( 
                    (window->left) +offset, 
                    (window->top)  +8, 
                    COLOR_BLACK, window->name );
            }
    }


    //#todo:
    if ( (unsigned long) window->type == WT_EDITBOX )
    {
        gwssrv_debug_print ("redraw_window: [TODO] Type Editbox\n");
        //...
    }
    
    // more types ?...

draw_frame:

    // #todo
    // Precisamos de uma rotina que redesenhe o frame,
    // sem alocar criar objetos novos.

    if ( window->type == WT_OVERLAPPED || 
         window->type == WT_EDITBOX || 
         window->type == WT_BUTTON )
    {
        if ( (void*) window != NULL )
        {
            if( (void*) window->parent != NULL )
            {
                if( window->parent->magic == 1234 )
                {
                    wmCreateWindowFrame ( 
                        (struct gws_window_d *) window->parent,  //parent.
                        (struct gws_window_d *) window,      //bg do botão em relação à sua parent. 
                        METRICS_BORDER_SIZE,       //border size
                        (unsigned int)COLOR_BLACK, //border color 1
                        (unsigned int)COLOR_BLACK, //border color 2
                        (unsigned int)COLOR_BLACK, //border color 3
                        (unsigned int)COLOR_BLACK, //ornament color 1
                        (unsigned int)COLOR_BLACK, //ornament color 2
                        1 );  //style
                }
            }
        }
    }

    if (flags == TRUE){
        gws_show_window_rect(window);
    }

    return 0;
}



int redraw_window_by_id(int wid, unsigned long flags)
{
    struct gws_window_d *w;
    
    if(wid<0 || wid>=WINDOW_COUNT_MAX)
        return -1;
    
    w = (void*) windowList[wid];
    
    if( (void*) w != NULL ){
        redraw_window(w,flags);
        return 0;
    }
    return -1;
}


// Here we're gonna redraw the given window
// and invalidate it.
int 
update_window ( 
    struct gws_window_d *window, 
    unsigned long flags )
{

    if ( (void*) window == NULL )
        return -1;

    return (int) redraw_window(window,flags);
}


/*
 * =====================================================
 * gwssrv_initialize_color_schemes:
 *     Isso configura os esquemas de cores utilizados 
 * pelo sistema.
 *     Essa rotina é chamada apenas uma vez na inicialização
 * do kernel.
 *     O esquema de cores a ser utilizado deve estar 
 * salvo no perfil do usuário que fez o logon.
 * Os dois esquemas padrão chamam-se: 'humility' e 'pride'.
 * + O esquema 'humility' são cores com tema cinza, 
 * lembrando interfaces antigas.
 * + O esquema 'pride' são cores modernas 
 *   ( Aquele verde e preto e cinza, das primeiras versões, 
 * com imagens publicadas. )
 * @todo: é preciso criar rotinas que selecionem entre os 
 * modo criados e habilitados.
 * É preciso criar rotinas que permitam que aplicativos 
 * em user mode criem esquemas de cores e habilite eles.
 */


void 
gwssrv_initialize_color_schemes (int selected_type)
{
    struct gws_color_scheme_d  *humility;
    struct gws_color_scheme_d  *pride;

	//
	// HUMILITY
	//
	
    //Criando o esquema de cores humility. (cinza)
    humility = (void *) malloc ( sizeof(struct gws_color_scheme_d) );

    if( (void *) humility == NULL ){
        gwssrv_debug_print ("gwssrv_initialize_color_schemes: humility\n");
        printf             ("gwssrv_initialize_color_schemes: humility\n"); 

        GWSHumilityColorScheme = NULL;

        // #bugbug
        // ? return ????
        
    }else{
		
		//Object.
		//humility->objectType = ObjectTypeColorScheme;
		//humility->objectClass = ObjectClassGuiObjects;

        humility->used  = TRUE;
        humility->magic = 1234;
        humility->name  = "Humility";
		
		//Colors
		//Definidas em ws.h
        humility->elements[csiNull] = 0;                             //0
        humility->elements[csiDesktop] = HUMILITY_COLOR_BACKGROUND;  //1		
        humility->elements[csiWindow] = HUMILITY_COLOR_WINDOW;       //2
        humility->elements[csiWindowBackground] = HUMILITY_COLOR_WINDOW_BACKGROUND;	 //3	
        humility->elements[csiActiveWindowBorder] = HUMILITY_COLOR_ACTIVE_WINDOW_BORDER;  //4
        humility->elements[csiInactiveWindowBorder] = HUMILITY_COLOR_INACTIVE_WINDOW_BORDER;  //5		
        humility->elements[csiActiveWindowTitleBar] = HUMILITY_COLOR_ACTIVE_WINDOW_TITLEBAR;  //6
        humility->elements[csiInactiveWindowTitleBar] = HUMILITY_COLOR_INACTIVE_WINDOW_TITLEBAR;  //7	 	
        humility->elements[csiMenuBar] = HUMILITY_COLOR_MENUBAR;                //8
        humility->elements[csiScrollBar] = HUMILITY_COLOR_SCROLLBAR;            //9  
        humility->elements[csiStatusBar] = HUMILITY_COLOR_STATUSBAR;            //10
        humility->elements[csiMessageBox] = HUMILITY_COLOR_MESSAGEBOX;		    //11
        humility->elements[csiSystemFontColor] = HUMILITY_COLOR_SYSTEMFONT;		//12
        humility->elements[csiTerminalFontColor] = HUMILITY_COLOR_TERMINALFONT;	//13
        // ...

		//Sanvando na estrutura padrão para o esquema humility.
        GWSHumilityColorScheme = (void*) humility;
    };

	//
	// PRIDE 
	//
	
    //Criando o esquema de cores PRIDE. (colorido)
    pride = (void *) malloc ( sizeof(struct gws_color_scheme_d) );

    if ( (void *) pride == NULL ){
        gwssrv_debug_print ("gwssrv_initialize_color_schemes: pride\n");
        printf ("gwssrv_initialize_color_schemes: pride\n"); 

        GWSPrideColorScheme = NULL;

        // #bugbug
        // ? return ????

    }else{

		//Object.
		//pride->objectType  = ObjectTypeColorScheme;
		//pride->objectClass = ObjectClassGuiObjects;

        pride->used  = TRUE;
        pride->magic = 1234;
        pride->name  = "Pride";

		//Colors
		//Definidas em ws.h
        pride->elements[csiNull] = 0;
        pride->elements[csiDesktop] = PRIDE_COLOR_BACKGROUND;  
        pride->elements[csiWindow] = PRIDE_COLOR_WINDOW;
        pride->elements[csiWindowBackground] = PRIDE_COLOR_WINDOW_BACKGROUND;
        pride->elements[csiActiveWindowBorder] = PRIDE_COLOR_ACTIVE_WINDOW_BORDER;  
        pride->elements[csiInactiveWindowBorder] = PRIDE_COLOR_INACTIVE_WINDOW_BORDER;  
        pride->elements[csiActiveWindowTitleBar] = PRIDE_COLOR_ACTIVE_WINDOW_TITLEBAR;    
        pride->elements[csiInactiveWindowTitleBar] = PRIDE_COLOR_INACTIVE_WINDOW_TITLEBAR;		
        pride->elements[csiMenuBar] = PRIDE_COLOR_MENUBAR;
        pride->elements[csiScrollBar] = PRIDE_COLOR_SCROLLBAR;  		
        pride->elements[csiStatusBar] = PRIDE_COLOR_STATUSBAR;    
        pride->elements[csiMessageBox] = PRIDE_COLOR_MESSAGEBOX;
        pride->elements[csiSystemFontColor] = PRIDE_COLOR_SYSTEMFONT;    //12
        pride->elements[csiTerminalFontColor] = PRIDE_COLOR_TERMINALFONT;  //13		
        // ...

		// Sanvando na estrutura padrão para o esquema pride.
        GWSPrideColorScheme = (void *) pride;
    };


    //
    // Select
    //

    // Configurando qual será o esquema padrão.
    // #todo: 
    // Criar uma função que selecione qual dois esquemas serão usados
    // apenas selecionando o ponteiro da estrutura.  


    switch (selected_type){

    case ColorSchemeNull:
        GWSCurrentColorScheme = (void *) GWSHumilityColorScheme;
        break;

    case ColorSchemeHumility:
        GWSCurrentColorScheme = (void *) GWSHumilityColorScheme;
        break;

    case ColorSchemePride:
        GWSCurrentColorScheme = (void *) GWSPrideColorScheme; 
        break;

    default:
        GWSCurrentColorScheme = (void *) GWSHumilityColorScheme;
        break;
    };
    
    //
    // Check current
    //
    
    if ( (void*) GWSCurrentColorScheme == NULL )
    {
        gwssrv_debug_print ("gwssrv_initialize_color_schemes: GWSCurrentColorScheme\n");
        printf             ("gwssrv_initialize_color_schemes: GWSCurrentColorScheme\n"); 
        exit(1);
    }
}


// seleciona o tipo ...isso é um serviço.
int gwssrv_select_color_scheme (int type)
{
	//#debug
	//printf("gwssrv_select_color_scheme: type={%d} \n", type);

    switch (type){
        case ColorSchemeHumility:  goto do_humility;  break;
        case ColorSchemePride:     goto do_pride;     break;
        default:
            gwssrv_debug_print("gwssrv_select_color_scheme: [FAIL] Type not defined\n");
            goto fail;
            break;
    };


do_humility:

    if ( (void *) GWSHumilityColorScheme == NULL ){
        gwssrv_debug_print("HumilityColorScheme fail\n");
        goto fail;
    }else{

        if ( GWSHumilityColorScheme->used  != TRUE || 
             GWSHumilityColorScheme->magic != 1234 )
        {
            gwssrv_debug_print("HumilityColorScheme sig fail\n");
            goto fail;
        }

        gwssrv_debug_print("Humility selected\n");
        GWSCurrentColorScheme = GWSHumilityColorScheme;
        goto done;
    };

do_pride:

    if ( (void *) GWSPrideColorScheme == NULL ){
        gwssrv_debug_print("GWSPrideColorScheme fail\n");
        goto fail;
    }else{
        if( GWSPrideColorScheme->used  != TRUE || 
            GWSPrideColorScheme->magic != 1234 )
        {
            gwssrv_debug_print("PrideColorScheme sig fail\n");
            goto fail;
        }

        gwssrv_debug_print ("Pride selected\n"); 
        GWSCurrentColorScheme = GWSPrideColorScheme;
        goto done;
    };

done:
    return 0;
fail:
    gwssrv_debug_print ("gwssrv_select_color_scheme: fail\n");
    return 1;
}


/*
 * gws_show_window_rect:
 *     Mostra o retângulo de uma janela que está no backbuffer.
 *     Tem uma janela no backbuffer e desejamos enviar ela 
 * para o frontbuffer.
 *     A rotina de refresh rectangle tem que ter o vsync
 *     #todo: criar um define chamado refresh_window.
 */

// ??
// Devemos validar essa janela, para que ela 
// não seja redesenhada sem antes ter sido suja?
// E se validarmos alguma janela que não está pronta?
// #test: validando

int gws_show_window_rect (struct gws_window_d *window)
{
    //struct gws_window_d  *p;

    debug_print("gws_show_window_rect:\n");


    if ( (void *) window == NULL )
    {
        debug_print ("gws_show_window_rect: [FAIL] window\n");
        printf      ("gws_show_window_rect: [FAIL] window\n");
        exit(1);
        //return (int) -1;
    }


    if ( window->used != TRUE || window->magic != 1234 )
    {
        debug_print ("gws_show_window_rect: window validation\n");
        printf      ("gws_show_window_rect: window validation\n");
        exit(1);
        //return (int) -1;
    }

//#shadow 
// ?? E se a janela tiver uma sombra, 
// então precisamos mostrar a sombra também. 
//#bugbug
//Extranhamente essa checagem atraza a pintura da janela.
//Ou talvez o novo tamanho favoreça o refresh rectangle,
//ja que tem rotinas diferentes para larguras diferentes

    //if ( window->shadowUsed == 1 )
    //{
        //window->width = window->width +4;
        //window->height = window->height +4;

        //refresh_rectangle ( window->left, window->top, 
        //    window->width +2, window->height +2 ); 
        //return (int) 0;
    //}

    //p = window->parent;


// Refresh rectangle
// See: rect.c   

    //debug_print("gws_show_window_rect: Calling gws_refresh_rectangle\n");
    gws_refresh_rectangle ( 
        window->left, 
        window->top, 
        window->width, 
        window->height ); 

// Validate window
// Com isso o compositor não vai redesenhar
// até que alguém invalide ela.

    //debug_print("gws_show_window_rect: Calling validate_window\n");
    validate_window(window);

//done:
    //debug_print("gws_show_window_rect: done\n");
    return 0;

fail:
    // fail.
    debug_print("gws_show_window_rect: fail\n");
    return (int) -1;
}


int flush_window (struct gws_window_d *window)
{
    return (int) gws_show_window_rect(window);
}


int flush_window_by_id(int wid)
{
    struct gws_window_d *w;
    
    if(wid<0 || wid>=WINDOW_COUNT_MAX)
        return -1;
    
    w = (void*) windowList[wid];
    
    if( (void*) w != NULL ){
        flush_window(w);
        return 0;
    }
    return -1;
}


/*
// #todo
// Retorna o ponteiro de estrutura de janela
// dado o id da janela.
struct gws_window_d *gws_window_from_id (int id);
struct gws_window_d *gws_window_from_id (int id)
{
    struct gws_window_d *w;
    
    // ...
    
    return (struct gws_window_d *) w;
}
*/


// Taskbar
void create_taskbar (void)
{

    int WindowId = -1;  // bar
    int menu_wid;       // button

    unsigned long w = gws_get_device_width();
    unsigned long h = gws_get_device_height();

    gwssrv_debug_print ("gwssrv: create_taskbar\n");


    if ( w==0 || h==0 )
    {
        gwssrv_debug_print ("create_taskbar: w h\n");
        printf             ("create_taskbar: w h\n");
        exit(1);
    }

// Taskbar.
// Create  window.

    unsigned long wLeft   = (unsigned long) 0;
    unsigned long wTop    = (unsigned long) (h-40);
    unsigned long wWidth  = (unsigned long) w;
    unsigned long wHeight = (unsigned long) 40;

    __taskbar_window = 
        (struct gws_window_d *) CreateWindow ( 
                                    WT_SIMPLE, 
                                    0, //style
                                    1, //status 
                                    1, //view
                                    "TaskBar",  
                                    wLeft, wTop, wWidth, wHeight,   
                                    gui->screen_window, 0, 
                                    COLOR_GRAY, COLOR_GRAY );



    if ( (void *) __taskbar_window == NULL )
    {
        gwssrv_debug_print ("create_taskbar: __taskbar_window\n"); 
        printf             ("create_taskbar: __taskbar_window\n");
        exit(1);
    }

    if ( __taskbar_window->used != TRUE || 
         __taskbar_window->magic != 1234 )
    {
        gwssrv_debug_print ("create_background: __taskbar_window validation\n"); 
        printf             ("create_background: __taskbar_window validation\n");
        exit(1);
    }

// Register the window.
    WindowId = (int) RegisterWindow(__taskbar_window);
    if (WindowId<0)
    {
        gwssrv_debug_print ("create_taskbar: Couldn't register window\n");
        printf             ("create_taskbar: Couldn't register window\n");
        exit(1);
    }

// wid
    __taskbar_window->id = WindowId;

// Setup Window manager.
    WindowManager.taskbar = (struct gws_window_d *) __taskbar_window;

// Show
    //flush_window(__taskbar_window);

// #debug

    /*
    printf ("bar: %d %d %d %d\n",
        __taskbar_window->left,
        __taskbar_window->top,
        __taskbar_window->width,
        __taskbar_window->height );

    //refresh_screen();
    //while(1){}
    */


// ===================================
// Clean the button list.
    int b=0;
    for(b=0; b<8; b++){
        tb_buttons[b]=0;
    };
    
    tb_buttons_count=0;


// ========================================
// start menu button

    //unsigned long Space = TB_BUTTON_PADDING;   //4;
    //unsigned long b_width  = TB_BUTTON_WIDTH;    //(8*10);
    //unsigned long b_height = TB_BUTTON_HEIGHT;   //40-(Space*2);
    //unsigned long b_left   = TB_BUTTON_PADDING;  //Space;
    //unsigned long b_top    = TB_BUTTON_PADDING;  //Space;

    unsigned long b_left   = 0; 
    unsigned long b_top    = TB_BUTTON_PADDING;
    unsigned long b_width  = TB_BUTTON_WIDTH;
    unsigned long b_height = TB_BUTTON_HEIGHT;


    int i=0;         //iterator
    int nbuttons=4;  //quantidade de botões na lista
    
    struct gws_window_d *tmp_button;
    int tmp_wid=-1;
    char button_label[32];
    
// Creating n buttons in the taskbar,

    for(i=0; i<nbuttons; i++){

    b_left = TB_BUTTON_PADDING + ( (TB_BUTTON_PADDING*i) + (b_width*i) );

    itoa(i,button_label);
    button_label[2] = 0;
    
    tmp_button = 
        (struct gws_window_d *) CreateWindow ( 
            WT_BUTTON, 0, 1, 1, 
            button_label,  //string  
            b_left, 
            b_top, 
            b_width, 
            b_height,   
            __taskbar_window, 0, COLOR_GRAY, COLOR_GRAY );    

    if ( (void *) tmp_button == NULL )
    {
        gwssrv_debug_print ("create_taskbar: tmp_button\n"); 
        printf             ("create_taskbar: tmp_button\n");
        exit(1);
    }

    if ( tmp_button->used != TRUE || 
         tmp_button->magic != 1234 )
    {
        gwssrv_debug_print ("create_background: tmp_button validation\n"); 
        printf             ("create_background: tmp_button validation\n");
        exit(1);
    }

// Register the button.
    tmp_wid = RegisterWindow(tmp_button);
    if (tmp_wid<0)
    {
        gwssrv_debug_print ("create_taskbar: Couldn't register button\n");
        printf             ("create_taskbar: Couldn't register button\n");
        exit(1);
    }
    
    if(i==0){
        __taskbar_startmenu_button_window = tmp_button;
    }

//save

    // id de janelas.
    tb_buttons[i] = (int) tmp_wid;

    // ponteiros de estruturas de janelas do tipo botão.
    tb_windows[i] = (unsigned long) tmp_button;

    // Número de botões criados.
    tb_buttons_count++;
    };


// Show.
    flush_window_by_id(__taskbar_window->id);
    //flush_window_by_id(__taskbar_startmenu_button_window->id);
    //flush_window(__taskbar_window);
    //flush_window(__taskbar_startmenu_button_window);

// #debug

/*
    printf ("button: %d %d %d %d\n",
        __taskbar_startmenu_button_window->left,
        __taskbar_startmenu_button_window->top,
        __taskbar_startmenu_button_window->width,
        __taskbar_startmenu_button_window->height );

    refresh_screen();
    while(1){}
*/

    gwssrv_debug_print ("gwssrv: create_taskbar: done\n");
}



// Create root window
// Called by gwsInit in gws.c.

struct gws_window_d *createwCreateRootWindow(void)
{
    struct gws_window_d *w;

    // It's because we need a window for drawind a frame.
    // WT_OVERLAPPED needs a window and WT_SIMPLE don't.
    unsigned long rootwindow_valid_type = WT_SIMPLE;

    unsigned long left   = 0;
    unsigned long top    = 0;
//#bugbug: Estamos confiando nesses valores.
    unsigned long width  = (__device_width  & 0xFFFF );
    unsigned long height = (__device_height & 0xFFFF );


// background color.
    unsigned int rootwindow_color;

    // default.
    __set_default_background_color( WM_DEFAULT_BACKGROUND_COLOR );
    rootwindow_color = (unsigned int) __get_default_background_color();

    // custom.
    __set_custom_background_color(WM_DEFAULT_BACKGROUND_COLOR);


// Begin paint
    asm("cli");

    debug_print("createwCreateRootWindow:\n");

// (root window)
// #bugbug: Estamos usado device info sem checar.

    w = (struct gws_window_d *) CreateWindow ( 
                                    rootwindow_valid_type,  
                                    0, //style
                                    1, //status
                                    1, //view
                                    "RootWindow",  
                                    left, top, width, height,
                                    NULL, 0, rootwindow_color, rootwindow_color );
    if ( (void*) w == NULL)
    {
        debug_print("createwCreateRootWindow: [FAIL] w\n");
        printf     ("createwCreateRootWindow: [FAIL] w\n");
        exit(1);
    }
    
// Setup the surface in ring0
    setup_surface_retangle(left,top,width,height);

// invalidate the surface in ring0.
    invalidate_surface_retangle();

    w->dirty = TRUE;  // Invalidate again.
    w->locked = TRUE;
    w->used  = TRUE;
    w->magic = 1234;

    // Register
    // WindowId = RegisterWindow (__root_window);

    // if (WindowId<0){
    // gwssrv_debug_print ("create_background: Couldn't register window\n");
    //return;
    //}

// Root window
    gwsDefineInitialRootWindow (w);

// End paint
    asm("sti");

    debug_print("createwCreateRootWindow: done\n");

    return (struct gws_window_d *) w;
}


int gwsDefineInitialRootWindow ( struct gws_window_d *window )
{
    if ( (void *) window == NULL )
    {
        debug_print("gwsInit: [FAIL] screen window\n");
        printf     ("gwsInit: [FAIL] screen window\n");
        exit(1);
        //return -1;
    }
    
    __root_window = window;


// Setup Window manager.

    WindowManager.root = (struct gws_window_d *) window;

    // ...
    
    return 0;  //ok
}



/*
 * get_active_window:
 *     Obtem o id da janela ativa.
 *     @todo: Mudar para windowGetActiveWindowId().
 */
int get_active_window (void)
{
    return (int) active_window;
}


/*
 * set_active_window:
 */

void set_active_window (int id)
{

	// @todo: Limits. Max.
    if (id < 0){
        return;
    }

    active_window = (int) id;
}


int get_window_with_focus(void)
{
    return (int) window_with_focus;
}


int set_window_with_focus(int id)
{
    
    if(id<0){
        return -1;
    }

    window_with_focus = (int) id;
    
/*  
//#test
    struct gws_window_d *w;
    w = (struct gws_window_d *) windowList[id];
    sc82 (10011,w->client_tid,w->client_tid,w->client_tid);
*/
}


// Pegando a z-order de uma janela.
int get_zorder ( struct gws_window_d *window )
{
    if ( (void *) window != NULL ){
        return (int) window->zIndex;
    }

    return (int) -1;
}


int get_top_window (void)
{
    return (int) top_window;
}


//Setando a top window.
void set_top_window (int id)
{
    top_window = (int) id;
}


int gwssrv_get_number_of_itens (struct gwssrv_menu_d *menu)
{
    if ( (void*) menu == NULL ){
        return -1;
    }

    return (int) menu->itens_count;
}


struct gwssrv_menu_d *gwssrv_create_menu (
    struct gws_window_d *parent,
    int highlight,
    int count,
    unsigned long x,
    unsigned long y,
    unsigned long width,
    unsigned long height,
    unsigned int color )
{

    struct gwssrv_menu_d  *menu;
    struct gws_window_d    *window;


    gwssrv_debug_print("gwssrv_create_menu:\n");


    menu = (struct gwssrv_menu_d *) malloc ( sizeof(struct gwssrv_menu_d) );

    if ( (void *) menu == NULL ){
        gwssrv_debug_print("gwssrv_create_menu: [FAIL] menu\n");
        return (struct gwssrv_menu_d *) 0;
    }

// Deslocamento em relação a janela mãe.
    menu->x = x;
    menu->y = y;
    menu->width  = width;
    menu->height = height;
    
    menu->color=color;
    menu->highlight = highlight;
    menu->itens_count = count;

    window = (struct gws_window_d *) CreateWindow ( 
        WT_SIMPLE, 
        0,  //style
        1,  //status
        1,  //view
        "menu-bg",  
        menu->x, menu->y, menu->width, menu->height,   
        (struct gws_window_d *) parent, 
        0, 
        color, 
        color ); 

    if ( (void *) window == NULL ){
        gwssrv_debug_print ("gwssrv_create_menu: window fail\n");  
        return NULL;
    }

//#todo register
    int m_wid=-1;
    m_wid = RegisterWindow(window);
    if(m_wid<0)
        return NULL;

// Save window pointer.
    menu->window = window; 
    menu->parent = parent;

    return (struct gwssrv_menu_d *) menu;
}


// Create menu item
struct gwssrv_menu_item_d *gwssrv_create_menu_item (
    char *label,
    int id,
    struct gwssrv_menu_d *menu)
{

    struct gwssrv_menu_item_d *item;
    struct gws_window_d       *window;  //menu item window
    
    
    gwssrv_debug_print("gwssrv_create_menu_item:\n");    
    
    if ( (void *) menu == NULL ){
        return (struct gwssrv_menu_item_d *) 0;
    }
    
    //create menu item.
    item = (struct gwssrv_menu_item_d *) malloc( sizeof(struct gwssrv_menu_item_d) );

    if ( (void *) item == NULL ){
        return (struct gwssrv_menu_item_d *) 0;
    }

    //provisório
    if(id>5 || id>menu->itens_count)
        return (struct gwssrv_menu_item_d *) 0;


    item->id = id;

    item->width  = (menu->width -8);
    item->height = (menu->height / menu->itens_count);
    item->x      = 4;
    item->y      = (item->height*id);

    if( menu->window == NULL )
    {
        return NULL;
    }

    window = (struct gws_window_d *) CreateWindow ( 
        WT_BUTTON,
        0, //style
        1, //status 
        1, //view 
        (char *) label,  
        item->x, item->y, item->width, item->height,   
        menu->window, 
        0, 
        COLOR_GRAY, 
        COLOR_GRAY );    

    if ( (void*) window == NULL )
    {
        item->window = NULL;
        goto fail;
    }

    //#todo:
    //register.

    int mi_wid=-1;
    mi_wid = RegisterWindow(window);
    if(mi_wid<0)
        return NULL;


// The window.
    item->window = window;
        
//ok
    return (struct gwssrv_menu_item_d *) item;

fail:
    return (struct gwssrv_menu_item_d *) 0;
}


/*
struct gwssrv_menu_item_d *gwssrv_get_menu_item(struct gwssrv_menu_d *menu, int i);
struct gwssrv_menu_item_d *gwssrv_get_menu_item(struct gwssrv_menu_d *menu, int i)
{


     //return (struct gwssrv_menu_item_d *) ?;
}
*/

/*
int gwssrv_redraw_menuitem(struct gwssrv_menu_item_d *);
int gwssrv_redraw_menuitem(struct gwssrv_menu_item_d *)
{
}
*/


/*
int gwssrv_redraw_menu ( struct gwssrv_menu_d *menu );
int gwssrv_redraw_menu ( struct gwssrv_menu_d *menu )
{
    int i=0;
    int n=0;

    if ( (void*) menu == NULL )
        return -1;
    
    n = (int) gwssrv_get_number_of_itens(menu);

    if ( n<=0 )
        return -1;
    
    for (i=0; i<n; i++)
    {
        mi = gwssrv_get_menu_item(i,menu);
        gwssrv_redraw_menuitem(mi);
    };
}
*/



// checa se o mouse esta passando sobre o main menu.
int __is_inside_menu(struct gwssrv_menu_d *menu, int x, int y)
{

// #todo
// Para as janelas do menu precisamos considerar
// a parent window para encontrarmos o 
// deslocamento correto.

    struct gwssrv_menu_d *m;

    struct gws_window_d *pw;
    struct gws_window_d *mw;

    m = (struct gwssrv_menu_d *) menu;

    if( (void*)m==NULL )
        return -1;

// parent window
    pw = m->parent;
    if( (void*)pw == NULL )
        return -1;

// menu window
    mw = m->window;
    if( (void*)mw == NULL )
        return -1;

//parent
    int x1= pw->left + mw->left; 
    int x2= x1 + mw->width;
    int y1= pw->top    + mw->top;
    int y2= y1 + mw->height;

    if( x > x1 && 
        x < x2 &&
        y > y1 && 
        y < y2 )
    {
        return TRUE;
    }

    return FALSE;
}


//test
int create_main_menu(int mousex, int mousey)
{

//#provisorio
    struct gwssrv_menu_d *menu;


    MainMenu->in_use = FALSE;

    // #testing (NEW)
    menu = gwssrv_create_menu (
               (int) gui->screen_window,  //parent
               (int) 0,   //highlight
               (int) 4,   //count
               (unsigned long) mousex,  //8, 
               (unsigned long) mousey,  //8,
               (unsigned long) 200,     //320,
               (unsigned long) 200,     //280,
               (unsigned long) COLOR_WHITE );

    if ( (void*) menu == NULL ){
        return -1;
    }

//menu item 0
    gwssrv_create_menu_item (
    "Test mouse F3",
    (int) 0,
    (struct gwssrv_menu_d *) menu );

//menu item 1
    gwssrv_create_menu_item (
        "Editor F10",
        (int) 1,
        (struct gwssrv_menu_d *) menu );

//menu item 2
    gwssrv_create_menu_item (
        "Terminal F12",
        (int) 2,
        (struct gwssrv_menu_d *) menu );

//menu item 3
    gwssrv_create_menu_item (
        "Reboot F4",
        (int) 3,
        (struct gwssrv_menu_d *) menu );  

//show
    gws_show_window_rect(menu->window);


// global
    MainMenu = (struct gwssrv_menu_d *) menu;
    MainMenu->in_use = TRUE;
    return 0;
}

/*
 * gws_resize_window:
 *     Muda as dimensões da janela.
 */
 
int 
gws_resize_window ( 
    struct gws_window_d *window, 
    unsigned long cx, 
    unsigned long cy )
{

    if ( (void *) window == NULL ){
        return -1;
    }

// #todo
    //if(window == __root_window)
        //return -1;


    // Só precisa mudar se for diferente.
    if ( window->width  != cx ||
         window->height != cy )
    {
        window->width  = (unsigned long) cx;
        window->height = (unsigned long) cy;
    }
    

    // #test
    //window->dirty = TRUE;
    invalidate_window(window);

    //__root_window->dirty = 1;

    return 0;
}



// #test
// Isso so faz sentido num contexto de reinicializaçao 
// do desktop.
void reset_zorder(void)
{
     //loop
     register int i=0;
     
     struct gws_window_d *w;


     for ( i=0; i<WINDOW_COUNT_MAX; ++i)
     {
         w = (struct gws_window_d *) windowList[i];
         if ( (void*) w != NULL )
         {
             if ( w->used == TRUE && w->magic == 1234 )
             {
                 // Coloca na zorder as janelas overlapped.
                 if ( w->type == WT_OVERLAPPED )
                 {
                     zList[i] = windowList[i];
                 }
             }
         }
     };
}


/*
 * gws_change_window_position:
 *     Muda os valores do posicionamento da janela.
 */

int 
gwssrv_change_window_position ( 
    struct gws_window_d *window, 
    unsigned long x, 
    unsigned long y )
{
    // #??
    // Isso deve mudar apenas o deslocamento em relacao
    // a margem e nao a margem ?

    if ( (void *) window == NULL ){
        gwssrv_debug_print("gwssrv_change_window_position: window\n");
        return -1;
    }


// #todo
    //if(window == __root_window)
        //return -1;


    /*
    if ( window->left != x ||
         window->top  != y )
    {
        window->left = (unsigned long) x;
        window->top  = (unsigned long) y;
    }
    */
    
    window->x = x;
    window->y = y;
    window->left = (window->parent->left + window->x); 
    window->top  = (window->parent->top  + window->y); 
    
    //#test
    //window->dirty = 1;
    invalidate_window(window);
    
    return 0;
}



/*
 * gwsWindowLock:
 *     Bloqueia uma janela.
 *     @todo: Quem pode realizar essa operação??
 */
 
void gwsWindowLock (struct gws_window_d *window)
{

    if ( (void *) window == NULL ){
        return;
    }

    window->locked = (int) WINDOW_LOCKED;  //1.
}


/*
 * gwsWindowUnlock:
 *     Desbloqueia uma janela.
 *     @todo: Quem pode realizar essa operação??
 */
 
void gwsWindowUnlock (struct gws_window_d *window)
{
    if ( (void *) window == NULL ){
        return;
    }

    window->locked = (int) WINDOW_UNLOCKED;  //0.
}


/*
 * gwssrv_init_windows:
 * 
 */

int gwssrv_init_windows (void)
{
    //loop
    register int i=0;

    //window.h
    windows_count     = 0;
    window_with_focus = 0;
    active_window     = 0;
    top_window        = 0;
    //...
    
    show_fps_window = FALSE;


    // Window list
    for (i=0; i<WINDOW_COUNT_MAX; ++i){  windowList[i] = 0;  };

    // z order list
    for (i=0; i<ZORDER_MAX; ++i){  zList[i] = 0;  };

    // ...

    return 0;
}


//
// == ajusting the window to fit in the screen. =======================
//

/*
 credits: hoppy os.
void 
set_window_hor (
    tss_struct *tss,
    int i,
    int j)
{
    
    int d = j-i;
    
    if ( i >= tss->crt_width) 
    {
        i = tss->crt_width-2;
        j = i+d;
    }
    
    if (j<0) 
    {
        j=0;
        i = j-d;
    }
    
    if (i>j) 
    {
        if (i>0)
           j=i;
        else
            i=j;
    }
    
    tss->window_left=i;
    tss->window_right=j;
}
*/


/*
 credits: hoppy os.
void 
set_window_vert ( 
    tss_struct *tss,
    int i,
    int j )
{

    int d = j-i;
    
    if (i >= tss->crt_height) 
    {
        // ajusta o i.
        i = tss->crt_height-1;
        j = i+d;
    }
    
    if (j<0) 
    {
        // ajusta o i.
        j = 0;
        i = j-d;
    }

    if (i>j) 
    {
        if (i>0)
            j=i;
        else
            i=j;
    }

    tss->window_top    = i;
    tss->window_bottom = j;
}
*/


/*
// process id
// Retorna o pid associado à essa janela.
// #todo: fazer essa rotina também na biblioteca client side.
pid_t get_window_pid( struct gws_window_d *window);
pid_t get_window_pid( struct gws_window_d *window)
{
}
*/

/*
// thread id
// Retorna o tid associado à essa janela.
// #todo: fazer essa rotina também na biblioteca client side.
int get_window_tid( struct gws_window_d *window);
int get_window_tid( struct gws_window_d *window)
{
}
*/

// teremos mais argumentos
void wm_Update_TaskBar( char *string )
{

    if( (void*) string == NULL )
        return;
    
    if(*string == 0)
        return;

// Fail
    if ( (void*) __taskbar_window == NULL )
        return;

// Redraw the bar.
// Redraw the button.

    redraw_window_by_id(
        __taskbar_window->id,TRUE);
    //redraw_window_by_id(
    //    __taskbar_startmenu_button_window->id,TRUE);
    //redraw_window(__taskbar_window,TRUE);
    //redraw_window(__taskbar_startmenu_button_window,TRUE);

// Redraw, all the valid buttons in the list.
    int i=0;
    for(i=0; i<TB_BUTTONS_MAX; i++)
    {
        if(tb_buttons[i] != 0){
            redraw_window_by_id(tb_buttons[i],TRUE);
        }
    };


//
// String
//

// String info.
    unsigned long string_left = (unsigned long) (__taskbar_window->width - 100);
    unsigned long string_top  = 8;
    unsigned int string_color = COLOR_YELLOW;
    size_t string_size;

    string_size = (size_t) strlen(string);

// Draw the text.
// less than 10 chars.
    if ( string_size < 10 )
    {
        dtextDrawText(
            __taskbar_window,
            string_left, string_top, 
            string_color, string );
    }

// Show the window.
    flush_window_by_id(__taskbar_window->id);
    //flush_window(__taskbar_window);
}


void wmInitializeGlobals(void)
{
    debug_print ("wmInitializeGlobals:\n");
    fps=0;
    frames_count=0;
    ____old_time=0;
    ____new_time=0;
    
    old_x=0;
    old_y=0;
}


//
// End.
//


