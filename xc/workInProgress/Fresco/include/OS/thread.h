/* DO NOT EDIT -- Automatically generated from Interfaces/thread.idl */

#ifndef Interfaces_thread_h
#define Interfaces_thread_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/Ox/object.h>

class BaseThreadsObjType;
typedef BaseThreadsObjType* BaseThreadsObjRef;
class BaseThreadsObj;
class _BaseThreadsObjExpr;
class _BaseThreadsObjElem;

class BaseThreadsObj {
public:
    BaseThreadsObjRef _obj;

    BaseThreadsObj() { _obj = 0; }
    BaseThreadsObj(BaseThreadsObjRef p) { _obj = p; }
    BaseThreadsObj& operator =(BaseThreadsObjRef p);
    BaseThreadsObj(const BaseThreadsObj&);
    BaseThreadsObj& operator =(const BaseThreadsObj& r);
    BaseThreadsObj(const _BaseThreadsObjExpr&);
    BaseThreadsObj& operator =(const _BaseThreadsObjExpr&);
    BaseThreadsObj(const _BaseThreadsObjElem&);
    BaseThreadsObj& operator =(const _BaseThreadsObjElem&);
    ~BaseThreadsObj();

    operator BaseThreadsObjRef() const { return _obj; }
    BaseThreadsObjRef operator ->() { return _obj; }

    static BaseThreadsObjRef _narrow(BaseObjectRef p);
    static _BaseThreadsObjExpr _narrow(const BaseObject& r);

    static BaseThreadsObjRef _duplicate(BaseThreadsObjRef obj);
    static _BaseThreadsObjExpr _duplicate(const BaseThreadsObj& r);
};

class _BaseThreadsObjExpr : public BaseThreadsObj {
public:
    _BaseThreadsObjExpr(BaseThreadsObjRef p) { _obj = p; }
    _BaseThreadsObjExpr(const BaseThreadsObj& r) { _obj = r._obj; }
    _BaseThreadsObjExpr(const _BaseThreadsObjExpr& r) { _obj = r._obj; }
    ~_BaseThreadsObjExpr();
};

class _BaseThreadsObjElem {
public:
    BaseThreadsObjRef _obj;

    _BaseThreadsObjElem(BaseThreadsObjRef p) { _obj = p; }
    operator BaseThreadsObjRef() const { return _obj; }
    BaseThreadsObjRef operator ->() { return _obj; }
};

class BaseThreadsObjType : public BaseObjectType {
protected:
    BaseThreadsObjType();
    virtual ~BaseThreadsObjType();
public:
    virtual Long ref__(Long references) = 0;

    _BaseThreadsObjExpr _ref();
    virtual TypeObjId _tid();
};

inline BaseThreadsObjRef BaseThreadsObj::_duplicate(BaseThreadsObjRef obj) {
    return (BaseThreadsObjRef)_BaseObject__duplicate(obj, 0);
}
inline BaseThreadsObj& BaseThreadsObj::operator =(BaseThreadsObjRef p) {
    _BaseObject__release(_obj);
    _obj = BaseThreadsObj::_duplicate(p);
    return *this;
}
inline BaseThreadsObj::BaseThreadsObj(const BaseThreadsObj& r) {
    _obj = BaseThreadsObj::_duplicate(r._obj);
}
inline BaseThreadsObj& BaseThreadsObj::operator =(const BaseThreadsObj& r) {
    _BaseObject__release(_obj);
    _obj = BaseThreadsObj::_duplicate(r._obj);
    return *this;
}
inline BaseThreadsObj::BaseThreadsObj(const _BaseThreadsObjExpr& r) {
    _obj = r._obj;
    ((_BaseThreadsObjExpr*)&r)->_obj = 0;
}
inline BaseThreadsObj& BaseThreadsObj::operator =(const _BaseThreadsObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_BaseThreadsObjExpr*)&r)->_obj = 0;
    return *this;
}
inline BaseThreadsObj::BaseThreadsObj(const _BaseThreadsObjElem& e) {
    _obj = BaseThreadsObj::_duplicate(e._obj);
}
inline BaseThreadsObj& BaseThreadsObj::operator =(const _BaseThreadsObjElem& e) {
    _BaseObject__release(_obj);
    _obj = BaseThreadsObj::_duplicate(e._obj);
    return *this;
}
inline BaseThreadsObj::~BaseThreadsObj() {
    _BaseObject__release(_obj);
}
inline _BaseThreadsObjExpr BaseThreadsObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _BaseThreadsObjExpr BaseThreadsObj::_duplicate(const BaseThreadsObj& r) {
    return _duplicate(r._obj);
}
inline _BaseThreadsObjExpr::~_BaseThreadsObjExpr() { }
inline _BaseThreadsObjExpr BaseThreadsObjType::_ref() { return this; }

