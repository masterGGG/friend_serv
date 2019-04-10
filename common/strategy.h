#ifndef STRATEGY_H
#define STRATEGY_H

#include <string>
#include <vector>
#include "common.h"

using std::string;
using std::vector;

// 多叉树状结构，若一个elment的所有m_children均disable，则此element也disable
// 若一个element的所有m_children中只要有一个enable，则此element也enable
// 非线程安全
class Element
{
public:
    Element();
    virtual ~Element() {
        // TODO
    };
    void Enable();
    void Disable();
    bool AddChild(Element *child);
    //void DeleteChild(Element *child);
    bool IsEnable() {
        return m_enable;
    }
    Element *GetParent(){
        return m_parent;
    }
    void *ChooseElement();

protected:
    // enable一个元素之后需要对其parent做的工作
    void AfterEnable();
    // disable一个元素之后需要对其parent做的工作
    void AfterDisable();

private:
    bool m_enable;
    Element *m_parent;
    vector<Element*> m_children;
};

class IChooseByNothing
{
public:
    virtual ~IChooseByNothing(){}
    virtual Element *ChooseByNothing() = 0;
};

class IChooseByInt
{
public:
    virtual ~IChooseByInt(){}
    virtual Element *ChooseByInt(int seed) = 0;
};

class IChooseByString
{
public:
    virtual ~IChooseByString(){}
    virtual Element *ChooseByString(const string &seed) = 0;
};


class Strategy
{
public:
    enum strategy_type{
        STRATEGY_MIN = 0,
        STRATEGY_FIXED,             // 固定在主，主不行就切到备1，主和备1均不行就切到备2
        STRATEGY_ROLL,              // 轮询，没有主备之分    
        STRATETY_INT_REGION,
        STRATETY_STRING_REGION,
        STRATEGY_MAX
    };

    // 无不带参数的构造函数，其子类必须在构造函数中调用此带参数的构造函数
    Strategy(const vector<Element*> &elements);

    virtual ~Strategy(){};
    
//    Element* ChooseByNothing() {
//        return _ChooseByNothing();
//    }
//    
//    Element* ChooseByInt(int seed) {
//        return _ChooseByInt(seed);
//    }
//    
//    Element* ChooseByString(const string &seed) {
//        return _ChooseByString(seed);
//    }
//    
//
//protected:
//    virtual Element* _ChooseByNothing() = 0;
//    virtual Element* _ChooseByInt(int seed) = 0;
//    virtual Element* _ChooseByString(string seed) = 0;

protected:
    const vector<Element*> &m_elements;   
};


// 固定策略，elements[0]先提供服务，失效后elements[1]提供服务，后elements[2]，以此类推
class FixedStrategy : public Strategy, public IChooseByNothing
{
public:
    FixedStrategy(const vector<Element*> &elements);
    //virtual Element* _ChooseByNothing();
    //virtual Element* _ChooseByInt(int seed);
    //virtual Element* _ChooseByString(string seed);
    Element* ChooseByNothing();
};

// 轮询策略
class RollStrategy : public Strategy, public IChooseByNothing
{
public:
    RollStrategy(const vector<Element*> &elements);
    //virtual Element* _ChooseByNothing();
    //virtual Element* _ChooseByInt(int seed);
    //virtual Element* _ChooseByString(string seed);
    Element* ChooseByNothing();
private:
    int m_current;
};

// 随机策略
//class RandomStrategy : public Strategy

// 区域策略
class IntRegionStrategy : public Strategy, public IChooseByInt
{
public:
    IntRegionStrategy(const vector<Element*> &elements);
    //virtual Element* _ChooseByNothing();
    //virtual Element* _ChooseByInt(int seed);
    //virtual Element* _ChooseByString(string seed);
    virtual Element *ChooseByInt(int seed);
private:
};

class StringRegionStrategy : public Strategy, public IChooseByString
{
public:
    StringRegionStrategy(const vector<Element*> &elements);
    //virtual Element* _ChooseByNothing();
    //virtual Element* _ChooseByInt(int seed);
    //virtual Element* _ChooseByString(string seed);
    virtual Element *ChooseByString(const string &seed);
private:
};

#endif
