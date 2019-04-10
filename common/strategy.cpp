#include "strategy.h"
#include "common.h"

Element::Element():
m_enable(false),
m_parent(NULL)
{
}

void Element::Enable()
{
    if (this->m_enable) return;

    this->m_enable = true;
    AfterEnable();
}

void Element::Disable()
{
    if (!this->m_enable) return;

    // 向上访问，若因其m_enable改变导致其父的所有m_children都disable了，则其父也disable。
    this->m_enable = false;
    AfterDisable();
}

bool Element::AddChild(Element *ele)
{
    if (!ele) {
        // TODO
        return false;
    }

    this->m_children.push_back(ele);
    ele->m_parent = this;

    if (ele->IsEnable()){
        AfterEnable();
    }

    return true;
}

void Element::AfterEnable()
{
    Element *now = this->m_parent;

    while(now) {
        if (now->IsEnable()) {
            // 其父并未disable
            break;
        }
        now->Enable();
        now = now->m_parent;
    }
}

void Element::AfterDisable()
{
    Element *now = this->m_parent;

    while (now) {
        vector<Element*>::const_iterator it = now->m_children.begin();
        bool all_disable = true;

        for (; it != now->m_children.end(); ++it) {
            if ((*it)->IsEnable()) {
                all_disable = false;
                break;
            }
        }

        if (all_disable) {
            now->Disable();
            now = now->m_parent;
            continue;
        } else {
            break;
        }
    }
}

// Strategy
Strategy::Strategy(const vector<Element*> &elements)
:m_elements(elements)
{
}

// FixedStrategy
FixedStrategy::FixedStrategy(const vector<Element*> &elements)
:Strategy(elements)
{
}

Element* FixedStrategy::ChooseByNothing()
{
    vector<Element*>::const_iterator it = m_elements.begin();
    for (; it != m_elements.end(); ++it) {
        if ((*it)->IsEnable()) {
            return *it;
        }
    }
    
    ERROR_LOG("FixedStrategy ChooseByNothing no enable element");
    return NULL;
}

//Element* FixedStrategy::_ChooseByInt(int seed) {
//    // TODO
//    return NULL;
//}
//
//Element* FixedStrategy::_ChooseByString(string seed) {
//    // TODO
//    return NULL;
//}

// RollStrategy
RollStrategy::RollStrategy(const vector<Element*> &elements)
:Strategy(elements)
,m_current(0)
{
}

Element* RollStrategy::ChooseByNothing()
{
    if (!m_elements.size()) {
        return NULL;
    }

    int index = m_current % m_elements.size();
    for (unsigned int i = 0; i < m_elements.size(); ++i) {
        if (!m_elements[index]->IsEnable()) {
            index = (index + 1) % m_elements.size();
            continue;
        }
        ++m_current;
        return m_elements[index];
    }

    ERROR_LOG("RollStrategy ChooseByNothing element no enable element");
    return NULL;
}

//Element* RollStrategy::_ChooseByInt(int seed) {
//    ERROR_LOG("RollStrategy not suppport _ChooseByInt");
//    return NULL;
//}
//
//Element* RollStrategy::_ChooseByString(string seed) {
//    ERROR_LOG("RollStrategy not suppport _ChooseByString");
//    return NULL;
//}

// IntRegionStrategy
IntRegionStrategy::IntRegionStrategy(const vector<Element*> &elements)
:Strategy(elements)
{
}

//Element *IntRegionStrategy::_ChooseByNothing()
//{
//    ERROR_LOG("IntRegionStrategy not suppport _ChooseByNothing");
//    return NULL;
//}

Element* IntRegionStrategy::ChooseByInt(int seed) {
    if (!m_elements.size()) {
        return NULL;
    }

    // Element *ele = m_elements[seed % (m_elements.size())];
    Element *ele = m_elements[seed % 100 / (100 / m_elements.size())];

    if (ele->IsEnable()) {
        return ele;
    }
    
    ERROR_LOG("IntRegionStrategy ChooseByInt element chosen not enable");
    return NULL;
}

//Element* IntRegionStrategy::_ChooseByString(string seed) {
//    ERROR_LOG("IntRegionStrategy not suppport _ChooseByString");
//    return NULL;
//}


// StringRegionStrategy
StringRegionStrategy::StringRegionStrategy(const vector<Element*> &elements)
:Strategy(elements)
{
}

//Element *StringRegionStrategy::_ChooseByNothing()
//{
//    ERROR_LOG("StringRegionStrategy not suppport _ChooseByNothing"); 
//    return NULL;
//}
//
//Element* StringRegionStrategy::_ChooseByInt(int seed) {
//    ERROR_LOG("StringRegionStrategy not suppport _ChooseByInt"); 
//    return NULL;
//}

Element* StringRegionStrategy::ChooseByString(const string &seed) {
    if (!m_elements.size()) {
        return NULL;
    }

    int key = 0;
    for (unsigned int i = 0; i < seed.size(); i++) {
        key += 31 * key + (uint8_t)seed[i];
    }

    Element *ele = m_elements[key % (m_elements.size())];
    if (ele->IsEnable()) {
        return ele;
    }
    return NULL;
}



