class ThreadObjType;
typedef ThreadObjType* ThreadObjRef;
class ThreadObj;
class _ThreadObjExpr;
class _ThreadObjElem;

class ThreadObj {
public:
    ThreadObjRef _obj;

    ThreadObj() { _obj = 0; }
    ThreadObj(ThreadObjRef p) { _obj = p; }
    ThreadObj& operator =(ThreadObjRef p);
    ThreadObj(const ThreadObj&);
    ThreadObj& operator =(const ThreadObj& r);
    ThreadObj(const _ThreadObjExpr&);
    ThreadObj& operator =(const _ThreadObjExpr&);
    ThreadObj(const _ThreadObjElem&);
    ThreadObj& operator =(const _ThreadObjElem&);
    ~ThreadObj();

    operator ThreadObjRef() const { return _obj; }
    ThreadObjRef operator ->() { return _obj; }

    operator BaseThreadsObj() const;
    static ThreadObjRef _narrow(BaseObjectRef p);
    static _ThreadObjExpr _narrow(const BaseObject& r);

    static ThreadObjRef _duplicate(ThreadObjRef obj);
    static _ThreadObjExpr _duplicate(const ThreadObj& r);
};

class _ThreadObjExpr : public ThreadObj {
public:
    _ThreadObjExpr(ThreadObjRef p) { _obj = p; }
    _ThreadObjExpr(const ThreadObj& r) { _obj = r._obj; }
    _ThreadObjExpr(const _ThreadObjExpr& r) { _obj = r._obj; }
    ~_ThreadObjExpr();
};

class _ThreadObjElem {
public:
    ThreadObjRef _obj;

    _ThreadObjElem(ThreadObjRef p) { _obj = p; }
    operator ThreadObjRef() const { return _obj; }
    ThreadObjRef operator ->() { return _obj; }
};

class ThreadObjType : public BaseThreadsObjType {
protected:
    ThreadObjType();
    virtual ~ThreadObjType();
public:
    virtual void run() = 0;
    virtual void terminate() = 0;
    virtual void wait(Long& status, Long& exitcode) = 0;

    _ThreadObjExpr _ref();
    virtual TypeObjId _tid();
};

inline ThreadObjRef ThreadObj::_duplicate(ThreadObjRef obj) {
    return (ThreadObjRef)_BaseObject__duplicate(obj, 0);
}
inline ThreadObj& ThreadObj::operator =(ThreadObjRef p) {
    _BaseObject__release(_obj);
    _obj = ThreadObj::_duplicate(p);
    return *this;
}
inline ThreadObj::ThreadObj(const ThreadObj& r) {
    _obj = ThreadObj::_duplicate(r._obj);
}
inline ThreadObj& ThreadObj::operator =(const ThreadObj& r) {
    _BaseObject__release(_obj);
    _obj = ThreadObj::_duplicate(r._obj);
    return *this;
}
inline ThreadObj::ThreadObj(const _ThreadObjExpr& r) {
    _obj = r._obj;
    ((_ThreadObjExpr*)&r)->_obj = 0;
}
inline ThreadObj& ThreadObj::operator =(const _ThreadObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ThreadObjExpr*)&r)->_obj = 0;
    return *this;
}
inline ThreadObj::ThreadObj(const _ThreadObjElem& e) {
    _obj = ThreadObj::_duplicate(e._obj);
}
inline ThreadObj& ThreadObj::operator =(const _ThreadObjElem& e) {
    _BaseObject__release(_obj);
    _obj = ThreadObj::_duplicate(e._obj);
    return *this;
}
inline ThreadObj::~ThreadObj() {
    _BaseObject__release(_obj);
}
inline _ThreadObjExpr ThreadObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ThreadObjExpr ThreadObj::_duplicate(const ThreadObj& r) {
    return _duplicate(r._obj);
}
inline ThreadObj::operator BaseThreadsObj() const {
    return _BaseThreadsObjExpr((BaseThreadsObjRef)_BaseObject__duplicate(_obj, 0));
}
inline _ThreadObjExpr::~_ThreadObjExpr() { }
inline _ThreadObjExpr ThreadObjType::_ref() { return this; }

