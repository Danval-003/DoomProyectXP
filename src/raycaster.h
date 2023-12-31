#pragma once

#include <algorithm>
#include <array>
#include <oneapi/tbb/concurrent_vector.h>
#include <oneapi/tbb/parallel_for.h>
#include <print.h>
#include <iostream>
#include <fstream>
#include <SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageloader.h"
#include <thread>
#include <mutex>
#include <tbb/tbb.h>
#include <tbb/concurrent_vector.h>


const Color B = {0, 0, 0};
const Color W = {255, 255, 255};


const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;
const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;


struct Player {
  float x;
  float y;
  float a;
  float fov;
};

struct Impact {
  float d;
  std::string mapHit;  // + | -
  int tx;
  float a;
};

struct Point {
  float x;
  float y;
  Color c;
};


class Raycaster {
public:
  Raycaster(SDL_Renderer* renderer)
    : renderer(renderer) {


    player.x = BLOCK + BLOCK / 2;
    player.y = BLOCK + BLOCK / 2;

    player.a = M_PI / 4.0f;
    player.fov = M_PI / 3.0f;

    scale = 50;
    tsize = 128;
    incertidumbreY = 0.0f;
    incertidumbreX = 0.0f;
  }

  void load_map(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
      map.push_back(line);
    }
    file.close();
  }

  void point(int x, int y, Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(renderer, x, y);
  }

  void drawPixel(int x, int y, const std::string& mapHit, double h, double f) {
      int tx = static_cast<int>((h * tsize));
      int ty = static_cast<int>((f * tsize));

      Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
      SDL_RenderDrawPoint(renderer, x, y);
  }


  void rect(int x, int y, const std::string& mapHit) {
    for(int cx = x; cx < x + BLOCK; cx++) {
      for(int cy = y; cy < y + BLOCK; cy++) {
        int tx = ((cx - x) * tsize) / BLOCK;
        int ty = ((cy - y) * tsize) / BLOCK;

        Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
        SDL_RenderDrawPoint(renderer, cx/2, cy/2);
      }
    }
  }

  void draw_ui(std::string playerImage, int width, int height) {
    int size = 64;
    ImageLoader::render(renderer, playerImage, SCREEN_WIDTH / 2.0f - width / 2.0f, SCREEN_HEIGHT - height);
  }

  Impact cast_ray2(int i) {
    double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
    float d = 0;
    std::string mapHit;
    int tx;

    while(true) {
      int x = static_cast<int>( player.x+ d * cos(a));
      int y = static_cast<int>(player.y + d * sin(a));

      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);


      if (map[j][i] != ' ') {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1) {
          maxhit = hity;
        } else {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }


      d += 1;
    }
    return Impact{d, mapHit, tx, (float) a};
  }

  Impact cast_ray(float a) {
    float d = 0;
    std::string mapHit;
    int tx;

    while(true) {
      int x = static_cast<int>( player.x+ d * cos(a));
      int y = static_cast<int>(player.y + d * sin(a));

      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);


      if (map[j][i] != ' ') {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1) {
          maxhit = hity;
        } else {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }

      if (static_cast<int>(( 3*BLOCK/2+ d * cos(a)))<=4*BLOCK &&  static_cast<int>( (3*BLOCK/2+ d * sin(a)))<=4*BLOCK) {
        point(static_cast<int>(( 3*BLOCK/2+ d * cos(a))), static_cast<int>( (3*BLOCK/2+ d * sin(a))), W);
      }



      d += 1;
    }
    return Impact{d, mapHit, tx, a};
  }

  bool isWallCollision(float newX, float newY){
    bool isWall = false;
    if(newY> map.size() || newX>map[0].size()){
      int y = static_cast<int>((newY+0.01 *BLOCK)/BLOCK);
      int x = static_cast<int>((newX+0.01 *BLOCK)/BLOCK);

      isWall = map[y][x]!=' ';

      if(!isWall){
        y = static_cast<int>((newY-0.01 *BLOCK)/BLOCK);
        x = static_cast<int>((newX-0.01 *BLOCK)/BLOCK);

        isWall= map[y][x] != ' ';
      }

    }

    return isWall;
  }

  void draw_stake(int x, float h, Impact i, float angleDiff) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f ;
    float end = start + h ;
    const float limit = SCREEN_HEIGHT*0.20f;
    const float limitx = 0.20f;

    for (int y = start; y < end; y++) {
        int ty = (y - start) * tsize / h;
        Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);

        float xDiff = std::clamp(((limitx - angleDiff)/ limitx), 0.0f, 1.0f);

        float yDiif = std::clamp((limit - abs(SCREEN_HEIGHT/2 - (float) y))/limit, 0.0f, 1.0f);

        float realDiff = yDiif *xDiff;

        realDiff = (realDiff>0.80f)? 0.80f: realDiff;



        // Calcula el factor de atenuación basado en la distancia
        float attenuationFactor = std::min(abs(15.0f / i.d), 1.0f); // Puedes ajustar este factor según tus preferencias

        // Aplica el efecto de la linterna atenuando el color
        int lightIntensity = 255 * realDiff ; // Ajusta la intensidad de la linterna

        c.r = static_cast<int>(c.r * lightIntensity / 255) + static_cast<int>(c.b * attenuationFactor);
        c.g = static_cast<int>(c.g * lightIntensity / 255) + static_cast<int>(c.b * attenuationFactor);
        c.b = static_cast<int>(c.b * lightIntensity / 255) + static_cast<int>(c.b * attenuationFactor);

        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderDrawPoint(renderer, x+ 3* cos(incertidumbreX), y+ 3* cos(incertidumbreY));
    }
  }

  void render() {

    tbb::concurrent_vector<Impact> rays;
    std::vector<Impact> rays2;
    std::vector<Point> points2;
    std::vector<std::thread> threadsRays;

    for(int i=0; i<SCREEN_WIDTH; i++){
      Impact h;
      rays.push_back(h);
    }

    tbb::parallel_for(0, SCREEN_WIDTH, [this, &rays](int i){
      rays[i] = cast_ray2(i);
    });

    // draw right side of the screen

    for (int i = 1; i < SCREEN_WIDTH; i++) {
        Impact impact = rays[i];
        float d = impact.d;
        Color c = Color(255, 0, 0);

        if (d == 0) {
            print("you lose");
            exit(1);
        }

        int x = i;
        double cos_a_minus_player_a = cos(impact.a - player.a);
        double inv_d_cos = 1.0 / (d * cos_a_minus_player_a);
        float h = static_cast<float>(SCREEN_HEIGHT) * scale * inv_d_cos;

        // Nuevo: Calcula el ángulo del rayo actual
        double angle = player.a - player.fov / 2.0 + player.fov * i / SCREEN_WIDTH;

        // Nuevo: Calcula la diferencia de ángulo entre la dirección del rayo y la linterna
        double angleDiff = std::abs(angle - player.a);

        // Nuevo: Define el ángulo máximo del cono de luz de la linterna (en radianes)
        double maxLightAngle = 0.2; // Ajusta este valor según tu preferencia

        // Nuevo: Verifica si el rayo está dentro del cono de luz

        draw_stake(x, h, impact, angleDiff);
    }

    int xD =player.x-3*BLOCK/2;
    int yD =player.y-3*BLOCK/2;
    int sizeX = map[0].size();
    int sizeY = map.size();

    for (int x = 0; x < 4 * BLOCK; x += 1) {
        for (int y = 0; y <4 * BLOCK; y += 1) {
            double i = 1.0* (xD + x) / BLOCK;
            double j = 1.0* (yD + y) / BLOCK;

            if (i >= sizeX || j >= sizeY)
                continue;

            int i_floor = static_cast<int>(i);
            int j_floor = static_cast<int>(j);
            double h = i - i_floor;
            double f = j - j_floor;

            if (map[j_floor][i_floor] != ' ') {
                std::string mapHit;
                mapHit = map[j_floor][i_floor];
                Color c = Color(255, 0, 0);
                drawPixel(x, y, mapHit, h, f);
            } else {
              point(x,y ,B);
            }
        }
    }

    for (int i = 1; i < SCREEN_WIDTH; i++) {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a);
    }
  }


  Player player;
  float incertidumbreY;
  float incertidumbreX;
  float incert_ang_x;
private:
  std::mutex sdlMutex;
  int scale;
  SDL_Renderer* renderer;
  std::vector<std::string> map;
  int tsize;
};
