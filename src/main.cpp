#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL2/SDL_mixer.h>
#include <print.h>

#include "color.h"
#include "imageloader.h"
#include "raycaster.h"

SDL_Window* window;
SDL_Renderer* renderer;

// Agrega una variable global para rastrear si el sonido de pasos está en reproducción
bool pasosPlaying = false;

void clear() {
  SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
  SDL_RenderClear(renderer);
}

void draw_floor() {
    // Define el color base del piso
    Color floorColor = {40, 30, 20, 255};

    // Loop a través de las filas de píxeles en el piso
    for (int y = SCREEN_HEIGHT / 2; y < SCREEN_HEIGHT; y++) {
        // Calcula la distancia relativa desde el centro de la pantalla
        float distanceFromCenter = static_cast<float>(y - SCREEN_HEIGHT / 2) / (SCREEN_HEIGHT * 2.0f );

        // Ajusta el color en función de la distancia desde el centro
        Color adjustedColor;
        adjustedColor.r = floorColor.r *  distanceFromCenter;
        adjustedColor.g = floorColor.g * distanceFromCenter;
        adjustedColor.b = floorColor.b * distanceFromCenter;
        adjustedColor.a = floorColor.a;

        // Dibuja una línea horizontal en el piso con el color ajustado
        SDL_SetRenderDrawColor(renderer, adjustedColor.r, adjustedColor.g, adjustedColor.b, adjustedColor.a);
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }
}

void draw_roof() {
    // Define el color base del piso
    Color floorColor = {25, 30, 40, 255};

    // Loop a través de las filas de píxeles en el piso
    for (int y = 0; y < SCREEN_HEIGHT / 2; y++) {
        // Calcula la distancia relativa desde el centro de la pantalla
        float distanceFromCenter = static_cast<float>(SCREEN_HEIGHT/2 - y) / (SCREEN_HEIGHT * 2);

        // Ajusta el color en función de la distancia desde el centro
        Color adjustedColor;
        adjustedColor.r = floorColor.r *  distanceFromCenter;
        adjustedColor.g = floorColor.g * distanceFromCenter;
        adjustedColor.b = floorColor.b * distanceFromCenter;
        adjustedColor.a = floorColor.a;

        // Dibuja una línea horizontal en el piso con el color ajustado
        SDL_SetRenderDrawColor(renderer, adjustedColor.r, adjustedColor.g, adjustedColor.b, adjustedColor.a);
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }
}

void ChannelFinishedCallback(int channel) {
    if (channel == 0) {
      pasosPlaying = false;
    }
}