class LockObjType;
typedef LockObjType* LockObjRef;
class LockObj;
class _LockObjExpr;
class _LockObjElem;

class LockObj {
public:
    LockObjRef _obj;

    LockObj() { _obj = 0; }
    LockObj(LockObjRef p) { _obj = p; }
    LockObj& operator =(LockObjRef p);
    LockObj(const LockObj&);
    LockObj& operator =(const LockObj& r);
    LockObj(const _LockObjExpr&);
    LockObj& operator =(const _LockObjExpr&);
    LockObj(const _LockObjElem&);
    LockObj& operator =(const _LockObjElem&);
    ~LockObj();

    operator LockObjRef() const { return _obj; }
    LockObjRef operator ->() { return _obj; }

    operator BaseThreadsObj() const;
    static LockObjRef _narrow(BaseObjectRef p);
    static _LockObjExpr _narrow(const BaseObject& r);

    static LockObjRef _duplicate(LockObjRef obj);
    static _LockObjExpr _duplicate(const LockObj& r);
};

class _LockObjExpr : public LockObj {
public:
    _LockObjExpr(LockObjRef p) { _obj = p; }
    _LockObjExpr(const LockObj& r) { _obj = r._obj; }
    _LockObjExpr(const _LockObjExpr& r) { _obj = r._obj; }
    ~_LockObjExpr();
};

class _LockObjElem {
public:
    LockObjRef _obj;

    _LockObjElem(LockObjRef p) { _obj = p; }
    operator LockObjRef() const { return _obj; }
    LockObjRef operator ->() { return _obj; }
};

class LockObjType : public BaseThreadsObjType {
protected:
    LockObjType();
    virtual ~LockObjType();
public:
    virtual void acquire() = 0;
    virtual void release() = 0;
    virtual Boolean try_acquire() = 0;

    _LockObjExpr _ref();
    virtual TypeObjId _tid();
};

inline LockObjRef LockObj::_duplicate(LockObjRef obj) {
    return (LockObjRef)_BaseObject__duplicate(obj, 0);
}
inline LockObj& LockObj::operator =(LockObjRef p) {
    _BaseObject__release(_obj);
    _obj = LockObj::_duplicate(p);
    return *this;
}
inline LockObj::LockObj(const LockObj& r) {
    _obj = LockObj::_duplicate(r._obj);
}
inline LockObj& LockObj::operator =(const LockObj& r) {
    _BaseObject__release(_obj);
    _obj = LockObj::_duplicate(r._obj);
    return *this;
}
inline LockObj::LockObj(const _LockObjExpr& r) {
    _obj = r._obj;
    ((_LockObjExpr*)&r)->_obj = 0;
}
inline LockObj& LockObj::operator =(const _LockObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_LockObjExpr*)&r)->_obj = 0;
    return *this;
}
inline LockObj::LockObj(const _LockObjElem& e) {
    _obj = LockObj::_duplicate(e._obj);
}
inline LockObj& LockObj::operator =(const _LockObjElem& e) {
    _BaseObject__release(_obj);
    _obj = LockObj::_duplicate(e._obj);
    return *this;
}
inline LockObj::~LockObj() {
    _BaseObject__release(_obj);
}
inline _LockObjExpr LockObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _LockObjExpr LockObj::_duplicate(const LockObj& r) {
    return _duplicate(r._obj);
}
inline LockObj::operator BaseThreadsObj() const {
    return _BaseThreadsObjExpr((BaseThreadsObjRef)_BaseObject__duplicate(_obj, 0));
}
inline _LockObjExpr::~_LockObjExpr() { }
inline _LockObjExpr LockObjType::_ref() { return this; }

class ConditionVariableType;
typedef ConditionVariableType* ConditionVariableRef;
class ConditionVariable;
class _ConditionVariableExpr;
class _ConditionVariableElem;

class ConditionVariable {
public:
    ConditionVariableRef _obj;

