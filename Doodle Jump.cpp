#include <SFML/Graphics.hpp>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string> // Нужно для работы с текстом и конвертации чисел

using namespace sf;

struct Point {
    int x, y;
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    RenderWindow app(VideoMode(400, 533), "Doodle Game!");
    app.setFramerateLimit(60);

    Texture t1, t2, t3;
    if (!t1.loadFromFile("images/background.png") ||
        !t2.loadFromFile("images/platform.png") ||
        !t3.loadFromFile("images/doodle.png"))
    {
        std::cerr << "Ошибка: Не удалось загрузить текстуры!" << std::endl;
        return -1;
    }

    // Загрузка шрифта
    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Ошибка: Не удалось загрузить шрифт arial.ttf!" << std::endl;
        return -1;
    }

    // Настройка текста для счета
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::Black);
    scoreText.setPosition(10, 10);

    // Настройка текста для экрана "Game Over"
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER!\nPress ENTER to restart");
    gameOverText.setCharacterSize(30);
    gameOverText.setFillColor(Color::Red);
    // Выставляем текст примерно по центру
    gameOverText.setPosition(40, 200);

    Sprite sBackground(t1), sPlat(t2), sPers(t3);

    // --- АВТОМАТИЧЕСКОЕ МАСШТАБИРОВАНИЕ ---

    // 1. Фон растягиваем ровно под размер окна (400x533)
    sBackground.setScale(
        400.0f / t1.getSize().x,
        533.0f / t1.getSize().y
    );

    // 2. Платформу подгоняем под размер 68x14 (как в твоем условии столкновения)
    sPlat.setScale(
        75.0f / t2.getSize().x,
        20.0f / t2.getSize().y
    );

    // 3. Дудла подгоняем под размер примерно 50x70 пикселей
    sPers.setScale(
        70.0f / t3.getSize().x,
        80.0f / t3.getSize().y
    );

    const int PLATFORM_COUNT = 10;
    Point plat[PLATFORM_COUNT];

    // Инициализация платформ
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        plat[i].x = std::rand() % 400;
        plat[i].y = std::rand() % 533;
    }

    int x = 100, y = 100, h = 200;
    float dx = 0, dy = 0;

    // Переменные для игровой логики
    int score = 0;
    bool isGameOver = false;

    while (app.isOpen()) {
        Event e;
        while (app.pollEvent(e)) {
            if (e.type == Event::Closed)
                app.close();
        }

        // Если мы живы — играем
        if (!isGameOver) {
            if (Keyboard::isKeyPressed(Keyboard::Right)) x += 3;
            if (Keyboard::isKeyPressed(Keyboard::Left)) x -= 3;

            // Телепортация через границы экрана
            if (x > 400) x = -70;
            if (x < -70) x = 400;

            dy += 0.2f;
            y += static_cast<int>(dy);

            // УСЛОВИЕ ПОРАЖЕНИЯ: если упали ниже экрана
            if (y > 533) {
                isGameOver = true;
            }

            // Прокрутка камеры и начисление очков
            if (y < h) {
                for (int i = 0; i < PLATFORM_COUNT; i++) {
                    y = h;
                    plat[i].y = plat[i].y - static_cast<int>(dy);
                    if (plat[i].y > 533) {
                        plat[i].y = 0;
                        plat[i].x = std::rand() % 400;
                    }
                }
                // Начисляем очки за движение вверх
                score += 1;
            }

            // Коллизия с платформами
            for (int i = 0; i < PLATFORM_COUNT; i++) {
                if ((x + 70 > plat[i].x) && (x + 20 < plat[i].x + 75) &&
                    (y + 80 > plat[i].y) && (y + 80 < plat[i].y + 20) && (dy > 0))
                {
                    dy = -10.0f;
                }
            }

            sPers.setPosition(static_cast<float>(x), static_cast<float>(y));
            scoreText.setString("Score: " + std::to_string(score));
        }
        // Если проиграли — ждем рестарта
        else {
            if (Keyboard::isKeyPressed(Keyboard::Enter)) {
                // Сброс всех переменных к начальным значениям
                isGameOver = false;
                score = 0;
                x = 100;
                y = 100;
                dy = 0;
                for (int i = 0; i < PLATFORM_COUNT; i++) {
                    plat[i].x = std::rand() % 400;
                    plat[i].y = std::rand() % 533;
                }
                // Гарантируем, что под нами будет стартовая платформа, чтобы сразу не упасть
                plat[0].x = x;
                plat[0].y = y + 80;
            }
        }

        // --- ОТРИСОВКА ---
        app.clear();
        app.draw(sBackground);

        for (int i = 0; i < PLATFORM_COUNT; i++) {
            sPlat.setPosition(static_cast<float>(plat[i].x), static_cast<float>(plat[i].y));
            app.draw(sPlat);
        }

        app.draw(sPers);

        // Рисуем счет поверх всего
        app.draw(scoreText);

        // Если игра окончена, выводим надпись по центру
        if (isGameOver) {
            app.draw(gameOverText);
        }

        app.display();
    }

    return 0;
}