#include "MainGameScene.h"
#include "SimpleAudioEngine.h"
#include "Constants.h"
#include "Match3Struct.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* MainGameScene::createScene()
{
    return MainGameScene::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool MainGameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    Vec2 origin = Director::getInstance()->getVisibleOrigin();


    /////////////////////////////
    // Создаём градиент для бэкграунда
    auto backGroundGradient = LayerGradient::create(Color4B(240, 200, 60, 255), Color4B(185, 85, 20, 255), Point(1, 1));
    backGroundGradient->setPosition(origin);
    addChild(backGroundGradient);

    /////////////////////////////
    // Создаём спрайт для игрового поля
    auto grid_sprite = Sprite::create("grid_sprite.png");
    grid_sprite->setContentSize(Size(600, 600));
    grid_sprite->setAnchorPoint(Vec2(0, 0));
    grid_sprite->setPosition(GRID_STARTPOSITION_X, GRID_STARTPOSITION_Y);
    this->addChild(grid_sprite, 0);

    /////////////////////////////
    // Создаём лейбл для отображения очков
    scoreLabel = Label::createWithTTF("Score:", "fonts/Marker Felt.ttf", 70);
    scoreLabel->setAnchorPoint(Point(0.5, 0.5));
    scoreLabel->setPosition(1000, 600);
    scoreLabel->setTextColor(Color4B::BLACK);
    this->addChild(scoreLabel, 1);

    /////////////////////////////
    // Создаём кнопку рестарта
    restartButton = ui::Button::create("normal_image.png", "selected_image.png", "disabled_image.png");
    restartButton->setTitleText("RESTART");
    restartButton->setEnabled(false);
    restartButton->setPosition(Vec2(1000, 400));
    restartButton->setAnchorPoint(Point(0.5, 0.5));
    restartButton->setTitleFontName("fonts/Marker Felt.ttf");
    restartButton->setTitleFontSize(44);
    restartButton->setTitleColor(Color3B::BLACK);
    restartButton->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::BEGAN:
            break;
        case ui::Widget::TouchEventType::ENDED:
            spawnSlots();
            score = 0;
            break;
        default:
            break;
        }
        });

    this->addChild(restartButton);


    /////////////////////////////
    // Создаём кнопку выхода
    auto quitButton = ui::Button::create("normal_image.png", "selected_image.png", "disabled_image.png");
    quitButton->setTitleText("QUIT");
    quitButton->setPosition(Vec2(1000, 250));
    quitButton->setAnchorPoint(Point(0.5, 0.5));
    quitButton->setTitleFontName("fonts/Marker Felt.ttf");
    quitButton->setTitleFontSize(44);
    quitButton->setTitleColor(Color3B::BLACK);
    quitButton->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::BEGAN:
            break;
        case ui::Widget::TouchEventType::ENDED:
            Director::getInstance()->end();
            break;
        default:
            break;
        }
        });

    this->addChild(quitButton);

    spawnSlots();

    score = 0;

    /////////////////////////////
    // Добавляем обработчик нажатий
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(MainGameScene::onTouchBegan, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

    /////////////////////////////
    // Обновляем игровую логику каждые 0.2 секунды
    schedule(CC_SCHEDULE_SELECTOR(MainGameScene::updateLogic), 0.2f);
    this->scheduleUpdate();

    return true;
}

void MainGameScene::updateLogic(float dt)    // Функция для обновления логики игры
{
    if (!isMoving)
    {
        findMatches();
        deleteSlots();
        refreshSlotsPositions();
        checkGameOver();

        if (gameOver)
        {
            scoreLabel->setString("GAME OVER");
            restartButton->setEnabled(true);
        }
    }
}

void MainGameScene::findMatches()   // Находим совпавшие фишки
{
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
        {
            if (slot[i][j].type == slot[i + 1][j].type && slot[i][j].type == slot[i - 1][j].type)
            {
                for (int n = -1; n <= 1; n++)
                    if (slot[i + n][j].sprite != nullptr) { slot[i + n][j].match = true; }
            }
            if (slot[i][j].type == slot[i][j + 1].type && slot[i][j].type == slot[i][j - 1].type)
            {
                for (int n = -1; n <= 1; n++)
                    if (slot[i][j + n].sprite != nullptr) { slot[i][j + n].match = true; }
            }
        }
}

