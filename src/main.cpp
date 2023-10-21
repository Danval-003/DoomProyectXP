#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <print.h>

#include "color.h"
#include "imageloader.h"
#include "raycaster.h"

SDL_Window* window;
SDL_Renderer* renderer;

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


int main() {
  print("hello world");

  SDL_Init(SDL_INIT_VIDEO);
  ImageLoader::init();

  window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  ImageLoader::loadImage("+", "assets/wall3.png");
  ImageLoader::loadImage("-", "assets/wall1.png");
  ImageLoader::loadImage("|", "assets/wall2.png");
  ImageLoader::loadImage("*", "assets/wall4.png");
  ImageLoader::loadImage("g", "assets/wall5.png");
  ImageLoader::loadImage("h", "assets/floor.png" );

  Raycaster r = { renderer };
  r.load_map("assets/map.txt");

  bool running = true;
  int previousMouseX = 0;
  int speed = 3;
  SDL_ShowCursor(SDL_DISABLE);  // Oculta el cursor
  //SDL_SetRelativeMouseMode(SDL_TRUE);
  float newPlayerX = 0.0f;
  float newPlayerY = 0.0f;

  while(running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      if (event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym ){
          case SDLK_LEFT:
            r.player.a += M_PI/500;
            break;
          case SDLK_RIGHT:
            r.player.a -= M_PI/500;
            break;
          case SDLK_w:
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
            break;
          case SDLK_s:
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
            break;
           default:
            break;
        }
      }

      if (event.type == SDL_MOUSEMOTION) {
    // Obtener la diferencia entre la posición anterior y la actual del mouse
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        int deltaX = (mouseX > SCREEN_WIDTH/2)? 1 : -1;
        deltaX = (mouseX == SCREEN_WIDTH/2)? 0:deltaX;

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

    // render

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
