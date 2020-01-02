/*
 *
 *
 *
 *
 *
 */
 
 /*
 #DEFINE GS_TITLESCREEN 0
#DEFINE GS_MENUSCREEN 1
#DEFINE GS_PLAYSCREEN 2

int state = 0;

void gameTitleScreen(Graphics *g, Input *i, double dt);
void gameMenuScreen(Graphics *g, Input *i, double dt);
void gamePlayScreen(Graphics *g, Input *i, double dt);

void *gameStates = {(void*)gameTitleScreen, (void*)gameMenuScreen, (void*)gamePlayScreen};
...

void *state = gameStates[state];
state( &g, &i, dt);

*/
 

#include "wingl.h"
#include "sound_xm.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#define SOUND_VOLUME 50
int SIZE_SQ = 20; // size square (размер квадрата)
int w,h;

typedef struct {int x,y;}vec2;
vec2 snake[400];
int snake_len;
int snake_lives = 3;

Font cFont;// consolas font
int cFontSize;// consolas font size


vec2 dir;
vec2 food,next_food;
bool blink_food , snake_collision_immunity, blink_snake;
bool tutorialReInit = TRUE;
bool keySpace;
void food_respawn();
void startGame();

enum GameState {GAME, GAME_OVER, GAME_FINISH, PAUSE, WAIT_KEY_PRESS,LEVEL_SUCCES,TUTORIAL};
enum GameState state = WAIT_KEY_PRESS;

// для задач прохождения уровней
int need_block , need_intersect_block , need_intersect_two_block;
short current_level = 0;
struct Level
{
    int need_block;
    int need_intersect_block;
    int need_intersect_two_blocks;
};

#define MAX_LEVEL 3
struct Level levels [MAX_LEVEL] =
{
{10,0,0},// level1 rules need_bloks 50, intersect_bloks 0,need_intersect_two_blocks 0
{0,3,0},
{0,0,1},
};

// для пересечения
#define INTERSECT_MAX 10
typedef struct {int x,y,z;}vec3;
vec3 intersect_pos[INTERSECT_MAX];
int intersect_count = 0;
// для пересечения "красные квадраты"
vec3 intersect_pos_tmp[INTERSECT_MAX];
int intersect_count_tmp = 0;


int initGL() 
{
    sound_volume(SOUND_VOLUME);
    startGame();
    return TRUE;                                        // Initialization Went OK
}


void resizeGL(int width, int height)
{
    w = width;
    h = height;
    int min_size = fmin(width,height);
    if(min_size < 440)// меняем размер квадрата
    {
        SIZE_SQ = (min_size - 40) / 20;
    }
    else
    {
        SIZE_SQ = 20;
    }
    cFontSize = SIZE_SQ*2;
    fontInit(&cFont,cFontSize,"Consolas");
    glViewport(0,0,width,height);
}