    ConditionVariable() { _obj = 0; }
    ConditionVariable(ConditionVariableRef p) { _obj = p; }
    ConditionVariable& operator =(ConditionVariableRef p);
    ConditionVariable(const ConditionVariable&);
    ConditionVariable& operator =(const ConditionVariable& r);
    ConditionVariable(const _ConditionVariableExpr&);
    ConditionVariable& operator =(const _ConditionVariableExpr&);
    ConditionVariable(const _ConditionVariableElem&);
    ConditionVariable& operator =(const _ConditionVariableElem&);
    ~ConditionVariable();

    operator ConditionVariableRef() const { return _obj; }
    ConditionVariableRef operator ->() { return _obj; }

    operator BaseThreadsObj() const;
    static ConditionVariableRef _narrow(BaseObjectRef p);
    static _ConditionVariableExpr _narrow(const BaseObject& r);

    static ConditionVariableRef _duplicate(ConditionVariableRef obj);
    static _ConditionVariableExpr _duplicate(const ConditionVariable& r);
};

class _ConditionVariableExpr : public ConditionVariable {
public:
    _ConditionVariableExpr(ConditionVariableRef p) { _obj = p; }
    _ConditionVariableExpr(const ConditionVariable& r) { _obj = r._obj; }
    _ConditionVariableExpr(const _ConditionVariableExpr& r) { _obj = r._obj; }
    ~_ConditionVariableExpr();
};

class _ConditionVariableElem {
public:
    ConditionVariableRef _obj;

    _ConditionVariableElem(ConditionVariableRef p) { _obj = p; }
    operator ConditionVariableRef() const { return _obj; }
    ConditionVariableRef operator ->() { return _obj; }
};

class ConditionVariableType : public BaseThreadsObjType {
protected:
    ConditionVariableType();
    virtual ~ConditionVariableType();
public:
    virtual void wait(LockObjRef lock) = 0;
    virtual void notify() = 0;
    virtual void broadcast() = 0;

    _ConditionVariableExpr _ref();
    virtual TypeObjId _tid();
};

inline ConditionVariableRef ConditionVariable::_duplicate(ConditionVariableRef obj) {
    return (ConditionVariableRef)_BaseObject__duplicate(obj, 0);
}
inline ConditionVariable& ConditionVariable::operator =(ConditionVariableRef p) {
    _BaseObject__release(_obj);
    _obj = ConditionVariable::_duplicate(p);
    return *this;
}
inline ConditionVariable::ConditionVariable(const ConditionVariable& r) {
    _obj = ConditionVariable::_duplicate(r._obj);
}
inline ConditionVariable& ConditionVariable::operator =(const ConditionVariable& r) {
    _BaseObject__release(_obj);
    _obj = ConditionVariable::_duplicate(r._obj);
    return *this;
}
inline ConditionVariable::ConditionVariable(const _ConditionVariableExpr& r) {
    _obj = r._obj;
    ((_ConditionVariableExpr*)&r)->_obj = 0;
}
inline ConditionVariable& ConditionVariable::operator =(const _ConditionVariableExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ConditionVariableExpr*)&r)->_obj = 0;
    return *this;
}
inline ConditionVariable::ConditionVariable(const _ConditionVariableElem& e) {
    _obj = ConditionVariable::_duplicate(e._obj);
}
inline ConditionVariable& ConditionVariable::operator =(const _ConditionVariableElem& e) {
    _BaseObject__release(_obj);
    _obj = ConditionVariable::_duplicate(e._obj);
    return *this;
}
inline ConditionVariable::~ConditionVariable() {
    _BaseObject__release(_obj);
}
inline _ConditionVariableExpr ConditionVariable::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ConditionVariableExpr ConditionVariable::_duplicate(const ConditionVariable& r) {
    return _duplicate(r._obj);
}
inline ConditionVariable::operator BaseThreadsObj() const {
    return _BaseThreadsObjExpr((BaseThreadsObjRef)_BaseObject__duplicate(_obj, 0));
}
inline _ConditionVariableExpr::~_ConditionVariableExpr() { }
inline _ConditionVariableExpr ConditionVariableType::_ref() { return this; }

class SemaphoreType;
typedef SemaphoreType* SemaphoreRef;
class Semaphore;
class _SemaphoreExpr;
class _SemaphoreElem;

class Semaphore {
public:
    SemaphoreRef _obj;