int main() {
  print("hello world");

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cout<<"Hay error"<<std::endl;
  }

  Mix_Music* backgroundMusic = Mix_LoadMUS("assets/fondo.mp3");
  Mix_Chunk* pasos = Mix_LoadWAV("assets/pasos2.wav");
  if (!backgroundMusic && !pasos) {
    std::cout<<"Hay error"<<std::endl;
  }
  ImageLoader::init();

  window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  ImageLoader::loadImage("+", "assets/sP2.png");
  ImageLoader::loadImage("-", "assets/sP.png");
  ImageLoader::loadImage("|", "assets/sP2.png");
  ImageLoader::loadImage("*", "assets/bloodWall.png");
  ImageLoader::loadImage("g", "assets/pWalls.png");
  ImageLoader::loadImage("h", "assets/floor.png" );
  ImageLoader::loadImage("<>", "assets/lanterns/lan0.png");
  ImageLoader::loadImage("<", "assets/lanterns/lan+1.png");
  ImageLoader::loadImage(">", "assets/lanterns/lan-1.png" );

  Raycaster r = { renderer };
  r.load_map("assets/map.txt");

  bool running = true;
  std::string playertexture= "<>";
  int previousMouseX = 0;
  int speed = 2;
  SDL_ShowCursor(SDL_DISABLE);  // Oculta el cursor
  //SDL_SetRelativeMouseMode(SDL_TRUE);
  float newPlayerX = 0.0f;
  float newPlayerY = 0.0f;
  Uint32 frameStart = SDL_GetTicks();;      // Tiempo de inicio del cuadro actual
  Uint32 frameTime;       // Tiempo transcurrido en el cuadro actual
  int frameCount = 0;     // Contador de cuadros renderizados
  int fps = 0;

  if (SDL_NumJoysticks() > 0) {
    SDL_Joystick* joystick = SDL_JoystickOpen(0); // Abre el primer joystick disponible
    if (joystick) {
      SDL_JoystickEventState(SDL_ENABLE); // Habilita los eventos del joystick

      // Puedes imprimir el nombre del joystick para referencia
      printf("Joystick Name: %s\n", SDL_JoystickName(joystick));
    }
  }
  int horizontalMotion = 0; // Rastrea el movimiento horizontal del joystick
  int verticalMotion = 0;   // Rastrea el movimiento vertical del joystick

  Mix_PlayMusic(backgroundMusic, -1);  // El segundo parámetro "-1" indica reproducción en bucle infinito

  // Función para manejar eventos de finalización de canales de audio


  // Registra la función de devolución de llamada para eventos de finalización de canales
  Mix_ChannelFinished(ChannelFinishedCallback);

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      newPlayerY = r.player.y + speed * sin(r.player.a);
      playertexture= "<>";

      if (event.type == SDL_JOYAXISMOTION) {
        if (event.jaxis.axis == 0) {
          playertexture= "<>";
          if (event.jaxis.value < 0) {
            // Eje X del joystick (izquierda-derecha) para girar
            r.player.a -= M_PI / 5000000 * event.jaxis.value;
            playertexture= "<";
            r.incert_ang_x = -0.05f;
          } else if (event.jaxis.value > 0) {
            // Eje X del joystick (izquierda-derecha) para girar
            r.player.a -= M_PI / 5000000 * event.jaxis.value;
            playertexture= ">";
            r.incert_ang_x = 0.05f;
          } else{
            playertexture= "<>";
          }
        }else {
          playertexture= "<>";
          r.incert_ang_x = 0.0f;
        }

        if (event.jaxis.axis == 1) {
          // Eje Y del joystick (arriba-abajo) para avanzar o retroceder
          if (event.jaxis.value < -10000) {
            // Avanzar
            r.incertidumbreX += M_PI/12;
            r.incertidumbreY += M_PI/8;
            newPlayerX = r.player.x + speed * cos(r.player.a);

            if (!r.isWallCollision(newPlayerX, newPlayerY)) {
              r.player.x = newPlayerX;
              r.player.y = newPlayerY;
            }

            if(!pasosPlaying){
              Mix_PlayChannel(-1, pasos, 0);
              pasosPlaying = true;
            }
          } else if (event.jaxis.value > 10000) {
            // Retroceder
            r.incertidumbreX += M_PI/12;
            r.incertidumbreY += M_PI/8;
            newPlayerX = r.player.x - speed * cos(r.player.a);
            newPlayerY = r.player.y - speed * sin(r.player.a);

            if (!r.isWallCollision(newPlayerX, newPlayerY)) {
              r.player.x = newPlayerX;
              r.player.y = newPlayerY;
            }

            if(!pasosPlaying){
              Mix_PlayChannel(-1, pasos, 0);
              pasosPlaying = true;
            }
          }
        }
      } else {
        playertexture= "<>";
      }

      if (event.type == SDL_KEYDOWN) {
        playertexture= "<>";
        if(event.key.keysym.sym == SDLK_LEFT){
          r.player.a += M_PI/50;
          playertexture= "<";
          r.incert_ang_x = -0.05f;
        }

        if(event.key.keysym.sym == SDLK_RIGHT){
          r.player.a -= M_PI/50;
          playertexture= ">";
          r.incert_ang_x = 0.05f;
        }

        if(event.key.keysym.sym == SDLK_w){
          newPlayerX = r.player.x;
          newPlayerY = r.player.y;
          r.incertidumbreX += M_PI/12;
          r.incertidumbreY += M_PI/8;
          newPlayerX += speed * cos(r.player.a);
          newPlayerY += speed * sin(r.player.a);


          if (!r.isWallCollision(newPlayerX, newPlayerY)) {
            r.player.x = newPlayerX;
            r.player.y = newPlayerY;
          }

          if(!pasosPlaying){
            Mix_PlayChannel(-1, pasos, 0);
            pasosPlaying = true;
          }
        }

        if(event.key.keysym.sym == SDLK_s){
          newPlayerX = r.player.x;
          newPlayerY = r.player.y;
          r.incertidumbreX -= M_PI/12;
          r.incertidumbreY += M_PI/8;
          newPlayerX -= speed * cos(r.player.a);
          newPlayerY -= speed * sin(r.player.a);


          if (!r.isWallCollision(newPlayerX, newPlayerY)) {
            r.player.x = newPlayerX;
            r.player.y = newPlayerY;
          }

          if(!pasosPlaying){
            Mix_PlayChannel(-1, pasos, 0);
            pasosPlaying = true;
          }
        }


        if(event.key.keysym.sym != SDLK_LEFT &&   event.key.keysym.sym != SDLK_RIGHT){
          playertexture= "<>";
        }
      }

      if (event.type == SDL_MOUSEMOTION) {
        // Obtener la diferencia entre la posición anterior y la actual del mouse
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        int deltaX = (mouseX > SCREEN_WIDTH/2) ? 1 : -1;
        deltaX = (mouseX == SCREEN_WIDTH/2) ? 0 : deltaX;

        // Ajustar la velocidad de rotación según la posición del mouse
        double rotationSpeed = - M_PI / 200;
        r.player.a += rotationSpeed * deltaX;
        SDL_WarpMouseInWindow(NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT / 2);
        // Actualizar la posición anterior del mouse
      }


    }

    clear();
    draw_roof();
    draw_floor();

    r.render();
    r.draw_ui(playertexture,100,125 );

    // render

    SDL_RenderPresent(renderer);
    frameTime = SDL_GetTicks() - frameStart;
    frameCount++;
    if (frameTime >= 1000) {
      fps = frameCount;
      frameCount = 0;
      frameStart = SDL_GetTicks(); // Reinicia el tiempo de inicio para el siguiente segundo
    }
    std::string fpsText = "FPS: " + std::to_string(fps);
    SDL_SetWindowTitle(window, fpsText.c_str());
  }

  Mix_FreeChunk(pasos);
  Mix_FreeMusic(backgroundMusic); // Libera la música cargada
  Mix_CloseAudio(); // Cierra el sistema de audio de SDL_mixer

  SDL_DestroyWindow(window);
  SDL_Quit();
}
