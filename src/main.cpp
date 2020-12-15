#include <allegro.h>
#include <math.h>

#define blank 0
#define full 1
#define shot 2

BITMAP* buffer;
BITMAP* gun;
BITMAP* cylinder;
BITMAP* python;
BITMAP* background;
BITMAP* wheel;
BITMAP* cursor;
BITMAP* bullet;
BITMAP* bullet_shot;
BITMAP* python_open;
BITMAP* trigger;
BITMAP* hammer;


SAMPLE* click;
SAMPLE* click_fast;
SAMPLE* shell_click;
SAMPLE* shell_drop;
SAMPLE* wheel_close;
SAMPLE* wheel_open;
SAMPLE* dryfire;
SAMPLE* fire;

int window;
float angle;
float angle_255;
float angle_trigger;
float speed;
float recoil;
int old_mouse_z;
int new_mouse_z;
int clickLoop;
int wheel_loop;
int toggle_loop;
int trigger_loop;
int click_x;
int wheelLocation;
int step;
int button_step;

bool wheel_in;
bool spin;
bool trigger_click;

bool sound=true;

bool close_button_pressed;

struct shells{
   int state;
}shell[8];

void close_button_handler(void){
  close_button_pressed = TRUE;
}
END_OF_FUNCTION(close_button_handler)

int roundUp(int numToRound, int multiple) {
  if(numToRound < 0){
    multiple = multiple*(-1);
  }
  if(multiple == 0) {
    return numToRound;
  }

  int remainder = numToRound % multiple;
  if (remainder == 0){
    return numToRound;
  }
  return numToRound + multiple - remainder;
}

//Random number generator. Use int random(highest,lowest);
int random(int newLowest, int newHighest){
  int lowest = newLowest, highest = newHighest;
  int range = (highest - lowest) + 1;
  int randomNumber = lowest+int(range*rand()/(RAND_MAX + 1.0));
  return randomNumber;
}

//Check to see if an area is clicked
bool location_clicked(int min_x,int max_x,int min_y,int max_y){
    if(mouse_x>min_x && mouse_x<max_x && mouse_y>min_y && mouse_y<max_y && mouse_b & 1)
        return true;
    else return false;
}

bool location_clicked(BITMAP* buffer,int color,int min_x,int max_x,int min_y,int max_y){
    rectfill(buffer,min_x,min_y,max_x,max_y,color);
    if(mouse_x>min_x && mouse_x<max_x && mouse_y>min_y && mouse_y<max_y && mouse_b & 1)
        return true;
    else return false;
}