// обновление состояния уровня и прорисовка HUD
void levelUpdate()
{
    if(state != LEVEL_SUCCES)
    {
    if(need_block == 0 && need_intersect_block == 0 && need_intersect_two_block == 0)
    {
        state = LEVEL_SUCCES;
    }

    if(need_block < 0) need_block = 0;
    if(need_intersect_block < 0) need_intersect_block = 0;
    if(need_intersect_two_block < 0) need_intersect_two_block = 0;
    }

    // жизни змеи
    {
        short heart[] = {
            0b0011110000111100,
            0b0111110000111110,
            0b0111111001111110,
            0b0111111111111110,
            0b1111111111111111,
            0b1111111111111111,
            0b1111111111111111,
            0b1111111111111111,
            0b0111111111111110,
            0b0011111111111100,
            0b0000111111110000,
            0b0000011111100000,
            0b0000001111000000,
            0b0000000110000000,
        };

        glColor3ub(255,0,0);//red
        int x = w;
        int y = 8;
        glPointSize(1);
        glBegin(GL_POINTS);
        for(int i = 0; i < snake_lives;i++)
        {
           x -= SIZE_SQ + 10;
           for(int yy = 0; yy < 14;yy++)
           {
               for(short xx = 0; xx < 16;xx++)
               {
                   if((heart[yy]>>xx) & 0x1)
                   {
                     glVertex2i(x+xx,y+yy);
                   }

               }
           }

        }
        glEnd();


    }


    // текуший уровень
    glColor3ub(0, 255,0);//green
    drawText(cFont,13,30,"Level");
    drawNum(cFont,100,30,current_level+1);
    // длинна змеи в кубиках
    glRecti(13 ,48,13+SIZE_SQ ,48+SIZE_SQ );
    drawNum(cFont,45,70,snake_len);

    int x = w/2-10;
    int y = 8;
    if(current_level == 0)
    {
        glColor3ub(0, 0, 255);//blue
        glRecti( x,y,x+SIZE_SQ ,y+SIZE_SQ );
        drawNum(cFont,w/2+SIZE_SQ,y+SIZE_SQ,need_block);
    }
    //y+=25;
    if(current_level == 1)
    {
    glColor3ub(153, 153, 153);//grey
    //glRecti( x,y,x+SIZE_SQ ,y+SIZE_SQ );
    glColor3ub(0, 255, 0);//green
    //glRecti( x+1,y+1,x-1+SIZE_SQ ,y-2+SIZE_SQ );
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2i(x+2,y+10);
    glVertex2i(x+18,y+10);
    glVertex2i(x+18,y+16);
    glVertex2i(x+13,y+16);
    glVertex2i(x+13,y+4);
    glEnd();
    glColor3ub(255, 0, 0);//red
    glRecti( x+12,y+8,x+12+3 ,y+8+3 );
    drawNum(cFont,w/2+SIZE_SQ,y+SIZE_SQ,need_intersect_block);
    }

    //y+=25;
    if(current_level == 2)
    {
    glColor3ub(153, 153, 153);//grey
    //glRecti( x,y,x+SIZE_SQ ,y+SIZE_SQ );
    glColor3ub(0, 255, 0);//green
    glBegin(GL_LINE_STRIP);
    glVertex2i(x+2,y+10);
    glVertex2i(x+18,y+10);
    glVertex2i(x+18,y+16);
    glVertex2i(x+13,y+16);
    glVertex2i(x+13,y+4);
    glVertex2i(x+6,y+4);
    glVertex2i(x+6,y+16);
    glEnd();
    glColor3ub(255, 0, 0);//red
    glRecti( x+12,y+8,x+12+3 ,y+8+3 );
    glRecti( x+4,y+8,x+4+3 ,y+8+3 );
    drawNum(cFont,w/2+SIZE_SQ,y+SIZE_SQ,need_intersect_two_block);
    }
    //drawNum(w/2+SIZE_SQ+40,y+SIZE_SQ,intersect_count);
    glLineWidth(1);

}

void startGame()
{
    snake_lives = 3;
    snake_len = 1;
    snake[0].x = 0;
    snake[0].y = 0;
    dir.x = 0;
    dir.y = 0;
    need_block = levels[current_level].need_block;
    need_intersect_block = levels[current_level].need_intersect_block;
    need_intersect_two_block = levels[current_level].need_intersect_two_blocks;
    intersect_count = 0;
    intersect_count_tmp = 0;
    food_respawn();
    snake_collision_immunity = FALSE;
    blink_snake = TRUE;

    // test
    //gameFinish = TRUE;
/*
if(!gameFinish && current_level > 0)
{
    int len = 25;
    if(current_level > 1)
        len = 50;

    int y=0,x=1;
    for(int i=1;i<len;i++)
    {
        snake[i].x = x++;
        snake[i].y = y;
        if(x == 19)
        {
            x = 0;
            y++;
        }
        snake_len++;
    }
    // если не дать направление,то будет коллизия с телом,
    // т.к оно движется постоянно
    dir.y = -1;
}
*/


}


int random_number(int min, int max)
{
   return min + (rand() %(max - min));
}

void food_respawn()
{
   static bool first_init = TRUE;
   srand(time(0));
   if(first_init)
   {
       food.x = rand()%20;
       food.y = rand()%20;
       first_init = FALSE;
   }
   else
   {
      food = next_food;
   }
   next_food.x = rand()%20;
   next_food.y = rand()%20;
   // если есть хоть одно пересечение,то спавню еду рядом с текушей едой
   //  это для добавил облегчения игры
    if(intersect_count > 0 )
   {
      int min = (food.x/10)*10;
      int max = min + 10;
      next_food.x = random_number(min,max);
      min = (food.y/10)*10;
      max =  min + 10;
      next_food.y = random_number(min,max);
   }
}


