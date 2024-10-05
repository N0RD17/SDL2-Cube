#include <SDL2/SDL.h>

#include <stdio.h>
#include <math.h>

#define width 640 / 2
#define height 480 / 2

float A=0.0f, B=0.0f, C=0.0f;

float cubeWidth = 60;
//int width = 160, height = 44;
float zBuffer[width * height];

// Structure to store both the SDL_Rect and the Color for each pixel
struct PixelBuffer {
    SDL_Rect rect;
    SDL_Color color;
};

struct PixelBuffer buffer[width * height];
int distanceFromCam = 200;
float horizontalOffset = 0.0f;
float K1 = 120;

float incrementSpeed = 0.9;

float x=0.0f, y=0.0f, z=0.0f;
float ooz = 0.0f;
int xp=0, yp=0;
int idx=0;

float calculateX(int i, int j, int k)
{
    return j * sin(A) * sin(B) * cos(C) -
        j * cos(A) * sin(C) + k * sin(A) * sin(C) +
        i * cos(B) * cos(C);
}

float calculateY(int i, int j, int k)
{
    return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
        j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
        i * cos(B) * sin(C);
}

float calculateZ(int i, int j, int k)
{
    return k * cos(A) * cos(B) - j * sin(A) *
        cos(B) + i * sin(B);
}

void calculateForSurface(float cubeX, float cubeY, float cubeZ, SDL_Rect *rect, SDL_Color* color)
{
    x = calculateX(cubeX, cubeY, cubeZ);
    y = calculateY(cubeX, cubeY, cubeZ);
    z = calculateZ(cubeX, cubeY, cubeZ) + distanceFromCam;

    ooz = 1/z;

    xp = (int)((float)width/2  + K1 * ooz * x * 2);
    yp = (int)((float)height/2 + K1 * ooz * y);

    idx = xp + yp * width;
    if(idx >= 0 && idx < width * height) {
        if(ooz > zBuffer[idx])
        {
            zBuffer[idx] = ooz;

            // Update rect position for each point
            rect->x = xp;
            rect->y = yp;
            buffer[idx].rect = *rect;
            buffer[idx].color = *color;
        }
    }
}

int main(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_INIT ERROR: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Pixeler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if(window == NULL) {
        fprintf(stderr, "SDL_CreateWindow ERROR: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(window == NULL) {
        fprintf(stderr, "SDL_CreateRenderer ERROR: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Rect RectPix;
    RectPix.w = 8;
    RectPix.h = 8;

    SDL_Color RedColor    = {.r=0xFF, .g=0x00, .b=0x00, .a=0xFF};
    SDL_Color GreenColor  = {.r=0x00, .g=0xFF, .b=0x00, .a=0xFF};
    SDL_Color BlueColor   = {.r=0x00, .g=0x00, .b=0xFF, .a=0xFF};
    SDL_Color YellowColor = {.r=0xFF, .g=0xFF, .b=0x00, .a=0xFF};
    SDL_Color PinkColor   = {.r=0xFF, .g=0x00, .b=0xFF, .a=0xFF};
    SDL_Color TealColor   = {.r=0x00, .g=0xFF, .b=0xFF, .a=0xFF};
    

    SDL_bool run = SDL_TRUE;
    SDL_Event event;
    while(run)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:{
                    run = SDL_FALSE;
                }break;

                default:break;
            }
        }

//        memset(zBuffer, 0, width * height * 4);
        for(int i = 0; i < width * height; i++)
        {
            zBuffer[i] = -INFINITY;
            buffer[i].rect.x = buffer[i].rect.y = buffer[i].rect.w =
                buffer[i].rect.h = 0;
            buffer[i].color.r = buffer[i].color.g = buffer[i].color.b =
                buffer[i].color.a = 0;
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        for(float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += incrementSpeed)
        {
            for(float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += incrementSpeed)
            {
                calculateForSurface(cubeX, cubeY, cubeWidth, &RectPix,
                                    &RedColor);
                calculateForSurface(cubeWidth, cubeY, cubeX, &RectPix,
                                    &BlueColor);   // (-z y x)
                calculateForSurface(-cubeWidth, cubeY, -cubeX, &RectPix,
                                    &GreenColor); // (z y -x)
                calculateForSurface(-cubeX, cubeY, -cubeWidth, &RectPix,
                                    &YellowColor);  // (-x y -z)
                calculateForSurface(cubeX, cubeWidth, -cubeY, &RectPix,
                                    &PinkColor);  // (x z -y)
                calculateForSurface(cubeX, -cubeWidth, cubeY, &RectPix,
                                    &TealColor);  // (x -z y)
            }
        }

        for(int k = 0; k < width * height; k++)
        {
            SDL_SetRenderDrawColor(renderer, buffer[k].color.r, buffer[k].color.g, buffer[k].color.b, buffer[k].color.a);
            SDL_RenderFillRect(renderer, &buffer[k].rect);
        }

        SDL_RenderPresent(renderer);

        A += 0.005;
        B += 0.055;
//        C += 0.005; // Hurts THE EYES
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