void game(){
  // Pull back trigger
  if(mouse_b & 1 && mouse_x<680 && mouse_x>550&& mouse_y>200 && mouse_y<400 &&  trigger_loop>9){
    position_mouse(620,280);
    click_x=mouse_x;
    trigger_loop=0;
    trigger_click=true;
  }
  if( mouse_b & 1 && mouse_x>click_x  && trigger_click){
    angle_trigger += click_x - mouse_x;
    click_x = mouse_x;
  }
  if( !mouse_b & 1){
    trigger_loop++;
    trigger_click=false;
    if( !key[KEY_SPACE]){
      angle_trigger=0;
    }
  }

  // Shoot
  if( key[KEY_SPACE]){
    angle_trigger -= 2;
  }
  else{
    if( !mouse_b & 1){
      angle_trigger -= 0;
    }
  }
  if(angle_trigger<-25){
    trigger_click=false;
    angle_trigger=0;

    if(!wheel_in){
      if(sound)play_sample(dryfire,255,125,1000,0);
    }
    else{
      if( shell[wheelLocation].state == full){
        if(sound)play_sample(fire,255,125,1000,0);
        recoil = 10;
        shell[wheelLocation].state = shot;
      }
      else{
        if(sound)play_sample(dryfire,255,125,1000,0);
      }
      angle += 32;
    }
  }

  // Restart angle
  if( angle > 224){
    angle = 0;
  }
  if( angle < 0){
    angle = 224;
  }

  // Restart recoil
  if( recoil != 0){
    recoil /= 1.2;
  }

  // Round angle
  if(speed == 0){
    angle = roundUp( angle, 32);
  }
  else{
    if( speed > 0){
      if( (int)speed == clickLoop ){
        play_sample(click,255,125,1000,0);
        clickLoop -= 1;
      }
      speed /= 3;
      if( speed < 0.01){
        speed = 0;
      }
    }
    else{
      if( (int)speed == clickLoop ){
        if(sound)play_sample(click,255,125,1000,0);
        clickLoop += 1;
      }
      speed /= 3;
      if( speed > -0.01){
        speed = 0;
      }
    }
  }

  //Location for in the chamber
  wheelLocation = angle/32 + 1;


  // Allows for spinning
  if( step > 10){
    // Spin barrel
    if(!wheel_in){
      if(mouse_z > old_mouse_z){
        if( mouse_z - old_mouse_z){
          angle += 32;
        }
        else{
          speed += pow(mouse_z - old_mouse_z, 3);
          clickLoop = speed;
        }
      }
      if(mouse_z < old_mouse_z){
        if( old_mouse_z - mouse_z){
          angle -= 32;
        }
        else{
          speed -= pow(old_mouse_z - mouse_z, 3);
          clickLoop = speed;
        }
      }
    }
    old_mouse_z = mouse_z;
    step = 0;
  }
  step ++;

  // Spin some more
  angle += speed * 32;
  angle_255 += speed * 32;

  if(angle_255>255 && angle_255>0)angle_255-=255;
  if(angle_255>-255 && angle_255<0)angle_255+=255;

  if(wheel_loop>9 && !wheel_in && key[KEY_E]){
      wheel_in=true; wheel_loop=0;
      if(sound)play_sample(wheel_close,255,125,1000,0);
  }

  if(wheel_loop>9 && wheel_in && key[KEY_E]){
      wheel_in=false; wheel_loop=0;
      if(sound)play_sample(wheel_open,255,125,1000,0);
  }

  wheel_loop++;



  // Put in some bullets
  if(key[KEY_1] && toggle_loop>9 && shell[1].state==blank){shell[1].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_1] && toggle_loop>9 && shell[1].state!=blank){shell[1].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_2] && toggle_loop>9 && shell[2].state==blank){shell[2].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_2] && toggle_loop>9 && shell[2].state!=blank){shell[2].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_3] && toggle_loop>9 && shell[3].state==blank){shell[3].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_3] && toggle_loop>9 && shell[3].state!=blank){shell[3].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_4] && toggle_loop>9 && shell[4].state==blank){shell[4].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_4] && toggle_loop>9 && shell[4].state!=blank){shell[4].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_5] && toggle_loop>9 && shell[5].state==blank){shell[5].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_5] && toggle_loop>9 && shell[5].state!=blank){shell[5].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_6] && toggle_loop>9 && shell[6].state==blank){shell[6].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_6] && toggle_loop>9 && shell[6].state!=blank){shell[6].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_7] && toggle_loop>9 && shell[7].state==blank){shell[7].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_7] && toggle_loop>9 && shell[7].state!=blank){shell[7].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}
  if(key[KEY_8] && toggle_loop>9 && shell[8].state==blank){shell[8].state=full; toggle_loop=0; if(sound)play_sample(shell_click,255,125,1000,0);}
  if(key[KEY_8] && toggle_loop>9 && shell[8].state!=blank){shell[8].state=blank; toggle_loop=0; if(sound)play_sample(shell_drop,255,125,random(600,1200),0);}

  toggle_loop++;



  // Draw some stuff
  rectfill( gun, 0, 0, 1000, 540, makecol( 255, 0, 255));
  draw_sprite(buffer,background,0,0);
  //rectfill(buffer,0,0,SCREEN_W,SCREEN_H,makecol(255,255,255));

  rotate_sprite(gun, hammer, 554, 26, itofix(angle_trigger-angle_trigger*2));
  rotate_sprite(gun, trigger, 576, 185, itofix(angle_trigger));
  if(wheel_in)draw_sprite(gun,python,0,0);
  if(!wheel_in)draw_sprite(gun,python_open,0,0);

  if(!wheel_in){
    rectfill( cylinder, 0, 0, 200, 200, makecol( 255, 0, 255));
    draw_sprite(cylinder, wheel, 0, 0);
    if(shell[1].state == full)draw_sprite(cylinder, bullet, 76, 9);
    if(shell[8].state == full)draw_sprite(cylinder, bullet, 120, 29);
    if(shell[7].state == full)draw_sprite(cylinder, bullet, 138, 72);
    if(shell[6].state == full)draw_sprite(cylinder, bullet, 122, 118);
    if(shell[5].state == full)draw_sprite(cylinder, bullet, 78, 137);
    if(shell[4].state == full)draw_sprite(cylinder, bullet, 32, 120);
    if(shell[3].state == full)draw_sprite(cylinder, bullet, 14, 74);
    if(shell[2].state == full)draw_sprite(cylinder, bullet, 31, 29);

    if(shell[1].state == shot)draw_sprite(cylinder, bullet_shot, 76, 9);
    if(shell[8].state == shot)draw_sprite(cylinder, bullet_shot, 120, 29);
    if(shell[7].state == shot)draw_sprite(cylinder, bullet_shot, 138, 72);
    if(shell[6].state == shot)draw_sprite(cylinder, bullet_shot, 122, 118);
    if(shell[5].state == shot)draw_sprite(cylinder, bullet_shot, 78, 137);
    if(shell[4].state == shot)draw_sprite(cylinder, bullet_shot, 32, 120);
    if(shell[3].state == shot)draw_sprite(cylinder, bullet_shot, 14, 74);
    if(shell[2].state == shot)draw_sprite(cylinder, bullet_shot, 31, 29);

    rotate_sprite( buffer, cylinder, 40, 250, itofix(angle));
  }

  //Sound button (in primitives, because screw bitmaps)
  rectfill(buffer,400,500,440,539,makecol(255,255,255));
  rect(buffer,400,500,440,539,makecol(0,0,0));

  if(sound){
    rectfill(buffer,410,515,420,525,makecol(0,0,0));
    triangle(buffer,410,520,423,530,423,510,makecol(0,0,0));
    line(buffer,425,505,430,510,makecol(0,0,0));
    line(buffer,430,510,430,530,makecol(0,0,0));
    line(buffer,430,530,425,535,makecol(0,0,0));
    line(buffer,427,502,435,510,makecol(0,0,0));
    line(buffer,435,510,435,530,makecol(0,0,0));
    line(buffer,435,530,428,537,makecol(0,0,0));
  }

  if(!sound){
    rectfill(buffer,410,515,420,525,makecol(255,0,0));
    triangle(buffer,410,520,423,530,423,510,makecol(255,0,0));
  }

  //Click handler for sound button
  if(location_clicked(400,440,500,540) && button_step>9){
    button_step=0;
    sound=!sound;
  }
  //Clear button
  rectfill(buffer,440,500,490,539,makecol(255,255,255));
  rect(buffer,440,500,490,539,makecol(0,0,0));
  textprintf_ex(buffer,font,445,517,makecol(0,0,0),makecol(0,-1,0),"Clear");

  //Click handler for clear button
  if(location_clicked(440,490,500,540) && button_step>9){
    button_step=0;
    wheel_in=false;
    if(sound){
      play_sample(shell_drop,255,125,1000,0);
      play_sample(shell_drop,255,125,750,0);
      play_sample(shell_drop,255,125,1250,0);
    }
    for(int i=0; i<9; i++){
      shell[i].state=blank;
    }
  }
  //Fill button
  rectfill(buffer,490,500,530,539,makecol(255,255,255));
  rect(buffer,490,500,530,539,makecol(0,0,0));
  textprintf_ex(buffer,font,495,517,makecol(0,0,0),makecol(0,-1,0),"Fill");

  //Click handler for fill button
  if(location_clicked(490,530,500,540) && button_step>9){
    button_step=0;
    if(sound){
      play_sample(shell_click,255,125,1000,0);
      play_sample(shell_click,255,125,750,0);
      play_sample(shell_click,255,125,1250,0);
    }
    for(int i=0; i<9; i++){
      shell[i].state=full;
    }
  }
  //Clickable wheel
  if(location_clicked(510,650,70,250)&& button_step>9){
    wheel_in=!wheel_in;
    button_step=0;
    if(sound && !wheel_in)play_sample(wheel_open,255,125,1000,0);
    if(sound && wheel_in)play_sample(wheel_close,255,125,1000,0);
  }


  pivot_sprite(buffer,gun, 740, 190, 740, 190, itofix(recoil));
  //textprintf_ex(buffer,font,20,20,makecol(255,255,255),makecol(0,-1,0),"wheelLocation:%i",wheelLocation );

  //Keys 1-8 instructions
  rectfill(buffer,345,485,610,500,makecol(255,255,255));
  rect(buffer,345,485,610,500,makecol(0,0,0));
  textprintf_ex(buffer,font,350,490,makecol(0,0,0),makecol(0,-1,0),"Keys 1-8 to insert/remove shells",wheelLocation );



  draw_sprite(buffer,cursor,mouse_x,mouse_y);
  draw_sprite(screen,buffer,0,0);

  button_step++;

  rest(10);
}