/// летяшие звёзды
void starsUpdate(float delta)
{
#define MAX_STARS 1000
#define SPEED 300
    struct vec3{int x,y,z;};
    static struct vec3 stars[MAX_STARS];
    static bool first_init = TRUE;
    if(first_init)
    {
        srand(time(0));
        // случайным образом раскидаем звёздочки
        for (int i = 0; i < MAX_STARS; i++) {
            stars[i].x = (rand()%(w * 4)) - w * 2;
            stars[i].y = (rand()%(h * 4)) - h * 2;
            stars[i].z = rand()%1900; // Откидываем звезду далеко вперёд :)
        }
        first_init = FALSE;
    }
    glPointSize(2);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_STARS; i++)
    {
        // Если звезда "улетела" за пределы дисплея - генерируем её вдали
        if (stars[i].z < -200 ) {
            //генерация новой позиции
            stars[i].x = (rand()%(w * 4)) - w * 2;
            stars[i].y = (rand()%(h * 4)) - h * 2;
            stars[i].z = 1900.f; // Откидываем звезду далеко вперёд :)
        }
        stars[i].z = stars[i].z - SPEED * delta;
        float z = (stars[i].z + 200);
        float x = (w / 2 + stars[i].x * 200 / z);
        float y =  (h / 2 - stars[i].y * 200 / z);
        float c = (255 - 255 * (stars[i].z + 200) / 2100);
        //printf("c %f \n",c);
        glColor3ub(c, c, c);
        glVertex2i(x, y);

    }
    glEnd();
}




/// алгоритм вечно закручиваюшейся спирали
void spiralUpdate() {

    float maxRadius = w/1.5f;
    int xPos = w / 2, yPos = h / 2;
    // с этими константами можно поэксперементировать
    int count = 500;  // Кол-во точек
    //static float angle = 91.1062f ; // стартовый угол (начало линиями)
    static float angle = 360.f ; // стартовый угол

        angle = angle + 0.0001f;

        for (int i = 0; i < count; i++) {
           float radius= (float) (maxRadius * sqrt(i / (float)count)) ;
           float theta = angle * i;
           int x = (int) (xPos + radius * cos(theta)) ;
           int y = (int) (yPos + radius * sin(theta)) ;
           //glColor3ub(0, 255, 0);// green
           glColor3ub(153,153,153);//grey
           glPointSize(4);
           glBegin(GL_POINTS);
           glVertex2i(x, y);
           glEnd();
        }

}


void drawGame()
{   
    glPushMatrix();
    int gw = SIZE_SQ*20;
    glTranslatef(w/2-gw/2,h/2-gw/2,0);// транслирую по центру экрана
    glColor3ub(0, 0, 0);//black
    glRecti( 0 ,0, 20 * SIZE_SQ ,20 * SIZE_SQ  );
    glColor3ub(0, 255, 0);//green
    if(blink_snake)
    for(int i = 0; i < snake_len;i++)//draw snake body
    {
        glRecti( snake[i].x*SIZE_SQ ,snake[i].y*SIZE_SQ ,snake[i].x*SIZE_SQ+SIZE_SQ , snake[i].y*SIZE_SQ+SIZE_SQ );
    }

    //draw snake eye
    {
    int x = snake[0].x*SIZE_SQ +SIZE_SQ/2 + dir.x*7;
    int y = snake[0].y*SIZE_SQ +SIZE_SQ/2 + dir.y*7;
    glColor3ub(255, 0, 0);// red
    glPointSize(4);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
    }

    //draw food
    glColor3ub(0, 0, 255);//blue
    glRecti( food.x*SIZE_SQ,  food.y*SIZE_SQ, food.x*SIZE_SQ+SIZE_SQ, food.y*SIZE_SQ+SIZE_SQ);

    //draw next_food
    if(blink_food)
    {
    glColor3ub(0, 0, 255);//blue
    glRecti( next_food.x*SIZE_SQ,  next_food.y*SIZE_SQ, next_food.x*SIZE_SQ+SIZE_SQ, next_food.y*SIZE_SQ+SIZE_SQ);
    }

    // draw intersect positions
    glColor3ub(255, 0, 0);//red
    for(int i = 0; i < intersect_count;i++)
    {
        int x = intersect_pos[i].x;
        int y = intersect_pos[i].y;
       glRecti( x*SIZE_SQ,  y*SIZE_SQ, x*SIZE_SQ+SIZE_SQ, y*SIZE_SQ+SIZE_SQ);
     }
    // draw two intersect positions
    if(blink_food)
    for(int i = 0; i < intersect_count_tmp;i++)
    {
        int x = intersect_pos_tmp[i].x;
        int y = intersect_pos_tmp[i].y;
       glRecti( x*SIZE_SQ,  y*SIZE_SQ, x*SIZE_SQ+SIZE_SQ, y*SIZE_SQ+SIZE_SQ);
     }

    // draw grid
    glColor3ub(153,153,153);//grey
    glBegin(GL_LINES);
    for(int y=0; y<20+1; y++) {
        glVertex2i(0, y * SIZE_SQ);
        glVertex2i(20 * SIZE_SQ, y * SIZE_SQ );
    }
    for (int x=0; x<20+1; x++) {
        glVertex2i(x * SIZE_SQ, 0);
        glVertex2i(x * SIZE_SQ, 20 * SIZE_SQ  );
    }
    glEnd();

    glPopMatrix();
}


