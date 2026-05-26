cmake_minimum_required(VERSION 3.10)
project(DoodleJump)

# Ставим 17-й стандарт (как в коде игры, который мы переписывали), 
# но можно вернуть и 14, если твой текущий код написан под него.
set(CMAKE_CXX_STANDARD 17)

# Генерирует compile_commands.json, чтобы IntelliSense в VS Code не паниковал
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Список исходников (если добавятся новые .cpp файлы, дописывай их сюда через пробел)
set(SOURCES main.cpp)

if(EMSCRIPTEN)
    # ==========================================
    # --- НАСТРОЙКИ ДЛЯ ВЕБА (EMSCRIPTEN) ---
    # ==========================================
    add_executable(DoodleJump ${SOURCES})

    # Для компиляции оставляем только стандарт, если нужен
    target_compile_options(DoodleJump PRIVATE "-O2") # Опционально: оптимизация кода

    # Передаем все флаги Emscripten в опции линковки цельными строками
    target_link_options(DoodleJump PRIVATE 
        "-sUSE_SDL=2"
        "-sUSE_SDL_IMAGE=2"
        "-sUSE_SDL_TTF=2"
        "-sSDL2_IMAGE_FORMATS=[\"png\"]"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
    )
    
    # Заставляем Emscripten генерировать готовую HTML-страницу вместо голого JS
    set_target_properties(DoodleJump PROPERTIES SUFFIX ".html")
    
    message(STATUS "Configuring DoodleJump for WebAssembly with Emscripten!")

else()
    # ==========================================
    # --- НАСТРОЙКИ ДЛЯ ДЕСКТОПА (MinGW) ---
    # ==========================================
    add_executable(DoodleJump ${SOURCES})

    # Базовая папка, куда распакован MinGW-пакет SDL2
    set(SDL_MINGW_DIR "C:/SDL2")

    # 1. Говорим, где искать заголовочные файлы (.h)
    target_include_directories(DoodleJump PRIVATE "${SDL_MINGW_DIR}/include")

    # 2. Говорим, где искать файлы библиотек (.a)
    target_link_directories(DoodleJump PRIVATE "${SDL_MINGW_DIR}/lib")

    # 3. Линкуем библиотеки напрямую
    target_link_libraries(DoodleJump PRIVATE 
        mingw32 
        SDL2main 
        SDL2 
        SDL2_image 
        SDL2_ttf
    )
    
    # АВТОМАТИЗАЦИЯ 1: Копируем папку assets в директорию сборки к .exe
    add_custom_command(TARGET DoodleJump POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:DoodleJump>/assets"
    )

    # АВТОМАТИЗАЦИЯ 2: Сами забираем нужные .dll из C:/SDL2/bin и кладем рядом с .exe
    # Теперь игра будет запускаться сразу после компиляции без ручного копирования!
    add_custom_command(TARGET DoodleJump POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL_MINGW_DIR}/bin/SDL2.dll"
        "${SDL_MINGW_DIR}/bin/SDL2_image.dll"
        "${SDL_MINGW_DIR}/bin/SDL2_ttf.dll"
        "$<TARGET_FILE_DIR:DoodleJump>/"
    )
    
    message(STATUS "Configuring DoodleJump for Desktop with DIRECT MinGW paths!")
endif()