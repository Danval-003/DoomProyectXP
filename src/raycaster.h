#pragma once

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


const Color B = {0, 0, 0};
const Color W = {255, 255, 255};

const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;
const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;


struct Player {
  int x;
  int y;
  float a;
  float fov;
}; 

struct Impact {
  float d;
  std::string mapHit;  // + | -
  int tx;
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
    return Impact{d, mapHit, tx};
  }

  void draw_stake(int x, float h, Impact i) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f;
    float end = start + h;

    for (int y = start; y < end; y++) {
      int ty = (y - start) * tsize / h;
      Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

      SDL_RenderDrawPoint(renderer, x, y);
    }
  } 
 
  void render() {
    
    // draw left side of the screen



    // draw right side of the screen
    
    for (int i = 1; i < SCREEN_WIDTH; i++) {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a);
      float d = impact.d;
      Color c = Color(255, 0, 0);

      if (d == 0) {
        print("you lose");
        exit(1);
      }
      int x =i;
      float h = static_cast<float>(SCREEN_HEIGHT)/static_cast<float>(d) * static_cast<float>(scale);
      draw_stake(x, h, impact);
    }

    int xD =(player.x-3*BLOCK/2 >0 )? player.x-3*BLOCK/2: 0;
    int yD =(player.y-3*BLOCK/2 >0 )? player.y-3*BLOCK/2: 0;
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
private:
  int scale;
  SDL_Renderer* renderer;
  std::vector<std::string> map;
  int tsize;
};