bool checkCollision()
{
    for(int i = 1; i < snake_len;i++)
    {
        if((snake[0].x == snake[i].x) &&
           (snake[0].y == snake[i].y) )
           return TRUE;
    }
    return FALSE;
}


void addIntersectPos(int x,int y)
{
    need_intersect_block--;
    // добавляю позиции пересечения,они будут рисоваться "красными квадратами"
    intersect_pos[intersect_count].x = x;
    intersect_pos[intersect_count].y = y;
    intersect_pos[intersect_count].z = 2;// это что-бы посчитать сколько хвост прошол по пересечению
    intersect_count++;
}



void checkIntersect()
{

/*
    static int tick_count;    
    static bool sub_ =  TRUE;

    if(intersect_count == 2)
    {
        if(sub_)
        {
            need_intersect_two_block--;
            sub_ = FALSE;
        }

    }
    else
    {
       tick_count = 0;
    }

    if( (tick_count++) > 10)// ~ 2 sec
    {
        intersect_count = 0;
        sub_ =  TRUE;
    }
*/
    // тут это "нагромаждение" для того ,что-бы "красные квадраты" не исчезали 2 секунды
    static int tick_count;
    if(intersect_count == 2)
    {
    need_intersect_two_block--;
    // копирую в tmp
     memcpy(intersect_pos_tmp, intersect_pos, sizeof(intersect_pos));
     intersect_count_tmp = intersect_count;
     intersect_count = 0;
    }


    if(intersect_count_tmp > 0)
    if( (tick_count++) > 20)// ~ 2 sec
    {
        tick_count = 0;
        intersect_count_tmp = 0;
    }

    // проверяю гогда хвост пройдёт по пересечению
    int tmp = intersect_count;
    for(int i = 0; i < intersect_count;i++)
    {
        if((intersect_pos[i].x == snake[snake_len].x) &&
           (intersect_pos[i].y == snake[snake_len].y) )
        {
            intersect_pos[i].z--;
            if(intersect_pos[i].z < 1)// если хвост прошол два раза по пересечению
            {   //то удаляю эту позицию,сдвигом хвоста массива
                memmove(intersect_pos+i, intersect_pos+i+1, (INTERSECT_MAX-i-1)*sizeof(*intersect_pos));
                tmp--;
            }

            if(tmp < 0) tmp=0;
        }
    }
    intersect_count=tmp;

}

// обновление состояния жизней
void livesUpdate(float delta)
{


    static float ms_count_immunity;
    if(snake_collision_immunity)
    if((ms_count_immunity += delta) > 2.f)// иммунитет после коллизии выключаю через 2 сек
    {
        ms_count_immunity = 0;
        //blink_snake = !blink_snake;
        snake_collision_immunity = FALSE;
        blink_snake = TRUE;

    }


    static float ms_count_blink;
    if(snake_collision_immunity)// пока действует иммунитет,идёт моргалка змеи
    if((ms_count_blink += delta) > 0.1f)
    {
        ms_count_blink = 0;
        blink_snake = !blink_snake;
    }



}


