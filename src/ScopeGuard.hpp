#pragma once
#include <utility>

class ScopeGuardBase{
public:
    void hire(){ m_fired = true; }
    void fire(){ m_fired = false; }

protected:
    ScopeGuardBase(const bool fired = false)
        : m_fired(fired){}
    
protected:
    bool m_fired{ false };
};

template<typename Func>
class ScopeGuard : public ScopeGuardBase{
public:
    explicit ScopeGuard(Func&& fn)
        : m_func(std::forward<Func>(fn)){}

    ScopeGuard(Func&& fn, const bool fire)
        :   ScopeGuardBase(fire),
            m_func(std::forward<Func>(fn)){}

    ~ScopeGuard(){
        if(!m_fired){
            m_func();
        }
    }

private:
    Func m_func{};
};