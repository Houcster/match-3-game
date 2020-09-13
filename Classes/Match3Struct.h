#include "cocos2d.h"

struct Slot                                 //������ ��������� ��� ������
{
    bool match = false;

    int posX; 
    int posY;
    int column;
    int row;
    int type = -1;

    cocos2d::Sprite* sprite;
} slot[10][10];                             //����� ������� ������ ������

void swapSlots(Slot slot1, Slot slot2)      //�������, ����� ������ ����� �������
{
    std::swap(slot1.column, slot2.column);
    std::swap(slot1.row, slot2.row);
    std::swap(slot1.posX, slot2.posX);
    std::swap(slot1.posY, slot2.posY);

    slot[slot1.row][slot1.column] = slot1;
    slot[slot2.row][slot2.column] = slot2;
}