void updateGame(float delta)
{
    static float ms_count;
    if((ms_count += delta) > 0.2f)// 0.2 sec
    {
        ms_count = 0;

        int x = snake[0].x;
        int y = snake[0].y;

        // подбираем еду если он перед змейкой
        if(( x == food.x) && ( y == food.y) )
        {            
            snake_len++;// увеличиваю длинну змейки
            need_block--;
            //printf("food up snake_len %d\n",snake_len);
            if(checkCollision())
                addIntersectPos(x,y);

            food_respawn();// респавню еду
        }
        else
        {
            // проверка столкновения головы с телом
            // проверка здесь позволяет пройти сквозь тело,если подобрана еда на теле :)
            if(checkCollision())
            {
                if(!snake_collision_immunity)// если нет иммунитета после коллизии
                  {
                    if(state != GAME_FINISH)// после прохождения игры, делаю бессмертие
                    snake_lives--;// уменьшается одна жизнь
                  }

                snake_collision_immunity = TRUE;// включение иммунитета

                if(snake_lives < 0)
                {
                    state = GAME_OVER;
                    snake_collision_immunity = FALSE;
                    return;
                }
            }
        }

        x += dir.x;
        y += dir.y;

        // движение тела за головой
        for(int i = snake_len; i > 0;i--)
        {
            snake[i] = snake[i-1];
        }

        checkIntersect();

        // если выехал за границы то делаю появление  с другой стoроны :)
        if(x > 20-1) x = 0;
        if(x < 0) x = 20-1;
        if(y > 20-1) y = 0;
        if(y < 0) y = 20-1;
        // ----------------
        snake[0].y = y;
        snake[0].x = x;

    }

}


//метод рисования плывущей строки с права на лево
//рисуется только отрезок текста который попадает в зону видимости
void drawFloatingText(float delta)
{

    static const char* text =
    "Game finished!"
    "               Thanks for playing!!! :)            "
    "For create this game was used: 'c language' "
    "'compiler-mingw32'  'IDE-QtCreator' "
    "'Graphics API-Opengl 1.0'  'Music lib-UFMOD'  "
    "'tracker music from keygen-'BReWErS-XBlade+7trn'  and  'Canterwood-Hex Workshop 4.22 kg' ";


    static Font courier100;
    static float x = 5000.f;
    static int y = 0;
    static int letter_pos;
    static char str[255];

    int text_len = strlen(text);// длинна текста в символах
    int font_width = 56;// ширина символа в пикселях
    int letter_buf = (w/font_width);// сколько символов влезает по ширине окна

    if(x > 4000)// первая инициализация
    {
        fontInit(&courier100,100,"Courier New");
        printf("strlen(text) %d \n",strlen(text));
        x = w;
        memcpy (str, text, letter_buf);
        str[letter_buf] = '\0'; // обязательно нужен символ конца строки,иначе текст будет весь рисоваться
        int gw = SIZE_SQ*20;
        y = ((h/2-gw/2) + 100)/ 2 ;
        //printf("%s \n",str);
    }

    x -= delta * 200;
    if(x < 5)
    {
        x += font_width;
        letter_pos ++;
        if(letter_pos > text_len)
        {
            letter_pos = 0;
            x = w;
        }
        int available = text_len+1-letter_pos;
        if(available < letter_buf )
        {
            letter_buf = available;
            //printf("letter_buf %d \n",letter_buf);
        }
        memcpy (str, text + letter_pos, letter_buf);
        str[letter_buf] = '\0';
        //printf("%s \n",str);
    }


    glColor3ub(0,255,0);// green
    drawText(courier100,x+2,y+2,str);
    glColor3ub(255,0,0);// red
    drawText(courier100,x,y,str);

    // mask
    // FIXME не получилась красивая маска,надо копировать пиксели фона
    //glColor3ub(0,0,0);// black
    //glRecti(0,h-100,50,h);
    //glRecti(w-50,h-100,w,h);
}