    Semaphore() { _obj = 0; }
    Semaphore(SemaphoreRef p) { _obj = p; }
    Semaphore& operator =(SemaphoreRef p);
    Semaphore(const Semaphore&);
    Semaphore& operator =(const Semaphore& r);
    Semaphore(const _SemaphoreExpr&);
    Semaphore& operator =(const _SemaphoreExpr&);
    Semaphore(const _SemaphoreElem&);
    Semaphore& operator =(const _SemaphoreElem&);
    ~Semaphore();

    operator SemaphoreRef() const { return _obj; }
    SemaphoreRef operator ->() { return _obj; }

    operator BaseThreadsObj() const;
    static SemaphoreRef _narrow(BaseObjectRef p);
    static _SemaphoreExpr _narrow(const BaseObject& r);

    static SemaphoreRef _duplicate(SemaphoreRef obj);
    static _SemaphoreExpr _duplicate(const Semaphore& r);
};

class _SemaphoreExpr : public Semaphore {
public:
    _SemaphoreExpr(SemaphoreRef p) { _obj = p; }
    _SemaphoreExpr(const Semaphore& r) { _obj = r._obj; }
    _SemaphoreExpr(const _SemaphoreExpr& r) { _obj = r._obj; }
    ~_SemaphoreExpr();
};

class _SemaphoreElem {
public:
    SemaphoreRef _obj;

    _SemaphoreElem(SemaphoreRef p) { _obj = p; }
    operator SemaphoreRef() const { return _obj; }
    SemaphoreRef operator ->() { return _obj; }
};

class SemaphoreType : public BaseThreadsObjType {
protected:
    SemaphoreType();
    virtual ~SemaphoreType();
public:
    virtual void P() = 0;
    virtual void V() = 0;

    _SemaphoreExpr _ref();
    virtual TypeObjId _tid();
};

inline SemaphoreRef Semaphore::_duplicate(SemaphoreRef obj) {
    return (SemaphoreRef)_BaseObject__duplicate(obj, 0);
}
inline Semaphore& Semaphore::operator =(SemaphoreRef p) {
    _BaseObject__release(_obj);
    _obj = Semaphore::_duplicate(p);
    return *this;
}
inline Semaphore::Semaphore(const Semaphore& r) {
    _obj = Semaphore::_duplicate(r._obj);
}
inline Semaphore& Semaphore::operator =(const Semaphore& r) {
    _BaseObject__release(_obj);
    _obj = Semaphore::_duplicate(r._obj);
    return *this;
}
inline Semaphore::Semaphore(const _SemaphoreExpr& r) {
    _obj = r._obj;
    ((_SemaphoreExpr*)&r)->_obj = 0;
}
inline Semaphore& Semaphore::operator =(const _SemaphoreExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_SemaphoreExpr*)&r)->_obj = 0;
    return *this;
}
inline Semaphore::Semaphore(const _SemaphoreElem& e) {
    _obj = Semaphore::_duplicate(e._obj);
}
inline Semaphore& Semaphore::operator =(const _SemaphoreElem& e) {
    _BaseObject__release(_obj);
    _obj = Semaphore::_duplicate(e._obj);
    return *this;
}
inline Semaphore::~Semaphore() {
    _BaseObject__release(_obj);
}
inline _SemaphoreExpr Semaphore::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _SemaphoreExpr Semaphore::_duplicate(const Semaphore& r) {
    return _duplicate(r._obj);
}
inline Semaphore::operator BaseThreadsObj() const {
    return _BaseThreadsObjExpr((BaseThreadsObjRef)_BaseObject__duplicate(_obj, 0));
}
inline _SemaphoreExpr::~_SemaphoreExpr() { }
inline _SemaphoreExpr SemaphoreType::_ref() { return this; }

class ThreadKitType;
typedef ThreadKitType* ThreadKitRef;
class ThreadKit;
class _ThreadKitExpr;
class _ThreadKitElem;

class ThreadKit {
public:
    ThreadKitRef _obj;

    ThreadKit() { _obj = 0; }
    ThreadKit(ThreadKitRef p) { _obj = p; }
    ThreadKit& operator =(ThreadKitRef p);
    ThreadKit(const ThreadKit&);
    ThreadKit& operator =(const ThreadKit& r);
    ThreadKit(const _ThreadKitExpr&);
    ThreadKit& operator =(const _ThreadKitExpr&);
    ThreadKit(const _ThreadKitElem&);
    ThreadKit& operator =(const _ThreadKitElem&);
    ~ThreadKit();

