#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const int PLATFORM_COUNT = 10;

struct Point {
    int x, y;
};

// Структура для хранения всего контекста игры
struct GameContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* t1;
    SDL_Texture* t2;
    SDL_Texture* t3;
    TTF_Font* fontScore;
    TTF_Font* fontGameOver;
    
    Point plat[PLATFORM_COUNT];
    int x = 100;
    int y = 100;
    int h = 200;
    float dx = 0;
    float dy = 0;
    int score = 0;
    bool isGameOver = false;
    bool isRunning = true;
    
    SDL_Color colorBlack = { 0, 0, 0, 255 };
    SDL_Color colorRed = { 255, 0, 0, 255 };
};

// Функция отрисовки текста
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Главный цикл, который Emscripten будет вызывать каждый кадр
void main_loop(void* arg) {
    GameContext* ctx = static_cast<GameContext*>(arg);
    SDL_Event e;

    // --- ОБРАБОТКА СОБЫТИЙ ---
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            ctx->isRunning = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);

    // --- ЛОГИКА ИГРЫ ---
    if (!ctx->isGameOver) {
        if (state[SDL_SCANCODE_RIGHT]) ctx->x += 3;
        if (state[SDL_SCANCODE_LEFT]) ctx->x -= 3;

        if (ctx->x > 400) ctx->x = -70;
        if (ctx->x < -70) ctx->x = 400;

        ctx->dy += 0.2f;
        ctx->y += static_cast<int>(ctx->dy);

        if (ctx->y > 533) {
            ctx->isGameOver = true;
        }

        if (ctx->y < ctx->h) {
            for (int i = 0; i < PLATFORM_COUNT; i++) {
                ctx->y = ctx->h;
                ctx->plat[i].y = ctx->plat[i].y - static_cast<int>(ctx->dy);
                if (ctx->plat[i].y > 533) {
                    ctx->plat[i].y = 0;
                    ctx->plat[i].x = std::rand() % 400;
                }
            }
            ctx->score += 1;
        }

        for (int i = 0; i < PLATFORM_COUNT; i++) {
            if ((ctx->x + 70 > ctx->plat[i].x) && (ctx->x + 20 < ctx->plat[i].x + 75) &&
                (ctx->y + 80 > ctx->plat[i].y) && (ctx->y + 80 < ctx->plat[i].y + 20) && (ctx->dy > 0)) 
            {
                ctx->dy = -10.0f;
            }
        }
    } else {
        if (state[SDL_SCANCODE_RETURN]) { // ENTER
            ctx->isGameOver = false;
            ctx->score = 0;
            ctx->x = 100; ctx->y = 100;
            ctx->dy = 0;
            for (int i = 0; i < PLATFORM_COUNT; i++) {
                ctx->plat[i].x = std::rand() % 400;
                ctx->plat[i].y = std::rand() % 533;
            }
            ctx->plat[0].x = ctx->x;
            ctx->plat[0].y = ctx->y + 80;
        }
    }

    // --- ОТРИСОВКА ---
    SDL_RenderClear(ctx->renderer);

    SDL_Rect bgRect = { 0, 0, 400, 533 };
    SDL_RenderCopy(ctx->renderer, ctx->t1, NULL, &bgRect);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        SDL_Rect pRect = { ctx->plat[i].x, ctx->plat[i].y, 75, 20 };
        SDL_RenderCopy(ctx->renderer, ctx->t2, NULL, &pRect);
    }

    SDL_Rect doodleRect = { ctx->x, ctx->y, 70, 80 };
    SDL_RenderCopy(ctx->renderer, ctx->t3, NULL, &doodleRect);

    std::string scoreStr = "Score: " + std::to_string(ctx->score);
    renderText(ctx->renderer, ctx->fontScore, scoreStr, ctx->colorBlack, 10, 10);

    if (ctx->isGameOver) {
        renderText(ctx->renderer, ctx->fontGameOver, "GAME OVER!", ctx->colorRed, 110, 200);
        renderText(ctx->renderer, ctx->fontGameOver, "Press ENTER to restart", ctx->colorRed, 40, 240);
    }

    SDL_RenderPresent(ctx->renderer);
}

int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return -1;
    if (TTF_Init() == -1) return -1;

    GameContext ctx;

    ctx.window = SDL_CreateWindow("Doodle Game!",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  400, 533, SDL_WINDOW_SHOWN);
    if (!ctx.window) return -1;

    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx.renderer) return -1;

    // Пути к ассетам остаются прежними, Emscripten упакует их в виртуальную файловую систему
    ctx.t1 = IMG_LoadTexture(ctx.renderer, "assets/background.png");
    ctx.t2 = IMG_LoadTexture(ctx.renderer, "assets/platform.png");
    ctx.t3 = IMG_LoadTexture(ctx.renderer, "assets/doodle.png");

    if (!ctx.t1 || !ctx.t2 || !ctx.t3) {
        std::cerr << "Ошибка загрузки текстур!" << std::endl;
        return -1;
    }

    ctx.fontScore = TTF_OpenFont("assets/arial.ttf", 24);
    ctx.fontGameOver = TTF_OpenFont("assets/arial.ttf", 30);
    if (!ctx.fontScore || !ctx.fontGameOver) {
        std::cerr << "Ошибка загрузки шрифтов!" << std::endl;
        return -1;
    }

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        ctx.plat[i].x = std::rand() % 400;
        ctx.plat[i].y = std::rand() % 533;
    }

#ifdef __EMSCRIPTEN__
    // 0 в параметрах означает использование requestAnimationFrame (60 FPS или герцовка монитора)
    // true — симулировать бесконечный цикл (Emscripten сам прервет выполнение main)
    emscripten_set_main_loop_arg(main_loop, &ctx, 0, true);
#else
    // На случай, если захочется запустить этот же код локально на ПК
    while (ctx.isRunning) {
        Uint32 frameStart = SDL_GetTicks();
        main_loop(&ctx);
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (1000 / 60 > frameTime) {
            SDL_Delay((1000 / 60) - frameTime);
        }
    }
#endif

    // Очистка памяти (вызовется на ПК, в браузере при закрытии вкладки произойдет автоматически)
    SDL_DestroyTexture(ctx.t1);
    SDL_DestroyTexture(ctx.t2);
    SDL_DestroyTexture(ctx.t3);
    TTF_CloseFont(ctx.fontScore);
    TTF_CloseFont(ctx.fontGameOver);
    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}