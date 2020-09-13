#ifndef __MAIN_GAMESCENE_H__
#define __MAIN_GAMESCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class MainGameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    cocos2d::Label* scoreLabel;
    cocos2d::ui::Button* restartButton;

    int score;
    int x0;
    int y0; 
    int x; 
    int y;
    int clickCount = 1;
    int slotsToRemove = 0;
    bool isMoving = false;
    bool gameOver = false;

    virtual bool init();
    void updateLogic(float dt);
    void findMatches();
    void deleteSlots();
    void refreshSlotsPositions();
    void checkGameOver();
    void checkAfterSwap();
    void stopMoving();
    void spawnSlots();  
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
      
    // implement the "static create()" method manually
    CREATE_FUNC(MainGameScene);
};

#endif // __MAIN_GAMESCENE_H__