void MainGameScene::deleteSlots()   // "Удаляем" с поля совпавшие фишки
{
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            if (slot[i][j].match)
            {
                slot[i][j].sprite->setOpacity(0);

                for (int n = j; n <= 8; n++)
                    if (!slot[i][n].match)
                    {
                        swapSlots(slot[i][j], slot[i][n]);
                        break;
                    }              
            }
}

void MainGameScene::refreshSlotsPositions() // Перемещаем фишки после совпадения и обновляем "удаленные" фишки
{
    std::vector<std::string> slotsNames = { "dude_sprite.png", "bird_sprite.png", "wolf_sprite.png", "deer_sprite.png", "skull_sprite.png", "crown_sprite.png", "glove_sprite.png" };
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            if (slot[i][j].sprite->getOpacity() == 0)
            {
                isMoving = true;

                score++;
                slotsToRemove++;
                int slotType = cocos2d::RandomHelper::random_int(0, 6);
                auto fadeIn = FadeIn::create(1.0f);

                auto callback = CallFunc::create(this, callfunc_selector(MainGameScene::stopMoving));

                auto sequence = Sequence::create(fadeIn, callback, NULL);

                slot[i][j].sprite->runAction(sequence);
                slot[i][j].type = slotType;
                slot[i][j].match = false;
                slot[i][j].sprite->setPosition(slot[i][j].posX, slot[i][j].posY);
                slot[i][j].sprite->setTexture(slotsNames.at(slotType));
                slot[i][j].sprite->setContentSize(Size(50, 50));
                slot[i][j].sprite->runAction(fadeIn);
            }
            else if (slot[i][j].sprite->getPosition().x != slot[i][j].posX || slot[i][j].sprite->getPosition().y != slot[i][j].posY)
            {
                isMoving = true;
                slotsToRemove++;
                auto MoveTo1 = MoveTo::create(1.0f, Vec2(slot[i][j].posX, slot[i][j].posY));
                auto callback = CallFunc::create(this, callfunc_selector(MainGameScene::stopMoving));

                auto sequence = Sequence::create(MoveTo1, callback, NULL);
                slot[i][j].sprite->runAction(sequence);
            }
        
    
    scoreLabel->setString("Score: " + std::to_string(score));
}

void MainGameScene::checkGameOver() //Проверяем не закончилась ли игра
{
    gameOver = true;

    //Ищем можно ли получить "match" по горизонтали
    for (int i = 2; i < 8; i++)
        for (int j = 1; j <= 8; j++)
            if (slot[i][j].type == slot[i - 1][j].type)
            {
                if (slot[i][j].type == slot[i + 2][j].type ||
                    slot[i][j].type == slot[i + 1][j - 1].type ||
                    slot[i][j].type == slot[i + 1][j + 1].type)
                {
                    gameOver = false;
                    return;
                }
            }
            else if (slot[i][j].type == slot[i + 1][j].type)
            {
                if (slot[i][j].type == slot[i - 2][j].type ||
                    slot[i][j].type == slot[i - 1][j - 1].type ||
                    slot[i][j].type == slot[i - 1][j + 1].type)
                {
                    gameOver = false;
                    return;
                }
            }
            else if (slot[i][j].type == slot[i + 2][j].type)
            {
                if (slot[i][j].type == slot[i + 1][j + 1].type ||
                    slot[i][j].type == slot[i + 1][j - 1].type)
                {
                    gameOver = false;
                    return;
                }
            }

    //Если не нашли, то ищем по вертикали
    for (int i = 1; i <= 8; i++)
        for (int j = 2; j < 8; j++)
            if (slot[i][j].type == slot[i][j - 1].type)
            {
                if (slot[i][j].type == slot[i][j + 2].type ||
                    slot[i][j].type == slot[i - 1][j + 1].type ||
                    slot[i][j].type == slot[i + 1][j + 1].type)
                {
                    gameOver = false;
                    return;
                }
            }
            else if (slot[i][j].type == slot[i][j + 1].type)
            {
                if (slot[i][j].type == slot[i][j - 2].type ||
                    slot[i][j].type == slot[i - 1][j - 1].type ||
                    slot[i][j].type == slot[i + 1][j - 1].type)
                {
                    gameOver = false;
                    return;
                }
            }
            else if (slot[i][j].type == slot[i][j + 2].type)
            {
                if (slot[i][j].type == slot[i - 1][j + 1].type ||
                    slot[i][j].type == slot[i + 1][j + 1].type)
                {
                    gameOver = false;
                    return;
                }
            }
}