void drawRotatetStars(float delta)
{
    static int to_angle = 180;
    static float  angle;
    static float  speed = 1;
    angle += speed;
    if(angle > to_angle || angle < 0)
    {
        to_angle = random_number(1,360);
        //printf("to_angle %d \n",to_angle);
        if(speed > 0)
        speed = -speed;
         else
        speed = abs(speed);
    }

    glPushMatrix();
    glTranslatef(w/2,h/2,0);// транслирую по центру экрана
    glRotatef(angle,0,0,1);
    glTranslatef(-w/2,-h/2,0);// транслирую обратно
    starsUpdate(delta);
    glPopMatrix();
}



void updateTutorial(float delta)
{
    //static bool tutorialReInit = TRUE;
    static int move_inc,step,food_inc;

    vec3 move[] =
    {
        {0,-1,1},
        {1,0,5},
        {0,1,10},
        {-1,0,3},
        {0,-1,3},
        {-1,0,6},
        {0,1,3},
        {1,0,7},
        {0,1,8},
        {-1,0,2},
        {0,-1,12},

    };

    vec2 food_pos[] =
    {
        {14,12},
        {11,9},
        {8,9},
    };


    if(tutorialReInit)
    {
        tutorialReInit = FALSE;
        move_inc = step = food_inc = 0;
        snake_len = 15;
        if(current_level == 2)
            snake_len = 30;
        int y=15,x=4;       
        for(int i=0;i<snake_len;i++)
        {
            snake[i].x = x++;
            snake[i].y = y;
            if(x == 20)
            {
                x = 0;
                y++;
            }
        }
        food.x = food_pos[food_inc].x;
        food.y = food_pos[food_inc++].y;
        next_food.x = food_pos[food_inc].x;
        next_food.y = food_pos[food_inc++].y;
        dir.y = -1;
        dir.x = 0;
    }



    static float ms_count;
    if((ms_count += delta) > 0.2f)// 0.2 sec
    {
        ms_count = 0;
        if(move[move_inc].z == step)
        {
            step = 0;
            dir.x = move[move_inc].x;
            dir.y = move[move_inc].y;
            move_inc++;
            if(move_inc > (sizeof(move)/3/4)-1)
                move_inc = 0;
        }

        step++;
        // подбираем еду если она перед змейкой
        if(( snake[0].x == food.x) && ( snake[0].y == food.y) )
        {
            if(checkCollision())
                addIntersectPos(snake[0].x,snake[0].y);
            food = next_food;
            next_food.x = food_pos[food_inc].x;
            next_food.y = food_pos[food_inc].y;
            food_inc++;
            if(food_inc > (sizeof(food_pos)/2/4)-1)
                food_inc = 0;
        }

        // движение тела за головой
        for(int i = snake_len; i > 0;i--)
        {
            snake[i] = snake[i-1];
        }

        snake[0].x += dir.x;
        snake[0].y += dir.y;

        checkIntersect();
    }

    drawGame();
}

int posYtopGrid()
{
    int gw = SIZE_SQ*20;
    int averagePos = ((h/2-gw/2) + cFontSize)/ 2 ;
    return averagePos;
}

int posYbottomGrid()
{
    int gw = SIZE_SQ*20;
    int averagePos = h - posYtopGrid() + cFontSize/2;
    return averagePos;
}