void setup(){
  //Create the buffer
  buffer = create_bitmap( 1000, 540);
  gun = create_bitmap( 1000, 540);
  cylinder = create_bitmap( 200, 200);

  wheelLocation = 1;

  // Close button
  LOCK_FUNCTION(close_button_handler);
  set_close_button_callback(close_button_handler);

  //Import bitmap images
  if (!(cursor = load_bitmap("images/cursor.bmp", NULL))){
    allegro_message("Cannot find image cursor.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(python = load_bitmap("images/python.bmp", NULL))){
    allegro_message("Cannot find image python.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(background = load_bitmap("images/background.bmp", NULL))){
    allegro_message("Cannot find image background.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(wheel = load_bitmap("images/wheel.bmp", NULL))){
    allegro_message("Cannot find image wheel.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(bullet = load_bitmap("images/bullet.bmp", NULL))){
    allegro_message("Cannot find image bullet.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(bullet_shot = load_bitmap("images/bullet_shot.bmp", NULL))){
    allegro_message("Cannot find image bullet_shot.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(python_open = load_bitmap("images/python_open.bmp", NULL))){
    allegro_message("Cannot find image python_open.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(trigger = load_bitmap("images/trigger.bmp", NULL))){
    allegro_message("Cannot find image trigger.bmp\nPlease check your files and try again");
  exit(-1);}
  if (!(hammer = load_bitmap("images/hammer.bmp", NULL))){
    allegro_message("Cannot find image hammer.bmp\nPlease check your files and try again");
  exit(-1);}


  //Import sounds
  if (!(click = load_sample( "sfx/click.wav"))){
   allegro_message("Cannot find sound sfx/click.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(click_fast = load_sample( "sfx/click_fast.wav"))){
   allegro_message("Cannot find sound sfx/click_fast.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(shell_click = load_sample( "sfx/shell_click.wav"))){
   allegro_message("Cannot find sound sfx/shell_click.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(shell_drop = load_sample( "sfx/shell_drop.wav"))){
   allegro_message("Cannot find sound sfx/shell_drop.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(wheel_close = load_sample( "sfx/wheel_close.wav"))){
   allegro_message("Cannot find sound sfx/wheel_close.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(wheel_open = load_sample( "sfx/wheel_open.wav"))){
   allegro_message("Cannot find sound sfx/wheel_open.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(dryfire = load_sample( "sfx/dryfire.wav"))){
   allegro_message("Cannot find sfx/dryfire.wav\nPlease check your files and try again");
  exit(-1);}
  if (!(fire = load_sample( "sfx/fire.wav"))){
   allegro_message("Cannot find sfx/fire.wav\nPlease check your files and try again");
  exit(-1);}
}
int main(){
  //Initializing
  allegro_init();
  install_keyboard();
  install_mouse();
  set_color_depth(32);
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1000, 540, 0, 0);
  install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,".");

  set_window_title("Error!");
  //Run setup void
  setup();
  set_window_title("Russian Roulette");

  //Run game until ESC key is pressed
  while(!key[KEY_ESC] && !close_button_pressed ){
    game();
  }
  return 0;
}
END_OF_MAIN()