void MainGameScene::checkAfterSwap()    //Смотрим дало ли эффект смена фишек местами. Если нет меняем их обратно
{
    bool gotMatch = false;

    findMatches();
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            if (slot[i][j].match)
            {
                gotMatch = true;
                break;
            }

    if (!gotMatch)
    {
        swapSlots(slot[x0][y0], slot[x][y]);

        isMoving = true;
        auto MoveTo1 = MoveTo::create(1.0f, Vec2(slot[x0][y0].posX, slot[x0][y0].posY));

        auto MoveTo2 = MoveTo::create(1.0f, Vec2(slot[x][y].posX, slot[x][y].posY));

        auto callback = CallFunc::create(this, callfunc_selector(MainGameScene::stopMoving));

        auto sequence = Sequence::create(MoveTo2, callback, NULL);
        slot[x0][y0].sprite->runAction(MoveTo1);
        slot[x][y].sprite->runAction(sequence);
    }
    else
    {
        slotsToRemove--;
        isMoving = false;
    }
}

void MainGameScene::stopMoving()    // Чекаем если на поле движущиеся фишки. Если нет, то разблокируем нажатия на фишки
{
    slotsToRemove--;
    if (slotsToRemove == 0)
        isMoving = false;
}

void MainGameScene::spawnSlots()    // Размещаем фишки на игровом поле
{
    std::vector<std::string> slotsNames = { "dude_sprite.png", "bird_sprite.png", "wolf_sprite.png", "deer_sprite.png", "skull_sprite.png", "crown_sprite.png", "glove_sprite.png" };

    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
        {
            int slotType = cocos2d::RandomHelper::random_int(0, 6);

            slot[i][j].type = slotType;
            slot[i][j].column = j;
            slot[i][j].row = i;
            slot[i][j].posX = (SLOT_STARTPOSITION_X + (SLOT_SIZE_WIDTH * i));
            slot[i][j].posY = (SLOT_STARTPOSITION_Y + (SLOT_SIZE_HEIGHT * j));

            if (slot[i][j].sprite == nullptr)
            {
                slot[i][j].sprite = Sprite::create(slotsNames.at(slotType));
                addChild(slot[i][j].sprite);
            }
            else
            {
                slot[i][j].sprite->setTexture(slotsNames.at(slotType));
            }
            slot[i][j].sprite->setContentSize(Size(50, 50));
            slot[i][j].sprite->setPosition(cocos2d::Point(slot[i][j].posX, slot[i][j].posY));           
        }         
}

bool MainGameScene::onTouchBegan(Touch* touch, Event* event)    // Выбираем две фишки для обмена
{
    if (!isMoving)
    {
        if (clickCount == 1)
        {
            x0 = touch->getLocation().x / SLOT_SIZE_WIDTH;
            y0 = touch->getLocation().y / SLOT_SIZE_HEIGHT;
            if (slot[x0][y0].sprite != nullptr)
            {
                slot[x0][y0].sprite->setContentSize(Size(65, 65));
                clickCount = 2;
            }
        }
        else if (clickCount == 2)
        {
            x = touch->getLocation().x / SLOT_SIZE_WIDTH;
            y = touch->getLocation().y / SLOT_SIZE_HEIGHT;
            if (abs(x - x0) + abs(y - y0) == 1 && slot[x][y].sprite != nullptr)
            {
                slot[x0][y0].sprite->setContentSize(Size(50, 50));

                swapSlots(slot[x0][y0], slot[x][y]);
                isMoving = true;
                slotsToRemove++;
                auto MoveTo1 = MoveTo::create(1.0f, Vec2(slot[x0][y0].posX, slot[x0][y0].posY));
                slot[x0][y0].sprite->runAction(MoveTo1);

                auto MoveTo2 = MoveTo::create(1.0f, Vec2(slot[x][y].posX, slot[x][y].posY));

                auto callback = CallFunc::create(this, callfunc_selector(MainGameScene::checkAfterSwap));

                auto sequence = Sequence::create(MoveTo2, callback, NULL);
                slot[x][y].sprite->runAction(sequence);

                clickCount = 1;

            }
            else {
                clickCount = 1;
                slot[x0][y0].sprite->setContentSize(Size(50, 50));
            }
        }
    }

    return false;
}