int drawGL(float delta)                                 // Here's Where We Do All The Drawing
{    

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen And Depth Buffer

#ifndef NDEBUG
    // fps
    glColor3ub(255,255,255);
    drawNum(cFont,20,120,getFPS());
#endif

    static float ms_count_blink;
    if((ms_count_blink += delta) > 0.5f) // 0.5 sec
    {
        ms_count_blink = 0;
        blink_food = !blink_food;
    }
    if(current_level < 1)
        blink_food = FALSE;

    switch(state)
    {
        case GAME:
        livesUpdate(delta);
        updateGame(delta);
         levelUpdate();
        starsUpdate(delta);
        static float ms_count_toggle_text;
        static bool toggle_text;
        if((ms_count_toggle_text += delta) > 5.f) // 5 sec
        {
            ms_count_toggle_text = 0;
            toggle_text = !toggle_text;
        }
        drawTextCenter(cFont,w/2,h-5,
            toggle_text?"press SPACE to pause":"press M to mute on/off");
        drawGame();
        if(keySpace)
        {
            state = PAUSE;
            sound_pause();
        }
            break;
        case GAME_OVER:
        sound_stop();
        livesUpdate(delta);
         levelUpdate();
         glColor3ub(255,0,0);// red
         drawTextCenter(cFont,w/2,posYtopGrid(),"GAME OVER");
         glColor3ub(0,255,0);// green
         drawTextCenter(cFont,w/2,h-5,"press SPACE to start");
          drawGame();
         if(keySpace)
         {
             startGame();
             state = WAIT_KEY_PRESS;
         }
            break;
        case GAME_FINISH:
        livesUpdate(delta);
         updateGame(delta);
        drawRotatetStars(delta);
        drawGame();
        // длинна змеи в кубиках
        glColor3ub(0,255,0);// green
        glRecti(13 ,8,13+SIZE_SQ ,8+SIZE_SQ );
        drawNum(cFont,45,30,snake_len);
        drawFloatingText(delta);
        drawGame();
            break;
        case PAUSE:
        livesUpdate(delta);
         levelUpdate();
         drawTextCenter(cFont,w/2,h-5,"press SPACE to resume");
         drawGame();
        if(keySpace)
        {
        state = GAME;
        sound_resume();
        }
            break;
        case WAIT_KEY_PRESS:
        livesUpdate(delta);
        updateGame(delta);
         levelUpdate();
         glColor3ub(153,153,153);//grey
         if(dir.x == 0 && dir.y == 0)
         {
            drawTextCenter(cFont,w/2,h-5,"use arrow keys");
         }
         else
         {
           state = GAME;
           sound_play("BReWErS-XBlade+7trn");
         }
         drawGame();
            break;
    case LEVEL_SUCCES:
    sound_stop();
    livesUpdate(delta);
     levelUpdate();
     glColor3ub(0,255,0);// green
     drawTextCenter(cFont,w/2,posYtopGrid(),"Level Finished");
     glColor3ub(0,255,0);// green
     drawTextCenter(cFont,w/2,h-5,"press SPACE to next");
     if(keySpace)
     {
         current_level++;
         if(current_level == MAX_LEVEL)
         {
             sound_play("Canterwood");
             state = GAME_FINISH;
         }
         else
         {
             tutorialReInit = TRUE;
             state = TUTORIAL;

         }
     }
     drawGame();
        break;
    case TUTORIAL:
    updateTutorial(delta);
    glColor3ub(153,153,153);//grey
    drawTextCenter(cFont,w/2,h-5,"press SPACE to game");
    if(blink_food)
        glColor3ub(255,0,0);//red
    drawTextCenter(cFont,w/2,posYtopGrid(),"need make intersect");
    if(keySpace)
    {
        startGame();
        state = WAIT_KEY_PRESS;
    }
        break;

    }// end switch

    keySpace = FALSE;// всегда сбрасываю флаг нажатия пробела,чтоб небыло мигалки

    return TRUE;                                        // Everything Went OK
}

void keyDown(int key)
{
    if(state == TUTORIAL) return;
    vec2 tmp = dir;
    switch(key)
    {
    case KEY_LEFT:
        dir.x = -1;
        dir.y = 0;
        break;
    case KEY_RIGHT:
        dir.x = 1;
        dir.y = 0;
        break;
    case KEY_UP:
        dir.x = 0;
        dir.y = -1;
        break;
    case KEY_DOWN:
        dir.x = 0;
        dir.y = 1;
        break;
    }
    // запрет на движение обратно на хвост
    if(snake_len > 1)
    if((snake[0].x + dir.x == snake[1].x) &&
       (snake[0].y + dir.y == snake[1].y) )
    {
        dir = tmp;
    }
}

void keyUp(int key)
{
    if(key == KEY_SPACE)
        keySpace = TRUE;
    if(key == KEY_M)
    {
        if(state == GAME || state == GAME_FINISH)
            sound_mute();
    }

}
// если скрылось окно
void hide()
{
    sound_pause();
    if(state == GAME)
    state = PAUSE;
}
// если показалось окно
void show()
{
    if(state == GAME_FINISH)// музыка включится только если игра пройдена
    sound_resume();
}

void destroy()
{
    sound_stop();
}