    operator ThreadKitRef() const { return _obj; }
    ThreadKitRef operator ->() { return _obj; }

    operator BaseThreadsObj() const;
    static ThreadKitRef _narrow(BaseObjectRef p);
    static _ThreadKitExpr _narrow(const BaseObject& r);

    static ThreadKitRef _duplicate(ThreadKitRef obj);
    static _ThreadKitExpr _duplicate(const ThreadKit& r);
};

class _ThreadKitExpr : public ThreadKit {
public:
    _ThreadKitExpr(ThreadKitRef p) { _obj = p; }
    _ThreadKitExpr(const ThreadKit& r) { _obj = r._obj; }
    _ThreadKitExpr(const _ThreadKitExpr& r) { _obj = r._obj; }
    ~_ThreadKitExpr();
};

class _ThreadKitElem {
public:
    ThreadKitRef _obj;

    _ThreadKitElem(ThreadKitRef p) { _obj = p; }
    operator ThreadKitRef() const { return _obj; }
    ThreadKitRef operator ->() { return _obj; }
};

class ThreadKitType : public BaseThreadsObjType {
protected:
    ThreadKitType();
    virtual ~ThreadKitType();
public:
    _ThreadObjExpr thread(ActionRef a) {
        return _c_thread(a);
    }
    virtual ThreadObjRef _c_thread(ActionRef a) = 0;
    _LockObjExpr lock() {
        return _c_lock();
    }
    virtual LockObjRef _c_lock() = 0;
    _ConditionVariableExpr condition() {
        return _c_condition();
    }
    virtual ConditionVariableRef _c_condition() = 0;
    _SemaphoreExpr general_semaphore(Long count) {
        return _c_general_semaphore(count);
    }
    virtual SemaphoreRef _c_general_semaphore(Long count) = 0;
    _SemaphoreExpr mutex_semaphore() {
        return _c_mutex_semaphore();
    }
    virtual SemaphoreRef _c_mutex_semaphore() = 0;
    _SemaphoreExpr wait_semaphore() {
        return _c_wait_semaphore();
    }
    virtual SemaphoreRef _c_wait_semaphore() = 0;

    _ThreadKitExpr _ref();
    virtual TypeObjId _tid();
};

inline ThreadKitRef ThreadKit::_duplicate(ThreadKitRef obj) {
    return (ThreadKitRef)_BaseObject__duplicate(obj, 0);
}
inline ThreadKit& ThreadKit::operator =(ThreadKitRef p) {
    _BaseObject__release(_obj);
    _obj = ThreadKit::_duplicate(p);
    return *this;
}
inline ThreadKit::ThreadKit(const ThreadKit& r) {
    _obj = ThreadKit::_duplicate(r._obj);
}
inline ThreadKit& ThreadKit::operator =(const ThreadKit& r) {
    _BaseObject__release(_obj);
    _obj = ThreadKit::_duplicate(r._obj);
    return *this;
}
inline ThreadKit::ThreadKit(const _ThreadKitExpr& r) {
    _obj = r._obj;
    ((_ThreadKitExpr*)&r)->_obj = 0;
}
inline ThreadKit& ThreadKit::operator =(const _ThreadKitExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ThreadKitExpr*)&r)->_obj = 0;
    return *this;
}
inline ThreadKit::ThreadKit(const _ThreadKitElem& e) {
    _obj = ThreadKit::_duplicate(e._obj);
}
inline ThreadKit& ThreadKit::operator =(const _ThreadKitElem& e) {
    _BaseObject__release(_obj);
    _obj = ThreadKit::_duplicate(e._obj);
    return *this;
}
inline ThreadKit::~ThreadKit() {
    _BaseObject__release(_obj);
}
inline _ThreadKitExpr ThreadKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ThreadKitExpr ThreadKit::_duplicate(const ThreadKit& r) {
    return _duplicate(r._obj);
}
inline ThreadKit::operator BaseThreadsObj() const {
    return _BaseThreadsObjExpr((BaseThreadsObjRef)_BaseObject__duplicate(_obj, 0));
}
inline _ThreadKitExpr::~_ThreadKitExpr() { }
inline _ThreadKitExpr ThreadKitType::_ref() { return this; }

#endif
